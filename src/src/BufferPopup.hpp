#pragma once
#include "Settings.hpp"

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

        auto panel = CCLayerColor::create(ccColor4B{25, 25, 40, 240}, 300.f, 260.f);
        panel->setPosition({cx - 150.f, cy - 130.f});
        m_mainLayer->addChild(panel);

        m_buttonMenu = CCMenu::create();
        m_buttonMenu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(m_buttonMenu);

        auto title = CCLabelBMFont::create("Input Buffer", "goldFont.fnt");
        title->setScale(0.8f);
        title->setPosition({cx, cy + 112.f});
        m_mainLayer->addChild(title);

        this->addToggleRow("Enabled",        0, cy + 80.f, cfgEnabled(),   cx);
        this->addToggleRow("Practice only",  1, cy + 50.f, cfgPractice(),  cx);
        this->addToggleRow("Show label",     2, cy + 20.f, cfgLabel(),     cx);
        this->addToggleRow("In-game button", 3, cy - 10.f, cfgInGameBtn(), cx);

        this->addStepperRow("Click buffer", 0, cy - 40.f, cfgClick(), cx, &m_clickLbl);
        this->addStepperRow("Wave buffer",  2, cy - 70.f, cfgWave(),  cx, &m_waveLbl);

        auto okSpr = ButtonSprite::create("OK");
        auto okBtn = CCMenuItemSpriteExtra::create(okSpr, this, menu_selector(BufferPopup::onOk));
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
        lbl->setScale(0.5f);
        lbl->setAnchorPoint({0.f, 0.5f});
        lbl->setPosition({cx - 125.f, y});
        m_mainLayer->addChild(lbl);

        auto minusSpr = ButtonSprite::create("-", 30, true, "bigFont.fnt", "GJ_button_04.png", 30.f, 0.6f);
        auto minusBtn = CCMenuItemSpriteExtra::create(minusSpr, this, menu_selector(BufferPopup::onStep));
        minusBtn->setTag(baseTag);
        minusBtn->setPosition({cx + 50.f, y});
        m_buttonMenu->addChild(minusBtn);

        auto valLbl = CCLabelBMFont::create(fmt::format("{}", value).c_str(), "goldFont.fnt");
        valLbl->setScale(0.6f);
        valLbl->setPosition({cx + 85.f, y});
        m_mainLayer->addChild(valLbl);
        *outLbl = valLbl;

        auto plusSpr = ButtonSprite::create("+", 30, true, "bigFont.fnt", "GJ_button_04.png", 30.f, 0.6f);
        auto plusBtn = CCMenuItemSpriteExtra::create(plusSpr, this, menu_selector(BufferPopup::onStep));
        plusBtn->setTag(baseTag + 1);
        plusBtn->setPosition({cx + 120.f, y});
        m_buttonMenu->addChild(plusBtn);
    }

    void onToggle(CCObject* sender) {
        auto t = static_cast<CCMenuItemToggler*>(sender);
        bool newValue = !t->isToggled();
        switch (t->getTag()) {
            case 0: Mod::get()->setSettingValue<bool>("enabled", newValue); break;
            case 1: Mod::get()->setSettingValue<bool>("practice-only", newValue); break;
            case 2: Mod::get()->setSettingValue<bool>("show-label", newValue); break;
            case 3: Mod::get()->setSettingValue<bool>("ingame-button", newValue); break;
        }
        updateLabel();
    }

    void onStep(CCObject* sender) {
        int tag = static_cast<CCNode*>(sender)->getTag();
        bool isClick = tag < 2;
        int v = isClick ? cfgClick() : cfgWave();
        v += (tag % 2 == 0) ? -1 : 1;
        if (v < 0) v = 0;
        if (v > 16) v = 16;

        if (isClick) {
            Mod::get()->setSettingValue<int64_t>("click-ticks", v);
            m_clickLbl->setString(fmt::format("{}", v).c_str());
        } else {
            Mod::get()->setSettingValue<int64_t>("wave-ticks", v);
            m_waveLbl->setString(fmt::format("{}", v).c_str());
        }
        updateLabel();
    }

    void onOk(CCObject*) {
        this->setKeypadEnabled(false);
        this->removeFromParentAndCleanup(true);
    }
};
