#include "gui/gui.hpp"
#include "gui/cocos/frontend.hpp"
#include "ToastyReplay.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

// ── Keyboard hook (desktop only) ─────────────────────────────────────────────

#ifndef GEODE_IS_MOBILE

#include <Geode/modify/CCKeyboardDispatcher.hpp>

class $modify(ToastyReplayKeyboardHook, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat) {
        if (!down || repeat) return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);

        auto* ui = MenuInterface::get();
        if (!ui) return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);

        int code = static_cast<int>(key);

        if (code == ui->keybinds.menu) {
            if (ui->shown) {
                ui->anim.closing = true;
                ui->anim.opening = false;
            } else {
                ui->shown        = true;
                ui->anim.opening = true;
                ui->anim.closing = false;
            }
            return true;
        }

        auto* engine = ReplayEngine::get();
        if (!engine) return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);

        if (code == ui->keybinds.replayToggle) {
            if (engine->engineMode == MODE_EXECUTE || engine->pendingPlaybackStart)
                engine->haltExecution();
            else if (engine->hasMacroInputs())
                engine->requestExecutionStart();
            return true;
        }

        if (code == ui->keybinds.noclip) {
            engine->collisionBypass = !engine->collisionBypass;
            return true;
        }
        if (code == ui->keybinds.safeMode) {
            engine->protectedMode = !engine->protectedMode;
            return true;
        }
        if (code == ui->keybinds.trajectory) {
            engine->pathPreview = !engine->pathPreview;
            return true;
        }
        if (code == ui->keybinds.audioPitch) {
            engine->audioPitchEnabled = !engine->audioPitchEnabled;
            return true;
        }
        if (code == ui->keybinds.rngLock) {
            engine->rngLocked = !engine->rngLocked;
            return true;
        }
        if (code == ui->keybinds.hitboxes) {
            engine->showHitboxes = !engine->showHitboxes;
            return true;
        }
        if (code == ui->keybinds.layoutMode) {
            engine->layoutMode = !engine->layoutMode;
            return true;
        }
        if (code == ui->keybinds.noMirror) {
            engine->noMirrorEffect = !engine->noMirrorEffect;
            return true;
        }
        if (code == ui->keybinds.disableShaders) {
            engine->disableShaders = !engine->disableShaders;
            return true;
        }
        if (code == ui->keybinds.frameAdvance && engine->tickStepping) {
            engine->pendingStep    = true;
            engine->singleTickStep = true;
            return true;
        }
        if (code == ui->keybinds.frameStep) {
            engine->tickStepping = !engine->tickStepping;
            if (auto* pl = PlayLayer::get())
                engine->setFrameStepEnabled(engine->tickStepping, pl);
            return true;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);
    }
};

#endif

// ── Mobile: replay controls in the pause menu ────────────────────────────────

#ifdef GEODE_IS_MOBILE

#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

class $modify(ToastyMobilePauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto* engine = ReplayEngine::get();
        if (!engine) return;

        // CCSize is a value type, not a pointer
        CCSize winSize = CCDirector::sharedDirector()->getWinSize();

        bool playbackActive = engine->engineMode == MODE_EXECUTE || engine->pendingPlaybackStart;
        bool captureActive  = engine->engineMode == MODE_CAPTURE;

        auto makePill = [&](const char* label, ccColor3B color, float yPos, SEL_MenuHandler sel) {
            auto* bg = CCScale9Sprite::create("GJ_button_01.png");
            bg->setContentSize(CCSizeMake(120.f, 38.f));
            bg->setColor(color);
            auto* lbl = CCLabelBMFont::create(label, "bigFont.fnt");
            lbl->setScale(0.45f);
            lbl->setPosition(ccp(60.f, 19.f));
            bg->addChild(lbl);
            auto* item = CCMenuItemSpriteExtra::create(bg, this, sel);
            auto* menu = CCMenu::create();
            menu->addChild(item);
            menu->setPosition(ccp(winSize.width / 2.f, yPos));
            addChild(menu, 10);
        };

        if (playbackActive) {
            makePill("Stop Replay", ccc3(200, 60, 60), 46.f,
                menu_selector(ToastyMobilePauseLayer::onStopReplay));
        } else if (engine->hasMacroInputs()) {
            makePill("Start Replay", ccc3(30, 180, 170), 46.f,
                menu_selector(ToastyMobilePauseLayer::onStartReplay));
        }

        if (captureActive) {
            makePill("Stop Capture", ccc3(200, 60, 60), 90.f,
                menu_selector(ToastyMobilePauseLayer::onStopCapture));
        } else if (!playbackActive) {
            makePill("Start Capture", ccc3(80, 80, 200),
                engine->hasMacroInputs() ? 90.f : 46.f,
                menu_selector(ToastyMobilePauseLayer::onStartCapture));
        }
    }

    void onStartReplay(CCObject*) {
        auto* engine = ReplayEngine::get();
        if (!engine || !engine->hasMacroInputs()) return;
        engine->requestExecutionStart();
        onResume(nullptr);
    }

    void onStopReplay(CCObject*) {
        auto* engine = ReplayEngine::get();
        if (!engine) return;
        engine->haltExecution();
        onResume(nullptr);
    }

    void onStartCapture(CCObject*) {
        auto* engine = ReplayEngine::get();
        if (!engine) return;
        if (auto* pl = PlayLayer::get())
            engine->beginCapture(pl->m_level);
        onResume(nullptr);
    }

    void onStopCapture(CCObject*) {
        auto* engine = ReplayEngine::get();
        if (!engine) return;
        engine->engineMode = MODE_DISABLED;
        engine->endReplayAccuracyEnvironment();
        onResume(nullptr);
    }
};

class $modify(ToastyMobileFirstRunLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto* mod = Mod::get();
        if (!mod->getSavedValue<bool>("first_run_shown", false)) {
            mod->setSavedValue("first_run_shown", true);
            Notification::create(
                "Tap the TR button in-game to open ToastyReplay!",
                NotificationIcon::Info, 5.0f
            )->show();
        }

        return true;
    }
};

#endif
