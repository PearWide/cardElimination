#ifndef __CARD_CONTROLLER_H__
#define __CARD_CONTROLLER_H__

#include "cocos2d.h"

// 前向声明
class GameModel;
class GameView;
class UndoManager;

/**
 * @class CardController
 * @brief 处理卡片相关的具体逻辑
 * 
 * 职责：
 * - 处理卡片点击事件
 * - 管理卡片匹配逻辑
 * - 协调卡片动画
 */
class CardController {
public:
    CardController();
    ~CardController();
    
    /**
     * @brief 初始化控制器
     * @param gameModel 游戏模型
     * @param gameView 游戏视图
     * @param undoManager 回退管理器
     */
    void init(GameModel* gameModel, GameView* gameView, UndoManager* undoManager);
    
    /**
     * @brief 处理卡片点击事件
     * @param cardId 卡片ID
     * @return 是否处理成功
     */
    bool handleCardClick(int cardId);
    
    /**
     * @brief 处理主牌堆卡片点击
     * @param cardId 卡片ID
     * @return 是否处理成功
     */
    bool handlePlayfieldCardClick(int cardId);
    
    /**
     * @brief 处理备用牌堆卡片点击
     * @param cardId 卡片ID
     * @return 是否处理成功
     */
    bool handleStackCardClick(int cardId);
    
    /**
     * @brief 播放回退动画
     */
    void playUndoAnimations();
    
    /**
     * @brief 计算卡牌的正确层级
     * @param cardId 卡牌ID
     * @return 正确的zOrder值
     */
    int calculateCorrectZOrder(int cardId);
    
    /**
     * @brief 替换顶部牌
     * @param newCardId 新卡片ID
     */
    void replaceTopCard(int newCardId);
    
    /**
     * @brief 设置动画播放状态
     * @param playing 是否正在播放动画
     */
    void setAnimationPlaying(bool playing);
    
    /**
     * @brief 检查是否正在播放动画
     * @return 是否正在播放动画
     */
    bool isAnimationPlaying() const;
    
    /**
     * @brief 检查并完成回退操作
     */
    void checkAndCompleteUndo();

private:
    GameModel* _gameModel;
    GameView* _gameView;
    UndoManager* _undoManager;
    
    // 动画状态管理
    bool _isAnimationPlaying;
    
    /**
     * @brief 检查卡片是否可以匹配
     * @param cardId1 卡片1 ID
     * @param cardId2 卡片2 ID
     * @return 是否可以匹配
     */
    bool canCardsMatch(int cardId1, int cardId2);
    
    /**
     * @brief 播放卡片匹配动画
     * @param cardId 卡片ID
     */
    void playMatchAnimation(int cardId);
};

#endif // __CARD_CONTROLLER_H__
