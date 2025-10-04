#include "Snake.h"
#include <algorithm>

Snake::Snake(int iLength, ST_Pixel stInitialBody[64]) {
    this->iLength = iLength;
    std::copy(stInitialBody, stInitialBody + iLength, this->stBody);
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

const ST_Pixel Snake::getNextHeadPosition() {
    ST_Pixel nextHead = stBody[0];

    switch (eDirection) {
        case E_Direction::Up:
            nextHead.y -= 1;
            break;
        case E_Direction::Down:
            nextHead.y += 1;
            break;
        case E_Direction::Left:
            nextHead.x -= 1;
            break;
        case E_Direction::Right:
            nextHead.x += 1;
            break;
        case E_Direction::Center:
            // Do nothing
            break;
    }

    // Wrap around screen edges
    if (nextHead.x > 7) {
        nextHead.x = 0;
    } else if (nextHead.x < 0) {
        nextHead.x = 7;
    }

    if (nextHead.y > 7) {
        nextHead.y = 0;
    } else if (nextHead.y < 0) {
        nextHead.y = 7;
    }

    return nextHead;
}

bool Snake::isPixelOnBody(const ST_Pixel pixel) {
    for (int i = 0; i < iLength; i++) {
        if (stBody[i].x == pixel.x && stBody[i].y == pixel.y) {
            return true;
        }
    }
    return false;
}

bool Snake::snakeBitItself() {
    ST_Pixel head = stBody[0];
    for (int i = 1; i < iLength; i++) {
        if (stBody[i].x == head.x && stBody[i].y == head.y) {
            return true;
        }
    }
    return false;
}

void Snake::setDirection(enum E_Direction eDirection) {
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
    else if (eDirection == E_Direction::Center) {
        // Do nothing
    }
}

void Snake::move() {
    // Körper nach hinten verschieben (vom Ende zum Anfang!)
    for (int i = iLength - 1; i > 0; --i) {
        stBody[i] = stBody[i - 1];
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

void Snake::grow() {
    iLength++;
    move();
}