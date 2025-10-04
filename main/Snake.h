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
    Snake(int iLength, ST_Pixel stInitialBody[64]);
    int getLength() const;
    const ST_Pixel (&getBody() const)[64];
    const bool (&getBodyAsImage() )[8][8];
    const ST_Pixel getNextHeadPosition();
    bool isPixelOnBody(const ST_Pixel pixel);
    bool snakeBitItself();

    void setDirection(enum E_Direction eDirection);
    void move();
    void grow();

private:
    int iLength;
    enum E_Direction eDirection;
    ST_Pixel stBody[64];
    bool arbBodyAsImage[8][8];
};

#endif