#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

static const float COIN_DROP_SPEED = -0.7f;
static const float ENEMY_WIDTH   = 3.0f;
static const float ENEMY_HEIGHT  = 2.0f;
static const float FRICTION = 0.2f;
static const float GRAVITY = 0.03f;
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

typedef struct SObject{
    float x;
    float y;
    float width;
    float height;
    float vertical_speed;
    float horizontal_speed;
    bool is_flying;
    char object_type;
} Object;

Object mario;
Object *brick = NULL;
Object *moving_objects = NULL;
char map[MAP_HEIGHT][MAP_WIDTH+1];

float camera_x = 0.0f;
int brick_counts = 0;
int moving_objects_count = 0;
int max_level = 3;
int current_level = 1;
int player_score = 0;

void clear_map();
void create_level(int lvl);
void delete_moving_objects(int i);
void free_game_resources();
Object* get_new_brick();
Object* get_new_moving_objects();
void horizontal_move_map(float dx);
void horizontal_move_obj(Object* obj);
void init_object(Object* obj, float x_pos, float y_pos, float obj_width, float obj_height, char cur_type);
bool is_collision(Object obj_1, Object obj_2);
bool is_position_on_map(int x, int y);
void mario_collision();
void player_dead();
void put_object_on_map(Object obj);
void put_score_on_map();
void set_object_pos(Object* obj, float x_pos, float y_pos);
void show_map();
void show_preview();
void vertical_move_object(Object* obj);

int main()
{
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, true);

    show_preview();

    nodelay(stdscr, true);

    create_level(current_level);

    bool is_running = true;
    while(is_running) {
        clear_map();

        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 27) {
                is_running = false;
            }
            if (ch == ' ' && mario.is_flying == false) {
                mario.vertical_speed = JUMP_POWER;
            }
            if (ch == 'a' || ch == 'A' || ch == KEY_LEFT) {
                horizontal_move_map(1);
            }
            if (ch == 'd' || ch == 'D' || ch == KEY_RIGHT) {
                horizontal_move_map(-1);
            }
        }

        if (!is_running) break;

        if (MAP_HEIGHT < mario.y || mario.y < 0)
        {
            player_dead();
        }

        vertical_move_object(&mario);
        mario_collision();

        for(int i = 0; i < brick_counts; i++)
        {
            put_object_on_map(brick[i]);
        }
        for(int i = 0; i < moving_objects_count; i++)
        {
            vertical_move_object(moving_objects + i);
            horizontal_move_obj(moving_objects + i);
            put_object_on_map(moving_objects[i]);
        }

        put_object_on_map(mario);
        put_score_on_map();

        clear();
        show_map();

        napms(FRAME_DELAY_MS);
    }

    endwin();
    return 0;
}

void clear_map(){
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        map[0][i] = ' ';
    }
    map[0][MAP_WIDTH] = '\0';

    for (int i = 0; i < MAP_WIDTH; i++)
    {
        map[1][i] = ' ';
    }

    for (int j = 1; j < MAP_HEIGHT; j++)
    {
        strcpy(map[j], map[0]);
    }
}

void create_level(int lvl)
{
    free_game_resources();

    brick_counts = 0;
    moving_objects_count = 0;

    init_object(&mario, MARIO_START_X, MARIO_START_Y, MARIO_WIDTH, MARIO_HEIGHT, TYPE_MARIO);
    player_score = 0;
    camera_x = 0;

    if(lvl == 1)
    {
        init_object(get_new_brick(), 20, 20, 40, 5, TYPE_BRICK);
            init_object(get_new_brick(), 30, 10, 5, 3, TYPE_BOX);
            init_object(get_new_brick(), 50, 10, 5, 3, TYPE_BOX);
        init_object(get_new_brick(), 60, 15, 40, 10, TYPE_BRICK);
            init_object(get_new_brick(), 60, 5, 10, 3, TYPE_LINES);
            init_object(get_new_brick(), 70, 5, 5, 3, TYPE_BOX);
            init_object(get_new_brick(), 75, 5, 5, 3, TYPE_LINES);
            init_object(get_new_brick(), 80, 5, 5, 3, TYPE_BOX);
            init_object(get_new_brick(), 80, 5, 10, 3, TYPE_LINES);
        init_object(get_new_brick(), 100, 20, 20, 5, TYPE_BRICK);
        init_object(get_new_brick(), 120, 15, 10, 10, TYPE_BRICK);
        init_object(get_new_brick(), 150, 20, 40, 5, TYPE_BRICK);
        init_object(get_new_brick(), 210, 15, 10, 10, TYPE_EXIT);

        init_object(get_new_moving_objects(), 25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
    }

    if(lvl == 2)
    {
        init_object(get_new_brick(), 20, 20, 40, 5, TYPE_BRICK);
            init_object(get_new_brick(), 30, 10, 5, 3, TYPE_BOX);
            init_object(get_new_brick(), 50, 10, 5, 3, TYPE_BOX);
        init_object(get_new_brick(), 60, 15, 10, 10, TYPE_BRICK);
        init_object(get_new_brick(), 80, 20, 20, 5, TYPE_BRICK);
        init_object(get_new_brick(), 120, 15, 10, 10, TYPE_BRICK);
            init_object(get_new_brick(), 122, 5, 5, 3, TYPE_BOX);
        init_object(get_new_brick(), 150, 20, 40, 5, TYPE_BRICK);
        init_object(get_new_brick(), 210, 15, 10, 10, TYPE_EXIT);

        init_object(get_new_moving_objects(), 25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 65, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 120, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 160, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 175, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);

        init_object(get_new_moving_objects(), 25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
    }

    if(lvl == 3)
    {
        init_object(get_new_brick(), 20, 20, 40, 5, TYPE_BRICK);
        init_object(get_new_brick(), 80, 20, 15, 5, TYPE_BRICK);
        init_object(get_new_brick(), 120, 15, 15, 10, TYPE_BRICK);
        init_object(get_new_brick(), 160, 10, 15, 15, TYPE_EXIT);

        init_object(get_new_moving_objects(), 25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 50, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 90, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 120, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(), 130, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
    }
}

void delete_moving_objects(int i)
{
    moving_objects_count--;
    moving_objects[i] = moving_objects[moving_objects_count];

    if(moving_objects_count == 0)
    {
        free(moving_objects);
        moving_objects = NULL;
    }
    else
    {
        moving_objects = (Object*)realloc(moving_objects, sizeof(*moving_objects) * moving_objects_count);
    }
}

void free_game_resources() {
    if (brick != NULL)
    {
        free(brick);
        brick = NULL;
    }

    if (moving_objects != NULL)
    {
        free(moving_objects);
        moving_objects = NULL;
    }
}

Object *get_new_brick()
{
    brick_counts++;
    brick = (Object*)realloc(brick, sizeof(*brick) * brick_counts);
    return brick + brick_counts - 1;
}

Object *get_new_moving_objects()
{
    moving_objects_count++;
    moving_objects = (Object*)realloc(moving_objects, sizeof(*brick) * moving_objects_count);
    return moving_objects + moving_objects_count - 1;
}

void horizontal_move_map(float dx)
{
    float old_x = mario.x;
    mario.x -= dx;

    for (int i = 0; i < brick_counts; i++)
    {
        if (is_collision(mario, brick[i]))
        {
            mario.x = old_x;
            return;
        }
    }

    camera_x = mario.x - MAP_WIDTH / 2.0f;

    if (camera_x < 0)
    {
        camera_x = 0;
    }

    for(int i = 0; i < brick_counts; i++)
    {
        brick[i].x += dx;
    }

    for(int i = 0; i < moving_objects_count; i++)
    {
        moving_objects[i].x += dx;
    }
}

void horizontal_move_obj(Object *obj)
{
        (*obj).x += (*obj).horizontal_speed;

        for (int i = 0; i < brick_counts; i++)
        {
            if (is_collision(obj[0], brick[i]))
            {
            obj[0].x -= obj[0].horizontal_speed;
            obj[0].horizontal_speed = -obj[0].horizontal_speed;
            return;
            }
        }

        if (obj[0].object_type == TYPE_ENEMY)
        {
            Object tmp = *obj;
            vertical_move_object(&tmp);
            if(tmp.is_flying == true)
            {
                obj[0].x -= obj[0].horizontal_speed;
                obj[0].horizontal_speed = -obj[0].horizontal_speed;
            }
        }
}

void init_object(Object *obj, float x_pos, float y_pos, float obj_width, float obj_height, char cur_type){
    set_object_pos(obj, x_pos, y_pos);
    (*obj).width = obj_width;
    (*obj).height = obj_height;
    (*obj).vertical_speed = 0;
    (*obj).object_type = cur_type;
    (*obj).horizontal_speed = FRICTION;
}

bool is_collision(Object obj_1, Object obj_2)
{
    return ((obj_1.x + obj_1.width) > obj_2.x) && (obj_1.x < (obj_2.x + obj_2.width)) && ((obj_1.y + obj_1.height) > obj_2.y) && (obj_1.y < (obj_2.y + obj_2.height));
}

bool is_position_on_map(int x, int y)
{
    return ( (x >= 0) && (x < MAP_WIDTH) && (y >= 0) && (y < MAP_HEIGHT));
}

void mario_collision()
{
    for(int i = 0; i < moving_objects_count; i++)
    {
        if (is_collision(mario, moving_objects[i]))
        {
            if(moving_objects[i].object_type == TYPE_ENEMY)
            {
                if ( (mario.is_flying == true)
                    && (mario.vertical_speed > 0)
                    && (mario.y + mario.height < moving_objects[i].y + moving_objects[i].height * 0.5)
                )
                {
                    player_score += SCORE_FOR_KILL;
                    delete_moving_objects(i);
                    i--;
                    continue;
                }
                else
                {
                    player_dead();
                }
            }

            if(moving_objects[i].object_type == TYPE_COIN)
            {
                player_score += SCORE_FOR_COIN;
                delete_moving_objects(i);
                i--;
                continue;
            }
        }
    }
}

void player_dead()
{
    napms(RESTART_DELAY_MS);
    create_level(current_level);
}

void put_object_on_map(Object obj){

    const int ix = (int)round(obj.x - camera_x);
    const int iy = (int)round(obj.y);
    const int iWidth = (int)round(obj.width);
    const int iHeight = (int)round(obj.height);

    for(int i = ix; i < (ix + iWidth); i++)
    {
        for (int j = iy; j < (iy + iHeight); j++)
        {
            if (is_position_on_map(i, j) &&  j > 1)
            {
                map[j][i] = obj.object_type;
            }
        }
    }
}

void put_score_on_map()
{
     for(int i = SCORE_X_OFFSET; i < SCORE_AREA_WIDTH; i++)
     {
        map[1][i] = ' ';
     }

    char c[30];
    snprintf(c, sizeof(c), "player_score: %d", player_score);
    const int len = (int)strlen(c);
    for(int i = 0; i < len; i++)
    {
        map[1][i + SCORE_X_OFFSET] = c[i];
    }
}

void set_object_pos(Object *obj, float x_pos, float y_pos){
    (*obj).x = x_pos;
    (*obj).y = y_pos;
}

void show_map(){
    map[MAP_HEIGHT - 1][MAP_WIDTH - 1]= '\0';
    for (int j = 0; j < MAP_HEIGHT; j++)
    {
        mvprintw(j, 0, "%s", map[j]);
    }
    refresh();
}

void show_preview()
{
    clear();
    printw("=== МАРИО НА СИ ===\n");
    printw("Управление: A/D - движение, Пробел - прыжок, ESC - выход\n");
    printw("Нажмите любую клавишу для начала...");
    refresh();
    nodelay(stdscr, false);
    getch();
}

void vertical_move_object(Object *obj)
{
    (*obj).is_flying = true;
    (*obj).vertical_speed += GRAVITY;
    set_object_pos(obj, (*obj).x, (*obj).y + (*obj).vertical_speed);

    for (int i = 0; i < brick_counts; i++)
    {
        if (is_collision(*obj, brick[i]))
        {
            if (obj[0].vertical_speed > 0)
            {
                obj[0].is_flying = false;
            }

            if ( (brick[i].object_type == TYPE_BOX) && (obj[0].vertical_speed < 0) && (obj == &mario) )
            {
                brick[i].object_type = '-';
                init_object(get_new_moving_objects(), brick[i].x, brick[i].y-3, 3, 2, TYPE_COIN);
                moving_objects[moving_objects_count - 1].vertical_speed = COIN_DROP_SPEED;
            }
            (*obj).y -= (*obj).vertical_speed;
            (*obj).vertical_speed = 0;

            if(brick[i].object_type == TYPE_EXIT)
            {
                current_level++;
                if (current_level > max_level)
                {
                    current_level = 1;
                }

                napms(RESTART_DELAY_MS);
                create_level(current_level);
            }

            break;
        }
    }
}