#ifndef LIFEGAME_UTIL_H
#define LIFEGAME_UTIL_H

#include <cstddef>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "font_load_exception.h"

namespace lifegame {

    using std::cout;
    using std::cerr;
    using std::endl;
    using std::string;
    using std::size_t;
    using namespace sf;

    /**
     * Выделяет двумерный массив указанных размеров одним блоком
     * Для удаления нужно просто вызвать delete[] array;
     */
    template<typename T>
    static T** new_2d_array(size_t width, size_t height, size_t offset = 0) {
        T** array = static_cast<T**>(static_cast<void*>(new char[width * sizeof(T*) + width * height * sizeof(T)]));

        T* const data = static_cast<T*>(static_cast<void*>(array + width));
        for(size_t i = 0; i < width; ++i) {
            array[i] = data + i * height + offset;
        }

        return array + offset;
    }

    template<typename T>
    static T** new_2d_array(size_t width, size_t height, size_t offset, T fillValue) {
        T** array = new_2d_array<T>(width, height, offset);

        for(T *data = static_cast<T*>(static_cast<void*>(array - offset + width)), *end = data + width * height; data < end; ++data) {
            *data = fillValue;
        }

        return array;
    }

    Font loadFont(string name);
}

#endif // LIFEGAME_UTIL_H
