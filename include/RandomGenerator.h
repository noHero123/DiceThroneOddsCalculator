#pragma once

#include<random>

class RandomGenerator {
public:
    static inline uint32_t uniform_int_distribution(uint32_t min, uint32_t max, std::mt19937& RandomBitGenerator)
    {
        uint32_t randomzahl = random_bounded_nearlydivisionless32(max - min + 1, RandomBitGenerator);
        return (min + randomzahl);
    }

    static inline uint32_t random_bounded_nearlydivisionless32(uint32_t range, std::mt19937& RandomBitGenerator) {
        uint64_t random32bit, multiresult;
        uint32_t leftover;
        uint32_t threshold;
        random32bit = RandomBitGenerator();
        multiresult = random32bit * range;
        leftover = (uint32_t)multiresult;
        if (leftover < range) {
            threshold = (0U - range) % range;
            while (leftover < threshold) {
                random32bit = RandomBitGenerator();
                multiresult = random32bit * range;
                leftover = (uint32_t)multiresult;
            }
        }
        return (multiresult >> 32); // [0, range)
    }
};