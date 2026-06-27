#include "MobileFloatingButton.hpp"

#ifdef GEODE_IS_MOBILE

#include "gui/gui.hpp"
#include "gui/cocos/frontend.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

static MobileFloatingButton* s_instance = nullptr;

MobileFloatingButton* MobileFloatingButton::create() {
    auto* ret = new MobileFloatingButton();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

MobileFloatingButton* MobileFloatingButton::get() {
    return s_instance;
}

bool MobileFloatingButton::init() {
    if (!CCNode::init()) return false;

    s_instance = this;

    auto* bgSprite = CCScale9Sprite::create("GJ_button_01.png");
    bgSprite->setContentSize(CCSizeMake(52.f, 52.f));
    bgSprite->setColor(ccc3(30, 180, 170));

    auto* label = CCLabelBMFont::create("TR", "bigFont.fnt");
    label->setScale(0.55f);
    label->setColor(ccc3(255, 255, 255));
    label->setPosition(ccp(26.f, 26.f));
    bgSprite->addChild(label);

    m_item = CCMenuItemSpriteExtra::create(bgSprite, this,
        menu_selector(MobileFloatingButton::onTouch));
    m_item->setPosition(ccp(26.f, 26.f));

    m_menu = CCMenu::create();
    m_menu->addChild(m_item);
    m_menu->setPosition(ccp(0.f, 0.f));
    addChild(m_menu);

    setContentSize(CCSizeMake(52.f, 52.f));

    float opacity = static_cast<float>(
        Mod::get()->getSettingValue<double>("mobile_fab_opacity"));
    auto glubyte = static_cast<GLubyte>(std::clamp(opacity, 0.f, 1.f) * 255.f);
    m_menu->setOpacity(glubyte);
    bgSprite->setOpacity(glubyte);
    label->setOpacity(glubyte);

    CCTouchDispatcher::get()->addTargetedDelegate(this, 0, true);

    return true;
}

void MobileFloatingButton::onExit() {
    CCNode::onExit();
    CCTouchDispatcher::get()->removeDelegate(this);
    if (s_instance == this) s_instance = nullptr;
}

bool MobileFloatingButton::ccTouchBegan(CCTouch* touch, CCEvent*) {
    CCPoint loc = convertToNodeSpace(touch->getLocation());
    bool hit = loc.x >= 0 && loc.x <= 52.f && loc.y >= 0 && loc.y <= 52.f;
    if (hit) {
        m_dragging  = false;
        m_dragStart = touch->getLocation();
        m_nodeStart = getPosition();
    }
    return hit;
}

void MobileFloatingButton::ccTouchMoved(CCTouch* touch, CCEvent*) {
    CCPoint delta = ccpSub(touch->getLocation(), m_dragStart);
    if (!m_dragging && (std::abs(delta.x) > 6.f || std::abs(delta.y) > 6.f)) {
        m_dragging = true;
    }
    if (m_dragging) {
        auto* parent = getParent();
        if (!parent) return;
        CCSize cs = parent->getContentSize();
        float nx = std::clamp(m_nodeStart.x + delta.x, 26.f, cs.width  - 26.f);
        float ny = std::clamp(m_nodeStart.y + delta.y, 26.f, cs.height - 26.f);
        setPosition(ccp(nx, ny));
    }
}

void MobileFloatingButton::ccTouchEnded(CCTouch*, CCEvent*) {
    if (!m_dragging) {
        onTouch(nullptr);
    }
    m_dragging = false;
}

void MobileFloatingButton::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
    ccTouchEnded(touch, event);
}

void MobileFloatingButton::onTouch(CCObject*) {
    toasty::frontend::toggleMenu();
}

void MobileFloatingButton::reposition() {
    auto* parent = getParent();
    if (!parent) return;

    CCSize cs = parent->getContentSize();
    std::string pos = Mod::get()->getSettingValue<std::string>("mobile_fab_position");

    CCPoint target;
    if      (pos == "Bottom Right") target = ccp(cs.width - 42.f, 42.f);
    else if (pos == "Bottom Left")  target = ccp(42.f,            42.f);
    else if (pos == "Top Right")    target = ccp(cs.width - 42.f, cs.height - 42.f);
    else                            target = ccp(42.f,            cs.height - 42.f);

    setPosition(target);
}

#endif
