#ifndef __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
#define __GAME_MODEL_FROM_LEVEL_GENERATOR_H__

#include "../configs/models/LevelConfig.h"
#include "../models/GameModel.h"

/**
 * @class GameModelFromLevelGenerator
 * @brief 将静态配置（LevelConfig）转换为动态运行时数据（GameModel）的服务
 * 
 * 职责：
 * - 处理关卡配置到游戏模型的转换
 * - 处理卡牌随机生成策略
 * - 提供无状态的服务，不管理数据生命周期
 */
class GameModelFromLevelGenerator {
public:
    /**
     * @brief 从关卡配置生成游戏模型
     * @param levelConfig 关卡配置
     * @return 生成的游戏模型
     */
    static GameModel generateGameModel(const LevelConfig& levelConfig);
    
    /**
     * @brief 验证关卡配置的有效性
     * @param levelConfig 关卡配置
     * @return 配置是否有效
     */
    static bool validateLevelConfig(const LevelConfig& levelConfig);
    
    /**
     * @brief 应用卡牌随机生成策略
     * @param gameModel 游戏模型
     * @param strategy 生成策略
     */
    static void applyCardGenerationStrategy(GameModel& gameModel, const std::string& strategy = "default");
};

#endif // __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
