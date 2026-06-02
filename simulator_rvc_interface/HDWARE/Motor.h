#pragma once

class Motor {
public:
    virtual ~Motor() = default;

    virtual void moveForward() = 0;
    virtual void moveBackward() = 0;
    virtual void turnLeft() = 0;
    virtual void turnRight() = 0;
};
