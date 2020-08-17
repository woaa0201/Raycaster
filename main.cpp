/*

In order to draw the world, a single ray is traced for every
column of screen pixels and a vertical slice of wall texture
is selected and scaled according to where in the world the ray
hits a wall and how far it travels before doing so.

From starting point, find distance to edge of first box, record with edge.
March out in that direction, adding the previous distance each time until you hit a wall.
Find walls by looking at the left/right vertices or the up/down vertices.
Distance to wall is the number of distances that had to be added together.

https://upload.wikimedia.org/wikipedia/commons/e/e7/Simple_raycasting_with_fisheye_correction.gif

TASK LIST

- DONE: Divide Screen Space into increments using FieldOfView

- DONE: Cast rays along each increment and record distances in an array

- DONE: Draw screen by iterating through each column. Distances in array define the heights of each environment segment.

*/


#define _USE_MATH_DEFINES

#include "map.h"
#include <iostream>
#include <cstdio>
#include <cmath>
#include <Windows.h>
#include <time.h>

using namespace std;

const int ViewScale = 10;
const int ScreenHeight = 2*ViewScale;
const int ScreenWidth = 5*ViewScale;
const int FieldOfView = 45;
float FovInRadians = FieldOfView * M_PI / 180;

const float MapDiagonal = sqrt(MapRows*MapRows + MapColumns*MapColumns);

float ScreenGrid[ScreenHeight][ScreenWidth];

float Distances[ScreenWidth];

float PlayerRowPosition = 5; // Player Y position on map
float PlayerColumnPosition = 5; // Player X position on map

float PlayerAngle = 0; // Player direction angle clockwise from positive X-axis (in radians)

int KeyPressFlag = 0;

void MovePlayer();
float RayDistance(float x, float y, float angle);

void hideCursor();
void clear_screen();
int CheckCollision(float RowPosition, float ColumnPosition);
void GetDistances();
void ScreenGridMerge();
void DrawScreen();

int main()
{
  hideCursor();
  system("cls");
  clear_screen();
  while(GetAsyncKeyState(VK_ESCAPE) == 0)
  {
    GetDistances();
    ScreenGridMerge();
    DrawScreen();
    MovePlayer();
    clear_screen();
  }
  system("cls");
  return 0;
}

void MovePlayer()
{
  float MovementDelta = 0.1;
  if(GetAsyncKeyState(VK_LEFT) < 0) // Turn Player Left
  {
    PlayerAngle = PlayerAngle - MovementDelta;
    if(PlayerAngle < 0)
    {
      PlayerAngle = PlayerAngle + 2*M_PI;
    }
  }
  if(GetAsyncKeyState(VK_RIGHT) < 0) // Turn Player Right
  {
    PlayerAngle = PlayerAngle + MovementDelta;
    if(PlayerAngle >= 2*M_PI)
    {
      PlayerAngle = PlayerAngle - 2*M_PI;
    }
  }
  if(GetAsyncKeyState(VK_UP) < 0 || GetAsyncKeyState('W') < 0) // Move Player Forward
  {

      PlayerRowPosition = PlayerRowPosition + MovementDelta*sin(PlayerAngle); // Increase Y position
      PlayerColumnPosition = PlayerColumnPosition + MovementDelta*cos(PlayerAngle); // Increase X position
      if(CheckCollision(round(PlayerRowPosition), round(PlayerColumnPosition)) == 1) // Undo if there is a collision
      {
        PlayerRowPosition = PlayerRowPosition - MovementDelta*sin(PlayerAngle); // Decrease Y position
        PlayerColumnPosition = PlayerColumnPosition - MovementDelta*cos(PlayerAngle); // Decrease X position
      }
  }
  if(GetAsyncKeyState(VK_DOWN) < 0 || GetAsyncKeyState('S') < 0) // Move Player Backward
  {
      PlayerRowPosition = PlayerRowPosition - MovementDelta*sin(PlayerAngle); // Decrease Y position
      PlayerColumnPosition = PlayerColumnPosition - MovementDelta*cos(PlayerAngle); // Decrease X position
      if(CheckCollision(round(PlayerRowPosition), round(PlayerColumnPosition)) == 1) // Undo if there is a collision
      {
        PlayerRowPosition = PlayerRowPosition + MovementDelta*sin(PlayerAngle); // Increase Y position
        PlayerColumnPosition = PlayerColumnPosition + MovementDelta*cos(PlayerAngle); // Increase X position
      }
  }
  if(GetAsyncKeyState('A') < 0 || GetAsyncKeyState(VK_OEM_4) < 0) // Strafe Player Left
  {

      PlayerRowPosition = PlayerRowPosition - MovementDelta*cos(PlayerAngle); // Increase Y position
      PlayerColumnPosition = PlayerColumnPosition + MovementDelta*sin(PlayerAngle); // Increase X position
      if(CheckCollision(round(PlayerRowPosition), round(PlayerColumnPosition)) == 1) // Undo if there is a collision
      {
        PlayerRowPosition = PlayerRowPosition + MovementDelta*cos(PlayerAngle); // Decrease Y position
        PlayerColumnPosition = PlayerColumnPosition - MovementDelta*sin(PlayerAngle); // Decrease X position
      }
  }
  if(GetAsyncKeyState('D') < 0 || GetAsyncKeyState(VK_OEM_6) < 0) // Strafe Player Right
  {

      PlayerRowPosition = PlayerRowPosition + MovementDelta*cos(PlayerAngle); // Increase Y position
      PlayerColumnPosition = PlayerColumnPosition - MovementDelta*sin(PlayerAngle); // Increase X position
      if(CheckCollision(round(PlayerRowPosition), round(PlayerColumnPosition)) == 1) // Undo if there is a collision
      {
        PlayerRowPosition = PlayerRowPosition - MovementDelta*cos(PlayerAngle); // Decrease Y position
        PlayerColumnPosition = PlayerColumnPosition + MovementDelta*sin(PlayerAngle); // Decrease X position
      }
  }

  /*
  if((GetAsyncKeyState('-') & 0x8000) && (KeyPressFlag == 0))
  {
    KeyPressFlag = 1;
    if(ViewScale >= 1 && ViewScale <= 4)
    {
      ViewScale--;
    }
  }
  if((GetAsyncKeyState('+') & 0x8000) && (KeyPressFlag == 0))
  {
    KeyPressFlag = 1;
    if(ViewScale >= 1 && ViewScale <= 4)
    {
      ViewScale++;
    }
  }
  if(GetAsyncKeyState('+') == 0 && GetAsyncKeyState('-') == 0)
  {
    KeyPressFlag = 0;
  }
  */
}

float RayDistance(float x, float y, float angle) // Send ray from player position at a given angle, send back distance to first wall hit.
{
    float delta = 0.01;
    float distance = 0;

    while(CheckCollision(floor(y*100+0.5)/100,floor(x*100+0.5)/100) != 1)
    {
      y = y + delta*sin(angle); // Advance y position
      x = x + delta*cos(angle); // Advance x position
      distance += delta;
    }
    return distance - delta;
}

void hideCursor()
{
	HANDLE hStdOut = NULL;
	CONSOLE_CURSOR_INFO curInfo;

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleCursorInfo(hStdOut, &curInfo);
	curInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hStdOut, &curInfo);
}

void clear_screen()
{
	// system("cls"); // Use for Windows
	// system("clear"); // Use for Linux
	COORD cursorPosition;
	cursorPosition.X = 0;
	cursorPosition.Y = 0;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

int CheckCollision(float RowPosition, float ColumnPosition)
{
  if(map[(int)RowPosition][(int)ColumnPosition] == 1)
  {
    return 1;
  }
  if(ColumnPosition == (int)ColumnPosition)
  {
    if(map[(int)floor(RowPosition)][(int)ColumnPosition] == 1 && map[(int)ceil(RowPosition)][(int)ColumnPosition] == 1)
    {
      return 1;
    }
  }
  if(RowPosition == (int)RowPosition)
  {
    if(map[(int)RowPosition][(int)floor(ColumnPosition)] == 1 && map[(int)RowPosition][(int)ceil(ColumnPosition)] == 1)
    {
      return 1;
    }
  }
  return 0;
}

void GetDistances()
{
  for(int i = 0; i < ScreenWidth; i++)
  {
    float StartingAngle = PlayerAngle - FovInRadians/2;
    if(StartingAngle >= 2*M_PI)
    {
      StartingAngle = StartingAngle - 2*M_PI;
    }
    if(StartingAngle < 0)
    {
      StartingAngle = StartingAngle + 2*M_PI;
    }
    float ColumnAngle = StartingAngle + i*(FovInRadians / ScreenWidth);
    if(ColumnAngle >= 2*M_PI)
    {
      ColumnAngle = ColumnAngle - 2*M_PI;
    }
    if(ColumnAngle < 0)
    {
      ColumnAngle = ColumnAngle + 2*M_PI;
    }
    Distances[i] = RayDistance(PlayerColumnPosition, PlayerRowPosition, ColumnAngle)*cos(ColumnAngle);
    if(Distances[i] < 0)
    {
      Distances[i] *= -1;
    }
  }
}

void ScreenGridMerge()
{
  for(int col = 0; col < ScreenWidth; col++)
  {
    int ColumnHeight = ScreenHeight/Distances[col];
    for(int row = 0; row < ScreenHeight; row++)
    {
      if(row >= (ScreenHeight - ColumnHeight)/2 && row <= (ScreenHeight + ColumnHeight)/2)
      {
        ScreenGrid[row][col] = Distances[col];
      }
      else
      {
        ScreenGrid[row][col] = 0;
      }
    }
  }
}

void DrawScreen()
{

/*
for(int i = 0; i < ScreenWidth; i++)
{
  cout << ceil(Distances[i]);
}


  for(int j = 0; j < ScreenHeight; j++)
  {
    for(int i = 0; i < ScreenWidth; i++)
    {
      printf("%d", ScreenGrid[j][i]);
    }
    printf("\n");
  }

  printf("\n");
*/

  for(int j = 0; j < ScreenHeight; j++)
  {
    for(int i = 0; i < ScreenWidth; i++)
    {
      if(ScreenGrid[j][i] > MapDiagonal/1.5)
      {
        // printf("\u2593");
        printf(".");
      }
      else if(ScreenGrid[j][i] <= MapDiagonal/1.5 && ScreenGrid[j][i] > MapDiagonal/3)
      {
        // printf("\u2592");
        printf(",");
      }
      else if(ScreenGrid[j][i] <= MapDiagonal/3 && ScreenGrid[j][i] > 0)
      {
        // printf("\u2591");
        printf("#");
      }
      else if(ScreenGrid[j][i] == 0)
      {
        printf(" ");
      }
    }
    printf("\n");
  }

}
