#include "GameView.h"
#include "CardView.h"
#include "../models/GameModel.h"
#include "../utils/GameUtils.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

// 包含GameController.h，确保方法可见
#include "../controllers/GameController.h"

USING_NS_CC;


bool GameView::init() {
    // 使用Node::init()进行初始化
    if (!Node::init()) return false;
    
    // 设置节点的基本属性
    // 使用左下角锚点，这样位置计算更直观
    this->setAnchorPoint(Vec2(0, 0));
    
    setupUI();
    return true;
}

void GameView::initializeWithModel(const GameModel& model) {
    // 清理现有视图
    for (auto& kv : _cardViews) {
        if (kv.second) {
            kv.second->removeFromParent();
        }
    }
    _cardViews.clear();

    // 创建所有卡牌视图
    createCardViews(model.getPlayfieldCardIds(), model);
    createCardViews(model.getStackCardIds(), model);
    createCardViews(model.getBottomCardIds(), model);
    
    // 构建依赖图
    if (_controller && _controller->getModel()) {
        std::unordered_map<int, CardView*> cardViewPtrs;
        for (const auto& pair : _cardViews) {
            cardViewPtrs[pair.first] = pair.second.get();
        }
        _controller->getModel()->buildDependencyGraphWithViews(cardViewPtrs);
    }
    
    updateTopCardDisplay();
}

void GameView::createCardViews(const std::vector<int>& cardIds, const GameModel& model) {
    for (int cardId : cardIds) {
        const CardModel* cm = model.getCard(cardId);
        if (cm) {
            createCardView(*cm);
        }
    }
}

void GameView::setupUI() {
    // 设置游戏区域大小
    this->setContentSize(Size(1080, 2080));

    // 创建游戏区域背景（绿色）
    // 创建游戏区域背景
    auto playfieldBg = LayerColor::create(Color4B(50, 150, 50, 100), 1080, 1500);
    playfieldBg->setPosition(0, 580);
    playfieldBg->setAnchorPoint(Vec2(0, 0)); // 设置锚点
    this->addChild(playfieldBg, -100);
    
    // 创建手牌区背景
    auto stackBg = LayerColor::create(Color4B(50, 50, 150, 100), 1080, 580);
    stackBg->setPosition(0, 0);
    stackBg->setAnchorPoint(Vec2(0, 0)); // 设置锚点
    this->addChild(stackBg, -100);
    
    // 添加区域标签
    auto playfieldLabel = Label::createWithTTF("Playfield", "fonts/arial.ttf", 24);
    playfieldLabel->setPosition(540, 1330);
    playfieldLabel->setColor(Color3B::WHITE);
    this->addChild(playfieldLabel, 1);
    
    auto stackLabel = Label::createWithTTF("Stack", "fonts/arial.ttf", 24);
    stackLabel->setPosition(540, 290);
    stackLabel->setColor(Color3B::WHITE);
    this->addChild(stackLabel, 1);
    
    // 添加回退按钮
    createUndoButton();
}

void GameView::createUndoButton() {
    CCLOG("Creating undo button...");
    
    // 使用更简单的方法：直接创建一个可点击的Node
    auto undoButton = Node::create();
    undoButton->setContentSize(Size(120, 60));
    undoButton->setPosition(Vec2(1000, 300)); // 移动到更右边，避免被卡牌遮挡
    undoButton->setAnchorPoint(Vec2(0.5f, 0.5f));
    
    // 创建按钮背景
    auto buttonBg = LayerColor::create(Color4B(100, 100, 200, 255), 120, 60);
    buttonBg->setPosition(-60, -30); // 相对于按钮中心
    buttonBg->setAnchorPoint(Vec2(0, 0));
    undoButton->addChild(buttonBg, -1);
    
    // 创建按钮文字
    auto buttonLabel = Label::createWithTTF("Undo", "fonts/arial.ttf", 20);
    buttonLabel->setPosition(0, 0); // 按钮中心
    buttonLabel->setColor(Color3B::WHITE);
    undoButton->addChild(buttonLabel, 1);
    
    // 设置触摸事件监听器
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    
    touchListener->onTouchBegan = [this, undoButton, buttonBg, buttonLabel](Touch* touch, Event* event) -> bool {
        // 检查时间锁
        if (_controller && !_controller->canUndo()) {
            CCLOG("Undo button touch ignored - cooldown time not reached");
            return false; // 不消费触摸事件
        }
        
        // 检查触摸点是否在按钮范围内
        Vec2 touchLocation = touch->getLocation();
        Vec2 buttonPos = undoButton->getPosition();
        Rect buttonRect = Rect(buttonPos.x - 60, buttonPos.y - 30, 120, 60);
        
        if (buttonRect.containsPoint(touchLocation)) {
            CCLOG("Undo button touched at (%.1f, %.1f)", touchLocation.x, touchLocation.y);
            
            // 按压效果
            undoButton->runAction(ScaleTo::create(0.1f, 0.9f));
            buttonBg->setColor(Color3B(80, 80, 160));
            buttonLabel->setColor(Color3B(200, 200, 200));
            
            return true; // 消费触摸事件
        }
        return false;
    };
    
    touchListener->onTouchEnded = [this, undoButton, buttonBg, buttonLabel](Touch* touch, Event* event) {
        CCLOG("Undo button touch ended");
        
        // 检查时间锁
        if (_controller && !_controller->canUndo()) {
            CCLOG("Undo button click ignored - cooldown time not reached");
            // 恢复效果
            undoButton->runAction(ScaleTo::create(0.1f, 1.0f));
            buttonBg->setColor(Color3B(100, 100, 200));
            buttonLabel->setColor(Color3B::WHITE);
            return;
        }
        
        // 恢复效果
        undoButton->runAction(ScaleTo::create(0.1f, 1.0f));
        buttonBg->setColor(Color3B(100, 100, 200));
        buttonLabel->setColor(Color3B::WHITE);
        
        // 执行回退（控制器内部会检查是否有可回退的操作）
        if (_controller) {
            CCLOG("Calling controller handleUndo");
            _controller->handleUndo();
        } else {
            CCLOGERROR("Controller is null!");
        }
    };
    
    touchListener->onTouchCancelled = [this, undoButton, buttonBg, buttonLabel](Touch* touch, Event* event) {
        CCLOG("Undo button touch cancelled");
        
        // 恢复效果
        undoButton->runAction(ScaleTo::create(0.1f, 1.0f));
        buttonBg->setColor(Color3B(100, 100, 200));
        buttonLabel->setColor(Color3B::WHITE);
    };
    
    // 注册触摸监听器
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, undoButton);
    
    // 添加到场景
    this->addChild(undoButton, 100);
    
    CCLOG("Undo button created successfully at position (1000, 300)");
    CCLOG("Button size: 120x60, Controller status: %s", _controller ? "SET" : "NULL");
}

void GameView::updateAllCardViews() {
    CCLOG("Updating all card views for undo");
    
    if (!_controller || !_controller->getModel()) {
        CCLOGERROR("Controller or model is null, cannot update card views");
        return;
    }
    
    GameModel* model = _controller->getModel();
    
    // 更新所有卡牌视图的位置和状态
    for (auto& pair : _cardViews) {
        if (pair.second) {
            CardView* cardView = pair.second.get();
            if (cardView && cardView->getParent()) {
                int cardId = pair.first;
                CardModel* cardModel = model->getCard(cardId);
                
                if (cardModel) {
                    // 应用与createCardView相同的位置调整逻辑
                    Vec2 pos = cardModel->getPosition();
                    
                    if (cardModel->isInPlayfield()) {
                        // 游戏区域卡牌：将y坐标向上调整，让卡牌位置更高
                        pos.y += 500.0f; // 将原始y坐标向上移动500像素
                    } else {
                        // 手牌区卡牌：如果位置为0，使用默认布局
                        if (pos == Vec2::ZERO) {
                            // 计算手牌区卡牌的索引（从0开始）
                            int stackIndex = cardId - 1006; // 假设手牌区卡牌ID从1006开始
                            float baseX = 200.0f + stackIndex * 120.0f; // 从x=200开始，间隔120像素
                            float baseY = 300.0f;
                            pos = Vec2(baseX, baseY);
                        }
                    }
                    
                    // 同步卡牌位置
                    cardView->setPosition(pos);
                    
                    // 同步卡牌层级：根据JSON顺序设置层级
                    int zOrder;
                    if (cardModel->isInPlayfield()) {
                        int jsonOrder = getCardJsonOrder(cardId);
                        zOrder = 2000 + jsonOrder; // JSON顺序越靠后，zOrder越高
                    } else {
                        zOrder = 1000; // 手牌区卡牌正常层级
                    }
                    cardView->setZOrder(zOrder);
                    
                    // 更新卡牌显示
                    cardView->updateDisplay();
                    
                    CCLOG("Updated card view %d: modelPos(%.1f, %.1f) -> viewPos(%.1f, %.1f), zOrder(%d)", 
                          cardId, cardModel->getPosition().x, cardModel->getPosition().y, pos.x, pos.y, zOrder);
                }
            }
        }
    }
    
    CCLOG("All card views updated");
}

CardView* GameView::getCardView(int cardId) {
    auto it = _cardViews.find(cardId);
    if (it != _cardViews.end() && it->second) {
        return it->second.get();
    }
    return nullptr;
}

void GameView::updateCardView(int cardId) {
    // 更新指定卡牌的显示
}

void GameView::createCardView(const CardModel& cardModel) {
    int cardId = cardModel.getCardId();
    
    // 检查是否已存在该卡牌的视图
    if (_cardViews.count(cardId)) {
        return;
    }
    
    auto cardView = CardView::create();
    if (!cardView) return;

    // 设置卡牌模型和更新显示
    cardView->setCardModel(&cardModel);
    cardView->setCardId(cardModel.getCardId());
    cardView->updateDisplay();

    // 位置：根据卡牌类型调整位置
    Vec2 pos = cardModel.getPosition();
    
    if (cardModel.isInPlayfield()) {
        // 游戏区域卡牌：将y坐标向上调整，让卡牌位置更高
        pos.y += 500.0f; // 将原始y坐标向上移动500像素
    } else {
        // 手牌区卡牌：如果位置为0，使用默认布局
        if (pos == Vec2::ZERO) {
            // 计算手牌区卡牌的索引（从0开始）
            int stackIndex = cardModel.getCardId() - 1006; // 假设手牌区卡牌ID从1006开始
            float baseX = 200.0f + stackIndex * 120.0f; // 从x=200开始，间隔120像素
            float baseY = 300.0f;
            pos = Vec2(baseX, baseY);
        }
    }
    
    cardView->setPosition(pos);

    // 设置点击回调
    if (_controller) {
        CCLOG("Setting click callback for card %d", cardModel.getCardId());
        cardView->setClickCallback([this](int id) {
            CCLOG("CardView click callback triggered for card %d", id);
            if (_controller) {
                _controller->handleCardClick(id);
            } else {
                CCLOGERROR("Controller is null in click callback!");
            }
        });
    } else {
        CCLOGERROR("Controller is null, cannot set click callback for card %d", cardModel.getCardId());
    }

    // 添加到场景
    this->addChild(cardView);
    
    // 使用工具类设置正确的层级顺序
    int zOrder = GameUtils::calculateCorrectZOrder(cardModel.getCardId(), _controller->getModel());
    cardView->setZOrder(zOrder);
    CCLOG("Card %d: zOrder=%d", cardModel.getCardId(), zOrder);
    
    _cardViews[cardModel.getCardId()] = cardView;
}

int GameView::getCardJsonOrder(int cardId) {
    return GameUtils::getCardJsonOrder(cardId, _controller->getModel());
}

void GameView::playCardMoveAnimation(int cardId, const Vec2& targetPosition) {
    auto cardView = getCardView(cardId);
    if (cardView) {
        cardView->playMoveToAnimation(targetPosition);
    }
}

void GameView::playMatchAnimation(int cardId) {
    auto cardView = getCardView(cardId);
    if (cardView) {
        cardView->playMatchAnimation();
    }
}

void GameView::playCardMoveToTopAnimation(int cardId, const cocos2d::Vec2& originalPosition) {
    CCLOG("Playing card move to top animation for card: %d", cardId);
    
    auto cardView = getCardView(cardId);
    if (!cardView) {
        CCLOGERROR("CardView not found for card: %d", cardId);
        return;
    }
    
    // 设置移动中的卡牌为最高层级
    cardView->setZOrder(GameUtils::MOVING_CARD_ZORDER);
    CCLOG("Card %d set to moving zOrder=%d", cardId, GameUtils::MOVING_CARD_ZORDER);
    
    // 设置目标位置（手牌区顶部）
    Vec2 targetPosition = Vec2(GameUtils::TOP_CARD_X, GameUtils::TOP_CARD_Y);
    
    // 创建移动动画
    auto moveAction = MoveTo::create(GameUtils::ANIMATION_DURATION, targetPosition);
    auto easeAction = EaseInOut::create(moveAction, 2.0f);
    
    // 创建缩放动画（移动到顶部时稍微放大）
    auto scaleAction = ScaleTo::create(GameUtils::ANIMATION_DURATION, GameUtils::CARD_SCALE_FACTOR);
    
    // 组合移动和缩放动画
    auto spawnAction = Spawn::create(easeAction, scaleAction, nullptr);
    
    // 动画完成回调
    auto callback = CallFunc::create([this, cardId]() {
        CCLOG("Card %d move to top animation completed", cardId);
        
        // 动画完成后设置正确的层级
        if (_controller && _controller->getModel()) {
            int correctZOrder = GameUtils::calculateCorrectZOrder(cardId, _controller->getModel());
            auto cardView = this->getCardView(cardId);
            if (cardView) {
                cardView->setZOrder(correctZOrder);
                CCLOG("Card %d animation completed with zOrder=%d", cardId, correctZOrder);
            }
        }
        
        // 更新顶部牌显示
        this->updateTopCardDisplay();
        // 重置动画状态 - 通过GameController重置CardController的状态
        if (_controller) {
            _controller->setAnimationPlaying(false);
            CCLOG("Animation state reset to false");
        }
    });
    
    auto sequence = Sequence::create(spawnAction, callback, nullptr);
    cardView->runAction(sequence);
    
    CCLOG("Card %d move animation started from (%.1f, %.1f) to (%.1f, %.1f)", 
          cardId, originalPosition.x, originalPosition.y, targetPosition.x, targetPosition.y);
}


void GameView::updateTopCardDisplay() {
    if (!_controller) {
        CCLOGERROR("Controller is null in updateTopCardDisplay");
        return;
    }
    
    if (_cardViews.empty()) {
        return;
    }
    
    // 获取当前顶部牌ID
    int topCardId = -1;
    auto gameModel = _controller->getModel();
    if (gameModel) {
        auto topCard = gameModel->getTopCard();
        if (topCard) {
            topCardId = topCard->getCardId();
        }
    }
    
    if (topCardId <= 0) {
        return;
    }
    
    // 创建容器的副本以避免并发访问问题
    auto cardViewsCopy = _cardViews;
        
        // 设置顶部牌
        auto it = cardViewsCopy.find(topCardId);
        if (it != cardViewsCopy.end() && it->second) {
            CardView* topCardView = it->second.get();
            
            if (topCardView && topCardView->getParent()) {
                Vec2 pos = Vec2(GameUtils::TOP_CARD_X, GameUtils::TOP_CARD_Y);
                topCardView->setPosition(pos);
                topCardView->setScale(1.0f);
                
                // 顶部牌应该使用底牌堆的最高层级
                int zOrder = GameUtils::calculateCorrectZOrder(topCardId, _controller->getModel());
                topCardView->setZOrder(zOrder);
            }
        }
}
