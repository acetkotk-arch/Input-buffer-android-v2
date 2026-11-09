#include <Geode/modify/GJBaseGameLayer.hpp>
#include "Settings.hpp"

class $modify(BufGJBGL, GJBaseGameLayer) {
    struct Fields {
        int  tick = 0;
        bool held[2]        = {false, false};
        int  pressTick[2]   = {-9999, -9999};
        bool armed[2]       = {false, false};
        bool releasePending[2]  = {false, false};
        int  releaseDueTick[2]  = {0, 0};
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

    bool canActNow(PlayerObject* p) {
        if (!p) return true;
        bool ringTouch = p->m_touchingRings && p->m_touchingRings->count() > 0;
        if (ringTouch) return true;
        bool groundMode = !(p->m_isShip || p->m_isBird || p->m_isDart || p->m_isSwing);
        return groundMode && p->m_isOnGround;
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto f = m_fields.self();
        f->tick++;

        if (bufferActive()) {
            for (int i = 0; i < 2; i++) {
                auto* p = playerFor(i);
                if (!p) continue;

                if (f->held[i] && f->armed[i] &&
                    f->tick - f->pressTick[i] <= cfgClick() &&
                    canActNow(p)) {
                    f->armed[i] = false;
                    GJBaseGameLayer::handleButton(false, 1, i == 0);
                    GJBaseGameLayer::handleButton(true, 1, i == 0);
                }

                if (f->releasePending[i] && f->tick >= f->releaseDueTick[i]) {
                    f->releasePending[i] = false;
                    GJBaseGameLayer::handleButton(false, 1, i == 0);
                }
            }
        }
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
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
            f->armed[i] = (p != nullptr) && !canActNow(p);
            f->releasePending[i] = false;
            GJBaseGameLayer::handleButton(true, button, isPlayer1);
        } else {
            f->held[i] = false;
            f->armed[i] = false;
            if (p && p->m_isDart && cfgWave() > 0) {
                f->releasePending[i] = true;
                f->releaseDueTick[i] = f->tick + cfgWave();
            } else {
                GJBaseGameLayer::handleButton(false, button, isPlayer1);
            }
        }
    }
};
