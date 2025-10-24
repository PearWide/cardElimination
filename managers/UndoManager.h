#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include "cocos2d.h"
#include "../models/UndoModel.h"

// 前向声明
class GameModel;

/**
 * @class UndoManager
 * @brief 回退管理器，负责管理游戏中的撤销操作
 */
class UndoManager {
public:
    UndoManager();
    ~UndoManager();

    // 初始化回退管理器
    void init(GameModel* gameModel);

    // 记录卡牌匹配操作
    void recordCardMatch(int playfieldCardId, int trayCardId);

    // 记录抽牌操作
    void recordStackDraw(int drawnCardId, int previousTrayCardId);

    // 执行回退操作
    bool undo();

    // 检查是否可以回退
    bool canUndo() const;

    // 清空所有回退记录
    void clear();

    // 获取回退步骤数量
    size_t getStepCount() const;

    // 设置最大回退步骤数
    void setMaxSteps(size_t maxSteps);

private:
    // 恢复卡牌匹配操作
    bool restoreCardMatch(const UndoStep& step);

    // 恢复抽牌操作
    bool restoreStackDraw(const UndoStep& step);

    // 查找卡牌的原始位置
    cocos2d::Vec2 findCardOriginalPosition(int cardId) const;

    UndoModel _undoModel;
    GameModel* _gameModel;
};

#endif // __UNDO_MANAGER_H__