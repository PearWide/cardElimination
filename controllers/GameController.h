#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include "cocos2d.h"

// 前向声明
class GameView;
class GameModel;
class UndoManager;
class CardController;
class UndoController;

class GameController {
public:
    GameController();
    ~GameController();

    bool init();
    void startGame(int levelId);
    void handleCardClick(int cardId);
    void handleUndo();

    // 获取游戏视图
    GameView* getView() const { return _gameView; }
    
    // 获取游戏模型（用于测试）
    GameModel* getModel() const { return _gameModel.get(); }
    
    // 动画状态管理
    bool isAnimationPlaying() const { return _isAnimationPlaying; }
    void setAnimationPlaying(bool playing);
    
    // 回退按钮时间锁管理
    bool canUndo() const;
    void setUndoCooldown();

private:
    void setupSubControllers();
    void setupViewCallbacks();
    void loadLevelFromConfig(int levelId);

    std::unique_ptr<GameModel> _gameModel;
    GameView* _gameView;
    
    // 卡片控制器 - 处理卡片相关逻辑
    std::unique_ptr<CardController> _cardController;
    
    // 回退控制器 - 处理回退逻辑
    std::unique_ptr<UndoController> _undoController;
    
    // 撤销管理器
    std::unique_ptr<UndoManager> _undoManager;

    int _currentLevelId;
    
    // 动画状态管理
    bool _isAnimationPlaying;
    
    // 回退按钮时间锁
    float _lastUndoTime;
    static const float UNDO_COOLDOWN_TIME;
};

#endif // __GAME_CONTROLLER_H__