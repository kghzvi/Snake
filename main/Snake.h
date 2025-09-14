#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

enum class E_Direction {
    Up,
    Down,
    Left,
    Right,
    Center
};

struct ST_Pixel {
    int8_t x;
    int8_t y;
};

class Snake {
public:
    Snake(int iLength, ST_Pixel stBody[64]);
    int getLength() const;
    const ST_Pixel (&getBody() const)[64];
    const bool (&getBodyAsImage() )[8][8];

    void move(enum E_Direction eDirection);
    void grow(enum E_Direction eDirection);

private:
    int iLength;
    enum E_Direction eDirection;
    ST_Pixel stBody[64];
    bool arbBodyAsImage[8][8];
};

#endif