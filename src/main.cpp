#pragma once
#include <Geode/Geode.hpp>
using namespace geode::prelude;

inline bool cfgEnabled()   { return Mod::get()->getSettingValue<bool>("enabled"); }
inline bool cfgPractice()  { return Mod::get()->getSettingValue<bool>("practice-only"); }
inline int  cfgClick()     { return (int)Mod::get()->getSettingValue<int64_t>("click-ticks"); }
inline int  cfgWave()      { return (int)Mod::get()->getSettingValue<int64_t>("wave-ticks"); }
inline bool cfgLabel()     { return Mod::get()->getSettingValue<bool>("show-label"); }
inline bool cfgInGameBtn() { return Mod::get()->getSettingValue<bool>("ingame-button"); }

inline void updateLabel() {
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
