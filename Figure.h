#include "Block.h"
#include <SFML/Graphics.hpp>
#include <iostream>

#pragma once

class Figure {
private:
    Block blocks[4];
    bool statisticsUi = false;
    bool placed = false;
    int typeFigure = 0;

public:
    Figure(int type, int& currentLevel, sf::Texture* blockTextures, bool statisticsUi);

    void placeFigure(std::vector<std::vector<int>>& field);
    void move(float dx, float dy);
    void draw(sf::RenderWindow& window);
    void rotate();
    Block getBlock(int ind);
};