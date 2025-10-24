#include "GameModelFromLevelGenerator.h"
#include "../models/CardModel.h"
#include "../models/GameModel.h"
#include "../configs/models/LevelConfig.h"
#include "cocos2d.h"

GameModel GameModelFromLevelGenerator::generateGameModel(const LevelConfig& levelConfig) {
    GameModel gameModel;
    
    // 验证配置有效性
    if (!validateLevelConfig(levelConfig)) {
        CCLOGERROR("Invalid level config");
        return gameModel;
    }
    
    // 生成主牌堆卡牌
    int cardId = 1000;
    for (const auto& cardConfig : levelConfig.playfieldCards) {
        CardModel card(cardId,
                      cardConfig.face,
                      cardConfig.suit,
                      cardConfig.position);
        card.setCovered(cardConfig.isCovered);
        card.setIsInPlayfield(true);
        gameModel.addCard(card, true);
        cardId++;
    }
    
    // 生成备用牌堆卡牌
    for (const auto& cardConfig : levelConfig.stackCards) {
        CardModel card(cardId,
                      cardConfig.face,
                      cardConfig.suit,
                      cardConfig.position);
        card.setCovered(cardConfig.isCovered);
        card.setIsInPlayfield(false);
        gameModel.addCard(card, false);
        cardId++;
    }
    
    // 初始化备用牌堆和底牌堆
    if (!levelConfig.stackCards.empty()) {
        const auto& stackCards = gameModel.getStackCardIds();
        
        // 将所有备用牌添加到栈中
        for (int stackCardId : stackCards) {
            gameModel.pushToStackPile(stackCardId);
        }
        
        // 自动弹栈：将备用牌堆的第一张牌弹到底牌堆
        if (!gameModel.isStackPileEmpty()) {
            int firstStackCard = gameModel.popFromStackPile();
            gameModel.pushToBottomPile(firstStackCard);
            gameModel.setTopCard(firstStackCard);
        }
    }
    
    return gameModel;
}

bool GameModelFromLevelGenerator::validateLevelConfig(const LevelConfig& levelConfig) {
    // 检查是否有卡牌
    if (levelConfig.playfieldCards.empty() && levelConfig.stackCards.empty()) {
        CCLOGERROR("Level config has no cards");
        return false;
    }
    
    // 检查卡牌数据有效性
    for (const auto& card : levelConfig.playfieldCards) {
        if (card.face < 0 || card.face >= CFT_NUM_CARD_FACE_TYPES ||
            card.suit < 0 || card.suit >= CST_NUM_CARD_SUIT_TYPES) {
            CCLOGERROR("Invalid card data in playfield");
            return false;
        }
    }
    
    for (const auto& card : levelConfig.stackCards) {
        if (card.face < 0 || card.face >= CFT_NUM_CARD_FACE_TYPES ||
            card.suit < 0 || card.suit >= CST_NUM_CARD_SUIT_TYPES) {
            CCLOGERROR("Invalid card data in stack");
            return false;
        }
    }
    
    return true;
}

void GameModelFromLevelGenerator::applyCardGenerationStrategy(GameModel& gameModel, const std::string& strategy) {
    // 默认策略：按配置顺序生成
    // 未来可以扩展其他策略，如随机生成、难度调整等
    CCLOG("Applying card generation strategy: %s", strategy.c_str());
}
