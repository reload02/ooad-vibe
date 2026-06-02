#pragma once

#include "Simulator/Point.h"

#include <cstdint>
#include <string>
#include <vector>

enum class CellType {
    Empty,
    Wall,
    Dust,
    Cleaned
};

class GridMap {
public:
    GridMap();

    void resetDefault();
    void resetSystemTestBase();
    bool loadLayout(const std::vector<std::string>& rowsTopToBottom);
    /// seed가 0이면 매번 다른 맵, 0이 아니면 재현 가능한 난수 시퀀스
    void resetRandom(uint32_t seed = 0);
    int getWidth() const;
    int getHeight() const;
    bool isInside(Point point) const;
    bool isWall(Point point) const;
    bool hasDust(Point point) const;
    bool isCleaned(Point point) const;
    bool clean(Point point);
    bool addDust(Point point);
    bool toggleWall(Point point);
    CellType getCell(Point point) const;
    std::vector<std::string> render(Point robot, Point direction) const;

private:
    int width = 12;
    int height = 12;
    std::vector<std::vector<CellType>> cells;

    void fillBorderWalls();
    void setWall(Point point);
    void setDust(Point point);

    static char robotSymbol(Point direction);
    static char cellSymbol(CellType cell);
    std::string renderRow(int y, Point robot, Point direction) const;
};
