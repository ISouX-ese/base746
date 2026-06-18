#include "game.h"
#include <math.h>

float get_ground_y_level3(float world_x) {
    if (world_x > 400.0f && world_x < 524.0f) return 100.0f;  // Gouffre 1
    if (world_x > 800.0f && world_x < 996.0f) return 100.0f;  // Gouffre 2
    if (world_x > 1200.0f && world_x < 1354.0f) return 100.0f; // Gouffre 3
    return 28.0f; 
}
float get_ceiling_y_level3(float world_x) { 
    // 🌟 CORRECTION : Le plafond est repoussé très loin hors de l'écran
    return -50.0f; 
}

void init_level_3() {
    WORLD_W = 1800; 
    flag = {1650.0f, 10.0f, 0.0f, 0.0f, 0.0f, 1, 18, true, 0};
    int b=0, c=0, t=0, p=0, pl=0, fb=0, bc=0, f=0;

    blocks[b++] = {100.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; 
    blocks[b++] = {300.0f, 10.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 5}; // ⭐ Etoile
    turtles[t++] = {250.0f, 24.0f, -0.4f, 0.0f, 0.0f, 4, 4, true, 0};

    // --- Sauts calibrés au pixel (Gaps de 16px exacts) ---
    blocks[b++] = {416.0f, 14.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4}; 
    blocks[b++] = {452.0f, 8.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {488.0f, 2.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};

    flyers[f++] = {434.0f, 0.0f, -0.6f, 0.0f, 0.0f, 4, 4, true, 0}; 
    coins[c++] = {424.0f, 4.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};
    coins[c++] = {460.0f, -2.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};

    bouncers[bc++] = {600.0f, 24.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0}; 
    bouncers[bc++] = {680.0f, 24.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};
    blocks[b++] = {640.0f, 10.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; 
    blocks[b++] = {750.0f, 10.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 6}; // 🍄 1-UP Vert

    // --- Descente fluide (Gaps de 16px) ---
    blocks[b++] = {816.0f, 16.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {852.0f, 10.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {888.0f, 4.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {924.0f, -2.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {960.0f, 4.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};

    flyers[f++] = {870.0f, -5.0f, -0.8f, 0.0f, -5.0f, 4, 4, true, 0};
    flyers[f++] = {910.0f, 5.0f, -0.7f, 0.0f, 5.0f, 4, 4, true, 0};

    pipes[p++] = {1050.0f, 16.0f, 0.0f, 0.0f, 0.0f, 8, 12, true, 0};
    plants[pl++] = {1051.0f, 16.0f, 0.0f, -0.5f, 0.0f, 6, 6, true, 0};
    turtles[t++] = {1120.0f, 24.0f, -0.5f, 0.0f, 0.0f, 4, 4, true, 10}; 

    // --- Les derniers grands sauts (Gaps de 16px) ---
    blocks[b++] = {1216.0f, 20.0f, 0.0f, 0.0f, 0.0f, 30, 5, true, 4};
    bouncers[bc++] = {1225.0f, 16.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0}; 
    
    blocks[b++] = {1262.0f, 12.0f, 0.0f, 0.0f, 0.0f, 30, 5, true, 4};
    bouncers[bc++] = {1275.0f, 8.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};

    blocks[b++] = {1308.0f, 16.0f, 0.0f, 0.0f, 0.0f, 30, 5, true, 4};

    blocks[b++] = {1450.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; 
    for(int i=0; i<3; i++) { for(int j=0; j<=i; j++) { blocks[b++] = {1500.0f + i*5.0f, 23.0f - j*5.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 0}; } }
}