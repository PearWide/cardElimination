#include "CardController.h"
#include "../models/GameModel.h"
#include "../views/GameView.h"
#include "../managers/UndoManager.h"
#include "../utils/GameUtils.h"

USING_NS_CC;

CardController::CardController()
    : _gameModel(nullptr)
    , _gameView(nullptr)
    , _undoManager(nullptr)
    , _isAnimationPlaying(false) {
}

CardController::~CardController() {
}

void CardController::init(GameModel* gameModel, GameView* gameView, UndoManager* undoManager) {
    CCLOG("CardController::init called with gameModel: %p, gameView: %p, undoManager: %p", 
          gameModel, gameView, undoManager);
    
    _gameModel = gameModel;
    _gameView = gameView;
    _undoManager = undoManager;
    
    CCLOG("CardController::init completed - _gameModel: %p, _gameView: %p, _undoManager: %p", 
          _gameModel, _gameView, _undoManager);
}

bool CardController::handleCardClick(int cardId) {
    // 检查是否有动画正在播放
    if (_isAnimationPlaying) {
        CCLOG("Card click ignored - animation is playing");
        return false;
    }
    
    CCLOG("CardController::handleCardClick called for card %d", cardId);
    CCLOG("CardController state - _gameModel: %p, _gameView: %p, _undoManager: %p", 
          _gameModel, _gameView, _undoManager);
    
    if (!_gameModel || !_gameView || !_undoManager) {
        CCLOGERROR("CardController not properly initialized");
        if (!_gameModel) CCLOGERROR("  - _gameModel is null");
        if (!_gameView) CCLOGERROR("  - _gameView is null");
        if (!_undoManager) CCLOGERROR("  - _undoManager is null");
        return false;
    }
    
    // 获取被点击的卡片
    CCLOG("CardController::handleCardClick - attempting to get card %d", cardId);
    CardModel* clickedCard = _gameModel->getCard(cardId);
    if (!clickedCard) {
        CCLOGERROR("Clicked card not found: %d", cardId);
        return false;
    }
    
    CCLOG("CardController::handleCardClick - card %d found, isInPlayfield: %s", 
          cardId, clickedCard->isInPlayfield() ? "true" : "false");
    
    // 根据卡片类型处理点击
    if (clickedCard->isInPlayfield()) {
        return handlePlayfieldCardClick(cardId);
    } else {
        return handleStackCardClick(cardId);
    }
}

bool CardController::handlePlayfieldCardClick(int cardId) {
    CCLOG("CardController handling playfield card click: %d", cardId);
    
    if (!_gameModel || !_undoManager) {
        CCLOGERROR("CardController not properly initialized");
        return false;
    }

    try {
        // 获取被点击的卡牌
        CardModel* clickedCard = _gameModel->getCard(cardId);
        if (!clickedCard) {
            CCLOGERROR("Clicked card not found: %d", cardId);
            return false;
        }

        // 检查卡牌是否可操作
        if (!clickedCard->isOperatable() || !clickedCard->isInPlayfield()) {
            CCLOG("Card %d is not operatable", cardId);
            return false;
        }
        
        // 检查卡牌是否被覆盖
        if (_gameModel->isCardCovered(cardId)) {
            CCLOG("Card %d is covered, cannot move", cardId);
            return false;
        }

        // 获取当前顶部牌
        const CardModel* topCard = _gameModel->getTopCard();
        if (!topCard) {
            CCLOGERROR("No top card available");
            return false;
        }

        // 检查是否可以匹配
        if (!_gameModel->canMatchCards(cardId, topCard->getCardId())) {
            CCLOG("Cards %d and %d cannot match", cardId, topCard->getCardId());
            return false;
        }

        CCLOG("Cards can match, executing match logic");

        // 记录撤销操作
        _undoManager->recordCardMatch(cardId, topCard->getCardId());

        // 执行匹配操作
        // 1. 从主牌堆移除卡牌（使用正确的方法更新状态）
        _gameModel->removeCardFromPlayfield(cardId);
        CCLOG("Removed card %d from playfield", cardId);
        
        // 2. 将卡牌添加到底牌堆
        _gameModel->pushToBottomPile(cardId);
        CCLOG("Added card %d to bottom pile", cardId);
        
        // 3. 设置新的顶部牌
        _gameModel->setTopCard(cardId);
        CCLOG("Top card set to: %d", cardId);
        
        // 4. 增加移动计数
        _gameModel->incrementMoveCount();
        
        // 5. 播放移动动画
        if (_gameView) {
            // 设置动画状态
            setAnimationPlaying(true);
            
            // 获取卡牌的原位置（用于动画）
            Vec2 originalPosition = clickedCard->getPosition();
            
            // 播放移动到顶部的动画
            _gameView->playCardMoveToTopAnimation(cardId, originalPosition);
        }
        
        CCLOG("Playfield card match completed successfully");
        return true;
    } catch (...) {
        CCLOGERROR("Exception in handlePlayfieldCardClick for card: %d", cardId);
        return false;
    }
}

bool CardController::handleStackCardClick(int cardId) {
    CCLOG("CardController handling stack card click: %d", cardId);
    
    if (!_gameModel || !_undoManager) {
        CCLOGERROR("CardController not properly initialized");
        return false;
    }

    try {
        // 获取被点击的卡牌
        CardModel* clickedCard = _gameModel->getCard(cardId);
        if (!clickedCard) {
            CCLOGERROR("Clicked card not found: %d", cardId);
            return false;
        }

        // 检查卡牌是否可操作（备用牌堆不需要检查覆盖状态）
        if (clickedCard->isInPlayfield()) {
            CCLOG("Card %d is in playfield, not stack", cardId);
            return false;
        }

        // 备用牌堆：只有栈顶可以点击
        int stackTop = _gameModel->getStackPileTop();
        CCLOG("Stack top: %d", stackTop);
        CCLOG("Clicked card: %d", cardId);
        
        if (stackTop == -1 || cardId != stackTop) {
            CCLOG("Card %d is not the stack top (top is %d)", cardId, stackTop);
            return false;
        }

        // 记录撤销操作
        int currentTopCardId = _gameModel->getTopCard() ? _gameModel->getTopCard()->getCardId() : -1;
        if (currentTopCardId != -1) {
            _undoManager->recordStackDraw(cardId, currentTopCardId);
        }

        // 执行替换操作
        // 1. 先更新卡牌状态，再修改容器
        clickedCard->setIsInPlayfield(false);
        clickedCard->setCovered(false);
        
        // 2. 从备用牌堆移除卡牌
        _gameModel->removeFromStack(cardId);
        CCLOG("Removed card %d from stack", cardId);
        
        // 3. 将卡牌添加到底牌堆
        _gameModel->pushToBottomPile(cardId);
        CCLOG("Added card %d to bottom pile", cardId);
        
        // 4. 设置新的顶部牌
        _gameModel->setTopCard(cardId);
        CCLOG("Top card set to: %d", cardId);
        
        // 5. 播放移动动画
        if (_gameView) {
            // 设置动画状态
            setAnimationPlaying(true);
            
            // 获取卡牌的原位置（用于动画）
            Vec2 originalPosition = clickedCard->getPosition();
            
            // 播放移动到顶部的动画
            _gameView->playCardMoveToTopAnimation(cardId, originalPosition);
        }
        
        CCLOG("Stack card replacement completed successfully");
        return true;
    } catch (...) {
        CCLOGERROR("Exception in handleStackCardClick for card: %d", cardId);
        return false;
    }
}

void CardController::replaceTopCard(int newCardId) {
    _gameModel->setTopCard(newCardId);
}

bool CardController::canCardsMatch(int cardId1, int cardId2) {
    const CardModel* card1 = _gameModel->getCard(cardId1);
    const CardModel* card2 = _gameModel->getCard(cardId2);
    
    if (!card1 || !card2) {
        return false;
    }
    
    // 匹配规则：数字相差1即可，不考虑花色
    int diff = std::abs(card1->getFaceValue() - card2->getFaceValue());
    return diff == 1;
}

void CardController::playMatchAnimation(int cardId) {
    if (_gameView) {
        // 设置动画状态
        setAnimationPlaying(true);
        
        CardModel* card = _gameModel->getCard(cardId);
        if (card) {
            Vec2 originalPosition = card->getPosition();
            _gameView->playCardMoveToTopAnimation(cardId, originalPosition);
        }
    }
}

void CardController::playUndoAnimations() {
    CCLOG("=== UNDO ANIMATION DEBUG START ===");
    
    if (!_gameView || !_gameModel) {
        CCLOGERROR("GameView or GameModel is null, cannot play undo animations");
        return;
    }
    
    CCLOG("Step 1: Updating only restored card positions");
    int updatedCount = 0;
    
    // 只更新被回退的卡牌，而不是所有卡牌
    // UndoManager已恢复卡牌位置，只需同步视图
    for (auto& pair : _gameView->getCardViews()) {
        if (pair.second) {
            CardView* cardView = pair.second.get();
            if (cardView && cardView->getParent()) {
                int cardId = pair.first;
                CardModel* cardModel = _gameModel->getCard(cardId);
                
                if (cardModel) {
                    // 排除底牌堆的卡牌，因为底牌堆的卡牌不应该被移动
                    const auto& bottomCardIds = _gameModel->getBottomCardIds();
                    if (std::find(bottomCardIds.begin(), bottomCardIds.end(), cardId) != bottomCardIds.end()) {
                        CCLOG("Skipping bottom pile card %d (should not be moved)", cardId);
                        continue;
                    }
                    
                    // 检查卡牌是否被回退（位置是否发生了变化）
                    Vec2 modelPos = cardModel->getPosition();
                    Vec2 viewPos = cardView->getPosition();
                    
                    // 应用位置调整逻辑
                    if (cardModel->isInPlayfield()) {
                        modelPos.y += 500.0f; // 游戏区域卡牌位置调整
                    } else if (modelPos == Vec2::ZERO) {
                        // 手牌区卡牌默认布局
                        int stackIndex = cardId - 1006;
                        float baseX = 200.0f + stackIndex * 120.0f;
                        float baseY = 300.0f;
                        modelPos = Vec2(baseX, baseY);
                    }
                    
                    // 只有当视图位置与模型位置不同时才更新
                    if (viewPos != modelPos) {
                        CCLOG("Card %d position needs update: view(%.1f,%.1f) -> model(%.1f,%.1f)", 
                              cardId, viewPos.x, viewPos.y, modelPos.x, modelPos.y);
                        
                        // 为备用牌堆卡牌添加回退动画
                        if (!cardModel->isInPlayfield()) {
                            // 备用牌堆卡牌：从底牌堆位置回到备用牌堆位置
                            CCLOG("Playing stack card undo animation for card %d", cardId);
                            
                            // 设置移动中的卡牌为最高层级
                            cardView->setZOrder(GameUtils::MOVING_CARD_ZORDER);
                            CCLOG("Card %d set to moving zOrder=%d for stack undo animation", cardId, GameUtils::MOVING_CARD_ZORDER);
                            
                            // 播放从底牌堆位置到备用牌堆位置的动画
                            auto moveAction = MoveTo::create(0.5f, modelPos);
                            auto easeAction = EaseInOut::create(moveAction, 2.0f);
                            
                            // 添加缩放效果，让回退更明显
                            auto scaleUp = ScaleTo::create(0.25f, 1.1f);
                            auto scaleDown = ScaleTo::create(0.25f, 1.0f);
                            auto scaleSequence = Sequence::create(scaleUp, scaleDown, nullptr);
                            
                            // 同时执行移动和缩放
                            auto spawnAction = Spawn::create(easeAction, scaleSequence, nullptr);
                            
                            // 动画完成后设置最终位置和正确的层级
                            auto finalizeAction = CallFunc::create([cardView, modelPos, cardId, this]() {
                                cardView->setPosition(modelPos);
                                
                                // 根据卡牌类型设置正确的层级
                                int correctZOrder = this->calculateCorrectZOrder(cardId);
                                cardView->setZOrder(correctZOrder);
                                CCLOG("Stack card %d undo animation completed with zOrder=%d", cardId, correctZOrder);
                                
                                // 检查是否所有动画都完成了
                                this->checkAndCompleteUndo();
                            });
                            
                            auto completeAction = Sequence::create(spawnAction, finalizeAction, nullptr);
                            cardView->runAction(completeAction);
                            
                        } else {
                            // 主牌堆卡牌：从底牌堆位置回到主牌堆位置
                            CCLOG("Playing playfield card undo animation for card %d", cardId);
                            
                            // 设置移动中的卡牌为最高层级
                            cardView->setZOrder(GameUtils::MOVING_CARD_ZORDER);
                            CCLOG("Card %d set to moving zOrder=%d for playfield undo animation", cardId, GameUtils::MOVING_CARD_ZORDER);
                            
                            // 播放从底牌堆位置到主牌堆位置的动画
                            auto moveAction = MoveTo::create(0.5f, modelPos);
                            auto easeAction = EaseInOut::create(moveAction, 2.0f);
                            
                            // 添加缩放效果，让回退更明显
                            auto scaleUp = ScaleTo::create(0.25f, 1.1f);
                            auto scaleDown = ScaleTo::create(0.25f, 1.0f);
                            auto scaleSequence = Sequence::create(scaleUp, scaleDown, nullptr);
                            
                            // 同时执行移动和缩放
                            auto spawnAction = Spawn::create(easeAction, scaleSequence, nullptr);
                            
                            // 动画完成后设置最终位置和正确的层级
                            auto finalizeAction = CallFunc::create([cardView, modelPos, cardId, this]() {
                                cardView->setPosition(modelPos);
                                
                                // 根据卡牌类型设置正确的层级
                                int correctZOrder = this->calculateCorrectZOrder(cardId);
                                cardView->setZOrder(correctZOrder);
                                CCLOG("Playfield card %d undo animation completed with zOrder=%d", cardId, correctZOrder);
                                
                                // 检查是否所有动画都完成了
                                this->checkAndCompleteUndo();
                            });
                            
                            auto completeAction = Sequence::create(spawnAction, finalizeAction, nullptr);
                            cardView->runAction(completeAction);
                        }
                        
                        updatedCount++;
                    }
                }
            }
        }
    }
    
    CCLOG("Step 2: Updated %d cards with undo animations", updatedCount);
    
    // 更新所有卡牌视图和顶部牌显示
    _gameView->updateAllCardViews();
    _gameView->updateTopCardDisplay();
    
    CCLOG("Step 3: Updated all card views and top card display");
    
    
    CCLOG("=== UNDO ANIMATION DEBUG END ===");
}

int CardController::calculateCorrectZOrder(int cardId) {
    return GameUtils::calculateCorrectZOrder(cardId, _gameModel);
}

void CardController::setAnimationPlaying(bool playing) {
    _isAnimationPlaying = playing;
}

void CardController::checkAndCompleteUndo() {
    // 检查是否还有动画正在播放
    if (_gameView) {
        bool hasRunningAnimations = false;
        for (auto& pair : _gameView->getCardViews()) {
            if (pair.second && pair.second->getNumberOfRunningActions() > 0) {
                hasRunningAnimations = true;
                break;
            }
        }
        
        if (!hasRunningAnimations) {
            // 所有动画都完成了，恢复所有卡牌的正确层级关系
            if (_gameView && _gameModel) {
                // 遍历所有卡牌视图，恢复正确的层级关系
                for (auto& pair : _gameView->getCardViews()) {
                    if (pair.second) {
                        CardView* cardView = pair.second.get();
                        if (cardView && cardView->getParent()) {
                            int cardId = pair.first;
                            
                            // 计算正确的层级
                            int correctZOrder = calculateCorrectZOrder(cardId);
                            
                            // 设置正确的层级
                            cardView->setZOrder(correctZOrder);
                            
                            CCLOG("CardController restored card %d zOrder to %d", cardId, correctZOrder);
                        }
                    }
                }
            }
            
            // 重置动画状态
            setAnimationPlaying(false);
            CCLOG("All undo animations completed, restored all card z-orders");
        }
    }
}

bool CardController::isAnimationPlaying() const {
    return _isAnimationPlaying;
}
