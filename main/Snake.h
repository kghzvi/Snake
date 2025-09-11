#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

enum class E_Direction {
    Up,
    Down,
    Left,
    Right
};


class Snake {
public:
    Snake();
    int getLength() const;
    const bool (&getBody() const)[64];
    const bool (&getBodyAsImage() const)[8][8];

    void move(enum E_Direction eDirection);

    void grow(enum E_Direction eDirection);

private:
    int iLength;
    bool arbBody[64];
    bool arbBodyAsImage[8][8];
};

#endif