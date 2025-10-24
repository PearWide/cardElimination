#ifndef __UNDO_MODEL_H__
#define __UNDO_MODEL_H__

#include "cocos2d.h"
#include <vector>

/**
 * @enum UndoActionType
 * @brief 回退操作类型枚举
 */
enum class UndoActionType {
    CARD_MATCH,     ///< 卡牌匹配操作
    STACK_DRAW      ///< 抽牌操作
};

/**
 * @struct UndoStep
 * @brief 单步回退数据
 */
struct UndoStep {
    UndoActionType actionType;          ///< 操作类型
    int cardId1;                        ///< 卡牌1 ID
    int cardId2;                        ///< 卡牌2 ID
    cocos2d::Vec2 originalPos1;         ///< 卡牌1原始位置
    cocos2d::Vec2 originalPos2;         ///< 卡牌2原始位置
    bool wasCovered1;                   ///< 卡牌1是否覆盖
    bool wasCovered2;                   ///< 卡牌2是否覆盖
    bool wasInPlayfield1;               ///< 卡牌1是否在主牌区
    bool wasInPlayfield2;               ///< 卡牌2是否在主牌区
    int zOrder1;                        ///< 卡牌1层级
    int zOrder2;                        ///< 卡牌2层级

    UndoStep()
        : actionType(UndoActionType::CARD_MATCH)
        , cardId1(-1), cardId2(-1)
        , originalPos1(cocos2d::Vec2::ZERO), originalPos2(cocos2d::Vec2::ZERO)
        , wasCovered1(false), wasCovered2(false)
        , wasInPlayfield1(true), wasInPlayfield2(false)
        , zOrder1(0), zOrder2(0) {}
};

/**
 * @class UndoModel
 * @brief 回退数据模型，存储所有回退步骤
 */
class UndoModel {
public:
    UndoModel(size_t maxSteps = 100);
    ~UndoModel();

    // 添加回退步骤
    void addStep(const UndoStep& step);

    // 获取并移除最后一步
    UndoStep popStep();

    // 检查是否有可回退步骤
    bool canUndo() const;

    // 获取回退步骤数量
    size_t getStepCount() const;

    // 清空所有回退记录
    void clear();

    // 设置最大步骤数
    void setMaxSteps(size_t maxSteps);

private:
    // 限制步骤数量，移除最早步骤
    void trimSteps();

    std::vector<UndoStep> _undoSteps;   ///< 回退步骤栈
    size_t _maxSteps;                   ///< 最大步骤数
};

#endif // __UNDO_MODEL_H__