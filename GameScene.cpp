#include "GameScene.h"
#include "Classes/views/GameView.h"

USING_NS_CC;

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    setupMVCArchitecture();

    if (_gameController && _gameController->init()) {
        if (_gameController->getView()) {
            _gameController->getView()->setPosition(0, 0);
            this->addChild(_gameController->getView());
        }
        _gameController->startGame(1);
    } else {
        CCLOGERROR("Failed to initialize GameController!");
        auto errorLabel = Label::createWithSystemFont("Initialization failed!", "Arial", 36);
        errorLabel->setPosition(540, 1040);
        this->addChild(errorLabel);
    }

    return true;
}

void GameScene::setupMVCArchitecture() {
    _gameController = std::make_unique<GameController>();
    if (!_gameController) {
        CCLOGERROR("Failed to create GameController!");
        return;
    }
}

void GameScene::connectMVCComponents() {
    // MVC components are connected in GameController
}

void GameScene::onEnter() {
    Scene::onEnter();
}

void GameScene::onExit() {
    Scene::onExit();
}