#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>

#define mapWidth 80
#define mapHeight 25

typedef struct SObject{
    float x, y;
    float width, height;
} TObject;

char map[mapHeight][mapWidth+1];
TObject mario;

void ClearMap(){
    for (int i = 0; i < mapWidth; i++)
        map[0][i] = '.';
    map[0][mapWidth] = '\0';
    for (int j = 1; j < mapHeight; j++)
        sprintf(map[j], map[0]);
}

void ShowMap(){
    map[mapHeight - 1][mapWidth - 1]= '\0';

    clear();
    for (int j = 0; j < mapHeight; j++)
        mvprintw(j, 0, "%s", map[j]);
    refresh();
}

void SetObjectPos(TObject *obj, float xPos, float yPos){
    (*obj).x = xPos;
    (*obj).y = yPos;
}

void InitObject(TObject *obj, float xPos, float yPos, float oWidth, float oHeight){
    SetObjectPos(obj, xPos, yPos);
    (*obj).width = oWidth;
    (*obj).height = oHeight;
}

void PutObjectOnMap(TObject obj){
    int ix = (int)round(obj.x);
    int iy = (int)round(obj.y);
    int iWidth = (int)round(obj.width);
    int iHeight = (int)round(obj.height);

    for(int i = ix; i < (ix + iWidth); i++)
        for (int j = iy; j < (iy + iHeight); j++)
            map[iy][ix] = '@';
}

int main(){
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    InitObject(&mario, 20, 10, 1, 1);
    ClearMap();
    PutObjectOnMap(mario);
    ShowMap();

    while (getch() != 'q');

    endwin();

    return 0;
}