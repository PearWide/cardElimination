#ifndef __GAME_MODEL_H__
#define __GAME_MODEL_H__

#include "cocos2d.h"
#include "CardModel.h"
#include <vector>
#include <unordered_map>

#include <functional>

// 前向声明
class CardView;

class GameModel {
public:
    enum class GameState {
        INITIALIZING,
        PLAYING,
        PAUSED,
        GAME_OVER,
        VICTORY
    };

    GameModel();

    // �������ݷ���
    const std::unordered_map<int, CardModel>& getAllCards() const { return _allCards; }
    const std::vector<int>& getPlayfieldCardIds() const { return _playfieldCardIds; }
    const std::vector<int>& getStackCardIds() const { return _stackCardIds; }
    const std::vector<int>& getBottomCardIds() const { return _bottomCardIds; }

    // ���ƹ���
    CardModel* getCard(int cardId);
    const CardModel* getCard(int cardId) const;
    void addCard(const CardModel& card, bool isPlayfield);

    // �����ƹ���
    CardModel* getTopCard();
    const CardModel* getTopCard() const;
    void setTopCard(int cardId);
    bool isTopCard(int cardId) const;
    void removeFromPlayfield(int cardId);

    // ��Ϸ״̬
    GameState getGameState() const { return _gameState; }
    void setGameState(GameState newState);

    // �������ƶ�
    int getScore() const { return _score; }
    void setScore(int score) { _score = score; }
    int getMoveCount() const { return _moveCount; }
    void incrementMoveCount() { _moveCount++; }

    // ��Ϸ�߼�
    bool checkGameWin() const;
    bool canMatchCards(int cardId1, int cardId2) const;
    void removeFromStack(int cardId);
    
    // 栈结构管理
    void pushToStackPile(int cardId);
    void pushToStackPileAndContainer(int cardId); // Added for undo functionality
    int popFromStackPile();
    void pushToBottomPile(int cardId);
    int popFromBottomPile(); // Added for undo functionality
    int getStackPileTop() const;
    int getBottomPileTop() const;
    bool isStackPileEmpty() const;
    bool isBottomPileEmpty() const;
    
    // 依赖图管理
    void buildDependencyGraph();
    void buildDependencyGraphWithViews(const std::unordered_map<int, CardView*>& cardViews);
    void addDependency(int cardId, int coveredCardId);
    bool isCardCovered(int cardId) const;
    void removeCardFromPlayfield(int cardId);
    void restoreCardToPlayfield(int cardId); // 用于回退功能

private:
    std::unordered_map<int, CardModel> _allCards;
    std::vector<int> _playfieldCardIds;
    std::vector<int> _stackCardIds;
    std::vector<int> _bottomCardIds; // Added for bottom pile management
    int _currentTopCardId;

    GameState _gameState;
    int _score;
    int _moveCount;
    
    // 栈结构：备用牌库和底牌库
    std::vector<int> _stackPile;      // 备用牌库栈
    std::vector<int> _bottomPile;     // 底牌库栈
    
    // 主牌堆依赖图和状态
    std::unordered_map<int, std::vector<int>> _dependencyGraph;  // 依赖图：卡牌ID -> 被覆盖的卡牌ID列表
    std::unordered_map<int, bool> _playfieldStatus;              // 主牌堆状态：卡牌ID -> 是否还在主牌堆
};

#endif // __GAME_MODEL_H__