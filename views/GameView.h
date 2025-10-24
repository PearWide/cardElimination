#ifndef __GAME_VIEW_H__
#define __GAME_VIEW_H__

#include "cocos2d.h"
#include "CardView.h"

// 前向声明
class GameModel;
class CardModel;

// 包含完整的GameController头文件，因为需要调用其方法
#include "../controllers/GameController.h"

class GameView : public cocos2d::Node {
public:
    CREATE_FUNC(GameView);

    virtual bool init() override;

    void initializeWithModel(const GameModel& model);
    void setController(GameController* controller) { _controller = controller; }

    CardView* getCardView(int cardId);
    void updateCardView(int cardId);
    
    // 动画功能
    void playCardMoveAnimation(int cardId, const cocos2d::Vec2& targetPosition);
    void playMatchAnimation(int cardId);
    
    // 卡牌移动到顶部动画
    void playCardMoveToTopAnimation(int cardId, const cocos2d::Vec2& originalPosition);
    
    // 顶部牌显示
    void updateTopCardDisplay();
    
    // 测试点击功能
    void testClickFunctionality();
    
    // 调试方法
    void debugContainerState();
    
    // 回退功能相关方法
    void updateAllCardViews();
    const std::unordered_map<int, cocos2d::RefPtr<CardView>>& getCardViews() const { return _cardViews; }

private:
    void setupUI();
    void createCardView(const CardModel& cardModel);
    void createCardViews(const std::vector<int>& cardIds, const GameModel& model);
    void createUndoButton();
    int getCardJsonOrder(int cardId); // 获取卡牌在JSON中的顺序

    std::unordered_map<int, cocos2d::RefPtr<CardView>> _cardViews;
    GameController* _controller;
};

#endif // __GAME_VIEW_H__