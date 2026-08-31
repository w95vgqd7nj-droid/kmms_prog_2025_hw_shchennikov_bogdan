#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <vector>

static const float COIN_DROP_SPEED = -0.7f;
static const float ENEMY_WIDTH   = 3.0f;
static const float ENEMY_HEIGHT  = 2.0f;
static const float FRICTION = 0.2f;
static const float GRAVITY = 0.04f;
static const float MARIO_START_X = 39.0f;
static const float MARIO_START_Y = 10.0f;
static const float MARIO_WIDTH   = 3.0f;
static const float MARIO_HEIGHT  = 3.0f;
static const float JUMP_POWER = -1.0f;

static const int FRAME_DELAY_MS   = 10;
static const int RESTART_DELAY_MS = 500;

static const int MAP_HEIGHT = 25;
static const int MAP_WIDTH = 80;

static const int SCORE_AREA_WIDTH = 45;
static const int SCORE_FOR_COIN = 100;
static const int SCORE_FOR_KILL = 50;
static const int SCORE_X_OFFSET = 5;

static const char TYPE_BOX = '?';
static const char TYPE_BRICK = '#';
static const char TYPE_COIN = '$';
static const char TYPE_ENEMY = '^';
static const char TYPE_EXIT = '+';
static const char TYPE_LINES = '-';
static const char TYPE_MARIO = '@';

class GameObject {
public:
    float x;
    float y;
    float width;
    float height;
    float vertical_speed;
    float horizontal_speed;
    bool is_flying;
    char object_type;

    GameObject() {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
        vertical_speed = 0;
        horizontal_speed = FRICTION;
        is_flying = true;
        object_type = ' ';
    }

    GameObject(float x_pos, float y_pos, float obj_width, float obj_height, char cur_type) {
        x = x_pos;
        y = y_pos;
        width = obj_width;
        height = obj_height;
        vertical_speed = 0;
        horizontal_speed = FRICTION;
        is_flying = true;
        object_type = cur_type;
    }

    bool check_collision(const GameObject& other) const {
        return ((x + width) > other.x) &&
               (x < (other.x + other.width)) &&
               ((y + height) > other.y) &&
               (y < (other.y + other.height));
    }
};

class GameEngine {
private:
    GameObject mario;
    std::vector<GameObject> bricks;
    std::vector<GameObject> moving_objects;
    char map[MAP_HEIGHT][MAP_WIDTH+1];

    float camera_x;
    int max_level;
    int current_level;
    int player_score;

public:
    GameEngine() {
        camera_x = 0.0f;
        max_level = 3;
        current_level = 1;
        player_score = 0;
        create_level(current_level);
    }

    GameObject& get_mario() { return mario; }
    std::vector<GameObject>& get_bricks() { return bricks; }
    std::vector<GameObject>& get_moving_objects() { return moving_objects; }

    float get_camera_x() const { return camera_x; }
    void set_camera_x(float cx) { camera_x = cx; }

    int get_current_level() const { return current_level; }
    void set_current_level(int lvl) { current_level = lvl; }

    int get_max_level() const { return max_level; }

    void add_score(int s) { player_score += s; }

    void clear_map() {
        for (int i = 0; i < MAP_WIDTH; i++) {
            map[0][i] = ' ';
            map[1][i] = ' ';
        }
        map[0][MAP_WIDTH] = '\0';
        for (int j = 1; j < MAP_HEIGHT; j++) {
            strcpy(map[j], map[0]);
        }
    }

    void create_level(int lvl) {
        bricks.clear();
        moving_objects.clear();

        mario = GameObject(MARIO_START_X, MARIO_START_Y, MARIO_WIDTH, MARIO_HEIGHT, TYPE_MARIO);
        player_score = 0;
        camera_x = 0.0f;

        if(lvl == 1) {
            bricks.push_back(GameObject(20, 20, 40, 5, TYPE_BRICK));
            bricks.push_back(GameObject(30, 10, 5, 3, TYPE_BOX));
            bricks.push_back(GameObject(50, 10, 5, 3, TYPE_BOX));
            bricks.push_back(GameObject(60, 15, 40, 10, TYPE_BRICK));
            bricks.push_back(GameObject(60, 5, 10, 3, TYPE_LINES));
            bricks.push_back(GameObject(70, 5, 5, 3, TYPE_BOX));
            bricks.push_back(GameObject(75, 5, 5, 3, TYPE_LINES));
            bricks.push_back(GameObject(80, 5, 5, 3, TYPE_BOX));
            bricks.push_back(GameObject(80, 5, 10, 3, TYPE_LINES));
            bricks.push_back(GameObject(100, 20, 20, 5, TYPE_BRICK));
            bricks.push_back(GameObject(120, 15, 10, 10, TYPE_BRICK));
            bricks.push_back(GameObject(150, 20, 40, 5, TYPE_BRICK));
            bricks.push_back(GameObject(210, 15, 10, 10, TYPE_EXIT));

            moving_objects.push_back(GameObject(25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
        }

        if(lvl == 2) {
            bricks.push_back(GameObject(20, 20, 40, 5, TYPE_BRICK));
            bricks.push_back(GameObject(30, 10, 5, 3, TYPE_BOX));
            bricks.push_back(GameObject(50, 10, 5, 3, TYPE_BOX));
            bricks.push_back(GameObject(60, 15, 10, 10, TYPE_BRICK));
            bricks.push_back(GameObject(80, 20, 20, 5, TYPE_BRICK));
            bricks.push_back(GameObject(120, 15, 10, 10, TYPE_BRICK));
            bricks.push_back(GameObject(122, 5, 5, 3, TYPE_BOX));
            bricks.push_back(GameObject(150, 20, 40, 5, TYPE_BRICK));
            bricks.push_back(GameObject(210, 15, 10, 10, TYPE_EXIT));

            moving_objects.push_back(GameObject(25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(65, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(120, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(160, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(175, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
        }

        if(lvl == 3) {
            bricks.push_back(GameObject(20, 20, 40, 5, TYPE_BRICK));
            bricks.push_back(GameObject(80, 20, 15, 5, TYPE_BRICK));
            bricks.push_back(GameObject(120, 15, 15, 10, TYPE_BRICK));
            bricks.push_back(GameObject(160, 10, 15, 15, TYPE_EXIT));

            moving_objects.push_back(GameObject(25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(50, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(90, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(120, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
            moving_objects.push_back(GameObject(130, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY));
        }
    }

    bool is_position_on_map(int x, int y) const {
        return (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT);
    }

    void put_object_on_map(const GameObject& obj) {
        int ix = (int)round(obj.x - camera_x);
        int iy = (int)round(obj.y);
        int iWidth = (int)round(obj.width);
        int iHeight = (int)round(obj.height);

        for(int i = ix; i < (ix + iWidth); i++) {
            for (int j = iy; j < (iy + iHeight); j++) {
                if (is_position_on_map(i, j) && j > 1) {
                    map[j][i] = obj.object_type;
                }
            }
        }
    }

    void put_score_on_map() {
        for(int i = SCORE_X_OFFSET; i < SCORE_AREA_WIDTH; i++) {
            map[1][i] = ' ';
        }
        char c[30];
        snprintf(c, sizeof(c), "SCORE: %d", player_score);
        int len = (int)strlen(c);
        for(int i = 0; i < len; i++) {
            map[1][i + SCORE_X_OFFSET] = c[i];
        }
    }

    void show_map() const {
        for (int j = 0; j < MAP_HEIGHT; j++) {
            mvprintw(j, 0, "%s", map[j]);
        }
        refresh();
    }
};

void horizontal_move_map(float dx, GameEngine& engine);
void horizontal_move_obj(GameObject* obj, GameEngine& engine);
void mario_collision(GameEngine& engine);
void player_dead(GameEngine& engine);
void show_preview();
void vertical_move_object(GameObject* obj, GameEngine& engine);

int main()
{
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, true);

    show_preview();
    nodelay(stdscr, true);

    GameEngine engine;

    bool is_running = true;
    while(is_running) {
        engine.clear_map();

        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 27) {
                is_running = false;
            }
            if (ch == ' ' && engine.get_mario().is_flying == false) {
                engine.get_mario().vertical_speed = JUMP_POWER;
            }
            if (ch == 'a' || ch == 'A' || ch == KEY_LEFT) {
                horizontal_move_map(1, engine);
            }
            if (ch == 'd' || ch == 'D' || ch == KEY_RIGHT) {
                horizontal_move_map(-1, engine);
            }
        }

        if (!is_running) break;

        if (MAP_HEIGHT < engine.get_mario().y || engine.get_mario().y < 0) {
            player_dead(engine);
        }

        vertical_move_object(&engine.get_mario(), engine);
        mario_collision(engine);

        for(size_t i = 0; i < engine.get_bricks().size(); i++) {
            engine.put_object_on_map(engine.get_bricks()[i]);
        }

        for(size_t i = 0; i < engine.get_moving_objects().size(); i++) {
            vertical_move_object(&engine.get_moving_objects()[i], engine);
            horizontal_move_obj(&engine.get_moving_objects()[i], engine);
            engine.put_object_on_map(engine.get_moving_objects()[i]);
        }

        engine.put_object_on_map(engine.get_mario());
        engine.put_score_on_map();

        clear();
        engine.show_map();

        napms(FRAME_DELAY_MS);
    }

    endwin();
    return 0;
}

void horizontal_move_map(float dx, GameEngine& engine)
{
    GameObject& mario = engine.get_mario();
    std::vector<GameObject>& bricks = engine.get_bricks();
    std::vector<GameObject>& moving_objects = engine.get_moving_objects();

    float old_x = mario.x;
    mario.x -= dx;

    for (size_t i = 0; i < bricks.size(); i++) {
        if (mario.check_collision(bricks[i])) {
            mario.x = old_x;
            return;
        }
    }

    float new_camera_x = mario.x - MAP_WIDTH / 2.0f;
    if (new_camera_x < 0) {
        new_camera_x = 0;
    }
    engine.set_camera_x(new_camera_x);

    for(size_t i = 0; i < bricks.size(); i++) {
        bricks[i].x += dx;
    }

    for(size_t i = 0; i < moving_objects.size(); i++) {
        moving_objects[i].x += dx;
    }
}

void horizontal_move_obj(GameObject* obj, GameEngine& engine)
{
    std::vector<GameObject>& bricks = engine.get_bricks();

    obj->x += obj->horizontal_speed;

    for (size_t i = 0; i < bricks.size(); i++) {
        if (obj->check_collision(bricks[i])) {
            obj->x -= obj->horizontal_speed;
            obj->horizontal_speed = -obj->horizontal_speed;
            return;
        }
    }

    if (obj->object_type == TYPE_ENEMY) {
        GameObject tmp = *obj;
        vertical_move_object(&tmp, engine);
        if(tmp.is_flying == true) {
            obj->x -= obj->horizontal_speed;
            obj->horizontal_speed = -obj->horizontal_speed;
        }
    }
}

void mario_collision(GameEngine& engine)
{
    GameObject& mario = engine.get_mario();
    std::vector<GameObject>& moving_objects = engine.get_moving_objects();

    for(size_t i = 0; i < moving_objects.size(); i++) {
        if (mario.check_collision(moving_objects[i])) {
            if(moving_objects[i].object_type == TYPE_ENEMY) {
                float half_h = moving_objects[i].height * 0.5f;
                if ((mario.is_flying == true)
                    && (mario.vertical_speed > 0)
                    && (mario.y + mario.height < moving_objects[i].y + half_h))
                {
                    engine.add_score(SCORE_FOR_KILL);
                    moving_objects.erase(moving_objects.begin() + i);
                    i--;
                    continue;
                } else {
                    player_dead(engine);
                    return;
                }
            }

            if(moving_objects[i].object_type == TYPE_COIN) {
                engine.add_score(SCORE_FOR_COIN);
                moving_objects.erase(moving_objects.begin() + i);
                i--;
                continue;
            }
        }
    }
}

void player_dead(GameEngine& engine)
{
    napms(RESTART_DELAY_MS);
    engine.create_level(engine.get_current_level());
}

void show_preview()
{
    clear();
    printw("МАРИО НА C++\n");
    printw("Управление: A/D - движение, Пробел - прыжок, ESC - выход\n");
    printw("Нажмите любую клавишу для начала...");
    refresh();
    nodelay(stdscr, false);
    getch();
    nodelay(stdscr, true);
}

void vertical_move_object(GameObject *obj, GameEngine& engine)
{
    std::vector<GameObject>& bricks = engine.get_bricks();
    std::vector<GameObject>& moving_objects = engine.get_moving_objects();
    GameObject& mario = engine.get_mario();

    obj->is_flying = true;
    obj->vertical_speed += GRAVITY;
    obj->y += obj->vertical_speed;

    for (size_t i = 0; i < bricks.size(); i++) {
        if (obj->check_collision(bricks[i])) {
            if (obj->vertical_speed > 0) {
                obj->is_flying = false;
            }

            if ( (bricks[i].object_type == TYPE_BOX) && (obj->vertical_speed < 0) && (obj == &mario) ) {
                bricks[i].object_type = '-';
                GameObject coin(bricks[i].x, bricks[i].y-3, 3, 2, TYPE_COIN);
                coin.vertical_speed = COIN_DROP_SPEED;
                moving_objects.push_back(coin);
            }
            obj->y -= obj->vertical_speed;
            obj->vertical_speed = 0;

            if(bricks[i].object_type == TYPE_EXIT) {
                int next_lvl = engine.get_current_level() + 1;
                if (next_lvl > engine.get_max_level()) {
                    next_lvl = 1;
                }
                engine.set_current_level(next_lvl);

                napms(RESTART_DELAY_MS);
                engine.create_level(engine.get_current_level());
            }
            break;
        }
    }
}