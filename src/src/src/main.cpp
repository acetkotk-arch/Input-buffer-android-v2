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
            btn->setPosition({40.f, win.height - 40.f});
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
        log::info("Safe Input Buffer: adding pause button");

        auto win = CCDirector::get()->getWinSize();
        auto sprite = ButtonSprite::create("Buffer", 80, true, "goldFont.fnt", "GJ_button_01.png", 35.f, 0.7f);
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(BufPauseLayer::onBufferOpen));

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        btn->setPosition({100.f, win.height - 45.f});
        menu->addChild(btn);
        this->addChild(menu, 1000);
    }

    void onBufferOpen(CCObject*) {
        BufferPopup::create()->show();
    }
};
