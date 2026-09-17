#include <cmath>
#include <string>
#include <vector>
#include <ncurses.h>

namespace Config {
    const float MARIO_SPEED = 0.5f;
    const float COIN_DROP_SPEED = -0.7f;
    const float ENEMY_WIDTH = 3.0f;
    const float ENEMY_HEIGHT = 2.0f;
    const float FRICTION = 0.2f;
    const float GRAVITY = 0.045f;
    const float MARIO_START_X = 39.0f;
    const float MARIO_START_Y = 10.0f;
    const float MARIO_WIDTH = 3.0f;
    const float MARIO_HEIGHT = 3.0f;
    const float JUMP_POWER = -1.0f;
    const int FRAME_DELAY_MS = 10;
    const int RESTART_DELAY_MS = 500;
    const int MAP_HEIGHT = 25;
    const int MAP_WIDTH = 80;
    const int SCORE_AREA_WIDTH = 45;
    const int SCORE_FOR_COIN = 100;
    const int SCORE_FOR_KILL = 50;
    const int SCORE_X_OFFSET = 5;
    const char TYPE_BOX = '?';
    const char TYPE_BRICK = '#';
    const char TYPE_COIN = '$';
    const char TYPE_ENEMY = '^';
    const char TYPE_EXIT = '+';
    const char TYPE_LINES = '-';
    const char TYPE_MARIO = '@';
}

class BaseObject {
public:
    float x;
    float y;
    float width;
    float height;
    char object_type;

    BaseObject() {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
        object_type = ' ';
    }

    BaseObject(float x_pos, float y_pos, float obj_w, float obj_h, char type) {
        x = x_pos;
        y = y_pos;
        width = obj_w;
        height = obj_h;
        object_type = type;
    }

    virtual ~BaseObject() = default;

    bool checkCollision(const BaseObject& other) const {
        return x + width > other.x &&
               x < other.x + other.width &&
               y + height > other.y &&
               y < other.y + other.height;
    }
};

class MovableObject : public BaseObject {
public:
    float vertical_speed;
    float horizontal_speed;
    bool is_flying;

    MovableObject() : BaseObject() {
        vertical_speed = 0;
        horizontal_speed = Config::FRICTION;
        is_flying = true;
    }

    MovableObject(float x_pos, float y_pos, float obj_w, float obj_h, char t)
        : BaseObject(x_pos, y_pos, obj_w, obj_h, t) {
        vertical_speed = 0;
        horizontal_speed = Config::FRICTION;
        is_flying = true;
    }
};

class Board {
private:
    char map[Config::MAP_HEIGHT][Config::MAP_WIDTH + 1];

    bool isPositionOnMap(int x, int y) const {
        return x >= 0 && x < Config::MAP_WIDTH &&
               y >= 0 && y < Config::MAP_HEIGHT;
    }

public:
    void clearMap() {
        for (int i = 0; i < Config::MAP_WIDTH; i++) {
            map[0][i] = ' ';
            map[1][i] = ' ';
        }
        map[0][Config::MAP_WIDTH] = '\0';
        for (int j = 1; j < Config::MAP_HEIGHT; j++) {
            snprintf(map[j], sizeof(map[j]), "%s", map[0]);
        }
    }

    void drawObject(const BaseObject& obj, float camera_x) {
        int ix = (int)round(obj.x - camera_x);
        int iy = (int)round(obj.y);
        int iWidth = (int)round(obj.width);
        int iHeight = (int)round(obj.height);

        for (int i = ix; i < (ix + iWidth); i++) {
            for (int j = iy; j < (iy + iHeight); j++) {
                if (isPositionOnMap(i, j) && j > 1) {
                    map[j][i] = obj.object_type;
                }
            }
        }
    }

    void drawScore(int player_score) {
        for (int i = Config::SCORE_X_OFFSET; i < Config::SCORE_AREA_WIDTH; i++) {
            map[1][i] = ' ';
        }
        std::string score_str = "SCORE: " + std::to_string(player_score);
        for (size_t i = 0; i < score_str.length(); i++) {
            map[1][i + Config::SCORE_X_OFFSET] = score_str[i];
        }
    }

    void display() const {
        clear();
        for (int j = 0; j < Config::MAP_HEIGHT; j++) {
            mvprintw(j, 0, "%s", map[j]);
        }
        refresh();
    }
};

class GameEngine {
private:
    Board board;
    MovableObject mario;
    std::vector<BaseObject> bricks;
    std::vector<MovableObject> moving_objects;

    float camera_x;
    int max_level;
    int current_level;
    int player_score;
    bool is_running;
    float mario_dx;

    void applyVerticalPhysics(MovableObject& obj) {
        obj.is_flying = true;
        obj.vertical_speed += Config::GRAVITY;
        obj.y += obj.vertical_speed;

        for (size_t i = 0; i < bricks.size(); i++) {
            if (obj.checkCollision(bricks[i])) {
                if (obj.vertical_speed > 0) {
                    obj.is_flying = false;
                }

                if (bricks[i].object_type == Config::TYPE_BOX &&
                    obj.vertical_speed < 0 && &obj == &mario) {
                    bricks[i].object_type = '-';
                    MovableObject coin(bricks[i].x, bricks[i].y - 3, 3, 2,
                                       Config::TYPE_COIN);
                    coin.vertical_speed = Config::COIN_DROP_SPEED;
                    moving_objects.push_back(coin);
                }
                obj.y -= obj.vertical_speed;
                obj.vertical_speed = 0;

                if (bricks[i].object_type == Config::TYPE_EXIT) {
                    current_level++;
                    if (current_level > max_level) {
                        current_level = 1;
                    }
                    napms(Config::RESTART_DELAY_MS);
                    loadLevel(current_level);
                }
                break;
            }
        }
    }

    void applyHorizontalPhysics(MovableObject& obj) {
        obj.x += obj.horizontal_speed;
        for (size_t i = 0; i < bricks.size(); i++) {
            if (obj.checkCollision(bricks[i])) {
                obj.x -= obj.horizontal_speed;
                obj.horizontal_speed = -obj.horizontal_speed;
                return;
            }
        }
        if (obj.object_type == Config::TYPE_ENEMY) {
            MovableObject tmp = obj;
            applyVerticalPhysics(tmp);
            if (tmp.is_flying) {
                obj.x -= obj.horizontal_speed;
                obj.horizontal_speed = -obj.horizontal_speed;
            }
        }
    }

    void checkInteractions() {
        for (size_t i = 0; i < moving_objects.size(); i++) {
            if (mario.checkCollision(moving_objects[i])) {
                if (moving_objects[i].object_type == Config::TYPE_ENEMY) {
                    float half_h = moving_objects[i].height * 0.5f;
                    if (mario.is_flying && mario.vertical_speed > 0 &&
                        mario.y + mario.height < moving_objects[i].y + half_h) {
                        player_score += Config::SCORE_FOR_KILL;
                        moving_objects.erase(moving_objects.begin() + i);
                        i--;
                        continue;
                    } else {
                        playerDead();
                        return;
                    }
                }
                if (moving_objects[i].object_type == Config::TYPE_COIN) {
                    player_score += Config::SCORE_FOR_COIN;
                    moving_objects.erase(moving_objects.begin() + i);
                    i--;
                    continue;
                }
            }
        }
    }

    void playerDead() {
        napms(Config::RESTART_DELAY_MS);
        loadLevel(current_level);
    }

    void moveMapHorizontally(float dx) {
        float old_x = mario.x;
        mario.x += dx;

        for (size_t i = 0; i < bricks.size(); i++) {
            if (mario.checkCollision(bricks[i])) {
                mario.x = old_x;
                mario_dx = 0.0f;
                return;
            }
        }

        camera_x = mario.x - Config::MAP_WIDTH / 2.0f;
        if (camera_x < 0) camera_x = 0;
    }

    void loadLevel(int lvl) {
        bricks.clear();
        moving_objects.clear();
        mario = MovableObject(Config::MARIO_START_X, Config::MARIO_START_Y,
                              Config::MARIO_WIDTH, Config::MARIO_HEIGHT,
                              Config::TYPE_MARIO);
        player_score = 0;
        camera_x = 0.0f;
        mario_dx = 0.0f;

        if (lvl == 1) {
            bricks.push_back(BaseObject(20, 20, 40, 5, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(30, 10, 5, 3, Config::TYPE_BOX));
            bricks.push_back(BaseObject(50, 10, 5, 3, Config::TYPE_BOX));
            bricks.push_back(BaseObject(60, 15, 40, 10, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(60, 5, 10, 3, Config::TYPE_LINES));
            bricks.push_back(BaseObject(70, 5, 5, 3, Config::TYPE_BOX));
            bricks.push_back(BaseObject(75, 5, 5, 3, Config::TYPE_LINES));
            bricks.push_back(BaseObject(80, 5, 5, 3, Config::TYPE_BOX));
            bricks.push_back(BaseObject(80, 5, 10, 3, Config::TYPE_LINES));
            bricks.push_back(BaseObject(100, 20, 20, 5, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(120, 15, 10, 10, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(150, 20, 40, 5, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(210, 15, 10, 10, Config::TYPE_EXIT));

            moving_objects.push_back(MovableObject(
                25, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                80, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
        }

        if (lvl == 2) {
            bricks.push_back(BaseObject(20, 20, 40, 5, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(30, 10, 5, 3, Config::TYPE_BOX));
            bricks.push_back(BaseObject(50, 10, 5, 3, Config::TYPE_BOX));
            bricks.push_back(BaseObject(60, 15, 10, 10, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(80, 20, 20, 5, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(120, 15, 10, 10, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(122, 5, 5, 3, Config::TYPE_BOX));
            bricks.push_back(BaseObject(150, 20, 40, 5, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(210, 15, 10, 10, Config::TYPE_EXIT));

            moving_objects.push_back(MovableObject(
                25, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                80, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                65, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                120, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                160, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                175, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                25, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                80, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
        }

        if (lvl == 3) {
            bricks.push_back(BaseObject(20, 20, 40, 5, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(80, 20, 15, 5, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(120, 15, 15, 10, Config::TYPE_BRICK));
            bricks.push_back(BaseObject(160, 10, 15, 15, Config::TYPE_EXIT));

            moving_objects.push_back(MovableObject(
                25, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                50, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                80, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                90, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                120, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
            moving_objects.push_back(MovableObject(
                130, 10, Config::ENEMY_WIDTH, Config::ENEMY_HEIGHT,
                Config::TYPE_ENEMY));
        }
    }

    void update() {
        if (Config::MAP_HEIGHT < mario.y || mario.y < 0) {
            playerDead();
            return;
        }

        applyVerticalPhysics(mario);
        checkInteractions();

        for (size_t i = 0; i < moving_objects.size(); i++) {
            applyVerticalPhysics(moving_objects[i]);
            applyHorizontalPhysics(moving_objects[i]);
        }
    }

    void render() {
        board.clearMap();

        for (size_t i = 0; i < bricks.size(); i++) {
            board.drawObject(bricks[i], camera_x);
        }
        for (size_t i = 0; i < moving_objects.size(); i++) {
            board.drawObject(moving_objects[i], camera_x);
        }
        board.drawObject(mario, camera_x);
        board.drawScore(player_score);

        board.display();
    }

public:
    GameEngine() {
        camera_x = 0.0f;
        max_level = 3;
        current_level = 1;
        player_score = 0;
        is_running = true;
        mario_dx = 0.0f;
        loadLevel(current_level);
    }

    void run() {
        while (is_running) {
            int ch;
            while ((ch = getch()) != ERR) {
                if (ch == 27) {
                    is_running = false;
                }
                if (ch == ' ' && !mario.is_flying) {
                    mario.vertical_speed = Config::JUMP_POWER;
                }
                if (ch == 'a' || ch == 'A' || ch == KEY_LEFT) {
                    mario_dx = -Config::MARIO_SPEED;
                }
                if (ch == 'd' || ch == 'D' || ch == KEY_RIGHT) {
                    mario_dx = Config::MARIO_SPEED;
                }
                if (ch == 's' || ch == 'S' || ch == KEY_DOWN) {
                    mario_dx = 0.0f;
                }
            }

            if (!is_running) break;

            if (mario_dx != 0.0f) {
                moveMapHorizontally(mario_dx);
            }

            update();
            render();

            napms(Config::FRAME_DELAY_MS);
        }
    }
};

void showPreview() {
    clear();
    printw("МАРИО НА C++\n");
    printw("Управление: A/D - непрерывное движение, S - стоп, Пробел - прыжок, ESC\n");
    printw("Нажмите любую клавишу для начала...");
    refresh();
    nodelay(stdscr, false);
    getch();
    nodelay(stdscr, true);
}

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, true);

    showPreview();

    GameEngine engine;
    engine.run();

    endwin();
    return 0;
}