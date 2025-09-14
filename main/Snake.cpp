#include "Snake.h"
#include <algorithm>

Snake::Snake(int iLength, ST_Pixel stBody[64]) {
    this->iLength = iLength;
    std::copy(stBody, stBody + iLength, this->stBody);
}

int Snake::getLength() const {
    return iLength;
}

const ST_Pixel (&Snake::getBody() const)[64] {
    return stBody;
}

const bool (&Snake::getBodyAsImage() )[8][8] {
    // Clear image array
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            arbBodyAsImage[x][y] = false;
        }
    }

    // Set image array based on snake body
    for (int i = 0; i < iLength; i++) {
        arbBodyAsImage[stBody[i].x][stBody[i].y] = true;
    }

    return arbBodyAsImage;
}

void Snake::move(enum E_Direction eDirection) {
    // Körper nach hinten verschieben (vom Ende zum Anfang!)
    for (int i = iLength - 1; i > 0; --i) {
        stBody[i] = stBody[i - 1];
    }

    // Make sure the head does not turn 180 degrees
    if (eDirection == E_Direction::Up && this->eDirection != E_Direction::Down) {
        this->eDirection = eDirection;
    }    
    else if (eDirection == E_Direction::Down && this->eDirection != E_Direction::Up) {
        this->eDirection = eDirection;
    }
    else if (eDirection == E_Direction::Left && this->eDirection != E_Direction::Right) {
        this->eDirection = eDirection;
    }
    else if (eDirection == E_Direction::Right && this->eDirection != E_Direction::Left) {
        this->eDirection = eDirection;
    }
        
    switch (this->eDirection) {
    case E_Direction::Up:
        stBody[0].x = stBody[1].x;
        stBody[0].y = stBody[1].y - 1;
        break;
    
    case E_Direction::Down:
        stBody[0].x = stBody[1].x; 
        stBody[0].y = stBody[1].y + 1;
        break;

    case E_Direction::Left:
        stBody[0].x = stBody[1].x - 1; 
        stBody[0].y = stBody[1].y;
        break;

    case E_Direction::Right:
        stBody[0].x = stBody[1].x + 1; 
        stBody[0].y = stBody[1].y;
        break;

    case E_Direction::Center:
        // Do nothing
        break;
    };

    // Wrap around screen edges
    if (stBody[0].x > 7) {
        stBody[0].x = 0;
    } else if (stBody[0].x < 0) {
        stBody[0].x = 7;
    }

    if (stBody[0].y > 7) {
        stBody[0].y = 0;
    } else if (stBody[0].y < 0) {
        stBody[0].y = 7;
    }
}

void Snake::grow(enum E_Direction eDirection) {
    move(eDirection);
    iLength++;
}