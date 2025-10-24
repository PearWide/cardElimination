#ifndef __CARD_MODEL_H__
#define __CARD_MODEL_H__

#include "cocos2d.h"

enum CardSuitType {
    CST_NONE = -1,
    CST_CLUBS,
    CST_DIAMONDS,
    CST_HEARTS,
    CST_SPADES,
    CST_NUM_CARD_SUIT_TYPES
};

enum CardFaceType {
    CFT_NONE = -1,
    CFT_ACE,
    CFT_TWO,
    CFT_THREE,
    CFT_FOUR,
    CFT_FIVE,
    CFT_SIX,
    CFT_SEVEN,
    CFT_EIGHT,
    CFT_NINE,
    CFT_TEN,
    CFT_JACK,
    CFT_QUEEN,
    CFT_KING,
    CFT_NUM_CARD_FACE_TYPES
};

class CardModel {
public:
    CardModel();
    CardModel(int cardId, CardFaceType face, CardSuitType suit,
        const cocos2d::Vec2& position = cocos2d::Vec2::ZERO);

    // Getters
    int getCardId() const { return _cardId; }
    CardFaceType getFace() const { return _face; }
    CardSuitType getSuit() const { return _suit; }
    const cocos2d::Vec2& getPosition() const { return _position; }
    bool isCovered() const { return _isCovered; }
    bool isInPlayfield() const { return _isInPlayfield; }
    int getZOrder() const { return _zOrder; }

    // Setters  
    void setCardId(int cardId) { _cardId = cardId; }
    void setFace(CardFaceType face) { _face = face; }
    void setSuit(CardSuitType suit) { _suit = suit; }
    void setPosition(const cocos2d::Vec2& position) { _position = position; }
    void setCovered(bool covered) { _isCovered = covered; }
    void setIsInPlayfield(bool inPlayfield) { _isInPlayfield = inPlayfield; }
    void setZOrder(int zOrder) { _zOrder = zOrder; }

    // 游戏逻辑方法
    int getFaceValue() const;
    bool canMatchWith(const CardModel& other) const;
    bool isOperatable() const;

    // 调试方法
    std::string getDebugString() const;
    void printDebugInfo() const;

private:
    int _cardId;
    CardFaceType _face;
    CardSuitType _suit;
    cocos2d::Vec2 _position;
    bool _isCovered;
    bool _isInPlayfield;
    int _zOrder;
};

#endif // __CARD_MODEL_H__