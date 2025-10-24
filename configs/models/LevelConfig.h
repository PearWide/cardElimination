#ifndef __LEVEL_CONFIG_H__
#define __LEVEL_CONFIG_H__

#include "cocos2d.h"
#include "../../models/CardModel.h"

class LevelConfig {
public:
    struct CardConfig {
        CardFaceType face;
        CardSuitType suit;
        cocos2d::Vec2 position;
        bool isCovered;

        CardConfig() : face(CFT_NONE), suit(CST_NONE),
            position(cocos2d::Vec2::ZERO), isCovered(false) {}
    };

    std::vector<CardConfig> playfieldCards;
    std::vector<CardConfig> stackCards;
    int levelId;
    std::string levelName;

    LevelConfig() : levelId(0) {}

    bool fromJson(const std::string& jsonStr);
    void debugPrint() const;
};

#endif // __LEVEL_CONFIG_H__