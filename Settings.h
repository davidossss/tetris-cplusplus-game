#include <SFML/Graphics.hpp>

#pragma once

const bool TURN_MUSIC = true;

const int NES_FRAMES_PER_DROP[30] = {
    48, 43, 38, 33, 28, 23, 18, 13, 8, 6,
    5,  5,  5,  4,  4,  4,  3,  3,  3, 2,
    2,  2,  2,  2,  2,  2,  2,  2,  2, 1
};

const enum FigureType { Fig_I, Fig_L, Fig_J, Fig_O, Fig_S, Fig_Z, Fig_T };

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 700;

const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;

const int STATISTICS_UI_PIXEL_SIZE = 18;
const int PIXEL_SIZE = 25;

const int OFFSET_X = 300;
const int OFFSET_Y = 125;

inline const sf::Color LEVEL_COLORS[10][2] = {
    { {32, 56, 236},  {60, 188, 252}  },
    { {0, 168, 0},    {128, 208, 16}  },
    { {188, 0, 188},  {244, 120, 252} },
    { {32, 56, 236},  {76, 220, 72}   },
    { {228, 0, 88},  {88, 248, 152}   },
    { {88, 248, 152},  {92, 148, 252}   },
    { {216, 40, 0},  {116, 116, 116}   },
    { {128, 0, 240},  {168, 0, 16}   },
    { {32, 56, 236},  {216, 40, 0}   },
    { {216, 40, 0},  {252, 152, 56}   }
};