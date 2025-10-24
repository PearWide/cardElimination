#include "GameModel.h"
#include "../views/CardView.h"
#include <algorithm>
#include <iterator>

USING_NS_CC;

GameModel::GameModel()
    : _currentTopCardId(-1)
    , _gameState(GameState::INITIALIZING)
    , _score(0)
    , _moveCount(0) {
}

CardModel* GameModel::getCard(int cardId) {
    // 添加安全检查
    if (_allCards.empty()) {
        CCLOGERROR("GameModel::getCard - _allCards is empty, cardId: %d", cardId);
        return nullptr;
    }
    
    auto it = _allCards.find(cardId);
    if (it != _allCards.end()) {
        return &(it->second);
    }
    
    CCLOGERROR("GameModel::getCard - card not found: %d, total cards: %d", cardId, (int)_allCards.size());
    return nullptr;
}

const CardModel* GameModel::getCard(int cardId) const {
    auto it = _allCards.find(cardId);
    if (it != _allCards.end()) {
        return &(it->second);
    }
    return nullptr;
}

void GameModel::addCard(const CardModel& card, bool isPlayfield) {
    int cardId = card.getCardId();
    CCLOG("GameModel::addCard - adding card ID=%d, isPlayfield=%s", cardId, isPlayfield ? "true" : "false");
    
    _allCards[cardId] = card;
    CCLOG("GameModel::addCard - card added to _allCards, total cards: %d", (int)_allCards.size());

    if (isPlayfield) {
        _playfieldCardIds.push_back(cardId);
        CCLOG("GameModel::addCard - added to playfield, playfield count: %d", (int)_playfieldCardIds.size());
    } else {
        _stackCardIds.push_back(cardId);
        CCLOG("GameModel::addCard - added to stack, stack count: %d", (int)_stackCardIds.size());
    }
}

CardModel* GameModel::getTopCard() {
    if (_currentTopCardId == -1 && !_stackCardIds.empty()) {
        _currentTopCardId = _stackCardIds.front();
    }
    return getCard(_currentTopCardId);
}

const CardModel* GameModel::getTopCard() const {
    if (_currentTopCardId == -1 && !_stackCardIds.empty()) {
        return getCard(_stackCardIds.front());
    }
    return getCard(_currentTopCardId);
}

void GameModel::setTopCard(int cardId) {
    if (_allCards.find(cardId) == _allCards.end()) {
        CCLOGERROR("Cannot set top card: card %d not found", cardId);
        return;
    }

    _currentTopCardId = cardId;

    // ȷ���������ڶ�ջ��ǰ��
    auto it = std::find(_stackCardIds.begin(), _stackCardIds.end(), cardId);
    if (it == _stackCardIds.end()) {
        _stackCardIds.insert(_stackCardIds.begin(), cardId);
    }

    CCLOG("Top card set to: %d", cardId);
}

bool GameModel::isTopCard(int cardId) const {
    if (_currentTopCardId != -1) {
        return _currentTopCardId == cardId;
    }
    return !_stackCardIds.empty() && _stackCardIds.front() == cardId;
}

void GameModel::setGameState(GameState newState) {
    // ����thisָ����
    if (this == nullptr) {
        CCLOGERROR("CRITICAL ERROR: setGameState called on nullptr!");
        return;  // �������أ��������
    }

    if (_gameState != newState) {
        CCLOG("Game state changed: %d -> %d",
            static_cast<int>(_gameState), static_cast<int>(newState));
        _gameState = newState;
    }
}

bool GameModel::checkGameWin() const {
    return _playfieldCardIds.empty();
}

bool GameModel::canMatchCards(int cardId1, int cardId2) const {
    const CardModel* card1 = getCard(cardId1);
    const CardModel* card2 = getCard(cardId2);
    return card1 && card2 && card1->canMatchWith(*card2);
}

void GameModel::removeFromPlayfield(int cardId) {
    // 先获取卡牌并更新状态，再移除
    auto card = getCard(cardId);
    if (card) {
        card->setIsInPlayfield(false);
    }
    
    // 然后从容器中移除
    auto it = std::find(_playfieldCardIds.begin(), _playfieldCardIds.end(), cardId);
    if (it != _playfieldCardIds.end()) {
        _playfieldCardIds.erase(it);
        CCLOG("Removed card %d from playfield", cardId);
    }
}

void GameModel::removeFromStack(int cardId) {
    // 先获取卡牌并更新状态，再移除
    auto card = getCard(cardId);
    if (card) {
        card->setIsInPlayfield(false);
    }
    
    // 从 _stackCardIds 中移除
    auto it = std::find(_stackCardIds.begin(), _stackCardIds.end(), cardId);
    if (it != _stackCardIds.end()) {
        _stackCardIds.erase(it);
    }
    
    // 从 _stackPile 中移除
    auto pileIt = std::find(_stackPile.begin(), _stackPile.end(), cardId);
    if (pileIt != _stackPile.end()) {
        _stackPile.erase(pileIt);
    }
}

// 栈结构管理
void GameModel::pushToStackPile(int cardId) {
    _stackPile.push_back(cardId);
}

void GameModel::pushToStackPileAndContainer(int cardId) {
    _stackPile.push_back(cardId);
    _stackCardIds.push_back(cardId);
}

int GameModel::popFromStackPile() {
    if (_stackPile.empty()) {
        CCLOGERROR("Stack pile is empty, cannot pop");
        return -1;
    }
    int cardId = _stackPile.back();
    _stackPile.pop_back();
    
    // 同时从 _stackCardIds 中移除
    CCLOG("Before removing from _stackCardIds: [");
    for (int id : _stackCardIds) {
        CCLOG("  %d", id);
    }
    CCLOG("]");
    
    auto it = std::find(_stackCardIds.begin(), _stackCardIds.end(), cardId);
    if (it != _stackCardIds.end()) {
        _stackCardIds.erase(it);
        CCLOG("Removed card %d from stack card IDs", cardId);
    } else {
        CCLOGERROR("Card %d not found in _stackCardIds!", cardId);
    }
    
    CCLOG("After removing from _stackCardIds: [");
    for (int id : _stackCardIds) {
        CCLOG("  %d", id);
    }
    CCLOG("]");
    
    CCLOG("Popped card %d from stack pile", cardId);
    return cardId;
}

void GameModel::pushToBottomPile(int cardId) {
    CCLOG("=== PUSH TO BOTTOM PILE DEBUG ===");
    CCLOG("Before pushToBottomPile - _stackCardIds: [");
    for (int id : _stackCardIds) {
        CCLOG("  %d", id);
    }
    CCLOG("]");
    
    // 设置卡牌状态：底牌堆卡牌既不在主牌堆也不在备用牌堆
    auto card = getCard(cardId);
    if (card) {
        card->setIsInPlayfield(false); // 不在主牌堆
        // 注意：底牌堆卡牌保持 isInPlayfield = false，但通过 _bottomCardIds 来识别
    }
    
    _bottomPile.push_back(cardId);
    _bottomCardIds.push_back(cardId);
    
    CCLOG("After pushToBottomPile - _stackCardIds: [");
    for (int id : _stackCardIds) {
        CCLOG("  %d", id);
    }
    CCLOG("]");
    
    CCLOG("Pushed card %d to bottom pile", cardId);
    CCLOG("Bottom pile now has %d cards", (int)_bottomPile.size());
    CCLOG("=== PUSH TO BOTTOM PILE DEBUG END ===");
}

int GameModel::popFromBottomPile() {
    if (_bottomPile.empty()) {
        CCLOGERROR("Bottom pile is empty, cannot pop");
        return -1;
    }
    int cardId = _bottomPile.back();
    _bottomPile.pop_back();
    
    // 同时从 _bottomCardIds 中移除
    auto it = std::find(_bottomCardIds.begin(), _bottomCardIds.end(), cardId);
    if (it != _bottomCardIds.end()) {
        _bottomCardIds.erase(it);
        CCLOG("Removed card %d from bottom card IDs", cardId);
    }
    
    CCLOG("Popped card %d from bottom pile", cardId);
    CCLOG("Bottom pile now has %d cards", (int)_bottomPile.size());
    return cardId;
}

int GameModel::getStackPileTop() const {
    if (_stackPile.empty()) {
        return -1;
    }
    return _stackPile.back();
}

int GameModel::getBottomPileTop() const {
    if (_bottomPile.empty()) {
        return -1;
    }
    return _bottomPile.back();
}

bool GameModel::isStackPileEmpty() const {
    return _stackPile.empty();
}

bool GameModel::isBottomPileEmpty() const {
    return _bottomPile.empty();
}

// 依赖图管理
void GameModel::buildDependencyGraph() {
    _dependencyGraph.clear();
    _playfieldStatus.clear();
    
    // 初始化所有主牌堆卡牌状态
    for (int cardId : _playfieldCardIds) {
        _playfieldStatus[cardId] = true;
    }
    
    // 初始化所有备用牌堆卡牌状态（虽然它们不会覆盖主牌堆卡牌，但需要避免访问异常）
    for (int cardId : _stackCardIds) {
        _playfieldStatus[cardId] = false;  // 备用牌堆卡牌不在主牌堆中
    }
    
    // 使用简化的JSON顺序逻辑构建依赖图
    // 按JSON顺序，后面的卡牌覆盖前面的卡牌
    CCLOG("=== Building Dependency Graph ===");
    CCLOG("Playfield cards in order: [");
    for (size_t i = 0; i < _playfieldCardIds.size(); ++i) {
        CCLOG("  [%d] Card ID: %d", (int)i, _playfieldCardIds[i]);
    }
    CCLOG("]");
    
    for (size_t i = 0; i < _playfieldCardIds.size(); ++i) {
        int cardId = _playfieldCardIds[i];
        CCLOG("Processing card %d (index %d)", cardId, (int)i);
        
        // 每个卡牌覆盖它之前的所有卡牌
        for (size_t j = 0; j < i; ++j) {
            int coveredCardId = _playfieldCardIds[j];
            addDependency(cardId, coveredCardId);
            CCLOG("  -> Card %d covers card %d", cardId, coveredCardId);
        }
    }
    
    CCLOG("=== Dependency Graph Summary ===");
    CCLOG("Total playfield cards: %d", (int)_playfieldCardIds.size());
    CCLOG("Dependency relationships:");
    for (const auto& pair : _dependencyGraph) {
        int coveringCard = pair.first;
        const std::vector<int>& coveredCards = pair.second;
        CCLOG("  Card %d covers %d cards: [", coveringCard, (int)coveredCards.size());
        for (int coveredCard : coveredCards) {
            CCLOG("    %d", coveredCard);
        }
        CCLOG("  ]");
    }
    CCLOG("=== Dependency Graph Complete ===");
}

void GameModel::buildDependencyGraphWithViews(const std::unordered_map<int, CardView*>& cardViews) {
    CCLOG("=== Building Dependency Graph with Real Collision Detection ===");
    
    // 清空之前的依赖图
    _dependencyGraph.clear();
    _playfieldStatus.clear();
    
    // 初始化所有主牌堆卡牌状态
    for (int cardId : _playfieldCardIds) {
        _playfieldStatus[cardId] = true;
    }
    
    // 初始化所有备用牌堆卡牌状态（虽然它们不会覆盖主牌堆卡牌，但需要避免访问异常）
    for (int cardId : _stackCardIds) {
        _playfieldStatus[cardId] = false;  // 备用牌堆卡牌不在主牌堆中
    }
    
    CCLOG("Playfield cards: [");
    for (int cardId : _playfieldCardIds) {
        CCLOG("  %d", cardId);
    }
    CCLOG("]");
    
    // 使用3D思维构建依赖图：后摆放的卡牌覆盖先摆放的卡牌
    for (size_t i = 0; i < _playfieldCardIds.size(); ++i) {
        int cardId1 = _playfieldCardIds[i];
        auto cardView1 = cardViews.find(cardId1);
        if (cardView1 == cardViews.end()) continue;
        
        Rect rect1 = cardView1->second->getBoundingBox();
        CCLOG("Card %d (index %d) bounding box: (%.1f, %.1f, %.1f, %.1f)", 
              cardId1, (int)i, rect1.origin.x, rect1.origin.y, rect1.size.width, rect1.size.height);
        
        // 检查这张卡牌是否覆盖其他卡牌（只检查之前摆放的卡牌）
        for (size_t j = 0; j < i; ++j) {
            int cardId2 = _playfieldCardIds[j];
            auto cardView2 = cardViews.find(cardId2);
            if (cardView2 == cardViews.end()) continue;
            
            Rect rect2 = cardView2->second->getBoundingBox();
            
            // 检查是否有重叠（使用较小的重叠阈值，因为卡牌可能只是部分重叠）
            if (rect1.intersectsRect(rect2)) {
                // 卡牌1（后摆放）覆盖卡牌2（先摆放）：3D思维
                addDependency(cardId1, cardId2);
                CCLOG("  -> Card %d (later) covers card %d (earlier) - 3D layering", cardId1, cardId2);
            }
        }
    }
    
    // 输出依赖图内容用于调试
    CCLOG("=== Dependency Graph Content ===");
    for (const auto& pair : _dependencyGraph) {
        int coveringCard = pair.first;
        const std::vector<int>& coveredCards = pair.second;
        CCLOG("Card %d covers: [", coveringCard);
        for (int coveredCard : coveredCards) {
            CCLOG("  %d", coveredCard);
        }
        CCLOG("]");
    }
    CCLOG("=== Real Collision-based Dependency Graph Complete ===");
}

void GameModel::addDependency(int cardId, int coveredCardId) {
    _dependencyGraph[cardId].push_back(coveredCardId);
}


bool GameModel::isCardCovered(int cardId) const {
    // 检查是否有其他卡牌覆盖这张卡牌
    for (const auto& pair : _dependencyGraph) {
        int coveringCardId = pair.first;
        const std::vector<int>& coveredCards = pair.second;
        
        // 跳过被检查的卡牌自己
        if (coveringCardId == cardId) {
            continue;
        }
        
        // 安全检查：确保coveringCardId在_playfieldStatus中存在
        auto statusIt = _playfieldStatus.find(coveringCardId);
        if (statusIt == _playfieldStatus.end()) {
            continue;
        }
        
        // 如果覆盖卡牌还在主牌堆中，且被覆盖的卡牌包含当前卡牌
        if (statusIt->second && 
            std::find(coveredCards.begin(), coveredCards.end(), cardId) != coveredCards.end()) {
            return true;
        }
    }
    
    return false;
}

void GameModel::removeCardFromPlayfield(int cardId) {
    // 更新状态表（保留依赖图不变，用于回退功能）
    _playfieldStatus[cardId] = false;
    
    // 从主牌堆容器中移除
    auto it = std::find(_playfieldCardIds.begin(), _playfieldCardIds.end(), cardId);
    if (it != _playfieldCardIds.end()) {
        _playfieldCardIds.erase(it);
    }
}

void GameModel::restoreCardToPlayfield(int cardId) {
    // 恢复卡牌状态（用于回退功能）
    _playfieldStatus[cardId] = true;
    
    // 重新添加到主牌堆容器
    _playfieldCardIds.push_back(cardId);
}
