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
} TObject;

char map[mapHeight][mapWidth+1];
TObject mario;

TObject *brick = NULL;
int brickLength;

TObject *moving = NULL;
int movingLength;

float cameraX = 0;

int level = 1;

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
}

bool IsCollision(TObject o1, TObject o2);
void CreateLevel(int lvl);

void VertMoveObject(TObject *obj)
{
    (*obj).IsFly = true;
    (*obj).vertSpeed += 0.05;
    SetObjectPos(obj, (*obj).x, (*obj).y + (*obj).vertSpeed);

    for (int i = 0; i < brickLength; i++)
       if (IsCollision(*obj, brick[i]))
       {
          (*obj).y -= (*obj).vertSpeed;
          (*obj).vertSpeed = 0;
          (*obj).IsFly = false;
          if(brick[i].cType == '+')
          {
             level++;
             if (level > 2) level = 1;
             CreateLevel(level);
             napms(1000);
          }

          break;
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
}

bool IsCollision(TObject o1, TObject o2)
{
    return ((o1.x + o1.width) > o2.x) && (o1.x < (o2.x + o2.width)) && ((o1.y + o1.height) > o2.y) && (o1.y < (o2.y + o2.height));
}

void CreateLevel(int lvl)
{
    if (brick != NULL)
    {
       free(brick);
       brick = NULL;
    }

    InitObject(&mario, 39, 10, 3, 3, '@');

    cameraX = 0;

    if(lvl == 1)
    {
       brickLength = 6;
       brick = (TObject*)realloc(brick, sizeof(*brick) * brickLength);

       InitObject(brick+0, 20, 20, 40, 5, '#');
       InitObject(brick+1, 60, 15, 10, 10, '#');
       InitObject(brick+2, 80, 20, 20, 5, '#');
       InitObject(brick+3, 120, 15, 10, 10, '#');
       InitObject(brick+4, 150, 20, 40, 5, '#');
       InitObject(brick+5, 210, 15, 10, 10, '+');
    }

    if(lvl == 2)
    {
       brickLength = 4;
       brick = (TObject*)realloc(brick, sizeof(*brick) * brickLength);

       InitObject(brick+0, 20, 20, 40, 5, '#');
       InitObject(brick+1, 80, 20, 15, 5, '#');
       InitObject(brick+2, 120, 15, 15, 10, '#');
       InitObject(brick+3, 160, 10, 15, 15, '+');
    }
}

int main()
{
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    nodelay(stdscr, TRUE);

    CreateLevel(level);

    bool isRunning = true;
    while(isRunning)
    {
       ClearMap();

       int ch;
       while ((ch = getch()) != ERR) {
           if (ch == 27) {
               isRunning = false;
           }
           if (ch == ' ' && mario.IsFly == false) {
               mario.vertSpeed = -1;
           }
           if (ch == 'a' || ch == 'A') {
               HorizonMoveMap(1);
           }
           if (ch == 'd' || ch == 'D') {
               HorizonMoveMap(-1);
           }
       }

       if (!isRunning) break;

       if (mapHeight < mario.y || mario.y < 0)
       {
          CreateLevel(level);
          napms(200);
       }

       VertMoveObject(&mario);

       for(int i = 0; i < brickLength; i++)
          PutObjectOnMap(brick[i]);

       PutObjectOnMap(mario);

       clear();
       ShowMap();

       napms(10);
    }

    endwin();
    return 0;
}