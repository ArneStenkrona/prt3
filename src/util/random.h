#ifndef PRT3_RANDOM_H
#define PRT3_RANDOM_H

#include <random>

template<typename T>
T random_int() {
    thread_local std::random_device rd;
    thread_local std::mt19937_64 gen{rd()};
    thread_local std::uniform_int_distribution<T> dis{
        std::numeric_limits<T>::min(),
        std::numeric_limits<T>::max()
    };
    return dis(gen);
}

#endif
