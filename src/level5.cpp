#include "game.h"
#include <math.h>

float get_ground_y_level5(float world_x) {
    if (world_x > 200.0f && world_x < 220.0f) return 100.0f; // Fosse de lave
    return 28.0f;
}

float get_ceiling_y_level5(float world_x) { return 0.0f; }

void init_level_5() {
    WORLD_W = 700; 
    flag = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, false, 0};
    int b=0, c=0, t=0, p=0, pl=0, fb=0, bc=0, f=0;

    // --- LES BONUS ---
    blocks[b++] = {80.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 6}; // 🍄 1-UP Vert
    blocks[b++] = {120.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 5}; // ⭐ Etoile Instantanée !

    // 🐢 Ajout d'un Goomba dans le couloir pour te mettre dans le bain
    turtles[t++] = {160.0f, 24.0f, -0.4f, 0.0f, 0.0f, 4, 4, true, 0}; 

    // --- LA LAVE ---
    fireballs[fb++] = {210.0f, 32.0f, 0.0f, -4.5f, 0.0f, 4, 4, true, 0};
    
    // 🦇 Ajout d'une chauve-souris qui survole la lave !
    flyers[f++] = {210.0f, 8.0f, -0.6f, 0.0f, 8.0f, 4, 4, true, 0};

    // --- L'ARÈNE ---
    blocks[b++] = {380.0f, 16.0f, 0.0f, 0.0f, 0.0f, 25, 5, true, 4}; 
    blocks[b++] = {460.0f, 10.0f, 0.0f, 0.0f, 0.0f, 25, 5, true, 4}; 
    blocks[b++] = {540.0f, 16.0f, 0.0f, 0.0f, 0.0f, 25, 5, true, 4}; 

    fireballs[fb++] = {430.0f, 32.0f, 0.0f, -5.0f, 0.0f, 4, 4, true, 0};
    fireballs[fb++] = {510.0f, 32.0f, 0.0f, -4.5f, 0.0f, 4, 4, true, 0};

    // 🐢 Ajout d'un Koopa (Tortue) qui patrouille à l'entrée de l'arène
    turtles[t++] = {350.0f, 24.0f, -0.6f, 0.0f, 0.0f, 4, 4, true, 10};

    // 🦇 Ajout d'une chauve-souris en hauteur pour gêner tes sauts sur Bowser
    flyers[f++] = {480.0f, -2.0f, -0.8f, 0.0f, -2.0f, 4, 4, true, 0};

    // --- LE BOSS ---
    boss.alive = true;
    boss.x = 620.0f; 
    boss.y = 10.0f;
    boss.vy = 1.0f;
    boss.hp = 5; 
}