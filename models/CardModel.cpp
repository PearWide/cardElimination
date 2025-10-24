#include "CardModel.h"
#include <cmath>

USING_NS_CC;

CardModel::CardModel()
    : _cardId(-1)
    , _face(CFT_NONE)
    , _suit(CST_NONE)
    , _position(Vec2::ZERO)
    , _isCovered(true)
    , _isInPlayfield(true)
    , _zOrder(0) {
}

CardModel::CardModel(int cardId, CardFaceType face, CardSuitType suit, const Vec2& position)
    : _cardId(cardId)
    , _face(face)
    , _suit(suit)
    , _position(position)
    , _isCovered(true)
    , _isInPlayfield(true)
    , _zOrder(0) {
}

int CardModel::getFaceValue() const {
    return static_cast<int>(_face);
}

bool CardModel::canMatchWith(const CardModel& other) const {
    int value1 = this->getFaceValue();
    int value2 = other.getFaceValue();

    // ����1: A(0) �� K(12) ����ƥ��
    if ((value1 == CFT_ACE && value2 == CFT_KING) ||
        (value1 == CFT_KING && value2 == CFT_ACE)) {
        CCLOG("Match: Ace and King");
        return true;
    }

    // ����2: �������1
    if (std::abs(value1 - value2) == 1) {
        CCLOG("Match: %d and %d (diff=1)", value1, value2);
        return true;
    }

    CCLOG("No match: %d and %d", value1, value2);
    return false;
}

bool CardModel::isOperatable() const {
    // �ɲ���������δ�����Ҵ��ڳ���
    if (_isCovered) {
        return false;
    }
    // 游戏区域或手牌区的卡牌都可以操作
    // 移除_isInPlayfield检查，让手牌区卡牌也可操作
    return true;
}

std::string CardModel::getDebugString() const {
    return StringUtils::format("Card[ID:%d, Face:%d, Suit:%d, Pos:(%.1f,%.1f)]",
        _cardId, static_cast<int>(_face),
        static_cast<int>(_suit), _position.x, _position.y);
}

void CardModel::printDebugInfo() const {
    CCLOG("%s", getDebugString().c_str());
}