#ifndef __ID_GENERATOR_H__
#define __ID_GENERATOR_H__

/**
 * @class IDGenerator
 * @brief 提供通用ID生成功能
 * 
 * 职责：
 * - 生成唯一的卡牌ID
 * - 提供通用独立的辅助功能
 * - 不涉及业务逻辑，完全独立
 */
class IDGenerator {
public:
    /**
     * @brief 生成卡牌ID
     * @return 新的卡牌ID
     */
    static int generateCardId() {
        static int nextId = 1000; // 从1000开始
        return nextId++;
    }
    
    /**
     * @brief 重置ID生成器
     */
    static void reset() {
        static int nextId = 1000;
        nextId = 1000;
    }
};

#endif // __ID_GENERATOR_H__
