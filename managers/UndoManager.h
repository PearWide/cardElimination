#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include "cocos2d.h"
#include "../models/UndoModel.h"

// ǰ������
class GameModel;

/**
 * @class UndoManager
 * @brief ���˹����������������Ϸ�еĳ�������
 */
class UndoManager {
public:
    UndoManager();
    ~UndoManager();

    // ��ʼ�����˹�����
    void init(GameModel* gameModel);

    // ��¼����ƥ�����
    void recordCardMatch(int playfieldCardId, int trayCardId);

    // ��¼���Ʋ���
    void recordStackDraw(int drawnCardId, int previousTrayCardId);

    // ִ�л��˲���
    bool undo();

    // ����Ƿ���Ի���
    bool canUndo() const;

    // ������л��˼�¼
    void clear();

    // ��ȡ���˲�������
    size_t getStepCount() const;

    // ���������˲�����
    void setMaxSteps(size_t maxSteps);

private:
    // �ָ�����ƥ�����
    bool restoreCardMatch(const UndoStep& step);

    // �ָ����Ʋ���
    bool restoreStackDraw(const UndoStep& step);

    // ���ҿ��Ƶ�ԭʼλ��
    cocos2d::Vec2 findCardOriginalPosition(int cardId) const;

    UndoModel _undoModel;
    GameModel* _gameModel;
};

#endif // __UNDO_MANAGER_H__