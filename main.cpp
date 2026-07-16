#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <ctime>
#include "Figure.h"
#include "Settings.h"
#include <iostream>
#include <windows.h>

using namespace sf;
using namespace std;

enum SoundID {
    Move,
    Rotate,
    Place,
    Line,
    Tetris,
    GameOver,
    NewLevel,
    MusicCount
};

struct GameAudio {
    Sound sounds[MusicCount];
    Sound musicGame;
    Sound musicDangerGame;
};

struct GameState {
    float timer = 0.f;
    float animationRemove = 0.f;
    float animationGameOver = 0.f;
    float restartClean = 0.f;
    float restartGameFromGameOver = 0.f;
    float delayPlace = 0.f;
    int statCounters[7] = { 0, 0, 0, 0, 0, 0, 0 };

    int levelNum = 0;

    int score = 0;
    int topScore = 10000;

    int stateFigure = 0;
    int nextStateFigure = 0;

    int cntLines = 0;

    int animLineRemoverStep = 0;
    int iAnimGameOver = 0;
    int jAnimGameOver = 0;

    bool gameOverSoundPlayed = false;
    bool removingLines = false;
    bool gameOver = false;
    bool danger = false;

    vector<std::vector<int>> field;
    vector<int> removeJ;
    vector<Block> gameOverBlocks;
};

static bool checkInBounds(Figure& figure, vector<vector<int>>& field) {
    // Проверяем, игрок не попал за экран или не на блоке (кроме y, в таком случае мы просто не смотрим на field[posY][posX] и нам помогает !field.empty())
    for (int i = 0; i < 4; i++) {
        int posX = figure.getBlock(i).getPosition().x;
        int posY = figure.getBlock(i).getPosition().y;

        if (posX < 0 || posX >= BOARD_WIDTH || posY >= BOARD_HEIGHT) {
            return false;
        }

        if (posY >= 0 && !field.empty() && field[posY][posX]) {
            return false;
        }
    }
    return true;
}

static int getStatIndex(int figureType) {
    switch (figureType) {
    case Fig_T: return 0;
    case Fig_J: return 1;
    case Fig_Z: return 2;
    case Fig_O: return 3;
    case Fig_S: return 4;
    case Fig_L: return 5;
    case Fig_I: return 6;
    default:    return 0;
    }
}

static float getNesDropDelay(int level) {
    int clampedLevel = (level > 29) ? 29 : level;
    int frames = NES_FRAMES_PER_DROP[clampedLevel];

    return static_cast<float>(frames) / 60.0988f;
}

static void init_game(GameAudio& audio, GameState& state, Figure& figure, sf::Texture* blockTextures) {
    state.topScore = max(state.topScore, state.score);
    state.score = 0;
    
    audio.musicDangerGame.stop();
    if (TURN_MUSIC) {
        audio.musicGame.play();
    }

    state.timer = state.animationRemove = state.animationGameOver = 0.f;
    state.restartClean = state.restartGameFromGameOver = state.delayPlace = 0.f;
    state.animLineRemoverStep = state.iAnimGameOver = state.jAnimGameOver = state.cntLines = state.levelNum = state.gameOverSoundPlayed = 0;
    state.removingLines = state.gameOver = state.danger = false;
    
    for (int i = 0; i < 7; i++) state.statCounters[i] = 0;

    state.stateFigure = rand() % 7;
    state.statCounters[getStatIndex(state.stateFigure)]++;
    state.nextStateFigure = rand() % 7;
    figure = Figure(state.stateFigure, state.levelNum, blockTextures, false);

    state.field.assign(BOARD_HEIGHT, vector<int>(BOARD_WIDTH, 0));
    state.removeJ.clear();
    state.gameOverBlocks.clear();
}

static string formatZeros(int value, int numDigits) {
    int maxLimit = static_cast<int>(pow(10, numDigits) - 1);
    if (value > maxLimit) value = maxLimit;

    string result = to_string(value);

    if (result.size() < numDigits) {
        return string(numDigits - result.size(), '0') + result;
    }

    return result;
}


int main()
{
    // Настройки окна, игры и рандомы
    srand(static_cast<unsigned int>(time(nullptr)));
    RenderWindow window(VideoMode({ SCREEN_WIDTH, SCREEN_HEIGHT }), "Tetris", Style::Close);
    window.setMouseCursorVisible(false);
    window.setFramerateLimit(60);
    Clock clock;

    // Ставим иконку
    Image icon;
    if (!icon.loadFromFile("Resources/Images/icon.png")) {
        MessageBoxA(NULL, "Не удалось загрузить: Resources/Images/icon.png", "Ошибка загрузки", MB_OK | MB_ICONERROR);
        return -1;
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // Инициализируем звук
    static GameAudio audio;

    // Добавляем звук с буфферами
    const string soundFiles[] = {
        "Resources/Sounds/movement.wav",
        "Resources/Sounds/rotate.wav",
        "Resources/Sounds/place.wav",
        "Resources/Sounds/line_remove.wav",
        "Resources/Sounds/line_tetris.wav",
        "Resources/Sounds/game_over.wav",
        "Resources/Sounds/new_level.wav"
    };
    static SoundBuffer buffers[MusicCount];
    for (int i = 0; i < MusicCount; i++) {
        if (!buffers[i].loadFromFile(soundFiles[i])) {
            string msg = "Не удалось загрузить звук: " + soundFiles[i];
            MessageBoxA(NULL, msg.c_str(), "Ошибка загрузки", MB_OK | MB_ICONERROR);
            return -1;
        }
        audio.sounds[i].setBuffer(buffers[i]);
    }

    // Добавляем музыку
    SoundBuffer bufferMusic, bufferMusicDanger;
    if (!bufferMusic.loadFromFile("Resources/Sounds/music.wav")) {
        MessageBoxA(NULL, "Не удалось загрузить: Resources/Sounds/music.wav", "Ошибка загрузки", MB_OK | MB_ICONERROR);
        return -1;
    }
    if (!bufferMusicDanger.loadFromFile("Resources/Sounds/music_danger.wav")) {
        MessageBoxA(NULL, "Не удалось загрузить: Resources/Sounds/music_danger.wav", "Ошибка загрузки", MB_OK | MB_ICONERROR);
        return -1;
    }
    audio.musicGame.setBuffer(bufferMusic);
    audio.musicGame.setLoop(true);
    audio.musicDangerGame.setBuffer(bufferMusicDanger);
    audio.musicDangerGame.setLoop(true);

    // Добавляем текстуры для блок для фигур
    Texture blockTextures[2];
    if (!blockTextures[0].loadFromFile("Resources/Images/texture_1.png")) {
        MessageBoxA(NULL, "Не удалось загрузить: Resources/Images/texture_1.png", "Ошибка загрузки", MB_OK | MB_ICONERROR);
        return -1;
    }
    if (!blockTextures[1].loadFromFile("Resources/Images/texture_2.png")) {
        MessageBoxA(NULL, "Не удалось загрузить: Resources/Images/texture_2.png", "Ошибка загрузки", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Импортируем арену
    Texture bgTexture;
    if (!bgTexture.loadFromFile("Resources/Images/background.png")) {
        MessageBoxA(NULL, "Не удалось загрузить: Resources/Images/background.png", "Ошибка загрузки", MB_OK | MB_ICONERROR);
        return -1;
    }
    Sprite bgSprite(bgTexture);
    float scaleBG_X = static_cast<float>(SCREEN_WIDTH) / bgTexture.getSize().x;
    float scaleBG_Y = static_cast<float>(SCREEN_HEIGHT) / bgTexture.getSize().y;
    bgSprite.setScale({ scaleBG_X, scaleBG_Y });

    // Импортируем шрифт
    Font font;
    if (!font.loadFromFile("Resources/Fonts/PressStart2P-Regular.ttf")) {
        MessageBoxA(NULL, "Не удалось загрузить шрифт: PressStart2P-Regular.ttf", "Ошибка загрузки", MB_OK | MB_ICONERROR);
        return -1;
    }
    Text textLines;
    textLines.setFont(font);
    textLines.setCharacterSize(24);
    textLines.setFillColor(sf::Color::White);
    textLines.setPosition(475, 51);

    Text textLevel;
    textLevel.setFont(font);
    textLevel.setCharacterSize(24);
    textLevel.setFillColor(sf::Color::White);
    textLevel.setPosition(650, 500);

    Text statTexts[7];

    float textStartX = 162.0f;
    float textStartY = 260.0f;
    float textStepY = 52.0f;

    for (int i = 0; i < 7; i++) {
        statTexts[i].setFont(font);
        statTexts[i].setCharacterSize(24);
        statTexts[i].setFillColor(Color::Color(216, 58, 1));
        statTexts[i].setPosition(textStartX, textStartY + (i * textStepY));
        statTexts[i].setString("000");
        window.draw(statTexts[i]);
    }

    Text textScore;
    textScore.setFont(font);
    textScore.setCharacterSize(24);
    textScore.setFillColor(sf::Color::White);
    textScore.setPosition(600, 175);

    Text textTopScore;
    textTopScore.setFont(font);
    textTopScore.setCharacterSize(24);
    textTopScore.setFillColor(sf::Color::White);
    textTopScore.setPosition(600, 100);

    // Инициализируем переменные для легкой инициализации игры
    GameState state;
    Figure figure = Figure(0, state.levelNum, blockTextures, false);
    Figure nextFigure = Figure(0, state.levelNum, blockTextures, false);
    init_game(audio, state, figure, blockTextures);

    while (window.isOpen())
    {
        // Цвет фона
        Color backgroundColor = Color::Color(116, 116, 116);

        // Таймер
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();

        // Сбор секунд для таймера анимаций
        state.timer += time;
        state.delayPlace = min(state.delayPlace + time, 1.0f);
        if (state.removingLines) {
            state.animationRemove += time;
        }
        if (state.gameOver) {
            state.animationGameOver += time;
        }

        // Считывание действий игрока
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) {
                window.close();
            }
            if (event.type == Event::KeyPressed && !state.removingLines && !state.gameOver) {
                if (state.delayPlace >= 0.2f) {
                    if (event.key.code == Keyboard::Left) {
                        figure.move(-1, 0);
                        if (!checkInBounds(figure, state.field)) { figure.move(1, 0); }
                        else { audio.sounds[Move].play(); }
                    }
                    if (event.key.code == Keyboard::Right) {
                        figure.move(1, 0);
                        if (!checkInBounds(figure, state.field)) { figure.move(-1, 0); }
                        else { audio.sounds[Move].play(); }
                    }
                    if (event.key.code == Keyboard::Down) {
                        figure.move(0, 1);
                        if (!checkInBounds(figure, state.field)) { figure.move(0, -1);  }
                        else { state.score += 1; if (state.score > state.topScore) state.topScore = state.score; }
                    }
                    if (event.key.code == Keyboard::Space) {
                        int cellsDropped = 0;
                        while (checkInBounds(figure, state.field)) {
                            figure.move(0, 1);
                            cellsDropped += 1;
                        }
                        figure.move(0, -1);
                        state.timer = getNesDropDelay(state.levelNum);
                        cellsDropped -= 1;
                        if (cellsDropped > 0) {
                            state.score += cellsDropped * 2;
                            if (state.score > state.topScore) {
                                state.topScore = state.score;
                            }
                        }
                    }
                    if (event.key.code == Keyboard::Z) {
                        figure.rotate();
                        if (!checkInBounds(figure, state.field)) { figure.rotate(); figure.rotate(); figure.rotate(); }
                        else { audio.sounds[Rotate].play(); }
                    }
                    if (event.key.code == Keyboard::F4) {
                        state.levelNum++;
                        audio.sounds[NewLevel].play();
                        figure = Figure(state.stateFigure, state.levelNum, blockTextures, false);
                    }
                }
                if (event.key.code == Keyboard::F5) { 
                    state.gameOver = true;
                }
            }
        }

        // Проверяем, проиграл ли игрок или нет
        if (!state.field.empty()) {
            for (int i = 0; i < state.field[0].size(); i++) {
                if (state.field[0][i]) {
                    state.gameOver = true;
                    break;
                }
            }
        }

        // Если игрок проиграл, останавливаем музыку
        if (state.gameOver && state.timer >= 0.05f) {
            if (audio.musicGame.getStatus() == Sound::Playing) {
                audio.musicGame.stop();
            }
            if (audio.musicDangerGame.getStatus() == Sound::Playing) {
                audio.musicDangerGame.stop();
            }

            if (!state.gameOverSoundPlayed) {
                audio.sounds[GameOver].play();
                state.gameOverSoundPlayed = true;
            }
        }

        // Если игрок в опасности, играем музыку опасности
        bool oldDanger = state.danger;
        state.danger = false;
        if (!state.field.empty()) {
            for (int j = 0; j < 10; j++) {
                for (int i = 0; i < state.field[j].size(); i++) {
                    if (state.field[j][i]) {
                        state.danger = true;
                        break;
                    }
                }
                if (state.danger) {
                    break;
                }
            }
        }
        if (!state.gameOver && TURN_MUSIC) {
            if (state.danger && !oldDanger) {
                audio.musicGame.stop();
                audio.musicDangerGame.play();
            } else if (!state.danger && oldDanger) {
                audio.musicDangerGame.stop();
                audio.musicGame.play();
            }
        }

        // Если игрок убрал линию, мы убираем с анимацией
        if (!state.removingLines && !state.gameOver) {
            for (int j = 0; j < state.field.size(); j++) {
                bool getLine = true;
                for (int i = 0; i < state.field[j].size(); i++) {
                    if (!state.field[j][i]) {
                        getLine = false;
                        break;
                    }
                }
                if (getLine) {
                    state.removeJ.push_back(j);
                    state.removingLines = true;
                }
            }

            if (state.removingLines) {
                if (state.removeJ.size() == 4) {
                    audio.sounds[Tetris].play();
                } else {
                    audio.sounds[Line].play();
                }
            }
        }
        if (state.removingLines && state.animationRemove >= 0.075 && !state.gameOver) {
            int mid = static_cast<int>(state.field[0].size() / 2);
            if (state.animLineRemoverStep <= mid) {
                for (auto j : state.removeJ) {
                    size_t idxJ = static_cast<size_t>(j);
                    int step = state.animLineRemoverStep;

                    if (mid - 1 - step >= 0) {
                        size_t index1 = static_cast<size_t>(mid - 1 - step);
                        state.field[idxJ][index1] = 0;
                    }
                    if (mid + step < static_cast<int>(state.field[idxJ].size())) {
                        size_t index2 = static_cast<size_t>(mid + step);
                        state.field[idxJ][index2] = 0;
                    }
                }

                state.animLineRemoverStep++;
            } else {
                vector<vector<int>> newField(state.field.size(), vector<int>(state.field[0].size(), 0));
                int writeY = static_cast<int>(state.field.size() - 1);
                for (int j = static_cast<int>(state.field.size() - 1); j >= 0; j--) {
                    bool isRemoved = false;
                    for (int r : state.removeJ) {
                        if (r == j) {
                            isRemoved = true;
                            break;
                        }
                    }
                    if (!isRemoved) {
                        newField[writeY] = state.field[j];
                        writeY--;
                    }
                }
                state.field = newField;
                state.removingLines = false;
                state.animLineRemoverStep = 0;

                int linesCleared = static_cast<int>(state.removeJ.size());
                int oldLines = state.cntLines;
                state.cntLines += linesCleared;

                int basePoints = 0;
                switch (linesCleared) {
                case 1:
                    basePoints = 40;
                    break;
                case 2:
                    basePoints = 100;
                    break;
                case 3:
                    basePoints = 300;
                    break;
                case 4:
                    basePoints = 1200;
                    break;
                }
                state.score += basePoints * (state.levelNum + 1);
                if (state.score > state.topScore) {
                    state.topScore = state.score;
                }

                if (state.cntLines / 10 > oldLines / 10) {
                    state.levelNum++;
                    audio.sounds[NewLevel].play();
                    figure = Figure(state.stateFigure, state.levelNum, blockTextures, false);
                }
                state.removeJ.clear();
            }
            state.animationRemove = 0;
        } // Дополнительная анимация
        if (state.removingLines && state.removeJ.size() == 4) {
            if (state.animLineRemoverStep % 2 == 0) {
                backgroundColor = Color::White;
            } else {
                backgroundColor = Color::Color(116, 116, 116);
            }
        }

        // Физика, если игрок поставил фигуру - выбираем новую фигуру
        if (state.timer >= getNesDropDelay(state.levelNum) && !state.removingLines && !state.gameOver && state.delayPlace >= 0.2f) {
            figure.move(0, 1);
            if (!checkInBounds(figure, state.field)) {
                audio.sounds[Place].play();
                figure.move(0, -1);
                figure.placeFigure(state.field);
                state.stateFigure = state.nextStateFigure;
                state.statCounters[getStatIndex(state.stateFigure)]++;
                state.nextStateFigure = rand() % 7;
                figure = Figure(state.stateFigure, state.levelNum, blockTextures, false);
                state.delayPlace = 0;
            }
            state.timer = 0;
        }

        // Отрисовка
        window.clear(backgroundColor);
        window.draw(bgSprite);

        // Отрисовываем HUD
        textLines.setString(formatZeros(state.cntLines, 3));
        window.draw(textLines);
        textLevel.setString(formatZeros(state.levelNum, 2));
        window.draw(textLevel);
        textScore.setString(formatZeros(state.score, 6));
        window.draw(textScore);
        textTopScore.setString(formatZeros(state.topScore, 6));
        window.draw(textTopScore);

        // Отрисовываем статистику
        const int STAT_FIGURE_IDS[7] = { 6, 1, 4, 3, 5, 2, 0 };
        float startBlockX = -15.0f, startBlockY = 7.0f, stepY = 3.0f;
        for (int i = 0; i < 7; i++) {
            int currentType = STAT_FIGURE_IDS[i];
            Figure miniFigure = Figure(currentType, state.levelNum, blockTextures, true);
            float currentY = startBlockY + (i * stepY);
            miniFigure.move(startBlockX, currentY);
            miniFigure.draw(window);
        }
        for (int i = 0; i < 7; i++) {
            statTexts[i].setString(formatZeros(state.statCounters[i], 3));
            window.draw(statTexts[i]);
        }

        // Отрисовываем следующую фигуру
        int figureDraw = state.nextStateFigure;
        if (state.removingLines || (state.timer == 0.f && state.delayPlace == 0.f)) {
            figureDraw = state.stateFigure;
        }
        nextFigure = Figure(figureDraw, state.levelNum, blockTextures, false);
        switch (figureDraw) {
        case 0:
            nextFigure.move(9.0f, 9.4f);
            break;
        case 1:
            nextFigure.move(9.4f, 9);
            break;
        case 2:
            nextFigure.move(9.4f, 9);
            break;
        case 3:
            nextFigure.move(10, 9);
            break;
        case 4:
            nextFigure.move(9.4f, 9);
            break;
        case 5:
            nextFigure.move(9.4f, 9);
            break;
        case 6:
            nextFigure.move(9.4f, 9);
            break;
        }
        if (state.restartClean < 3.0f) {
            nextFigure.draw(window);
        }

        // Рисуем фигуру
        if (!state.removingLines && !state.gameOver && state.delayPlace >= 0.2f) {
            figure.draw(window);
        }

        // Рисуем фигуры, которые уже упали
        for (int i = 0; i < BOARD_WIDTH; i++) {
            for (int j = 0; j < BOARD_HEIGHT; j++) {
                int typeFigure = state.field[j][i];

                if (typeFigure) {
                    int type = typeFigure - 1;

                    // Текстура в зависимости от фигуры
                    int textureIndex = 0;
                    if (type == Fig_L || type == Fig_J || type == Fig_S || type == Fig_Z) {
                        textureIndex = 1;
                    }

                    // Цвет в зависимости от уровня и фигуры
                    int colorIndex = state.levelNum % 10;
                    sf::Color blockColor = LEVEL_COLORS[colorIndex][1];
                    if (type == Fig_I || type == Fig_O || type == Fig_T) {
                        blockColor = LEVEL_COLORS[colorIndex][0];
                    }

                    Block blockPlaced(i, j, textureIndex, blockTextures, blockColor, false);
                    blockPlaced.draw(window);
                }
            }
        }

        // Если игрок проиграл, поигрываем анимацию
        if (state.gameOver) {
            if (state.jAnimGameOver < BOARD_HEIGHT) {
                if (state.animationGameOver >= 0.05f) {
                    int typeFigure = state.field[state.jAnimGameOver][state.iAnimGameOver];
                    if (typeFigure) {
                        int type = typeFigure - 1;

                        int textureIndex = 0;
                        if (type == Fig_L || type == Fig_J || type == Fig_S || type == Fig_Z) {
                            textureIndex = 1;
                        }

                        Block animBlock(state.iAnimGameOver, state.jAnimGameOver, textureIndex, blockTextures, sf::Color(206, 206, 206), false);
                        state.gameOverBlocks.push_back(animBlock);
                    }
                    state.iAnimGameOver += 1;
                    if (state.iAnimGameOver >= BOARD_WIDTH) {
                        state.iAnimGameOver = 0;
                        state.jAnimGameOver += 1;
                    }
                }
            } else if (state.restartClean < 3.0f) {
                state.restartClean += time;
            }
            else {
                if (!state.gameOverBlocks.empty()) {
                    state.gameOverBlocks.clear();
                }
                state.restartGameFromGameOver += time;
                for (int j = 0; j < BOARD_HEIGHT; j++) {
                    for (int i = 0; i < BOARD_WIDTH; i++) {
                        state.field[j][i] = 0;
                    }
                }

                if (state.restartGameFromGameOver >= 1.0f) {
                    init_game(audio, state, figure, blockTextures);
                }
            }
        }
        if (state.gameOver) {
            for (auto& block : state.gameOverBlocks) {
                block.draw(window);
            }
        }

        window.display();
    }
}