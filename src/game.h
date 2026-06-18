#pragma once
#include <stdint.h>

struct Entity { float x, y, vx, vy, base_y; int w, h; bool alive; int dest_x; }; 
struct Boss { float x, y, vy; int hp; bool alive; };

extern Entity blocks[100]; extern Entity pipes[15]; extern Entity plants[15]; extern Entity coins[60];
extern Entity turtles[20]; extern Entity bouncers[15]; extern Entity flyers[15]; extern Entity fireballs[15];
extern Entity flag; extern Boss boss; 

extern Entity mario; 
extern Entity mush;

extern int WORLD_W;
extern bool ground_r; extern bool ground_g; extern bool ground_b; 

float get_ground_y_level1(float world_x); 
float get_ceiling_y_level1(float world_x); 
void init_level_1();

float get_ground_y_level2(float world_x); 
float get_ceiling_y_level2(float world_x); 
void init_level_2();

float get_ground_y_level3(float world_x); 
float get_ceiling_y_level3(float world_x); // <-- AJOUTÉ !
void init_level_3();

float get_ground_y_level4(float world_x); 
float get_ceiling_y_level4(float world_x); // <-- AJOUTÉ !
void init_level_4();

float get_ground_y_level5(float world_x); 
float get_ceiling_y_level5(float world_x); // <-- AJOUTÉ !
void init_level_5();