#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <ncurses.h>

static const float FRICTION = 0.2f;
static const float GRAVITY = 0.05f;
static const float JUMP_POWER = -1.0f;
static const int MAX_LEVEL = 3;

static const int MAP_WIDTH = 80;
static const int MAP_HEIGHT = 25;

static const char TYPE_BRICK = '#';
static const char TYPE_COIN = '$';
static const char TYPE_ENEMY = '^';
static const char TYPE_EXIT = '+';
static const char TYPE_MARIO = '@';
static const char TYPE_BOX = '?';
static const char TYPE_LINES = '-';

typedef struct {
    float x, y;
    float width, height;
    float vertical_speed;
    float horizontal_speed;
    bool is_flying;
    char object_type;
} Object;

char map[MAP_HEIGHT][MAP_WIDTH+1];
Object mario;

Object *brick = NULL;
int brick_counts = 0;

Object *moving_objects = NULL;
int moving_objects_count = 0;

float camera_x = 0.0f;
int current_level = 1;
int player_score = 0;

void ClearMap() {
    for (int i = 0; i < MAP_WIDTH; i++)
        map[0][i] = ' ';
    map[0][MAP_WIDTH] = '\0';
    for (int j = 1; j < MAP_HEIGHT; j++)
        strcpy(map[j], map[0]);
}

void ShowMap() {
    map[MAP_HEIGHT - 1][MAP_WIDTH - 1] = '\0';
    for (int j = 0; j < MAP_HEIGHT; j++)
        mvprintw(j, 0, "%s", map[j]);
    refresh();
}

void SetObjectPos(Object *obj, float xPos, float yPos) {
    (*obj).x = xPos;
    (*obj).y = yPos;
}

void InitObject(Object *obj, float xPos, float yPos, float oWidth, float oHeight, char inType) {
    SetObjectPos(obj, xPos, yPos);
    (*obj).width = oWidth;
    (*obj).height = oHeight;
    (*obj).vertical_speed = 0;
    (*obj).horizontal_speed = FRICTION;
    (*obj).is_flying = true;
    (*obj).object_type = inType;
}

void CreateLevel(int lvl);

void PlayerDead() {
    napms(500);
    CreateLevel(current_level);
}

bool IsCollision(Object o1, Object o2);
Object *GetNewMovingObject();

void VertMoveObject(Object *obj) {
    (*obj).is_flying = true;
    (*obj).vertical_speed += GRAVITY;
    SetObjectPos(obj, (*obj).x, (*obj).y + (*obj).vertical_speed);

    for (int i = 0; i < brick_counts; i++) {
        if (IsCollision(*obj, brick[i])) {
            if (obj[0].vertical_speed > 0)
                obj[0].is_flying = false;

            if ((brick[i].object_type == TYPE_BOX) && (obj[0].vertical_speed < 0) && (obj == &mario)) {
                brick[i].object_type = TYPE_LINES;
                InitObject(GetNewMovingObject(), brick[i].x, brick[i].y - 3, 3, 2, TYPE_COIN);
                moving_objects[moving_objects_count - 1].vertical_speed = -0.7f;
            }

            (*obj).y -= (*obj).vertical_speed;
            (*obj).vertical_speed = 0;

            if (brick[i].object_type == TYPE_EXIT) {
                current_level++;
                if (current_level > MAX_LEVEL) current_level = 1;
                napms(500);
                CreateLevel(current_level);
            }
            break;
        }
    }
}

void DeleteMovingObject(int i) {
    moving_objects_count--;
    moving_objects[i] = moving_objects[moving_objects_count];

    if (moving_objects_count == 0) {
        free(moving_objects);
        moving_objects = NULL;
    } else {
        moving_objects = (Object*)realloc(moving_objects, sizeof(*moving_objects) * moving_objects_count);
    }
}

void MarioCollision() {
    for (int i = 0; i < moving_objects_count; i++) {
        if (IsCollision(mario, moving_objects[i])) {

            if (moving_objects[i].object_type == TYPE_ENEMY) {
                if ((mario.is_flying == true) &&
                    (mario.vertical_speed > 0) &&
                    (mario.y + mario.height < moving_objects[i].y + moving_objects[i].height * 0.5f)) {

                    player_score += 50;
                    DeleteMovingObject(i);
                    i--;
                    continue;
                } else {
                    PlayerDead();
                }
            }

            if (moving_objects[i].object_type == TYPE_COIN) {
                player_score += 100;
                DeleteMovingObject(i);
                i--;
                continue;
            }
        }
    }
}

void HorizonMoveObj(Object *obj) {
    (*obj).x += (*obj).horizontal_speed;

    for (int i = 0; i < brick_counts; i++) {
        if (IsCollision(obj[0], brick[i])) {
            obj[0].x -= obj[0].horizontal_speed;
            obj[0].horizontal_speed = -obj[0].horizontal_speed;
            return;
        }
    }

    if (obj[0].object_type == TYPE_ENEMY) {
        Object tmp = *obj;
        VertMoveObject(&tmp);
        if (tmp.is_flying == true) {
            obj[0].x -= obj[0].horizontal_speed;
            obj[0].horizontal_speed = -obj[0].horizontal_speed;
        }
    }
}

bool IsPositionMap(int x, int y) {
    return ((x >= 0) && (x < MAP_WIDTH) && (y >= 0) && (y < MAP_HEIGHT));
}

void PutObjectOnMap(Object obj) {
    int ix = (int)round(obj.x - camera_x);
    int iy = (int)round(obj.y);
    int iWidth = (int)round(obj.width);
    int iHeight = (int)round(obj.height);

    for (int i = ix; i < (ix + iWidth); i++)
        for (int j = iy; j < (iy + iHeight); j++)
            if (IsPositionMap(i, j))
                map[j][i] = obj.object_type;
}

void HorizonMoveMap(float dx) {
    float oldX = mario.x;
    mario.x -= dx;

    for (int i = 0; i < brick_counts; i++) {
        if (IsCollision(mario, brick[i])) {
            mario.x = oldX;
            return;
        }
    }

    camera_x = mario.x - MAP_WIDTH / 2.0f;
    if (camera_x < 0) camera_x = 0;

    for (int i = 0; i < brick_counts; i++)
        brick[i].x += dx;
    for (int i = 0; i < moving_objects_count; i++)
        moving_objects[i].x += dx;
}

bool IsCollision(Object o1, Object o2) {
    return ((o1.x + o1.width) > o2.x) && (o1.x < (o2.x + o2.width)) &&
           ((o1.y + o1.height) > o2.y) && (o1.y < (o2.y + o2.height));
}

Object *GetNewBrick() {
    brick_counts++;
    brick = (Object*)realloc(brick, sizeof(*brick) * brick_counts);
    return brick + brick_counts - 1;
}

Object *GetNewMovingObject() {
    moving_objects_count++;
    moving_objects = (Object*)realloc(moving_objects, sizeof(*brick) * moving_objects_count);
    return moving_objects + moving_objects_count - 1;
}

void PutScoreOnMap() {
    char c[30];
    snprintf(c, sizeof(c), "player_score: %d", player_score);
    int len = strlen(c);
    for (int i = 0; i < len; i++) {
        map[1][i + 5] = c[i];
    }
}

void CreateLevel(int lvl) {
    if (brick != NULL) {
        free(brick);
        brick = NULL;
    }
    brick_counts = 0;

    if (moving_objects != NULL) {
        free(moving_objects);
        moving_objects = NULL;
    }
    moving_objects_count = 0;

    InitObject(&mario, 39, 10, 3, 3, TYPE_MARIO);
    player_score = 0;
    camera_x = 0;

    if (lvl == 1) {
        InitObject(GetNewBrick(), 20, 20, 40, 5, TYPE_BRICK);
        InitObject(GetNewBrick(), 30, 10, 5, 3, TYPE_BOX);
        InitObject(GetNewBrick(), 50, 10, 5, 3, TYPE_BOX);
        InitObject(GetNewBrick(), 60, 15, 40, 10, TYPE_BRICK);
        InitObject(GetNewBrick(), 60, 5, 10, 3, TYPE_LINES);
        InitObject(GetNewBrick(), 70, 5, 5, 3, TYPE_BOX);
        InitObject(GetNewBrick(), 75, 5, 5, 3, TYPE_LINES);
        InitObject(GetNewBrick(), 80, 5, 5, 3, TYPE_BOX);
        InitObject(GetNewBrick(), 80, 5, 10, 3, TYPE_LINES);
        InitObject(GetNewBrick(), 100, 20, 20, 5, TYPE_BRICK);
        InitObject(GetNewBrick(), 120, 15, 10, 10, TYPE_BRICK);
        InitObject(GetNewBrick(), 150, 20, 40, 5, TYPE_BRICK);
        InitObject(GetNewBrick(), 210, 15, 10, 10, TYPE_EXIT);

        InitObject(GetNewMovingObject(), 25, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 80, 10, 3, 2, TYPE_ENEMY);
    }

    if (lvl == 2) {
        InitObject(GetNewBrick(), 20, 20, 40, 5, TYPE_BRICK);
        InitObject(GetNewBrick(), 30, 10, 5, 3, TYPE_BOX);
        InitObject(GetNewBrick(), 50, 10, 5, 3, TYPE_BOX);
        InitObject(GetNewBrick(), 60, 15, 10, 10, TYPE_BRICK);
        InitObject(GetNewBrick(), 80, 20, 20, 5, TYPE_BRICK);
        InitObject(GetNewBrick(), 120, 15, 10, 10, TYPE_BRICK);
        InitObject(GetNewBrick(), 122, 5, 5, 3, TYPE_BOX);
        InitObject(GetNewBrick(), 150, 20, 40, 5, TYPE_BRICK);
        InitObject(GetNewBrick(), 210, 15, 10, 10, TYPE_EXIT);

        InitObject(GetNewMovingObject(), 25, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 80, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 65, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 120, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 160, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 175, 10, 3, 2, TYPE_ENEMY);
    }

    if (lvl == 3) {
        InitObject(GetNewBrick(), 20, 20, 40, 5, TYPE_BRICK);
        InitObject(GetNewBrick(), 80, 20, 15, 5, TYPE_BRICK);
        InitObject(GetNewBrick(), 120, 15, 15, 10, TYPE_BRICK);
        InitObject(GetNewBrick(), 160, 10, 15, 15, TYPE_EXIT);

        InitObject(GetNewMovingObject(), 25, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 50, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 80, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 90, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 120, 10, 3, 2, TYPE_ENEMY);
        InitObject(GetNewMovingObject(), 130, 10, 3, 2, TYPE_ENEMY);
    }
}

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    nodelay(stdscr, true);
    keypad(stdscr, true);

    CreateLevel(current_level);

    bool isRunning = true;
    while (isRunning) {
        ClearMap();

        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 27) {
                isRunning = false;
            }
            if (ch == ' ' && mario.is_flying == false) {
                mario.vertical_speed = JUMP_POWER;
            }
            if (ch == 'a' || ch == 'A' || ch == KEY_LEFT) {
                HorizonMoveMap(1);
            }
            if (ch == 'd' || ch == 'D' || ch == KEY_RIGHT) {
                HorizonMoveMap(-1);
            }
        }

        if (!isRunning) break;

        if (MAP_HEIGHT < mario.y || mario.y < 0) {
            PlayerDead();
        }

        VertMoveObject(&mario);
        MarioCollision();

        for (int i = 0; i < brick_counts; i++)
            PutObjectOnMap(brick[i]);

        for (int i = 0; i < moving_objects_count; i++) {
            VertMoveObject(moving_objects + i);
            HorizonMoveObj(moving_objects + i);
            PutObjectOnMap(moving_objects[i]);
        }

        PutObjectOnMap(mario);
        PutScoreOnMap();

        clear();
        ShowMap();

        napms(10);
    }

    endwin();
    return 0;
}