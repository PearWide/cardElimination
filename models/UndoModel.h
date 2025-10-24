#ifndef __UNDO_MODEL_H__
#define __UNDO_MODEL_H__

#include "cocos2d.h"
#include <vector>

/**
 * @enum UndoActionType
 * @brief ���˲�������ö��
 */
enum class UndoActionType {
    CARD_MATCH,     ///< ����ƥ�����
    STACK_DRAW      ///< ���Ʋ���
};

/**
 * @struct UndoStep
 * @brief ������������
 */
struct UndoStep {
    UndoActionType actionType;          ///< ��������
    int cardId1;                        ///< ����1 ID
    int cardId2;                        ///< ����2 ID
    cocos2d::Vec2 originalPos1;         ///< ����1ԭʼλ��
    cocos2d::Vec2 originalPos2;         ///< ����2ԭʼλ��
    bool wasCovered1;                   ///< ����1�Ƿ񸲸�
    bool wasCovered2;                   ///< ����2�Ƿ񸲸�
    bool wasInPlayfield1;               ///< ����1�Ƿ���������
    bool wasInPlayfield2;               ///< ����2�Ƿ���������
    int zOrder1;                        ///< ����1�㼶
    int zOrder2;                        ///< ����2�㼶

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
 * @brief ��������ģ�ͣ��洢���л��˲���
 */
class UndoModel {
public:
    UndoModel(size_t maxSteps = 100);
    ~UndoModel();

    // ��ӻ��˲���
    void addStep(const UndoStep& step);

    // ��ȡ���Ƴ����һ��
    UndoStep popStep();

    // ����Ƿ��пɻ��˲���
    bool canUndo() const;

    // ��ȡ���˲�������
    size_t getStepCount() const;

    // ������л��˼�¼
    void clear();

    // �����������
    void setMaxSteps(size_t maxSteps);

private:
    // ���Ʋ����������Ƴ����粽��
    void trimSteps();

    std::vector<UndoStep> _undoSteps;   ///< ���˲���ջ
    size_t _maxSteps;                   ///< �������
};

#endif // __UNDO_MODEL_H__