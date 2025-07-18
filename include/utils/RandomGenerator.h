//
// Created by 祁明 on 25-7-1.
//

#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

#include <algorithm>
#include <initializer_list>
#include <random>
#include <stdexcept>
#include <vector>

namespace qm {
    class RandomGenerator {
    public:
        static RandomGenerator &instance() {
            thread_local RandomGenerator instance;
            return instance;
        }

        RandomGenerator(const RandomGenerator &) = delete;

        RandomGenerator &operator=(const RandomGenerator &) = delete;

        /**
         * @brief 设置随机种子
         * @param seed 种子值
         */
        void setSeed(unsigned seed) {
            gen.seed(seed);
        }

        /**
         * @brief 生成[min, max]范围内的随机整数
         */
        int getInt(int min, int max) {
            if (min > max) {
                throw std::invalid_argument("min must be less than or equal to max");
            }
            std::uniform_int_distribution<int> dist(min, max);
            return dist(gen);
        }

        /**
         * @brief 生成[0, max-1]范围内的随机整数
         */
        int getInt(int max) {
            return getInt(0, max - 1);
        }

        /**
         * @brief 填充数组随机整数
         */
        void fillInts(int min, int max, int *output, size_t count) {
            if (min > max) {
                throw std::invalid_argument("min must be less than or equal to max");
            }
            std::uniform_int_distribution<int> dist(min, max);
            for (size_t i = 0; i < count; ++i) {
                output[i] = dist(gen);
            }
        }

        /**
         * @brief 生成[min, max)范围内的随机浮点数
         */
        template<typename T>
        std::enable_if_t<std::is_floating_point_v<T>, T>
        getUniform(T min, T max) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(gen);
        }

        /**
         * @brief 生成随机布尔值
         */
        bool getBool() {
            return getInt(0, 1) == 1;
        }

        /**
         * @brief 生成正态分布随机数
         */
        double getNormal(double mean = 0.0, double stddev = 1.0) {
            std::normal_distribution<double> dist(mean, stddev);
            return dist(gen);
        }

        /**
         * @brief 生成指数分布随机数
         */
        double getExponential(double lambda) {
            std::exponential_distribution<double> dist(lambda);
            return dist(gen);
        }

        /**
         * @brief 根据权重生成离散分布的随机整数
         * @param weights 权重列表，每个元素对应返回值的概率权重
         * @return 返回[0, weights.size()-1]范围内的整数，概率由权重决定
         */
        int getDiscrete(const std::vector<double> &weights) {
            if (weights.empty()) {
                throw std::invalid_argument("weights must not be empty");
            }

            // 检查是否全零权重
            if (std::ranges::all_of(weights, [](const double w) { return w == 0.0; })) {
                throw std::invalid_argument("weights cannot be all zeros");
            }
            std::discrete_distribution<int> dist(weights.begin(), weights.end());
            return dist(gen);
        }

        /**
         * @brief 根据权重生成离散分布的随机整数（初始化列表版本）
         */
        int getDiscrete(const std::initializer_list<double> weights) {
            return getDiscrete(std::vector<double>(weights));
        }

        /**
         * @brief 直接访问随机引擎
         */
        template<typename F>
        auto withEngine(F &&f) -> decltype(f(std::declval<std::mt19937 &>())) {
            return f(gen);
        }

    private:
        RandomGenerator() : gen(std::random_device{}()) {}

        std::mt19937 gen;
    };

// 便捷函数
    inline RandomGenerator &randomGenerator() {
        return RandomGenerator::instance();
    }

    inline void setRandomSeed(unsigned seed) {
        RandomGenerator::instance().setSeed(seed);
    }

    inline int randomIntBetween(int min, int max) {
        return RandomGenerator::instance().getInt(min, max);
    }

    inline int randomInt(int max) {
        return RandomGenerator::instance().getInt(max);
    }

    inline double randomDouble(double min, double max) {
        return RandomGenerator::instance().getUniform(min, max);
    }

    /**
     * @brief 根据权重生成离散分布的随机整数
     * @param weights 权重列表
     * @return 返回[0, weights.size()-1]范围内的整数
     */
    inline int randomDiscrete(const std::vector<double> &weights) {
        return RandomGenerator::instance().getDiscrete(weights);
    }

    /**
     * @brief 根据权重生成离散分布的随机整数（初始化列表版本）
     */
    inline int randomDiscrete(std::initializer_list<double> weights) {
        return RandomGenerator::instance().getDiscrete(weights);
    }
}
#endif  // RANDOM_GENERATOR_H