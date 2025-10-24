#include "UndoManager.h"
#include "../models/GameModel.h"
#include "../models/CardModel.h"

USING_NS_CC;

UndoManager::UndoManager()
    : _gameModel(nullptr) {
    CCLOG("UndoManager created");
}

UndoManager::~UndoManager() {
    CCLOG("UndoManager destroyed");
}

void UndoManager::init(GameModel* gameModel) {
    _gameModel = gameModel;
    _undoModel.clear(); // 清空之前的回退记录
    CCLOG("UndoManager initialized with GameModel");
}

void UndoManager::recordCardMatch(int playfieldCardId, int trayCardId) {
    if (!_gameModel) {
        CCLOGERROR("UndoManager not initialized");
        return;
    }

    UndoStep step;
    step.actionType = UndoActionType::CARD_MATCH;
    step.cardId1 = playfieldCardId;
    step.cardId2 = trayCardId;

    // ��¼����״̬
    // 记录卡牌状态和原始位置
    CardModel* card1 = _gameModel->getCard(playfieldCardId);
    CardModel* card2 = _gameModel->getCard(trayCardId);
    
    if (card1) {
        step.originalPos1 = card1->getPosition();
        step.wasCovered1 = card1->isCovered();
        step.wasInPlayfield1 = card1->isInPlayfield();
        step.zOrder1 = card1->getZOrder();
    }
    
    if (card2) {
        step.originalPos2 = card2->getPosition();
        step.wasCovered2 = card2->isCovered();
        step.wasInPlayfield2 = card2->isInPlayfield();
        step.zOrder2 = card2->getZOrder();
    }

    CCLOG("Recording card match: %d -> %d", playfieldCardId, trayCardId);
    CCLOG("Card1 original pos: (%.1f, %.1f)", step.originalPos1.x, step.originalPos1.y);
    CCLOG("Card2 original pos: (%.1f, %.1f)", step.originalPos2.x, step.originalPos2.y);

    _undoModel.addStep(step);
    CCLOG("Recorded card match: %d -> %d", playfieldCardId, trayCardId);
}

void UndoManager::recordStackDraw(int drawnCardId, int previousTrayCardId) {
    if (!_gameModel) {
        CCLOGERROR("UndoManager not initialized");
        return;
    }

    UndoStep step;
    step.actionType = UndoActionType::STACK_DRAW;
    step.cardId1 = drawnCardId;
    step.cardId2 = previousTrayCardId;

    // 记录卡牌状态和原始位置
    CardModel* card1 = _gameModel->getCard(drawnCardId);
    CardModel* card2 = _gameModel->getCard(previousTrayCardId);
    
    if (card1) {
        step.originalPos1 = card1->getPosition();
        step.wasCovered1 = card1->isCovered();
        step.wasInPlayfield1 = card1->isInPlayfield();
        step.zOrder1 = card1->getZOrder();
    }
    
    if (card2) {
        step.originalPos2 = card2->getPosition();
        step.wasCovered2 = card2->isCovered();
        step.wasInPlayfield2 = card2->isInPlayfield();
        step.zOrder2 = card2->getZOrder();
    }

    CCLOG("Recording stack draw: %d -> %d", drawnCardId, previousTrayCardId);
    CCLOG("Card1 original pos: (%.1f, %.1f)", step.originalPos1.x, step.originalPos1.y);
    CCLOG("Card2 original pos: (%.1f, %.1f)", step.originalPos2.x, step.originalPos2.y);

    _undoModel.addStep(step);
    CCLOG("Recorded stack draw");
}

bool UndoManager::undo() {
    if (!canUndo()) {
        CCLOG("No steps to undo");
        return false;
    }

    if (!_gameModel) {
        CCLOGERROR("UndoManager not initialized");
        return false;
    }

    UndoStep step = _undoModel.popStep();
    CCLOG("Undoing step");

    bool success = false;
    switch (step.actionType) {
    case UndoActionType::CARD_MATCH:
        success = restoreCardMatch(step);
        break;
    case UndoActionType::STACK_DRAW:
        success = restoreStackDraw(step);
        break;
    }

    return success;
}

bool UndoManager::canUndo() const {
    return _undoModel.canUndo();
}

void UndoManager::clear() {
    _undoModel.clear();
    CCLOG("UndoManager cleared");
}

size_t UndoManager::getStepCount() const {
    return _undoModel.getStepCount();
}

void UndoManager::setMaxSteps(size_t maxSteps) {
    _undoModel.setMaxSteps(maxSteps);
}

bool UndoManager::restoreCardMatch(const UndoStep& step) {
    CCLOG("Restoring card match: %d -> %d", step.cardId1, step.cardId2);
    
    // 恢复主牌堆卡牌
    CardModel* playfieldCard = _gameModel->getCard(step.cardId1);
    if (playfieldCard) {
        // 恢复卡牌到主牌堆
        _gameModel->restoreCardToPlayfield(step.cardId1);
        playfieldCard->setPosition(step.originalPos1);
        playfieldCard->setCovered(step.wasCovered1);
        playfieldCard->setIsInPlayfield(step.wasInPlayfield1);
        playfieldCard->setZOrder(step.zOrder1);
        CCLOG("Restored playfield card %d to position (%.1f, %.1f)", 
              step.cardId1, step.originalPos1.x, step.originalPos1.y);
    }

    // 恢复底牌堆卡牌
    CardModel* bottomCard = _gameModel->getCard(step.cardId2);
    if (bottomCard) {
        // 从底牌堆移除当前卡牌（被匹配的卡牌）
        if (!_gameModel->isBottomPileEmpty()) {
            int currentBottomTop = _gameModel->getBottomPileTop();
            if (currentBottomTop == step.cardId1) {
                _gameModel->popFromBottomPile();
                CCLOG("Removed card %d from bottom pile", step.cardId1);
            }
        }
        
        // 恢复之前的底牌堆顶部卡牌
        _gameModel->setTopCard(step.cardId2);
        bottomCard->setPosition(step.originalPos2);
        bottomCard->setCovered(step.wasCovered2);
        bottomCard->setIsInPlayfield(step.wasInPlayfield2);
        bottomCard->setZOrder(step.zOrder2);
        CCLOG("Restored bottom card %d to position (%.1f, %.1f)", 
              step.cardId2, step.originalPos2.x, step.originalPos2.y);
    }

    return (playfieldCard != nullptr && bottomCard != nullptr);
}

bool UndoManager::restoreStackDraw(const UndoStep& step) {
    CCLOG("Restoring stack draw: %d -> %d", step.cardId1, step.cardId2);
    
    // 1. 先从底牌堆移除当前卡牌（step.cardId1）
    if (!_gameModel->isBottomPileEmpty()) {
        int currentBottomTop = _gameModel->getBottomPileTop();
        if (currentBottomTop == step.cardId1) {
            _gameModel->popFromBottomPile();
            CCLOG("Removed card %d from bottom pile", step.cardId1);
        } else {
            CCLOGERROR("Bottom pile top (%d) doesn't match expected card (%d)", currentBottomTop, step.cardId1);
        }
    }
    
    // 2. 恢复卡牌到备用牌堆（正确维护数据容器）
    CardModel* stackCard = _gameModel->getCard(step.cardId1);
    if (stackCard) {
        // 恢复卡牌到备用牌堆
        _gameModel->pushToStackPileAndContainer(step.cardId1);
        stackCard->setPosition(step.originalPos1);
        stackCard->setCovered(step.wasCovered1);
        stackCard->setIsInPlayfield(step.wasInPlayfield1);
        stackCard->setZOrder(step.zOrder1);
        CCLOG("Restored stack card %d to position (%.1f, %.1f)", 
              step.cardId1, step.originalPos1.x, step.originalPos1.y);
    }

    // 3. 恢复之前的底牌堆顶部卡牌
    CardModel* bottomCard = _gameModel->getCard(step.cardId2);
    if (bottomCard) {
        _gameModel->setTopCard(step.cardId2);
        bottomCard->setPosition(step.originalPos2);
        bottomCard->setCovered(step.wasCovered2);
        bottomCard->setIsInPlayfield(step.wasInPlayfield2);
        bottomCard->setZOrder(step.zOrder2);
        CCLOG("Restored bottom card %d to position (%.1f, %.1f)", 
              step.cardId2, step.originalPos2.x, step.originalPos2.y);
    }

    return (stackCard != nullptr && bottomCard != nullptr);
}

Vec2 UndoManager::findCardOriginalPosition(int cardId) const {
    if (!_gameModel) {
        return Vec2::ZERO;
    }

    CardModel* card = _gameModel->getCard(cardId);
    return card ? card->getPosition() : Vec2::ZERO;
}