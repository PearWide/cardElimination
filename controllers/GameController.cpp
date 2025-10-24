#include "GameController.h"
#include "CardController.h"
#include "UndoController.h"
#include "../models/GameModel.h"
#include "../models/CardModel.h"
#include "../views/GameView.h"
#include "../managers/UndoManager.h"
#include "../configs/loaders/LevelConfigLoader.h"
#include <cmath>

USING_NS_CC;

// 定义静态常量
const float GameController::UNDO_COOLDOWN_TIME = 0.8f;

GameController::GameController()
    : _gameModel(nullptr)
    , _gameView(nullptr)
    , _cardController(nullptr)
    , _undoController(nullptr)
    , _currentLevelId(0)
    , _isAnimationPlaying(false)
    , _lastUndoTime(0.0f) {
}

GameController::~GameController() {
}

bool GameController::init() {
    _gameModel = std::make_unique<GameModel>();
    if (!_gameModel) {
        return false;
    }

    _undoManager = std::make_unique<UndoManager>();
    if (!_undoManager) {
        return false;
    }

    _undoManager->init(_gameModel.get());

    _gameView = GameView::create();
    if (_gameView) {
        _gameView->setController(this);
    }
    
    // 初始化卡片控制器（在_gameView创建之后）
    _cardController = std::make_unique<CardController>();
    if (!_cardController) {
        return false;
    }
    
    // 初始化回退控制器
    _undoController = std::make_unique<UndoController>();
    if (!_undoController) {
        return false;
    }
    
    setupSubControllers();

    return true;
}

void GameController::startGame(int levelId) {
    _gameModel = std::make_unique<GameModel>();
    if (!_gameModel) return;
    
    _undoManager = std::make_unique<UndoManager>();
    if (!_undoManager) return;
    
    _undoManager->init(_gameModel.get());
    loadLevelFromConfig(levelId);

    if (_gameView && _gameModel) {
        _gameView->initializeWithModel(*_gameModel);
    }

    _gameModel->setGameState(GameModel::GameState::PLAYING);
    
    // 重新初始化CardController
    if (_cardController) {
        _cardController->init(_gameModel.get(), _gameView, _undoManager.get());
    }
    
    if (_undoController) {
        _undoController->init(_gameModel.get(), _gameView, _undoManager.get());
    }
}


void GameController::loadLevelFromConfig(int levelId) {
    if (!_gameModel) return;

    LevelConfig config = LevelConfigLoader::loadLevelConfig(levelId);
    
    int cardId = 1000;
    CCLOG("Loading playfield cards, count: %d", (int)config.playfieldCards.size());
    for (const auto& cardConfig : config.playfieldCards) {
        CardModel card(cardId, 
                      cardConfig.face, 
                      cardConfig.suit,
                      cardConfig.position);
        card.setCovered(cardConfig.isCovered);
        card.setIsInPlayfield(true);
        CCLOG("Adding playfield card: ID=%d, face=%d, suit=%d", cardId, (int)cardConfig.face, (int)cardConfig.suit);
        _gameModel->addCard(card, true);
        cardId++;
    }
    
    CCLOG("Loading stack cards, count: %d", (int)config.stackCards.size());
    for (const auto& cardConfig : config.stackCards) {
        CardModel card(cardId, 
                      cardConfig.face, 
                      cardConfig.suit,
                      cardConfig.position);
        card.setCovered(cardConfig.isCovered);
        card.setIsInPlayfield(false);
        CCLOG("Adding stack card: ID=%d, face=%d, suit=%d", cardId, (int)cardConfig.face, (int)cardConfig.suit);
        _gameModel->addCard(card, false);
        cardId++;
    }
    
    // 初始化备用牌堆和底牌堆
    if (!config.stackCards.empty()) {
        const auto& stackCards = _gameModel->getStackCardIds();
        
        // 将所有备用牌添加到栈中（按正确顺序）
        for (int stackCardId : stackCards) {
            _gameModel->pushToStackPile(stackCardId);
        }
        
        // 自动弹栈：将备用牌堆的第一张牌弹到底牌堆（这是游戏规则，不记录到回退栈）
        if (!_gameModel->isStackPileEmpty()) {
            int firstStackCard = _gameModel->popFromStackPile();
            _gameModel->pushToBottomPile(firstStackCard);
            _gameModel->setTopCard(firstStackCard);
        }
    }
}

void GameController::handleCardClick(int cardId) {
    // 检查是否有动画正在播放
    if (_isAnimationPlaying) {
        CCLOG("Card click ignored - animation is playing");
        return;
    }
    
    if (!_cardController) {
        CCLOGERROR("CardController is null in handleCardClick");
        return;
    }

    // 委托给CardController处理
    _cardController->handleCardClick(cardId);
}


void GameController::handleUndo() {
    // 检查时间锁
    if (!canUndo()) {
        CCLOG("Undo ignored - cooldown time not reached");
        return;
    }
    
    if (!_undoController) {
        CCLOGERROR("UndoController is null");
        return;
    }

    if (!_undoController->canUndo()) {
        // 显示提示信息
        if (_gameView) {
            auto message = Label::createWithTTF("No moves to undo", "fonts/arial.ttf", 16);
            message->setPosition(Vec2(1000, 250));
            message->setColor(Color3B::RED);
            _gameView->addChild(message, 100);
            
            // 2秒后移除提示
            auto removeAction = Sequence::create(
                DelayTime::create(2.0f),
                CallFunc::create([message]() {
                    message->removeFromParent();
                }),
                nullptr
            );
            message->runAction(removeAction);
        }
        return;
    }

    // 设置时间锁
    setUndoCooldown();
    
    // 委托给UndoController执行回退操作
    _undoController->executeUndo();
}


void GameController::setupSubControllers() {
    // 初始化卡片控制器
    if (_cardController && _gameModel && _gameView && _undoManager) {
        _cardController->init(_gameModel.get(), _gameView, _undoManager.get());
    } else {
        CCLOGERROR("Failed to initialize CardController - missing dependencies");
    }
    
    // 初始化回退控制器
    if (_undoController && _gameModel && _gameView && _undoManager) {
        _undoController->init(_gameModel.get(), _gameView, _undoManager.get());
    } else {
        CCLOGERROR("Failed to initialize UndoController - missing dependencies");
    }
}

void GameController::setupViewCallbacks() {
    // 设置视图回调
}

void GameController::setAnimationPlaying(bool playing) {
    _isAnimationPlaying = playing;
    
    // 同时更新CardController的动画状态
    if (_cardController) {
        _cardController->setAnimationPlaying(playing);
    }
    
    // 同时更新UndoController的动画状态
    if (_undoController) {
        _undoController->setAnimationPlaying(playing);
    }
    
    CCLOG("GameController animation state set to: %s", playing ? "true" : "false");
}

bool GameController::canUndo() const {
    float currentTime = Director::getInstance()->getTotalFrames() * Director::getInstance()->getAnimationInterval();
    return (currentTime - _lastUndoTime) >= UNDO_COOLDOWN_TIME;
}

void GameController::setUndoCooldown() {
    _lastUndoTime = Director::getInstance()->getTotalFrames() * Director::getInstance()->getAnimationInterval();
    CCLOG("Undo cooldown set, next undo available in %.1f seconds", UNDO_COOLDOWN_TIME);
}