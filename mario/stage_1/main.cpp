#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <ncurses.h>

#define mapWidth 80
#define mapHeight 25

typedef struct SObject{
    float x, y;
    float width, height;
    float vertSpeed;
    bool IsFly;
    char cType;
    float horizSpeed;
} TObject;

char map[mapHeight][mapWidth+1];
TObject mario;

TObject *brick = NULL;
int brickLength;

TObject *moving = NULL;
int movingLength;

float cameraX = 0;

int level = 1;
int score;
int maxlvl;

void ClearMap(){
    for (int i = 0; i < mapWidth; i++)
        map[0][i] = ' ';
    map[0][mapWidth] = '\0';
    for (int j = 1; j < mapHeight; j++)
        strcpy(map[j], map[0]);
}

void ShowMap(){
    map[mapHeight - 1][mapWidth - 1]= '\0';
    for (int j = 0; j < mapHeight; j++)
        mvprintw(j, 0, "%s", map[j]);
    refresh();
}

void SetObjectPos(TObject *obj, float xPos, float yPos){
    (*obj).x = xPos;
    (*obj).y = yPos;
}

void InitObject(TObject *obj, float xPos, float yPos, float oWidth, float oHeight, char inType){
    SetObjectPos(obj, xPos, yPos);
    (*obj).width = oWidth;
    (*obj).height = oHeight;
    (*obj).vertSpeed = 0;
    (*obj).cType = inType;
    (*obj).horizSpeed = 0.2;
}

void CreateLevel(int lvl);

void PlayerDead()
{
    napms(500);
    CreateLevel(level);
}

bool IsCollision(TObject o1, TObject o2);
TObject *GetNewMoving();

void VertMoveObject(TObject *obj)
{
    (*obj).IsFly = true;
    (*obj).vertSpeed += 0.05;
    SetObjectPos(obj, (*obj).x, (*obj).y + (*obj).vertSpeed);

    for (int i = 0; i < brickLength; i++)
        if (IsCollision(*obj, brick[i]))
        {
            if (obj[0].vertSpeed > 0)
                obj[0].IsFly = false;

            if ( (brick[i].cType == '?') && (obj[0].vertSpeed < 0) && (obj == &mario) )
            {
                brick[i].cType = '-';
                InitObject(GetNewMoving(), brick[i].x, brick[i].y-3, 3, 2, '$');
                moving[movingLength - 1].vertSpeed = -0.7;
            }
            (*obj).y -= (*obj).vertSpeed;
            (*obj).vertSpeed = 0;

            if(brick[i].cType == '+')
            {
                level++;
                if (level > 3) level = 1;
                if (level > maxlvl) level = 1;

                napms(500);
                CreateLevel(level);
                napms(1000);
            }

            break;
        }
}

void DeleteMoving(int i)
{
    movingLength--;
    moving[i] = moving[movingLength];

    if(movingLength == 0)
    {
        free(moving);
        moving = NULL;
    }
    else
    {
        moving = (TObject*)realloc(moving, sizeof(*moving) * movingLength);
    }
}

void MarioCollision()
{
    for(int i = 0; i < movingLength; i++)
        if (IsCollision(mario, moving[i]))
        {
            if ( (mario.IsFly == true)
                && (mario.vertSpeed > 0)
                && (mario.y + mario.height < moving[i].y + moving[i].height * 0.5)
            )
            if(moving[i].cType == 'o')
            {
                if ( (mario.IsFly == true)
                    && (mario.vertSpeed > 0)
                    && (mario.y + mario.height < moving[i].y + moving[i].height * 0.5)
                )
                {
                    score += 50;
                    DeleteMoving(i);
                    i--;
                    continue;
                }
                else
                    PlayerDead();
            }

            if(moving[i].cType == '$')
            {
                score += 100;
                DeleteMoving(i);
                i--;
                continue;
            }
            else
                CreateLevel(level);
        }
}

void HorizonMoveObj(TObject *obj)
{
    (*obj).x += (*obj).horizSpeed;

    for (int i = 0; i < brickLength; i++)
        if (IsCollision(obj[0], brick[i]))
        {
            obj[0].x -= obj[0].horizSpeed;
            obj[0].horizSpeed = -obj[0].horizSpeed;
            return;
        }

    TObject tmp = *obj;
    VertMoveObject(&tmp);
    if(tmp.IsFly == true)
    {
        if (obj[0].cType == 'o')
        {
            obj[0].x -= obj[0].horizSpeed;
            obj[0].horizSpeed = -obj[0].horizSpeed;
            TObject tmp2 = *obj;
            VertMoveObject(&tmp2);
            if(tmp2.IsFly == true)
            {
                obj[0].x -= obj[0].horizSpeed;
                obj[0].horizSpeed = -obj[0].horizSpeed;
            }
        }
    }
}

bool IsPositionMap(int x, int y)
{
    return ( (x >= 0) && (x < mapWidth) && (y >= 0) && (y < mapHeight));
}

void PutObjectOnMap(TObject obj){
    int ix = (int)round(obj.x - cameraX);
    int iy = (int)round(obj.y);
    int iWidth = (int)round(obj.width);
    int iHeight = (int)round(obj.height);

    for(int i = ix; i < (ix + iWidth); i++)
        for (int j = iy; j < (iy + iHeight); j++)
            if (IsPositionMap(i, j))
                map[j][i] = obj.cType;
}

void HorizonMoveMap(float dx)
{
    float oldX = mario.x;
    mario.x -= dx;

    for (int i = 0; i < brickLength; i++)
        if (IsCollision(mario, brick[i]))
        {
            mario.x = oldX;
            return;
        }

    cameraX = mario.x - mapWidth / 2;

    if (cameraX < 0) cameraX = 0;

    for(int i = 0; i < brickLength; i++)
        brick[i].x += dx;
    for(int i = 0; i < movingLength; i++)
        moving[i].x += dx;
}

bool IsCollision(TObject o1, TObject o2)
{
    return ((o1.x + o1.width) > o2.x) && (o1.x < (o2.x + o2.width)) && ((o1.y + o1.height) > o2.y) && (o1.y < (o2.y + o2.height));
}

TObject *GetNewBrick()
{
    brickLength++;
    brick = (TObject*)realloc(brick, sizeof(*brick) * brickLength);
    return brick + brickLength - 1;
}

TObject *GetNewMoving()
{
    movingLength++;
    moving = (TObject*)realloc(moving, sizeof(*moving) * movingLength);
    return moving + movingLength - 1;
}

void PutScoreOnMap()
{
    char c[30];
    snprintf(c, sizeof(c), "Score: %d", score);
    int len = strlen(c);
    for(int i = 0; i < len; i++)
    {
        map[1][i+5] = c[i];
    }
}

void CreateLevel(int lvl)
{
    if (brick != NULL)
    {
        free(brick);
        brick = NULL;
    }
    brickLength = 0;

    if (moving != NULL)
    {
        free(moving);
        moving = NULL;
    }
    movingLength = 0;

    InitObject(&mario, 39, 10, 3, 3, '@');

    score = 0;
    cameraX = 0;

    if(lvl == 1)
    {
        brickLength = 0;

        InitObject(GetNewBrick(), 20, 20, 40, 5, '#');
        InitObject(GetNewBrick(), 60, 15, 10, 10, '#');
            InitObject(GetNewBrick(), 30, 10, 5, 3, '?');
            InitObject(GetNewBrick(), 50, 10, 5, 3, '?');
        InitObject(GetNewBrick(), 80, 20, 20, 5, '#');
        InitObject(GetNewBrick(), 60, 15, 40, 10, '#');
            InitObject(GetNewBrick(), 60, 5, 10, 3, '-');
            InitObject(GetNewBrick(), 70, 5, 5, 3, '?');
            InitObject(GetNewBrick(), 75, 5, 5, 3, '-');
            InitObject(GetNewBrick(), 80, 5, 5, 3, '?');
            InitObject(GetNewBrick(), 80, 5, 10, 3, '-');
        InitObject(GetNewBrick(), 100, 20, 20, 5, '#');
        InitObject(GetNewBrick(), 120, 15, 10, 10, '#');
        InitObject(GetNewBrick(), 150, 20, 40, 5, '#');
        InitObject(GetNewBrick(), 210, 15, 10, 10, '+');

        InitObject(GetNewMoving(), 25, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 80, 10, 3, 2, 'o');
    }

    if(lvl == 2)
    {
        brickLength = 0;

        InitObject(GetNewBrick(), 20, 20, 40, 5, '#');
        InitObject(GetNewBrick(), 60, 15, 10, 10, '#');
        InitObject(GetNewBrick(), 80, 20, 20, 5, '#');
        InitObject(GetNewBrick(), 120, 15, 10, 10, '#');
        InitObject(GetNewBrick(), 150, 20, 40, 5, '#');
        InitObject(GetNewBrick(), 210, 15, 10, 10, '+');

        movingLength = 0;
        InitObject(GetNewMoving(), 25, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 80, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 65, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 120, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 160, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 175, 10, 3, 2, 'o');

        InitObject(GetNewMoving(), 25, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 80, 10, 3, 2, 'o');
    }

    if(lvl == 3)
    {
        brickLength = 0;

        InitObject(GetNewBrick(), 20, 20, 40, 5, '#');
        InitObject(GetNewBrick(), 80, 20, 15, 5, '#');
        InitObject(GetNewBrick(), 120, 15, 15, 10, '#');
        InitObject(GetNewBrick(), 160, 10, 15, 15, '+');

        InitObject(GetNewMoving(), 25, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 50, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 80, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 90, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 120, 10, 3, 2, 'o');
        InitObject(GetNewMoving(), 130, 10, 3, 2, 'o');
    }

    maxlvl = 3;
}

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    nodelay(stdscr, true);
    keypad(stdscr, true);

    CreateLevel(level);

    bool isRunning = true;
    while(isRunning)
    {
        ClearMap();

        int ch;
        while ((ch = getch()) != ERR) {
            if (ch == 27) { // ESC
                isRunning = false;
            }
            if (ch == ' ' && mario.IsFly == false) {
                mario.vertSpeed = -1;
            }
            if (ch == 'a' || ch == 'A' || ch == KEY_LEFT) {
                HorizonMoveMap(1);
            }
            if (ch == 'd' || ch == 'D' || ch == KEY_RIGHT) {
                HorizonMoveMap(-1);
            }
        }

        if (!isRunning) break;

        if (mapHeight < mario.y || mario.y < 0)
        {
            CreateLevel(level);
            napms(200);
        }
        if (mapHeight < mario.y || mario.y < 0) PlayerDead();

        VertMoveObject(&mario);
        MarioCollision();

        for(int i = 0; i < brickLength; i++)
            PutObjectOnMap(brick[i]);

        for(int i = 0; i < movingLength; i++)
        {
            VertMoveObject(moving + i);
            HorizonMoveObj(moving + i);
            PutObjectOnMap(moving[i]);
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