#ifndef __LEVEL_CONFIG_LOADER_H__
#define __LEVEL_CONFIG_LOADER_H__

#include "cocos2d.h"
#include "../models/LevelConfig.h"

class LevelConfigLoader {
public:
    static LevelConfig loadLevelConfig(const std::string& configPath);
    static LevelConfig loadLevelConfig(int levelId);
    
private:
    static std::string getConfigPath(int levelId);
};

#endif // __LEVEL_CONFIG_LOADER_H__
