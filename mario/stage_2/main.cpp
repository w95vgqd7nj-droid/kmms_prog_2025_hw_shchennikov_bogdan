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

typedef struct SObject {
    float x;
    float y;
    float width;
    float height;
    float vertical_speed;
    float horizontal_speed;
    bool is_flying;
    char object_type;
} Object;

void clear_map(char map[MAP_HEIGHT][MAP_WIDTH+1]);
void create_level(int lvl, char map[MAP_HEIGHT][MAP_WIDTH+1], Object* mario, Object** brick, int* brick_counts, Object** moving_objects, int* moving_objects_count, int* player_score, float* camera_x, int max_level);
void delete_moving_objects(Object** moving_objects, int* moving_objects_count, int i);
void free_game_resources(Object** brick, Object** moving_objects);
Object* get_new_brick(Object** brick, int* brick_counts);
Object* get_new_moving_objects(Object** moving_objects, int* moving_objects_count);
void horizontal_move_map(Object* mario, float dx, Object** brick, int brick_counts, Object** moving_objects, int moving_objects_count, float* camera_x);
void horizontal_move_obj(Object* obj, Object** brick, int brick_counts, Object** moving_objects, int* moving_objects_count, int* current_level, int max_level, char map[MAP_HEIGHT][MAP_WIDTH+1], Object* mario, int* player_score, float* camera_x);
void init_object(Object* obj, float x_pos, float y_pos, float obj_width, float obj_height, char cur_type);
bool is_collision(const Object* obj_1, const Object* obj_2);
bool is_position_on_map(int x, int y);
void mario_collision(Object* mario, Object** moving_objects, int* moving_objects_count, int* player_score, int* current_level, char map[MAP_HEIGHT][MAP_WIDTH+1], Object** brick, int* brick_counts, float* camera_x, int max_level);
void player_dead(int* current_level, char map[MAP_HEIGHT][MAP_WIDTH+1], Object* mario, Object** brick, int* brick_counts, Object** moving_objects, int* moving_objects_count, int* player_score, float* camera_x, int max_level);
void put_object_on_map(char map[MAP_HEIGHT][MAP_WIDTH+1], const Object* obj, float camera_x);
void put_score_on_map(char map[MAP_HEIGHT][MAP_WIDTH+1], int player_score);
void set_object_pos(Object* obj, float x_pos, float y_pos);
void show_map(const char map[MAP_HEIGHT][MAP_WIDTH+1]);
void show_preview();
void vertical_move_object(Object* obj, Object** brick, int brick_counts, Object** moving_objects, int* moving_objects_count, int* current_level, int max_level, char map[MAP_HEIGHT][MAP_WIDTH+1], Object* mario, int* player_score, float* camera_x);

int main()
{
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

    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, true);

    show_preview();
    nodelay(stdscr, true);

    create_level(current_level, map, &mario, &brick, &brick_counts, &moving_objects, &moving_objects_count, &player_score, &camera_x, max_level);

    bool is_running = true;
    while(is_running) {
        clear_map(map);

        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 27) {
                is_running = false;
            }
            if (ch == ' ' && mario.is_flying == false) {
                mario.vertical_speed = JUMP_POWER;
            }
            if (ch == 'a' || ch == 'A' || ch == KEY_LEFT) {
                horizontal_move_map(&mario, 1, &brick, brick_counts, &moving_objects, moving_objects_count, &camera_x);
            }
            if (ch == 'd' || ch == 'D' || ch == KEY_RIGHT) {
                horizontal_move_map(&mario, -1, &brick, brick_counts, &moving_objects, moving_objects_count, &camera_x);
            }
        }

        if (!is_running) break;

        if (MAP_HEIGHT < mario.y || mario.y < 0)
        {
            player_dead(&current_level, map, &mario, &brick, &brick_counts, &moving_objects, &moving_objects_count, &player_score, &camera_x, max_level);
        }

        vertical_move_object(&mario, &brick, brick_counts, &moving_objects, &moving_objects_count, &current_level, max_level, map, &mario, &player_score, &camera_x);
        mario_collision(&mario, &moving_objects, &moving_objects_count, &player_score, &current_level, map, &brick, &brick_counts, &camera_x, max_level);

        for(int i = 0; i < brick_counts; i++)
        {
            put_object_on_map(map, &brick[i], camera_x);
        }

        for(int i = 0; i < moving_objects_count; i++)
        {
            vertical_move_object(&moving_objects[i], &brick, brick_counts, &moving_objects, &moving_objects_count, &current_level, max_level, map, &mario, &player_score, &camera_x);
            horizontal_move_obj(&moving_objects[i], &brick, brick_counts, &moving_objects, &moving_objects_count, &current_level, max_level, map, &mario, &player_score, &camera_x);
            put_object_on_map(map, &moving_objects[i], camera_x);
        }

        put_object_on_map(map, &mario, camera_x);
        put_score_on_map(map, player_score);

        clear();
        show_map(map);

        napms(FRAME_DELAY_MS);
    }

    free_game_resources(&brick, &moving_objects);
    endwin();

    return 0;
}

void clear_map(char map[MAP_HEIGHT][MAP_WIDTH+1]) {
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

void create_level(int lvl, char map[MAP_HEIGHT][MAP_WIDTH+1], Object* mario, Object** brick, int* brick_counts, Object** moving_objects, int* moving_objects_count, int* player_score, float* camera_x, int max_level)
{
    free_game_resources(brick, moving_objects);

    *brick_counts = 0;
    *moving_objects_count = 0;

    init_object(mario, MARIO_START_X, MARIO_START_Y, MARIO_WIDTH, MARIO_HEIGHT, TYPE_MARIO);
    *player_score = 0;
    *camera_x = 0.0f;

    if(lvl == 1)
    {
        init_object(get_new_brick(brick, brick_counts), 20, 20, 40, 5, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 30, 10, 5, 3, TYPE_BOX);
        init_object(get_new_brick(brick, brick_counts), 50, 10, 5, 3, TYPE_BOX);
        init_object(get_new_brick(brick, brick_counts), 60, 15, 40, 10, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 60, 5, 10, 3, TYPE_LINES);
        init_object(get_new_brick(brick, brick_counts), 70, 5, 5, 3, TYPE_BOX);
        init_object(get_new_brick(brick, brick_counts), 75, 5, 5, 3, TYPE_LINES);
        init_object(get_new_brick(brick, brick_counts), 80, 5, 5, 3, TYPE_BOX);
        init_object(get_new_brick(brick, brick_counts), 80, 5, 10, 3, TYPE_LINES);
        init_object(get_new_brick(brick, brick_counts), 100, 20, 20, 5, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 120, 15, 10, 10, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 150, 20, 40, 5, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 210, 15, 10, 10, TYPE_EXIT);

        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
    }

    if(lvl == 2)
    {
        init_object(get_new_brick(brick, brick_counts), 20, 20, 40, 5, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 30, 10, 5, 3, TYPE_BOX);
        init_object(get_new_brick(brick, brick_counts), 50, 10, 5, 3, TYPE_BOX);
        init_object(get_new_brick(brick, brick_counts), 60, 15, 10, 10, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 80, 20, 20, 5, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 120, 15, 10, 10, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 122, 5, 5, 3, TYPE_BOX);
        init_object(get_new_brick(brick, brick_counts), 150, 20, 40, 5, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 210, 15, 10, 10, TYPE_EXIT);

        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 65, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 120, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 160, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 175, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
    }

    if(lvl == 3)
    {
        init_object(get_new_brick(brick, brick_counts), 20, 20, 40, 5, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 80, 20, 15, 5, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 120, 15, 15, 10, TYPE_BRICK);
        init_object(get_new_brick(brick, brick_counts), 160, 10, 15, 15, TYPE_EXIT);

        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 25, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 50, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 80, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 90, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 120, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
        init_object(get_new_moving_objects(moving_objects, moving_objects_count), 130, 10, ENEMY_WIDTH, ENEMY_HEIGHT, TYPE_ENEMY);
    }
}

void delete_moving_objects(Object** moving_objects, int* moving_objects_count, int i)
{
    (*moving_objects_count)--;
    (*moving_objects)[i] = (*moving_objects)[*moving_objects_count];

    if(*moving_objects_count == 0)
    {
        free(*moving_objects);
        *moving_objects = NULL;
    }
    else
    {
        *moving_objects = (Object*)realloc(*moving_objects, sizeof(Object) * (*moving_objects_count));
    }
}

void free_game_resources(Object** brick, Object** moving_objects) {
    if (*brick != NULL)
    {
        free(*brick);
        *brick = NULL;
    }

    if (*moving_objects != NULL)
    {
        free(*moving_objects);
        *moving_objects = NULL;
    }
}

Object* get_new_brick(Object** brick, int* brick_counts)
{
    (*brick_counts)++;
    *brick = (Object*)realloc(*brick, sizeof(Object) * (*brick_counts));
    return (*brick) + (*brick_counts) - 1;
}

Object* get_new_moving_objects(Object** moving_objects, int* moving_objects_count)
{
    (*moving_objects_count)++;
    *moving_objects = (Object*)realloc(*moving_objects, sizeof(Object) * (*moving_objects_count));
    return (*moving_objects) + (*moving_objects_count) - 1;
}

void horizontal_move_map(Object* mario, float dx, Object** brick, int brick_counts, Object** moving_objects, int moving_objects_count, float* camera_x)
{
    float old_x = mario->x;
    mario->x -= dx;

    for (int i = 0; i < brick_counts; i++)
    {
        if (is_collision(mario, &(*brick)[i]))
        {
            mario->x = old_x;
            return;
        }
    }

    *camera_x = mario->x - MAP_WIDTH / 2.0f;

    if (*camera_x < 0)
    {
        *camera_x = 0;
    }

    for(int i = 0; i < brick_counts; i++)
    {
        (*brick)[i].x += dx;
    }

    for(int i = 0; i < moving_objects_count; i++)
    {
        (*moving_objects)[i].x += dx;
    }
}

void horizontal_move_obj(Object* obj, Object** brick, int brick_counts, Object** moving_objects, int* moving_objects_count, int* current_level, int max_level, char map[MAP_HEIGHT][MAP_WIDTH+1], Object* mario, int* player_score, float* camera_x)
{
    obj->x += obj->horizontal_speed;

    for (int i = 0; i < brick_counts; i++)
    {
        if (is_collision(obj, &(*brick)[i]))
        {
            obj->x -= obj->horizontal_speed;
            obj->horizontal_speed = -obj->horizontal_speed;
            return;
        }
    }

    if (obj->object_type == TYPE_ENEMY)
    {
        Object tmp = *obj;
        vertical_move_object(&tmp, brick, brick_counts, moving_objects, moving_objects_count, current_level, max_level, map, mario, player_score, camera_x);
        if(tmp.is_flying == true)
        {
            obj->x -= obj->horizontal_speed;
            obj->horizontal_speed = -obj->horizontal_speed;
        }
    }
}

void init_object(Object *obj, float x_pos, float y_pos, float obj_width, float obj_height, char cur_type){
    set_object_pos(obj, x_pos, y_pos);
    obj->width = obj_width;
    obj->height = obj_height;
    obj->vertical_speed = 0;
    obj->object_type = cur_type;
    obj->horizontal_speed = FRICTION;
}

bool is_collision(const Object* obj_1, const Object* obj_2)
{
    return ((obj_1->x + obj_1->width) > obj_2->x) && (obj_1->x < (obj_2->x + obj_2->width)) && ((obj_1->y + obj_1->height) > obj_2->y) && (obj_1->y < (obj_2->y + obj_2->height));
}

bool is_position_on_map(int x, int y)
{
    return ( (x >= 0) && (x < MAP_WIDTH) && (y >= 0) && (y < MAP_HEIGHT));
}

void mario_collision(Object* mario, Object** moving_objects, int* moving_objects_count, int* player_score, int* current_level, char map[MAP_HEIGHT][MAP_WIDTH+1], Object** brick, int* brick_counts, float* camera_x, int max_level)
{
    for(int i = 0; i < *moving_objects_count; i++)
    {
        if (is_collision(mario, &(*moving_objects)[i]))
        {
            if((*moving_objects)[i].object_type == TYPE_ENEMY)
            {
                float half_h = (*moving_objects)[i].height * 0.5f;
                if ((mario->is_flying == true)
                    && (mario->vertical_speed > 0)
                    && (mario->y + mario->height < (*moving_objects)[i].y + half_h))
                {
                    *player_score += SCORE_FOR_KILL;
                    delete_moving_objects(moving_objects, moving_objects_count, i);
                    i--;
                    continue;
                }
                else
                {
                    player_dead(current_level, map, mario, brick, brick_counts, moving_objects, moving_objects_count, player_score, camera_x, max_level);
                }
            }

            if((*moving_objects)[i].object_type == TYPE_COIN)
            {
                *player_score += SCORE_FOR_COIN;
                delete_moving_objects(moving_objects, moving_objects_count, i);
                i--;
                continue;
            }
        }
    }
}

void player_dead(int* current_level, char map[MAP_HEIGHT][MAP_WIDTH+1], Object* mario, Object** brick, int* brick_counts, Object** moving_objects, int* moving_objects_count, int* player_score, float* camera_x, int max_level)
{
    napms(RESTART_DELAY_MS);
    create_level(*current_level, map, mario, brick, brick_counts, moving_objects, moving_objects_count, player_score, camera_x, max_level);
}


void put_object_on_map(char map[MAP_HEIGHT][MAP_WIDTH+1], const Object* obj, float camera_x){

    const int ix = (int)round(obj->x - camera_x);
    const int iy = (int)round(obj->y);
    const int iWidth = (int)round(obj->width);
    const int iHeight = (int)round(obj->height);

    for(int i = ix; i < (ix + iWidth); i++)
    {
        for (int j = iy; j < (iy + iHeight); j++)
        {
            if (is_position_on_map(i, j) &&  j > 1)
            {
                map[j][i] = obj->object_type;
            }
        }
    }
}

void put_score_on_map(char map[MAP_HEIGHT][MAP_WIDTH+1], int player_score)
{
     for(int i = SCORE_X_OFFSET; i < SCORE_AREA_WIDTH; i++)
     {
        map[1][i] = ' ';
     }

    char c[30];
    snprintf(c, sizeof(c), "SCORE: %d", player_score);
    const int len = (int)strlen(c);
    for(int i = 0; i < len; i++)
    {
        map[1][i + SCORE_X_OFFSET] = c[i];
    }
}

void set_object_pos(Object *obj, float x_pos, float y_pos){
    obj->x = x_pos;
    obj->y = y_pos;
}

void show_map(const char map[MAP_HEIGHT][MAP_WIDTH+1]){
    for (int j = 0; j < MAP_HEIGHT; j++)
    {
        mvprintw(j, 0, "%s", map[j]);
    }
    refresh();
}

void show_preview()
{
    clear();
    printw("=== MAPUO HA CU ===\n");
    printw("YnpaB/eHue: A/D - gBu*eHue, npo6e/ - npbl*oK, ESC - BblXog\n");
    printw("Ha*MuTe /o6ylo K/aBuly g/q Ha4a/a...");
    refresh();
    nodelay(stdscr, false);
    getch();
}

void vertical_move_object(Object *obj, Object** brick, int brick_counts, Object** moving_objects, int* moving_objects_count, int* current_level, int max_level, char map[MAP_HEIGHT][MAP_WIDTH+1], Object* mario, int* player_score, float* camera_x)
{
    obj->is_flying = true;
    obj->vertical_speed += GRAVITY;
    set_object_pos(obj, obj->x, obj->y + obj->vertical_speed);

    for (int i = 0; i < brick_counts; i++)
    {
        if (is_collision(obj, &(*brick)[i]))
        {
            if (obj->vertical_speed > 0)
            {
                obj->is_flying = false;
            }

            if ( ((*brick)[i].object_type == TYPE_BOX) && (obj->vertical_speed < 0) && (obj == mario) )
            {
                (*brick)[i].object_type = '-';
                init_object(get_new_moving_objects(moving_objects, moving_objects_count), (*brick)[i].x, (*brick)[i].y-3, 3, 2, TYPE_COIN);
                (*moving_objects)[(*moving_objects_count) - 1].vertical_speed = COIN_DROP_SPEED;
            }
            obj->y -= obj->vertical_speed;
            obj->vertical_speed = 0;

            if((*brick)[i].object_type == TYPE_EXIT)
            {
                (*current_level)++;
                if (*current_level > max_level)
                {
                    *current_level = 1;
                }

                napms(RESTART_DELAY_MS);
                create_level(*current_level, map, mario, brick, &brick_counts, moving_objects, moving_objects_count, player_score, camera_x, max_level);
            }

            break;
        }
    }
}