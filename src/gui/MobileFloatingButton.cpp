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
    bgSprite->setContentSize({ 52.f, 52.f });
    bgSprite->setColor({ 30, 180, 170 });

    auto* label = CCLabelBMFont::create("TR", "bigFont.fnt");
    label->setScale(0.55f);
    label->setColor({ 255, 255, 255 });
    label->setPosition({ 26.f, 26.f });
    bgSprite->addChild(label);

    m_item = CCMenuItemSpriteExtra::create(bgSprite, this, menu_selector(MobileFloatingButton::onTouch));
    m_item->setPosition({ 26.f, 26.f });

    m_menu = CCMenu::create();
    m_menu->addChild(m_item);
    m_menu->setPosition({ 0.f, 0.f });
    addChild(m_menu);

    setContentSize({ 52.f, 52.f });

    applyOpacity();

    auto* touchListener = CCEventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(false);
    touchListener->onTouchBegan = [this](CCTouch* touch, CCEvent*) -> bool {
        auto loc = convertToNodeSpace(touch->getLocation());
        bool hit = loc.x >= 0 && loc.x <= 52.f && loc.y >= 0 && loc.y <= 52.f;
        if (hit) {
            m_dragging = false;
            m_dragStart = touch->getLocation();
            m_nodeStart = getPosition();
        }
        return hit;
    };
    touchListener->onTouchMoved = [this](CCTouch* touch, CCEvent*) {
        CCPoint delta = touch->getLocation() - m_dragStart;
        if (!m_dragging && (std::abs(delta.x) > 6.f || std::abs(delta.y) > 6.f)) {
            m_dragging = true;
        }
        if (m_dragging) {
            auto* parent = getParent();
            if (!parent) return;
            CCSize cs = parent->getContentSize();
            float nx = std::clamp(m_nodeStart.x + delta.x, 26.f, cs.width  - 26.f);
            float ny = std::clamp(m_nodeStart.y + delta.y, 26.f, cs.height - 26.f);
            setPosition({ nx, ny });
        }
    };
    touchListener->onTouchEnded = [this](CCTouch*, CCEvent*) {
        m_dragging = false;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    return true;
}

void MobileFloatingButton::onTouch(CCObject*) {
    if (m_dragging) return;

    auto* ui = MenuInterface::get();
    if (!ui) return;

    if (toasty::frontend::isCocos()) {
        toasty::frontend::openCocosMenu();
        return;
    }

    if (ui->shown) {
        ui->anim.closing = true;
        ui->anim.opening = false;
    } else {
        ui->shown = true;
        ui->anim.opening = true;
        ui->anim.closing = false;
    }
}

void MobileFloatingButton::applyOpacity() {
    float opacity = Mod::get()->getSettingValue<double>("mobile_fab_opacity");
    setOpacity(static_cast<GLubyte>(opacity * 255.f));
}

void MobileFloatingButton::reposition() {
    auto* parent = getParent();
    if (!parent) return;

    CCSize cs   = parent->getContentSize();
    std::string pos = Mod::get()->getSettingValue<std::string>("mobile_fab_position");
    CCPoint target;

    if (pos == "Bottom Right") target = { cs.width - 42.f, 42.f };
    else if (pos == "Bottom Left") target = { 42.f, 42.f };
    else if (pos == "Top Right") target = { cs.width - 42.f, cs.height - 42.f };
    else target = { 42.f, cs.height - 42.f };

    setPosition(target);
}

#endif
