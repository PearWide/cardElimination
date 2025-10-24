#ifndef __CARD_VIEW_H__
#define __CARD_VIEW_H__

#include "cocos2d.h"
#include "../models/CardModel.h"

class CardView : public cocos2d::Sprite {
public:
    using ClickCallback = std::function<void(int cardId)>;

    CREATE_FUNC(CardView);

    virtual bool init() override;

    void setCardId(int cardId) { _cardId = cardId; }
    int getCardId() const { return _cardId; }

    void setClickCallback(const ClickCallback& callback) {
        _clickCallback = callback;
    }

    void setClickEnabled(bool enabled);
    void updateDisplay();
    void setCardModel(const CardModel* cardModel) { _cardModel = cardModel; }
    
    // 动画功能
    void playMoveToAnimation(const cocos2d::Vec2& targetPosition, float duration = 0.5f);
    void playMatchAnimation();
    
    // 公有方法，用于测试
    void onClicked();

private:
    void setupTouchEvents();
    // 已废弃：现在使用Cocos2d-x内置的getBoundingBox().containsPoint()
    bool containsTouchPoint_DEPRECATED(cocos2d::Touch* touch);
    void createCompositeCard();
    std::string getImagePathForCard(CardFaceType face, CardSuitType suit) const;
    std::string getBigNumberImagePath(CardFaceType face, CardSuitType suit) const;
    std::string getSuitImagePath(CardSuitType suit) const;
    bool isRedSuit(CardSuitType suit) const;

    int _cardId;
    ClickCallback _clickCallback;
    cocos2d::EventListenerTouchOneByOne* _touchListener;
    const CardModel* _cardModel;
    
    // ���Ͽ������
    cocos2d::Sprite* _backgroundSprite;
    cocos2d::Sprite* _bigNumberSprite;
    cocos2d::Sprite* _smallNumberSprite;
    cocos2d::Sprite* _suitSprite;
};

#endif // __CARD_VIEW_H__