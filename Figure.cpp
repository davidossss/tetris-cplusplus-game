#include "Block.h"
#include "Figure.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Settings.h"

#pragma once

Figure::Figure(int type, int& currentLevel, sf::Texture* blockTextures, bool statisticsUi) {
    typeFigure = type + 1;
    placed = false;

    int types[7][4][2] = {
        { {0, 0}, {1, 0}, {2, 0}, {3, 0} }, // 0: I
        { {0, 0}, {1, 0}, {2, 0}, {2, 1} }, // 1: L
        { {0, 1}, {1, 1}, {2, 1}, {2, 0} }, // 2: J
        { {0, 0}, {1, 0}, {0, 1}, {1, 1} }, // 3: O
        { {1, 0}, {2, 0}, {0, 1}, {1, 1} }, // 4: S
        { {0, 0}, {1, 0}, {1, 1}, {2, 1} }, // 5: Z
        { {0, 0}, {1, 0}, {2, 0}, {1, 1} }  // 6: T
    };

    // Текстура в зависимости от фигуры
    int textureIndex = 0;
    if (type == Fig_L || type == Fig_J || type == Fig_S || type == Fig_Z) {
        textureIndex = 1;
    }

    // Цвет в зависимости от уровня и фигуры
    int colorIndex = currentLevel % 10;
    sf::Color figureColor = LEVEL_COLORS[colorIndex][1];
    if (type == Fig_I || type == Fig_O || type == Fig_T) {
        figureColor = LEVEL_COLORS[colorIndex][0];
    }

    for (int i = 0; i < 4; i++) {
        int gridX = types[type][i][0] + (BOARD_WIDTH / 2) - 2;
        int gridY = types[type][i][1];

        blocks[i] = Block(gridX, gridY, textureIndex, blockTextures, figureColor, statisticsUi);
    }
}

// Если мы поставили фигуру, мы должны это зафиксировать в field. Лучше также оставить проверку на x и y, иначе игра вылетит.
void Figure::placeFigure(std::vector<std::vector<int>>& field) {
    placed = true;

    for (int i = 0; i < 4; i++) {
        int px = blocks[i].getPosition().x;
        int py = blocks[i].getPosition().y;
        
        if (py >= 0 && py < field.size() && px >= 0 && px < field[0].size()) {
            field[py][px] = typeFigure;
        }
    }
}

void Figure::move(float dx, float dy) {
    if (!placed) {
        for (int i = 0; i < 4; i++) {
            blocks[i].move(dx, dy);
        }
    }
}

void Figure::draw(sf::RenderWindow& window) {
    for (int i = 0; i < 4; i++) {
        blocks[i].draw(window);
    }
}

void Figure::rotate() {
    int type = typeFigure - 1;

    if (type == Fig_O) {
        return;
    }

    if (type == Fig_I) {
        bool isVertical = (blocks[0].getPosition().x == blocks[1].getPosition().x);

        int cx = blocks[1].getPosition().x;
        int cy = blocks[1].getPosition().y;

        if (isVertical) {
            blocks[0].setPosition(cx - 1, cy);
            blocks[2].setPosition(cx + 1, cy);
            blocks[3].setPosition(cx + 2, cy);
        } else {
            blocks[0].setPosition(cx, cy - 1);
            blocks[2].setPosition(cx, cy + 1);
            blocks[3].setPosition(cx, cy + 2);
        }
        return;
    }

    sf::Vector2i center = blocks[1].getPosition();
    if (type == Fig_J || type == Fig_L) {
        center = blocks[2].getPosition();
    }

    for (int i = 0; i < 4; i++) {
        int oldX = blocks[i].getPosition().x;
        int oldY = blocks[i].getPosition().y;

        int newX = center.x + center.y - oldY;
        int newY = center.y - center.x + oldX;

        blocks[i].setPosition(newX, newY);
    }
}


Block Figure::getBlock(int ind) {
    return blocks[ind];
}