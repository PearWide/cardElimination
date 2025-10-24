#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include "Classes/controllers/GameController.h"

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    CREATE_FUNC(GameScene);

    virtual bool init() override;
    virtual void onEnter() override;
    virtual void onExit() override;

private:
    void setupMVCArchitecture();
    void connectMVCComponents();

    std::unique_ptr<GameController> _gameController;
};

#endif // __GAME_SCENE_H__