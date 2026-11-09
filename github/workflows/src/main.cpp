#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// ---------- settings helpers ----------
static bool cfgEnabled()   { return Mod::get()->getSettingValue<bool>("enabled"); }
static bool cfgPractice()  { return Mod::get()->getSettingValue<bool>("practice-only"); }
static int  cfgClick()     { return (int)Mod::get()->getSettingValue<int64_t>("click-ticks"); }
static int  cfgWave()      { return (int)Mod::get()->getSettingValue<int64_t>("wave-ticks"); }
static bool cfgLabel()     { return Mod::get()->getSettingValue<bool>("show-label"); }

// Presets cycled by the pause-menu button: {click, wave}
static constexpr int PRESETS[][2] = { {0,0}, {2,2}, {4,4}, {8,8}, {16,8} };
static constexpr int NUM_PRESETS = 5;

// ---------- core logic ----------
class $modify(BufGJBGL, GJBaseGameLayer) {
    struct Fields {
        int  tick = 0;
        bool held[2]        = {false, false};
        int  pressTick[2]   = {-9999, -9999};
        bool firedSincePress[2] = {false, false};
        bool releasePending[2]  = {false, false};
        int  releaseDueTick[2]  = {0, 0};
        bool assistedThisAttempt = false;
    };

    bool bufferActive() {
        if (!cfgEnabled()) return false;
        if (!cfgPractice()) return true;
        auto pl = PlayLayer::get();
        bool practice = pl && pl->m_isPracticeMode;
        bool editorTest = !pl;
        return practice || editorTest;
    }

    PlayerObject* playerFor(int idx) {
        return idx == 0 ? m_player1 : m_player2;
    }

    void processCommands(float dt) {
        auto f = m_fields.self();
        f->tick++;

        if (bufferActive()) {
            for (int i = 0; i < 2; i++) {
                auto* p = playerFor(i);
                if (!p) continue;

                if (f->held[i] && !f->firedSincePress[i] &&
                    f->tick - f->pressTick[i] <= cfgClick() &&
                    p->m_touchingRings && p->m_touchingRings->count() > 0) {
                    f->firedSincePress[i] = true;
                    f->assistedThisAttempt = true;
                    GJBaseGameLayer::handleButton(false, 1, i == 0);
                    GJBaseGameLayer::handleButton(true, 1, i == 0);
                }

                if (f->releasePending[i] && f->tick >= f->releaseDueTick[i]) {
                    f->releasePending[i] = false;
                    f->assistedThisAttempt = true;
                    GJBaseGameLayer::handleButton(false, 1, i == 0);
                }
            }
        }
        GJBaseGameLayer::processCommands(dt);
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        if (button != 1 || !bufferActive()) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }
        auto f = m_fields.self();
        int i = isPlayer1 ? 0 : 1;
        auto* p = playerFor(i);

        if (down) {
            f->held[i] = true;
            f->pressTick[i] = f->tick;
            f->firedSincePress[i] = false;
            f->releasePending[i] = false;
            GJBaseGameLayer::handleButton(true, button, isPlayer1);
        } else {
            f->held[i] = false;
            if (p && p->m_isDart && cfgWave() > 0) {
                f->releasePending[i] = true;
                f->releaseDueTick[i] = f->tick + cfgWave();
            } else {
                GJBaseGameLayer::handleButton(false, button, isPlayer1);
            }
        }
    }
};

// ---------- on-screen label ----------
class $modify(BufPlayLayer, PlayLayer) {
    struct Fields { CCLabelBMFont* label = nullptr; };

    void refreshLabel() {
        auto f = m_fields.self();
        if (!f->label) return;
        bool show = cfgLabel() && cfgEnabled();
        f->label->setVisible(show);
        if (show) {
            f->label->setString(
                fmt::format("Buffer {}/{}{}", cfgClick(), cfgWave(),
                            cfgPractice() ? " (practice)" : "").c_str());
        }
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        auto win = CCDirector::get()->getWinSize();
        auto lbl = CCLabelBMFont::create("", "bigFont.fnt");
        lbl->setScale(0.35f);
        lbl->setOpacity(140);
        lbl->setPosition({win.width - 8.f, 10.f});
        lbl->setAnchorPoint({1.f, 0.5f});
        lbl->setZOrder(100);
        this->addChild(lbl);
        m_fields->label = lbl;
        refreshLabel();
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        refreshLabel();
    }
};

// ---------- pause-menu UI ----------
class $modify(BufPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto win = CCDirector::get()->getWinSize();
        auto sprite = ButtonSprite::create("Buffer", 60, true, "goldFont.fnt", "GJ_button_04.png", 30.f, 0.6f);
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(BufPauseLayer::onBufferCycle));

        auto settingsSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
        settingsSpr->setScale(0.7f);
        auto settingsBtn = CCMenuItemSpriteExtra::create(settingsSpr, this, menu_selector(BufPauseLayer::onBufferSettings));

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        btn->setPosition({win.width - 60.f, 30.f});
        settingsBtn->setPosition({win.width - 115.f, 30.f});
        menu->addChild(btn);
        menu->addChild(settingsBtn);
        this->addChild(menu);
    }

    void onBufferCycle(CCObject*) {
        int cur = 0;
        for (int i = 0; i < NUM_PRESETS; i++) {
            if (cfgEnabled() && PRESETS[i][0] == cfgClick() && PRESETS[i][1] == cfgWave()) cur = i + 1;
        }
        int next = (cur + 1) % (NUM_PRESETS + 1);
        if (next == 0) {
            Mod::get()->setSettingValue<bool>("enabled", false);
            Notification::create("Input buffer: OFF", NotificationIcon::Info, 1.f)->show();
        } else {
            Mod::get()->setSettingValue<bool>("enabled", true);
            Mod::get()->setSettingValue<int64_t>("click-ticks", PRESETS[next - 1][0]);
            Mod::get()->setSettingValue<int64_t>("wave-ticks",  PRESETS[next - 1][1]);
            Notification::create(
                fmt::format("Input buffer: {} / {} ticks", PRESETS[next - 1][0], PRESETS[next - 1][1]),
                NotificationIcon::Success, 1.f)->show();
        }
    }

    void onBufferSettings(CCObject*) {
        openSettingsPopup(Mod::get());
    }
};
