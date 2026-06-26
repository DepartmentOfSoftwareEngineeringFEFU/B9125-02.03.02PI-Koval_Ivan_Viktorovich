#pragma once

#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

#define PI 3.14159
#define SQRT2 1.41421356237

HDC desktop;
HDC buffers[10];
const int dw = GetSystemMetrics(SM_CXSCREEN);
const int dh = GetSystemMetrics(SM_CYSCREEN);

int TPS = 60;

struct Point {
public:
    double x;
    double y;
};

class BouncingObject {
public:
    HDC bufferOneInUse;
    HDC bufferTwoInUse; // this one is used to apply raster operations
    Point center;
    POINT centerInBuf;
    double angleDeg;
    double angleRad;
    float motionX;
    float motionY;
    POINT rectPosInBuf;
    bool inBounds;

    virtual int GetCrossedBound() {
        return -1;
    };

    virtual void UpdatePoints() {

    }

    virtual void ProcessCollisions() {

    }

    virtual void Rotate(float angleDegs) {

    }

    // €блоко на бошку Єбнулось
    void UpdateMotion() {
        this->motionY += 1;
    }

    virtual void Move() {

    }

    virtual void DrawObject() {

    }

};

class Rect : public BouncingObject {
public:
    Point points[4];
    int side;

    void AlignPointsToCenter() {
        float q2 = -side / 2;
        float q1 = side / 2;
        float q3 = -side / 2;
        float q4 = side / 2;
        points[0] = { q1 * cos(angleRad) - q4 * sin(angleRad) + center.x, q1 * sin(angleRad) + q4 * cos(angleRad) + center.y };
        points[1] = { q2 * cos(angleRad) - q4 * sin(angleRad) + center.x, q2 * sin(angleRad) + q4 * cos(angleRad) + center.y };
        points[2] = { q1 * cos(angleRad) - q3 * sin(angleRad) + center.x, q1 * sin(angleRad) + q3 * cos(angleRad) + center.y };
        points[3] = { q2 * cos(angleRad) - q3 * sin(angleRad) + center.x, q2 * sin(angleRad) + q3 * cos(angleRad) + center.y };
    }

    // returns the crossed bound, -1 if rect is in bounds
    int GetCrossedBound() override {
        for (int i = 0; i < 4; i++) {
            if (points[i].x > dw || points[i].x < 0) {
                return dh;
            }
            if (points[i].y > dh || points[i].y < 0) {
                return dw;
            }
        }
        return -1;
    }

    void Rotate(float angleDegs) override {
        float angleRads = angleDegs * PI / 180;
        this->angleRad = angleRads;
        float q2 = -side / 2;
        float q1 = side / 2;
        float q3 = -side / 2;
        float q4 = side / 2;
        points[0] = { q1 * cos(angleRad) - q4 * sin(angleRad) + center.x, q1 * sin(angleRad) + q4 * cos(angleRad) + center.y };
        points[1] = { q2 * cos(angleRad) - q4 * sin(angleRad) + center.x, q2 * sin(angleRad) + q4 * cos(angleRad) + center.y };
        points[2] = { q1 * cos(angleRad) - q3 * sin(angleRad) + center.x, q1 * sin(angleRad) + q3 * cos(angleRad) + center.y };
        points[3] = { q2 * cos(angleRad) - q3 * sin(angleRad) + center.x, q2 * sin(angleRad) + q3 * cos(angleRad) + center.y };
    }

    void ProcessCollisions() override {
        int crossedBound = GetCrossedBound();
        if (crossedBound == dw) {
            if (center.y - side / 2 <= 0) {
                center.y = side / 2;
            }
            else {
                center.y = dh - side / 2;
            }
            AlignPointsToCenter();
            this->motionY *= -1;
        }
        if (crossedBound == dh) {
            if (center.x - side / 2 <= 0) {
                center.x = side / 2;
            }
            else {
                center.x = dw - side / 2;
            }
            AlignPointsToCenter();
            this->motionX *= -1;
        }

    }

    void Move() override {
        for (int i = 0; i < 4; i++) {
            points[i].x += this->motionX;
            points[i].y += this->motionY;
            //std::cout << this->motionX << "\n";
            //std::cout << this->motionY << "\n";
        }
        center.x += this->motionX;
        center.y += this->motionY;
    }

    void DrawObject() override {
        POINT threePoints[3] = { {points[0].x, points[0].y},{points[1].x, points[1].y}, { points[2].x, points[2].y } };
        double xes[] = { points[0].x, points[1].x, points[2].x, points[3].x };
        double ys[] = { points[0].y, points[1].y, points[2].y, points[3].y };
        PlgBlt(desktop, threePoints, bufferOneInUse, rectPosInBuf.x, rectPosInBuf.y, side, side, 0, 0, 0);
        //BitBlt(desktop, std::min_element(xes[0], xes[3]), std::max_element(ys[0], ys[3]), side * SQRT2, side * SQRT2, bufferTwoInUse, centerInBuf.x - SQRT2 * side / 2, centerInBuf.y + SQRT2 * side / 2, SRCINVERT);
    }

    Rect(Point center, HDC bufferOneInUse, HDC bufferTwoInUse, POINT centerInBuf, int side, double startAngleDeg, int startingMotionX, int startingMotionY) {
        this->center = center;
        this->bufferOneInUse = bufferOneInUse;
        this->bufferTwoInUse = bufferTwoInUse;
        this->centerInBuf = centerInBuf;
        this->side = side;
        this->motionX = startingMotionX;
        this->motionY = startingMotionY;
        this->angleDeg = startAngleDeg;
        this->angleRad = startAngleDeg * PI / 180;
        this->rectPosInBuf = { centerInBuf.x - side / 2, centerInBuf.y - side / 2 };
        AlignPointsToCenter();
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        SelectObject(this->bufferOneInUse, brush);
        Rectangle(bufferOneInUse, points[0].x, points[0].y, points[3].x, points[3].y);
        BitBlt(bufferOneInUse, 0, 0, dw, dh, desktop, 0, 0, SRCINVERT);
        BitBlt(bufferTwoInUse, 0, 0, dw, dh, desktop, 0, 0, BLACKNESS);
        BitBlt(desktop, points[0].x, points[0].y, side, side, bufferOneInUse, rectPosInBuf.x, rectPosInBuf.y, SRCINVERT);
    }

};

class Circle : BouncingObject {
public:
    int radius;
};

std::vector<BouncingObject*> objedki = std::vector<BouncingObject*>();

void UpdateBouncingObjects() {
    int angle = 10;
    while (true) {
        if (!objedki.empty()) {
            for (BouncingObject* objedok : objedki) {
                objedok->ProcessCollisions();
                objedok->Move();
                objedok->UpdateMotion();
                objedok->DrawObject();
            }
            Sleep(1000 / TPS);
        }
        angle += 10;
    }

}