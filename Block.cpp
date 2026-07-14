#include <SFML/Graphics.hpp>
#include "Settings.h"
#include "Block.h"
#include <cmath>

Block::Block(int x, int y, int& textureIndex, sf::Texture* blockTextures, sf::Color color, bool statisticsUi) {
    posX = static_cast<float>(x);
    posY = static_cast<float>(y);

    size_pixel = PIXEL_SIZE;
    if (statisticsUi) {
        size_pixel = STATISTICS_UI_PIXEL_SIZE;
    }

    float calculatedGap = static_cast<float>(size_pixel) * 0.12f;
    int intGap = std::max(static_cast<int>(std::roundf(calculatedGap)), 1);
    float visualSize = static_cast<float>(size_pixel - intGap);

    background.setSize(sf::Vector2f(visualSize, visualSize));
    spriteOverlay.setTexture(blockTextures[textureIndex]);
    spriteOverlay.setColor(color);

    float scaleX = visualSize / blockTextures[textureIndex].getSize().x;
    float scaleY = visualSize / blockTextures[textureIndex].getSize().y;
    spriteOverlay.setScale(scaleX, scaleY);
}

Block::Block() {
    posX = 0;
    posY = 0;
    size_pixel = PIXEL_SIZE;
}

sf::Vector2i Block::getPosition() {
    return sf::Vector2i(static_cast<int>(posX), static_cast<int>(posY));
}

void Block::setPosition(int x, int y) {
    posX = static_cast<float>(x);
    posY = static_cast<float>(y);
}

void Block::move(float dx, float dy) {
    posX += dx;
    posY += dy;
}

void Block::draw(sf::RenderWindow& window) {
    if (posY < 0) {
        return;
    }

    float screenX = OFFSET_X + posX * size_pixel;
    float screenY = OFFSET_Y + posY * size_pixel;

    background.setPosition(screenX, screenY);
    spriteOverlay.setPosition(screenX, screenY);

    window.draw(background);
    window.draw(spriteOverlay);
}