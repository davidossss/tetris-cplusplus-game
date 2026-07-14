#pragma once
#include <SFML/Graphics.hpp>

class Block {
private:
    int size_pixel;
    float posX, posY;
    sf::RectangleShape background;
    sf::Sprite spriteOverlay;
public:
    Block();
    Block(int x, int y, int& textureIndex, sf::Texture* blockTextures, sf::Color color, bool statisticsUi);

    sf::Vector2i getPosition();
    void setPosition(int x, int y);
    void move(float dx, float dy);
    void draw(sf::RenderWindow& window);
};
