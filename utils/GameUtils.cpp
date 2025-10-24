#include "GameUtils.h"
#include "../models/CardModel.h"
#include "../models/GameModel.h"
#include <algorithm>

USING_NS_CC;

int GameUtils::calculateCorrectZOrder(int cardId, const GameModel* gameModel) {
    if (!gameModel) {
        CCLOGERROR("GameModel is null, cannot calculate zOrder");
        return STACK_ZORDER_BASE; // 默认层级
    }
    
    const CardModel* cardModel = gameModel->getCard(cardId);
    if (!cardModel) {
        CCLOGERROR("CardModel not found for card %d", cardId);
        return STACK_ZORDER_BASE; // 默认层级
    }
    
    int zOrder;
    
    // 优先检查是否在底牌堆中（因为底牌堆卡牌的 isInPlayfield 也是 false）
    const auto& bottomCardIds = gameModel->getBottomCardIds();
    auto bottomIt = std::find(bottomCardIds.begin(), bottomCardIds.end(), cardId);
    
    if (bottomIt != bottomCardIds.end()) {
        // 底牌堆卡牌：根据在底牌堆中的位置确定层级（越靠后层级越高）
        size_t bottomIndex = std::distance(bottomCardIds.begin(), bottomIt);
        zOrder = BOTTOM_PILE_ZORDER_BASE + (int)bottomIndex;
        CCLOG("Card %d: bottom pile zOrder=%d (bottom index=%d)", cardId, zOrder, (int)bottomIndex);
        return zOrder;
    }
    
    if (cardModel->isInPlayfield()) {
        // 主牌堆卡牌：根据JSON顺序确定层级
        const auto& playfieldCardIds = gameModel->getPlayfieldCardIds();
        
        // 查找卡牌在JSON中的索引位置
        for (size_t i = 0; i < playfieldCardIds.size(); ++i) {
            if (playfieldCardIds[i] == cardId) {
                zOrder = PLAYFIELD_ZORDER_BASE + (int)i; // JSON顺序越靠后，zOrder越高
                CCLOG("Card %d: playfield zOrder=%d (JSON order=%d)", cardId, zOrder, (int)i);
                return zOrder;
            }
        }
        
        // 如果没找到，使用默认值
        zOrder = PLAYFIELD_ZORDER_BASE;
        CCLOG("Card %d: playfield zOrder=%d (default, not found in playfield)", cardId, zOrder);
        
    } else {
        // 备用牌堆卡牌：正常层级
        zOrder = STACK_ZORDER_BASE;
        CCLOG("Card %d: stack zOrder=%d", cardId, zOrder);
    }
    
    return zOrder;
}

Vec2 GameUtils::calculateTargetPosition(const CardModel* cardModel, int cardId) {
    if (!cardModel) {
        CCLOGERROR("CardModel is null, cannot calculate target position");
        return Vec2::ZERO;
    }
    
    Vec2 position = cardModel->getPosition();
    
    if (cardModel->isInPlayfield()) {
        // 游戏区域卡牌：将y坐标向上调整
        position.y += PLAYFIELD_Y_OFFSET;
    } else {
        // 手牌区卡牌：如果位置为0，使用默认布局
        if (position == Vec2::ZERO) {
            int stackIndex = cardId - 1006; // 假设手牌区卡牌ID从1006开始
            float baseX = STACK_BASE_X + stackIndex * STACK_CARD_SPACING;
            position = Vec2(baseX, STACK_BASE_Y);
        }
    }
    
    return position;
}

Sequence* GameUtils::createUndoAnimationSequence(const Vec2& targetPosition) {
    // 创建移动动画
    auto moveAction = MoveTo::create(ANIMATION_DURATION, targetPosition);
    auto easeAction = EaseInOut::create(moveAction, 2.0f);
    
    // 创建缩放动画
    auto scaleUp = ScaleTo::create(SCALE_ANIMATION_DURATION, CARD_SCALE_FACTOR);
    auto scaleDown = ScaleTo::create(SCALE_ANIMATION_DURATION, 1.0f);
    auto scaleSequence = Sequence::create(scaleUp, scaleDown, nullptr);
    
    // 组合移动和缩放动画
    auto spawnAction = Spawn::create(easeAction, scaleSequence, nullptr);
    
    return Sequence::create(spawnAction, nullptr);
}

Sequence* GameUtils::createMoveToTopAnimationSequence(const Vec2& targetPosition) {
    // 创建移动动画
    auto moveAction = MoveTo::create(ANIMATION_DURATION, targetPosition);
    auto easeAction = EaseInOut::create(moveAction, 2.0f);
    
    // 创建缩放动画（移动到顶部时稍微放大）
    auto scaleAction = ScaleTo::create(ANIMATION_DURATION, CARD_SCALE_FACTOR);
    
    // 组合移动和缩放动画
    auto spawnAction = Spawn::create(easeAction, scaleAction, nullptr);
    
    return Sequence::create(spawnAction, nullptr);
}

Sequence* GameUtils::createMatchAnimationSequence() {
    // 创建匹配动画
    auto scaleUp = ScaleTo::create(0.1f, 1.2f);
    auto scaleDown = ScaleTo::create(0.1f, 1.0f);
    
    // 使用缓动效果
    auto easeUp = EaseOut::create(scaleUp, 2.0f);
    auto easeDown = EaseIn::create(scaleDown, 2.0f);
    
    return Sequence::create(easeUp, easeDown, nullptr);
}

int GameUtils::getCardJsonOrder(int cardId, const GameModel* gameModel) {
    if (!gameModel) {
        CCLOGERROR("GameModel is null, cannot get JSON order");
        return 0;
    }
    
    // 从GameModel获取卡牌在JSON中的顺序
    const auto& playfieldCardIds = gameModel->getPlayfieldCardIds();
    
    // 查找卡牌在JSON中的索引位置
    for (size_t i = 0; i < playfieldCardIds.size(); ++i) {
        if (playfieldCardIds[i] == cardId) {
            return (int)i; // 返回JSON顺序（从0开始）
        }
    }
    
    CCLOGERROR("Card %d not found in playfield cards", cardId);
    return 0; // 默认返回0
}

bool GameUtils::isCardInBottomPile(int cardId, const GameModel* gameModel) {
    if (!gameModel) {
        return false;
    }
    
    const auto& bottomCardIds = gameModel->getBottomCardIds();
    return std::find(bottomCardIds.begin(), bottomCardIds.end(), cardId) != bottomCardIds.end();
}

Vec2 GameUtils::applyPositionAdjustment(const Vec2& position, const CardModel* cardModel, int cardId) {
    if (!cardModel) {
        return position;
    }
    
    Vec2 adjustedPos = position;
    
    if (cardModel->isInPlayfield()) {
        // 游戏区域卡牌：将y坐标向上调整
        adjustedPos.y += PLAYFIELD_Y_OFFSET;
    } else if (adjustedPos == Vec2::ZERO) {
        // 手牌区卡牌：如果位置为0，使用默认布局
        int stackIndex = cardId - 1006; // 假设手牌区卡牌ID从1006开始
        float baseX = STACK_BASE_X + stackIndex * STACK_CARD_SPACING;
        adjustedPos = Vec2(baseX, STACK_BASE_Y);
    }
    
    return adjustedPos;
}
