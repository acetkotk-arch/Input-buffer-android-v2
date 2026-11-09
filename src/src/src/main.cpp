#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "BufferPopup.hpp"

class $modify(BufPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool dontCreateObjects) {
        if (!PlayLayer::init(level, dontCreateObjects)) return false;
        auto win = CCDirector::get()->getWinSize();

        auto lbl = CCLabelBMFont::create("", "bigFont.fnt");
        lbl->setScale(0.35f);
        lbl->setOpacity(140);
        lbl->setPosition({win.width - 8.f, 10.f});
        lbl->setAnchorPoint({1.f, 0.5f});
        lbl->setZOrder(100);
        lbl->setID("buffer-label"_spr);
        this->addChild(lbl);

        if (cfgInGameBtn()) {
            auto spr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
            spr->setScale(0.5f);
            spr->setOpacity(130);
            auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(BufPlayLayer::onBufferTap));
            auto menu = CCMenu::create();
            menu->setPosition({0, 0});
            btn->setPosition({24.f, win.height - 24.f});
            menu->addChild(btn);
            menu->setZOrder(100);
            this->addChild(menu);
        }

        updateLabel();
        return true;
    }

    void onBufferTap(CCObject*) {
        this->pauseGame(false);
        BufferPopup::create()->show();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        updateLabel();
    }
};

class $modify(BufPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto win = CCDirector::get()->getWinSize();
        auto sprite = ButtonSprite::create("Buffer", 60, true, "goldFont.fnt", "GJ_button_04.png", 30.f, 0.6f);
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(BufPauseLayer::onBufferOpen));

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        btn->setPosition({win.width - 60.f, win.height - 30.f});
        menu->addChild(btn);
        this->addChild(menu);
    }

    void onBufferOpen(CCObject*) {
        BufferPopup::create()->show();
    }
};
