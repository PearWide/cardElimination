#ifndef __GAME_UTILS_H__
#define __GAME_UTILS_H__

#include "cocos2d.h"
#include "../models/GameModel.h"

/**
 * @class GameUtils
 * @brief 游戏通用工具类，提供静态方法消除代码重复
 * 
 * 职责：
 * - 提供统一的层级计算逻辑
 * - 提供统一的动画创建方法
 * - 提供统一的位置计算逻辑
 * - 提供通用的常量定义
 */
class GameUtils {
public:
    // 游戏常量定义
    static constexpr int PLAYFIELD_ZORDER_BASE = 2000;
    static constexpr int STACK_ZORDER_BASE = 1000;
    static constexpr int BOTTOM_PILE_ZORDER_BASE = 3000; // 底牌堆层级基础值
    static constexpr int MOVING_CARD_ZORDER = 5000; // 移动中卡牌的层级（最高）
    static constexpr float ANIMATION_DURATION = 0.5f;
    static constexpr float SCALE_ANIMATION_DURATION = 0.25f;
    static constexpr float PLAYFIELD_Y_OFFSET = 500.0f;
    static constexpr float STACK_BASE_X = 200.0f;
    static constexpr float STACK_BASE_Y = 300.0f;
    static constexpr float STACK_CARD_SPACING = 120.0f;
    static constexpr float TOP_CARD_X = 800.0f;
    static constexpr float TOP_CARD_Y = 300.0f;
    static constexpr float CARD_SCALE_FACTOR = 1.1f;
    
    /**
     * @brief 计算卡牌的正确层级
     * @param cardId 卡牌ID
     * @param gameModel 游戏模型
     * @return 正确的zOrder值
     */
    static int calculateCorrectZOrder(int cardId, const GameModel* gameModel);
    
    /**
     * @brief 计算卡牌的目标位置
     * @param cardModel 卡牌模型
     * @param cardId 卡牌ID
     * @return 计算后的目标位置
     */
    static cocos2d::Vec2 calculateTargetPosition(const CardModel* cardModel, int cardId);
    
    /**
     * @brief 创建回退动画序列（简化版本，不包含回调）
     * @param targetPosition 目标位置
     * @return 动画序列
     */
    static cocos2d::Sequence* createUndoAnimationSequence(const cocos2d::Vec2& targetPosition);
    
    /**
     * @brief 创建移动到顶部的动画序列（简化版本，不包含回调）
     * @param targetPosition 目标位置
     * @return 动画序列
     */
    static cocos2d::Sequence* createMoveToTopAnimationSequence(const cocos2d::Vec2& targetPosition);
    
    /**
     * @brief 创建匹配动画序列
     * @return 动画序列
     */
    static cocos2d::Sequence* createMatchAnimationSequence();
    
    /**
     * @brief 获取卡牌在JSON中的顺序
     * @param cardId 卡牌ID
     * @param gameModel 游戏模型
     * @return JSON顺序索引
     */
    static int getCardJsonOrder(int cardId, const GameModel* gameModel);
    
    /**
     * @brief 检查卡牌是否在底牌堆中
     * @param cardId 卡牌ID
     * @param gameModel 游戏模型
     * @return 是否在底牌堆中
     */
    static bool isCardInBottomPile(int cardId, const GameModel* gameModel);
    
    /**
     * @brief 应用位置调整逻辑
     * @param position 原始位置
     * @param cardModel 卡牌模型
     * @param cardId 卡牌ID
     * @return 调整后的位置
     */
    static cocos2d::Vec2 applyPositionAdjustment(
        const cocos2d::Vec2& position, 
        const CardModel* cardModel, 
        int cardId);
};

#endif // __GAME_UTILS_H__
