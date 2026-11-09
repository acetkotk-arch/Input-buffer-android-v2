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
static bool cfgInGameBtn() { return Mod::get()->getSettingValue<bool>("ingame-button"); }

// ---------- shared helpers ----------
static void updateLabel() {
    auto pl = PlayLayer::get();
    if (!pl) return;
    auto lbl = typeinfo_cast<CCLabelBMFont*>(pl->getChildByID("buffer-label"_spr));
    if (!lbl) return;
    bool show = cfgLabel() && cfgEnabled();
    lbl->setVisible(show);
    if (show) {
        lbl->setString(
            fmt::format("Buffer {}/{}{}", cfgClick(), cfgWave(),
                        cfgPractice() ? " (practice)" : "").c_str());
    }
}

// ---------- settings window ----------
class BufferPopup : public FLAlertLayer {
public:
    CCLabelBMFont* m_clickLbl = nullptr;
    CCLabelBMFont* m_waveLbl = nullptr;

    static BufferPopup* create() {
        auto ret = new BufferPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init() {
        if (!FLAlertLayer::init(150)) return false;

        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);

        m_mainLayer = CCLayer::create();
        this->addChild(m_mainLayer);

        auto win = CCDirector::get()->getWinSize();
        float cx = win.width / 2.f;
        float cy = win.height / 2.f;

        // Panel background
        auto panel = CCLayerColor::create(ccColor4B{25, 25, 40, 240}, 300.f, 260.f);
        panel->setPosition({cx - 150.f, cy - 130.f});
        m_mainLayer->addChild(panel);

        m_buttonMenu = CCMenu::create();
        m_buttonMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(m_buttonMenu);

        // Title
        auto title = CCLabelBMFont::create("Input Buffer", "goldFont.fnt");
        title->setScale(0.8f);
        title->setPosition({cx, cy + 112.f});
        m_mainLayer->addChild(title);

        // Toggle rows
        this->addToggleRow("Enabled",        0, cy + 80.f, cfgEnabled(),   cx);
        this->addToggleRow("Practice only",  1, cy + 50.f, cfgPractice(),  cx);
        this->addToggleRow("Show label",     2, cy + 20.f, cfgLabel(),     cx);
        this->addToggleRow("In-game button", 3, cy - 10.f, cfgInGameBtn(), cx);

        // Stepper rows
        this->addStepperRow("Click buffer", 0, cy - 40.f, cfgClick(), cx, &m_clickLbl);
        this->addStepperRow("Wave buffer",  2, cy - 70.f, cfgWave(),  cx, &m_waveLbl);

        // Close button
        auto okSpr = ButtonSprite::create("OK");
        auto okBtn = CCMenuItemSpriteExtra::create(okSpr, this, menu_selector(BufferPopup::onClose));
        okBtn->setPosition({cx, cy - 108.f});
        m_buttonMenu->addChild(okBtn);

        return true;
    }

    void addToggleRow(const char* text, int tag, float y, bool value, float cx) {
        auto lbl = CCLabelBMFont::create(text, "bigFont.fnt");
        lbl->setScale(0.5f);
        lbl->setAnchorPoint({0.f, 0.5f});
        lbl->setPosition({cx - 125.f, y});
        m_mainLayer->addChild(lbl);

        auto toggler = CCMenuItemToggler::createWithStandardSprites(
            this, menu_selector(BufferPopup::onToggle), 0.6f);
        toggler->setTag(tag);
        toggler->toggle(value);
        toggler->setPosition({cx + 105.f, y});
        m_buttonMenu->addChild(toggler);
    }

    void addStepperRow(const char* text, int baseTag, float y, int value, float cx, CCLabelBMFont** outLbl) {
        auto lbl = CCLabelBMFont::create(text, "bigFont.fnt");
