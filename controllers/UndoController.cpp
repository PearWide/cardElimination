#include "UndoController.h"
#include "../models/GameModel.h"
#include "../views/GameView.h"
#include "../managers/UndoManager.h"
#include "../utils/GameUtils.h"
#include "cocos2d.h"

USING_NS_CC;

UndoController::UndoController()
    : _gameModel(nullptr)
    , _gameView(nullptr)
    , _undoManager(nullptr)
    , _isAnimationPlaying(false) {
}

UndoController::~UndoController() {
}

void UndoController::init(GameModel* gameModel, GameView* gameView, UndoManager* undoManager) {
    _gameModel = gameModel;
    _gameView = gameView;
    _undoManager = undoManager;
}

bool UndoController::executeUndo() {
    if (!_gameModel || !_gameView || !_undoManager) {
        CCLOGERROR("UndoController not properly initialized");
        return false;
    }
    
    if (!_undoManager->canUndo()) {
        return false;
    }
    
    // 获取被回退影响的卡牌ID列表（在回退前记录）
    std::vector<int> affectedCardIds = getAffectedCardIds();
    
    // 执行回退操作
    bool success = _undoManager->undo();
    if (!success) {
        CCLOGERROR("Undo operation failed");
        return false;
    }
    
    // 设置动画状态
    setAnimationPlaying(true);
    
    // 只播放被影响卡牌的回退动画
    playUndoAnimations(affectedCardIds);
    
    // 更新被影响的卡牌视图
    updateAffectedCardViews(affectedCardIds);
    
    // 注意：restoreAllCardZOrders 应该在动画完成后调用，而不是在动画开始前
    // 这里先不调用，让动画自己处理层级设置
    
    // 更新顶部牌显示
    _gameView->updateTopCardDisplay();
    
    return true;
}

bool UndoController::canUndo() const {
    if (!_undoManager) {
        return false;
    }
    return _undoManager->canUndo();
}

void UndoController::playUndoAnimations(const std::vector<int>& affectedCardIds) {
    CCLOG("UndoController::playUndoAnimations - Playing animations for %d affected cards", (int)affectedCardIds.size());
    
    if (!_gameView || !_gameModel) {
        CCLOGERROR("GameView or GameModel is null, cannot play undo animations");
        return;
    }
    
    int animatedCount = 0;
    
    // 只对被影响的卡牌播放动画
    for (int cardId : affectedCardIds) {
        CardView* cardView = _gameView->getCardView(cardId);
        if (!cardView) {
            CCLOG("CardView not found for card %d, skipping animation", cardId);
            continue;
        }
        
        CardModel* cardModel = _gameModel->getCard(cardId);
        if (!cardModel) {
            CCLOG("CardModel not found for card %d, skipping animation", cardId);
            continue;
        }
        
        // 额外检查：确保不是底牌堆的卡牌
        const auto& bottomCardIds = _gameModel->getBottomCardIds();
        if (std::find(bottomCardIds.begin(), bottomCardIds.end(), cardId) != bottomCardIds.end()) {
            CCLOG("Skipping bottom pile card %d (should not be animated)", cardId);
            continue;
        }
        
        // 使用工具类计算目标位置
        Vec2 modelPos = GameUtils::calculateTargetPosition(cardModel, cardId);
        Vec2 viewPos = cardView->getPosition();
        
        // 只有当视图位置与模型位置不同时才播放动画
        if (viewPos != modelPos) {
            CCLOG("Card %d needs animation: view(%.1f,%.1f) -> model(%.1f,%.1f)", 
                  cardId, viewPos.x, viewPos.y, modelPos.x, modelPos.y);
            
            // 设置移动中的卡牌为最高层级
            cardView->setZOrder(GameUtils::MOVING_CARD_ZORDER);
            CCLOG("Card %d set to moving zOrder=%d for undo animation", cardId, GameUtils::MOVING_CARD_ZORDER);
            
            // 创建移动动画
            auto moveAction = MoveTo::create(GameUtils::ANIMATION_DURATION, modelPos);
            auto easeAction = EaseInOut::create(moveAction, 2.0f);
            
            // 创建缩放动画
            auto scaleUp = ScaleTo::create(GameUtils::SCALE_ANIMATION_DURATION, GameUtils::CARD_SCALE_FACTOR);
            auto scaleDown = ScaleTo::create(GameUtils::SCALE_ANIMATION_DURATION, 1.0f);
            auto scaleSequence = Sequence::create(scaleUp, scaleDown, nullptr);
            
            // 组合移动和缩放动画
            auto spawnAction = Spawn::create(easeAction, scaleSequence, nullptr);
            
            // 动画完成后设置最终位置和正确的层级
            auto finalizeAction = CallFunc::create([cardView, modelPos, cardId, this]() {
                cardView->setPosition(modelPos);
                
                // 使用工具类计算正确的层级
                int correctZOrder = GameUtils::calculateCorrectZOrder(cardId, this->_gameModel);
                cardView->setZOrder(correctZOrder);
                CCLOG("Card %d undo animation completed with zOrder=%d", cardId, correctZOrder);
                
                // 检查是否所有动画都完成了
                this->checkAndCompleteUndo();
            });
            
            auto completeAction = Sequence::create(spawnAction, finalizeAction, nullptr);
            cardView->runAction(completeAction);
            
            animatedCount++;
        }
    }
    
    CCLOG("UndoController::playUndoAnimations - Animated %d cards", animatedCount);
}

std::vector<int> UndoController::getAffectedCardIds() {
    std::vector<int> affectedCardIds;
    
    if (!_gameModel || !_gameView) {
        return affectedCardIds;
    }
    
    // 只获取备用牌堆和主牌堆的卡牌ID
    // 底牌堆的卡牌不应该参与回退动画，因为它们是固定的
    
    // 获取当前备用牌堆的卡牌ID（这些卡牌可能会被回退）
    const auto& stackCardIds = _gameModel->getStackCardIds();
    for (int cardId : stackCardIds) {
        affectedCardIds.push_back(cardId);
    }
    
    // 获取主牌堆的卡牌ID（这些卡牌可能会被恢复）
    const auto& playfieldCardIds = _gameModel->getPlayfieldCardIds();
    for (int cardId : playfieldCardIds) {
        affectedCardIds.push_back(cardId);
    }
    
    CCLOG("UndoController::getAffectedCardIds - Found %d affected cards (excluding bottom pile)", (int)affectedCardIds.size());
    
    return affectedCardIds;
}

void UndoController::updateAffectedCardViews(const std::vector<int>& affectedCardIds) {
    CCLOG("UndoController::updateAffectedCardViews - Updating %d affected card views", (int)affectedCardIds.size());
    
    if (!_gameView) {
        CCLOGERROR("GameView is null, cannot update card views");
        return;
    }
    
    // 只更新被影响的卡牌视图
    for (int cardId : affectedCardIds) {
        CardView* cardView = _gameView->getCardView(cardId);
        if (cardView) {
            // 额外检查：确保不是底牌堆的卡牌
            const auto& bottomCardIds = _gameModel->getBottomCardIds();
            if (std::find(bottomCardIds.begin(), bottomCardIds.end(), cardId) != bottomCardIds.end()) {
                CCLOG("Skipping bottom pile card %d (should not be updated)", cardId);
                continue;
            }
            
            cardView->updateDisplay();
            CCLOG("Updated card view for card %d", cardId);
        }
    }
    
    CCLOG("UndoController::updateAffectedCardViews - Updated %d card views", (int)affectedCardIds.size());
}

int UndoController::calculateCorrectZOrder(int cardId) {
    return GameUtils::calculateCorrectZOrder(cardId, _gameModel);
}

void UndoController::restoreAllCardZOrders() {
    CCLOG("UndoController::restoreAllCardZOrders - Restoring all card z-orders");
    
    if (!_gameView || !_gameModel) {
        CCLOGERROR("GameView or GameModel is null, cannot restore z-orders");
        return;
    }
    
    int restoredCount = 0;
    
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
                
                CCLOG("Restored card %d zOrder to %d", cardId, correctZOrder);
                restoredCount++;
            }
        }
    }
    
    CCLOG("UndoController::restoreAllCardZOrders - Restored %d card z-orders", restoredCount);
}

void UndoController::setAnimationPlaying(bool playing) {
    _isAnimationPlaying = playing;
}

bool UndoController::isAnimationPlaying() const {
    return _isAnimationPlaying;
}

void UndoController::checkAndCompleteUndo() {
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
            restoreAllCardZOrders();
            
            // 重置动画状态
            setAnimationPlaying(false);
            CCLOG("All undo animations completed, restored all card z-orders");
        }
    }
}
