#ifndef __UNDO_CONTROLLER_H__
#define __UNDO_CONTROLLER_H__

#include "cocos2d.h"

// 前向声明
class GameModel;
class GameView;
class UndoManager;

/**
 * @class UndoController
 * @brief 专门处理回退逻辑的控制器
 * 
 * 职责：
 * - 处理回退操作
 * - 管理回退动画
 * - 控制回退过程中的状态变化
 * - 防止不相干卡牌的闪现
 */
class UndoController {
public:
    UndoController();
    ~UndoController();
    
    /**
     * @brief 初始化控制器
     * @param gameModel 游戏模型
     * @param gameView 游戏视图
     * @param undoManager 回退管理器
     */
    void init(GameModel* gameModel, GameView* gameView, UndoManager* undoManager);
    
    /**
     * @brief 执行回退操作
     * @return 是否回退成功
     */
    bool executeUndo();
    
    /**
     * @brief 检查是否可以回退
     * @return 是否可以回退
     */
    bool canUndo() const;
    
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

private:
    GameModel* _gameModel;
    GameView* _gameView;
    UndoManager* _undoManager;
    
    /**
     * @brief 播放回退动画（只针对被回退的卡牌）
     * @param affectedCardIds 被回退影响的卡牌ID列表
     */
    void playUndoAnimations(const std::vector<int>& affectedCardIds);
    
    /**
     * @brief 获取被回退影响的卡牌ID列表
     * @return 被影响的卡牌ID列表
     */
    std::vector<int> getAffectedCardIds();
    
    /**
     * @brief 更新卡牌视图（只更新被影响的卡牌）
     * @param affectedCardIds 被影响的卡牌ID列表
     */
    void updateAffectedCardViews(const std::vector<int>& affectedCardIds);
    
    /**
     * @brief 计算卡牌的正确层级
     * @param cardId 卡牌ID
     * @return 正确的zOrder值
     */
    int calculateCorrectZOrder(int cardId);
    
    /**
     * @brief 恢复所有卡牌的正确层级关系
     */
    void restoreAllCardZOrders();
    
    /**
     * @brief 检查并完成回退操作
     */
    void checkAndCompleteUndo();

private:
    // 动画状态管理
    bool _isAnimationPlaying;
};

#endif // __UNDO_CONTROLLER_H__
