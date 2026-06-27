#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_MOBILE

class MobileFloatingButton : public CCNode, public CCTouchDelegate {
public:
    static MobileFloatingButton* create();
    static MobileFloatingButton* get();

    bool init() override;
    void onTouch(CCObject* sender);
    void reposition();

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override;

    void onExit() override;

private:
    CCMenuItemSpriteExtra* m_item    = nullptr;
    CCMenu*                m_menu    = nullptr;
    bool                   m_dragging = false;
    CCPoint                m_dragStart{};
    CCPoint                m_nodeStart{};
};

class $modify(MobileFABPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (auto* fab = MobileFloatingButton::create()) {
            fab->setID("toasty-fab");
            addChild(fab, 1000);
            fab->reposition();
        }

        return true;
    }
};

#endif
