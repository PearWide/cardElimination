#include "CardView.h"
#include "../utils/GameUtils.h"

USING_NS_CC;

bool CardView::init() {
    if (!Sprite::init()) {
        return false;
    }

    // ����Ĭ�ϴ�С
    // 设置精灵的默认大小和锚点
    // this->setContentSize(Size(80, 120)); // 使用原始图片尺寸
    this->setAnchorPoint(Vec2(0.5f, 0.5f)); // 设置锚点为中心
    
    // ��ʼ����Ա����
    _cardModel = nullptr;
    _backgroundSprite = nullptr;
    _bigNumberSprite = nullptr;
    _smallNumberSprite = nullptr;
    _suitSprite = nullptr;

    // �������Ͽ������
    createCompositeCard();

    // ���ô����¼�
    setupTouchEvents();

    CCLOG("CardView initialized");
    return true;
}

void CardView::setupTouchEvents() {
    // 使用EventListenerTouchOneByOne进行精确的触摸控制
    _touchListener = EventListenerTouchOneByOne::create();
    
    // 设置触摸优先级，确保卡牌能正确响应触摸
    _touchListener->setSwallowTouches(true);

    // onTouchBegan返回true表示消费此触摸事件
    _touchListener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
        if (!touch) {
            return false;
        }
        
        // 使用getBoundingBox()进行精确的边界检测
        if (this->getBoundingBox().containsPoint(touch->getLocation())) {
            CCLOG("CardView %d touched", _cardId);
            
            // 使用动作系统提供视觉反馈
            this->runAction(ScaleTo::create(0.1f, 0.95f));
            return true; // 消费此触摸事件
        }
        return false;
    };

    // onTouchEnded处理触摸结束
    _touchListener->onTouchEnded = [this](Touch* touch, Event* event) {
        // 恢复原始大小
        this->runAction(ScaleTo::create(0.1f, 1.0f));
        
        // 检查是否在卡牌区域内结束触摸
        if (this->getBoundingBox().containsPoint(touch->getLocation())) {
            onClicked();
        }
    };

    // onTouchCancelled处理触摸取消
    _touchListener->onTouchCancelled = [this](Touch* touch, Event* event) {
        this->runAction(ScaleTo::create(0.1f, 1.0f));
    };

    // 使用addEventListenerWithSceneGraphPriority注册事件监听器
    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);
}

void CardView::onClicked() {
    CCLOG("CardView %d clicked", _cardId);

    try {
        if (_clickCallback) {
            CCLOG("CardView %d calling click callback", _cardId);
            _clickCallback(_cardId);
            CCLOG("CardView %d click callback completed", _cardId);
        } else {
            CCLOGERROR("CardView %d has no click callback!", _cardId);
        }
    } catch (...) {
        CCLOGERROR("Exception in CardView::onClicked for card: %d", _cardId);
    }
}

// 实现动画功能
void CardView::playMoveToAnimation(const cocos2d::Vec2& targetPosition, float duration) {
    // 设置移动中的卡牌为最高层级
    this->setZOrder(GameUtils::MOVING_CARD_ZORDER);
    CCLOG("CardView %d set to moving zOrder=%d", _cardId, GameUtils::MOVING_CARD_ZORDER);
    
    // 创建移动动画
    auto moveAction = MoveTo::create(duration, targetPosition);
    auto easeAction = EaseInOut::create(moveAction, 2.0f);
    
    // 创建缩放动画
    auto scaleUp = ScaleTo::create(GameUtils::SCALE_ANIMATION_DURATION, GameUtils::CARD_SCALE_FACTOR);
    auto scaleDown = ScaleTo::create(GameUtils::SCALE_ANIMATION_DURATION, 1.0f);
    auto scaleSequence = Sequence::create(scaleUp, scaleDown, nullptr);
    
    // 组合移动和缩放动画
    auto spawnAction = Spawn::create(easeAction, scaleSequence, nullptr);
    
    // 动画完成回调
    auto callback = CallFunc::create([this]() {
        CCLOG("CardView %d move animation completed", _cardId);
    });
    
    auto sequence = Sequence::create(spawnAction, callback, nullptr);
    this->runAction(sequence);
}

void CardView::playMatchAnimation() {
    // 使用工具类创建匹配动画
    auto sequence = GameUtils::createMatchAnimationSequence();
    this->runAction(sequence);
}

// 已废弃：现在使用Cocos2d-x内置的getBoundingBox().containsPoint()
bool CardView::containsTouchPoint_DEPRECATED(Touch* touch) {
    if (!touch) {
        CCLOG("Touch is null in containsTouchPoint for CardView %d", _cardId);
        return false;
    }
    
    Vec2 locationInNode = this->convertToNodeSpace(touch->getLocation());
    Size size = this->getContentSize();
    
    // ʹ������ê��ļ������
    float halfWidth = size.width * 0.5f;
    float halfHeight = size.height * 0.5f;
    Rect rect = Rect(-halfWidth, -halfHeight, size.width, size.height);
    
    bool contains = rect.containsPoint(locationInNode);
    
    // 添加调试信息
    if (contains) {
        CCLOG("CardView %d HIT: touch(%.1f,%.1f) -> node(%.1f,%.1f)", 
              _cardId, touch->getLocation().x, touch->getLocation().y, locationInNode.x, locationInNode.y);
    } else {
        // 输出MISS调试信息
        CCLOG("CardView %d MISS: touch(%.1f,%.1f) -> node(%.1f,%.1f), cardPos(%.1f,%.1f), size(%.1fx%.1f), rect(%.1f,%.1f,%.1fx%.1f)", 
              _cardId, touch->getLocation().x, touch->getLocation().y, locationInNode.x, locationInNode.y, 
              this->getPosition().x, this->getPosition().y, size.width, size.height,
              rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    }
    
    return contains;
}

void CardView::setClickEnabled(bool enabled) {
    if (_touchListener) {
        _touchListener->setEnabled(enabled);
    }
}

void CardView::updateDisplay() {
    if (!_cardModel) {
        // ��ʾ���Ʊ���
        if (_backgroundSprite) {
            _backgroundSprite->setTexture("res/card_general.png");
        }
        if (_bigNumberSprite) _bigNumberSprite->setVisible(false);
        if (_smallNumberSprite) _smallNumberSprite->setVisible(false);
        if (_suitSprite) _suitSprite->setVisible(false);
        return;
    }

    if (_cardModel->isCovered()) {
        // ��ʾ���Ʊ���
        if (_backgroundSprite) {
            _backgroundSprite->setTexture("res/card_general.png");
        }
        if (_bigNumberSprite) _bigNumberSprite->setVisible(false);
        if (_smallNumberSprite) _smallNumberSprite->setVisible(false);
        if (_suitSprite) _suitSprite->setVisible(false);
    } else {
        // ��ʾ�������� - ���Ͼ���
        createCompositeCard();
    }
}

void CardView::createCompositeCard() {
    if (!_cardModel) return;
    
    // ��ȡ���Ƴߴ��������λ�ü���
    Size cardSize = this->getContentSize();
    float cardWidth = cardSize.width;
    float cardHeight = cardSize.height;
    
    // ����ߴ�Ϊ0��ʹ��Ĭ�ϳߴ�
    if (cardWidth <= 0 || cardHeight <= 0) {
        // 让卡牌使用背景图片的原始尺寸
        if (_backgroundSprite) {
            Size bgSize = _backgroundSprite->getContentSize();
            cardWidth = bgSize.width;
            cardHeight = bgSize.height;
            CCLOG("Using background image size: %.1f x %.1f", cardWidth, cardHeight);
        } else {
            // 如果背景图片不存在，使用一个合理的默认尺寸
            cardWidth = 100.0f;
            cardHeight = 150.0f;
            CCLOG("Using fallback card size: %.1f x %.1f", cardWidth, cardHeight);
        }
    }
    
    // ���������������������ƣ�
    if (!_backgroundSprite) {
        _backgroundSprite = Sprite::create("res/card_general.png");
        if (_backgroundSprite) {
            _backgroundSprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
            this->addChild(_backgroundSprite);
            
            // 使用背景图片的原始尺寸作为卡牌尺寸
            Size bgSize = _backgroundSprite->getContentSize();
            this->setContentSize(bgSize);
            cardWidth = bgSize.width;
            cardHeight = bgSize.height;
            CCLOG("Card %d using background size: %.1f x %.1f", _cardModel->getCardId(), cardWidth, cardHeight);
        }
    }
    if (_backgroundSprite) {
        _backgroundSprite->setPosition(Vec2(cardWidth * 0.5f, cardHeight * 0.5f)); // ��������
    }
    
    // ���������֣��м䣩
    if (!_bigNumberSprite) {
        _bigNumberSprite = Sprite::create();
        if (_bigNumberSprite) {
            _bigNumberSprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
            this->addChild(_bigNumberSprite);
        }
    }
    if (_bigNumberSprite) {
        _bigNumberSprite->setPosition(Vec2(cardWidth * 0.5f, cardHeight * 0.5f)); // ��������
    }
    
    // ����С���֣����Ͻǣ�
    if (!_smallNumberSprite) {
        _smallNumberSprite = Sprite::create();
        if (_smallNumberSprite) {
            _smallNumberSprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
            this->addChild(_smallNumberSprite);
        }
    }
    if (_smallNumberSprite) {
        // 小数字位置：右上角，距离边缘15%（85%内部）
        Vec2 smallPos = Vec2(cardWidth * 0.85f, cardHeight * 0.85f);
        _smallNumberSprite->setPosition(smallPos); // ���ϽǸ�Զ
        CCLOG("Small number sprite position set to (%.1f, %.1f)", smallPos.x, smallPos.y);
    }
    
    // ������ɫͼ�꣨���Ͻǣ�
    if (!_suitSprite) {
        _suitSprite = Sprite::create();
        if (_suitSprite) {
            _suitSprite->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
            this->addChild(_suitSprite);
        }
    }
    if (_suitSprite) {
        // 花色位置：左上角，距离边缘15%（85%内部）
        Vec2 suitPos = Vec2(cardWidth * 0.15f, cardHeight * 0.85f);
        _suitSprite->setPosition(suitPos); // ���ϽǸ�Զ
        CCLOG("Suit sprite position set to (%.1f, %.1f)", suitPos.x, suitPos.y);
    }
    
    // ����ͼƬ
    std::string bigNumberPath = getBigNumberImagePath(_cardModel->getFace(), _cardModel->getSuit());
    std::string smallNumberPath = getImagePathForCard(_cardModel->getFace(), _cardModel->getSuit());
    std::string suitPath = getSuitImagePath(_cardModel->getSuit());
    
    CCLOG("Card %d paths - Big: %s, Small: %s, Suit: %s", 
          _cardModel->getCardId(), bigNumberPath.c_str(), smallNumberPath.c_str(), suitPath.c_str());
    
    if (_bigNumberSprite) {
        _bigNumberSprite->setTexture(bigNumberPath);
        _bigNumberSprite->setVisible(true);
        Vec2 bigPos = _bigNumberSprite->getPosition();
        Size bigSize = _bigNumberSprite->getContentSize();
        CCLOG("Big number sprite: pos(%.1f, %.1f), size(%.1f x %.1f)", bigPos.x, bigPos.y, bigSize.width, bigSize.height);
    }
    
    if (_smallNumberSprite) {
        _smallNumberSprite->setTexture(smallNumberPath);
        _smallNumberSprite->setVisible(true);
        Vec2 smallPos = _smallNumberSprite->getPosition();
        Size smallSize = _smallNumberSprite->getContentSize();
        CCLOG("Small number sprite: pos(%.1f, %.1f), size(%.1f x %.1f)", smallPos.x, smallPos.y, smallSize.width, smallSize.height);
    }
    
    if (_suitSprite) {
        _suitSprite->setTexture(suitPath);
        _suitSprite->setVisible(true);
        Vec2 suitPos = _suitSprite->getPosition();
        Size suitSize = _suitSprite->getContentSize();
        CCLOG("Suit sprite: pos(%.1f, %.1f), size(%.1f x %.1f)", suitPos.x, suitPos.y, suitSize.width, suitSize.height);
    }
}

std::string CardView::getBigNumberImagePath(CardFaceType face, CardSuitType suit) const {
    std::string faceStr;
    std::string colorStr = isRedSuit(suit) ? "red" : "black";
    
    // ת����ֵΪ�ַ���
    switch (face) {
        case CFT_ACE: faceStr = "A"; break;
        case CFT_TWO: faceStr = "2"; break;
        case CFT_THREE: faceStr = "3"; break;
        case CFT_FOUR: faceStr = "4"; break;
        case CFT_FIVE: faceStr = "5"; break;
        case CFT_SIX: faceStr = "6"; break;
        case CFT_SEVEN: faceStr = "7"; break;
        case CFT_EIGHT: faceStr = "8"; break;
        case CFT_NINE: faceStr = "9"; break;
        case CFT_TEN: faceStr = "10"; break;
        case CFT_JACK: faceStr = "J"; break;
        case CFT_QUEEN: faceStr = "Q"; break;
        case CFT_KING: faceStr = "K"; break;
        default: faceStr = "A"; break;
    }
    
    return StringUtils::format("res/number/big_%s_%s.png", colorStr.c_str(), faceStr.c_str());
}

std::string CardView::getImagePathForCard(CardFaceType face, CardSuitType suit) const {
    std::string faceStr;
    std::string colorStr = isRedSuit(suit) ? "red" : "black";
    
    // ת����ֵΪ�ַ���
    switch (face) {
        case CFT_ACE: faceStr = "A"; break;
        case CFT_TWO: faceStr = "2"; break;
        case CFT_THREE: faceStr = "3"; break;
        case CFT_FOUR: faceStr = "4"; break;
        case CFT_FIVE: faceStr = "5"; break;
        case CFT_SIX: faceStr = "6"; break;
        case CFT_SEVEN: faceStr = "7"; break;
        case CFT_EIGHT: faceStr = "8"; break;
        case CFT_NINE: faceStr = "9"; break;
        case CFT_TEN: faceStr = "10"; break;
        case CFT_JACK: faceStr = "J"; break;
        case CFT_QUEEN: faceStr = "Q"; break;
        case CFT_KING: faceStr = "K"; break;
        default: faceStr = "A"; break;
    }
    
    return StringUtils::format("res/number/small_%s_%s.png", colorStr.c_str(), faceStr.c_str());
}

std::string CardView::getSuitImagePath(CardSuitType suit) const {
    switch (suit) {
        case CST_HEARTS: return "res/suits/heart.png";
        case CST_DIAMONDS: return "res/suits/diamond.png";
        case CST_CLUBS: return "res/suits/club.png";
        case CST_SPADES: return "res/suits/spade.png";
        default: return "res/suits/heart.png";
    }
}

bool CardView::isRedSuit(CardSuitType suit) const {
    return (suit == CST_HEARTS || suit == CST_DIAMONDS);
}

