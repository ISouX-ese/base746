#include "game.h"
#include "lvgl.h" 
#include <math.h>

// =========================================================================================================
// 1. STRUCTURES INTERNES POUR LES ENNEMIS DYNAMIQUES (THWOMPS & PLATEFORMES)
// =========================================================================================================
struct CustomObj {
    float start_x; float start_y; float curr_x; float curr_y;
    int state; int timer; bool active;
};

static CustomObj dyn_objs[100]; 
static bool level_initialized = false;

// =========================================================================================================
// 2. LE RELIEF GÉOMÉTRIQUE & LE SOUS-MOTEUR PHYSIQUE
// =========================================================================================================
float get_ground_y_level2(float world_x) {
    if (world_x >= 960.0f && world_x < 1000.0f) return 28.0f;
    if (world_x > 820.0f && world_x < 1150.0f) return 100.0f; 
    return 28.0f; 
}

float get_ceiling_y_level2_geo(float world_x) {
    if (world_x < 300.0f || world_x > 1700.0f) return -50.0f; 
    if (world_x >= 800.0f && world_x <= 1180.0f) return -50.0f; 
    for(int i = 50; i < 90; i++) {
        if (blocks[i].alive && blocks[i].dest_x == 1 && blocks[i].w == 12) {
            if (world_x >= blocks[i].x - 1.0f && world_x <= blocks[i].x + 13.0f) return 0.0f;
        }
    }
    return 8.0f; 
}

static bool checkLocalCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void update_level_2_subengine(float world_x) {
    // 🌟 3. CORRECTION DES TORTUES PASSE-MURAILLES : Le champ de force !
    for(int i=0; i<20; i++) {
        if (turtles[i].alive) {
            if (turtles[i].x < 365.0f && turtles[i].vx < 0) turtles[i].vx = -turtles[i].vx; // Rebond sur mur gauche de la grotte
            if (turtles[i].x > 1620.0f && turtles[i].vx > 0) turtles[i].vx = -turtles[i].vx; // Rebond sur mur droit de la grotte
        }
    }

    if (world_x != mario.x + 2) {
        for (int i = 50; i < 90; i++) {
            if (dyn_objs[i].active) {
                blocks[i].alive = true;
                blocks[i].x = dyn_objs[i].curr_x;
                blocks[i].y = dyn_objs[i].curr_y;
            }
        }
        return;
    }

    static uint32_t last_tick = 0;
    uint32_t now = lv_tick_get();
    if (now - last_tick < 30) {
        for (int i = 80; i <= 84; i++) {
            if (dyn_objs[i].active) {
                bool overlap = checkLocalCollision(mario.x, mario.y, 4, 8, dyn_objs[i].curr_x, dyn_objs[i].curr_y, blocks[i].w, blocks[i].h);
                if (overlap) {
                    if (mario.vy < 0 || (mario.y + 4) > dyn_objs[i].curr_y + 1.0f) blocks[i].alive = false; 
                }
            }
        }
        return;
    }
    last_tick = now;

    for (int i = 50; i < 90; i++) {
        if (!dyn_objs[i].active) continue;

        if (blocks[i].dest_x == 1) { 
            switch (dyn_objs[i].state) {
                case 0: if (fabs(mario.x - dyn_objs[i].curr_x) < 24.0f && mario.y > dyn_objs[i].curr_y) dyn_objs[i].state = 1; break;
                case 1: { 
                    dyn_objs[i].curr_y += 3.0f; 
                    float target_y = get_ground_y_level2(dyn_objs[i].curr_x) - 16.0f;
                    if (target_y > 36.0f) target_y = 36.0f; 
                    if (dyn_objs[i].curr_y >= target_y) { dyn_objs[i].curr_y = target_y; dyn_objs[i].state = 2; dyn_objs[i].timer = 40; }
                    break;
                }
                case 2: dyn_objs[i].timer--; if (dyn_objs[i].timer <= 0) dyn_objs[i].state = 3; break;
                case 3: dyn_objs[i].curr_y -= 0.5f; if (dyn_objs[i].curr_y <= dyn_objs[i].start_y) { dyn_objs[i].curr_y = dyn_objs[i].start_y; dyn_objs[i].state = 4; dyn_objs[i].timer = 100; } break;
                case 4: dyn_objs[i].timer--; if (dyn_objs[i].timer <= 0) dyn_objs[i].state = 0; break;
            }
        } 
        else if (blocks[i].dest_x == 2) {
            dyn_objs[i].curr_y += 0.25f; 
            if (dyn_objs[i].curr_y > 32.0f) dyn_objs[i].curr_y = -2.0f; 
        }

        blocks[i].x = dyn_objs[i].curr_x;
        blocks[i].y = dyn_objs[i].curr_y;
    }
}

float get_ceiling_y_level2(float world_x) {
    update_level_2_subengine(world_x);
    return get_ceiling_y_level2_geo(world_x);
}

// =========================================================================================================
// 3. ARCHITECTURE DE LA CARTE
// =========================================================================================================
void init_level_2() {
    WORLD_W = 2600; 
    level_initialized = true;
    for(int i=0; i<100; i++) dyn_objs[i].active = false;

    flag = {2450.0f, 10.0f, 0, 0, 0, 1, 18, true, 0};             
    int b=0, c=0, t=0, p=0, pl=0, f=0; 
    
    // --- 🎬 ETAPE 1 : LA SURFACE ---
    blocks[b++] = {45.0f, 14.0f, 0, 0, 0, 5, 5, true, 1}; 
    pipes[p++] = {90.0f, 16.0f, 0, 0, 0, 8, 12, true, 360}; 
    blocks[b++] = {120.0f, 0.0f, 0, 0, 0, 195, 28, true, 4}; 
    blocks[b++] = {315.0f, 0.0f, 0, 0, 0, 45, 28, true, 3};  

    // --- 🪨 ETAPE 2 : LA GROTTE ---
    int s_thwomp = 50; 
    for(int i=0; i<4; i++) {
        int idx = s_thwomp + i;
        blocks[idx] = {420.0f + (i * 85.0f), 0.0f, 0, 0, 0, 12, 16, true, 1}; 
        dyn_objs[idx] = {420.0f + (i * 85.0f), 0.0f, 420.0f + (i * 85.0f), 0.0f, 0, 0, true};
        turtles[t++] = {450.0f + (i * 85.0f), 24.0f, -0.4f, 0, 0, 4, 4, true, 10};
    }
    flyers[f++] = {500.0f, 10.0f, -0.7f, 0, 10.0f, 4, 4, true, 0};
    flyers[f++] = {670.0f, 14.0f, -0.8f, 0, 14.0f, 4, 4, true, 0};

    // 🛠️ ZONE 2 : 6 SAUTS DE PRÉCISION
    float p_x = 840.0f;
    for(int i=0; i<6; i++) {
        blocks[b++] = {p_x, 15.0f, 0, 0, 0, 5, 50, true, 3}; 
        p_x += 24.0f; 
    }
    flyers[f++] = {860.0f, 12.0f, -0.7f, 0, 12.0f, 4, 4, true, 0};
    flyers[f++] = {910.0f, 10.0f, -0.9f, 0, 10.0f, 4, 4, true, 0};
    flyers[f++] = {960.0f, 14.0f, -0.6f, 0, 14.0f, 4, 4, true, 0};

    // 🛠️ ZONE 3 : 5 PLATEFORMES ROULANTES
    int s_plat = 80;
    float start_plat_x = 1000.0f;
    for(int i=0; i<5; i++) {
        int idx = s_plat + i;
        float start_y = 24.0f - (i * 5.0f);
        blocks[idx] = {start_plat_x + (i * 32.0f), start_y, 0, 0, 0, 9, 3, true, 3}; 
        dyn_objs[idx] = {blocks[idx].x, start_y, blocks[idx].x, start_y, 0, 0, true};
        blocks[idx].dest_x = 2; 
        coins[c++] = {blocks[idx].x + 2.0f, 10.0f, 0, 0, 0, 4, 4, true, 0};
    }
    flyers[f++] = {1060.0f, 8.0f, -0.8f, 0, 8.0f, 4, 4, true, 0};
    flyers[f++] = {1120.0f, 6.0f, -0.7f, 0, 6.0f, 4, 4, true, 0};

    // PLATEFORME DE SECOURS
    blocks[b++] = {1140.0f, 20.0f, 0, 0, 0, 30, 5, true, 0};

    // 🛠️ ZONE 4 : L'ENFER DES ENNEMIS
    float z4_x = 1150.0f;
    blocks[b++] = {z4_x, 28.0f, 0, 0, 0, 600, 4, true, 3}; 

    pipes[p++] = {z4_x + 60.0f, 24.0f, 0, 0, 0, 8, 4, true, 0}; plants[pl++] = {z4_x + 61.0f, 20.0f, 0, -0.4f, 0, 6, 6, true, 0};
    pipes[p++] = {z4_x + 180.0f, 24.0f, 0, 0, 0, 8, 4, true, 0}; plants[pl++] = {z4_x + 181.0f, 20.0f, 0, -0.5f, 0, 6, 6, true, 0};
    pipes[p++] = {z4_x + 300.0f, 24.0f, 0, 0, 0, 8, 4, true, 0}; plants[pl++] = {z4_x + 301.0f, 20.0f, 0, -0.4f, 0, 6, 6, true, 0};
    
    pipes[p++] = {z4_x + 420.0f, 24.0f, 0, 0, 0, 8, 4, true, 100}; // Sortie vers désert

    for(int i=0; i<6; i++) {
        turtles[t++] = {z4_x + 80.0f + (i * 90.0f), 24.0f, -0.4f, 0, 0, 4, 4, true, 10}; 
        turtles[t++] = {z4_x + 110.0f + (i * 90.0f), 24.0f, -0.5f, 0, 0, 4, 4, true, 0};  
    }
    
    flyers[f++] = {z4_x + 100.0f, 12.0f, -0.6f, 0, 12.0f, 4, 4, true, 0};
    flyers[f++] = {z4_x + 220.0f, 16.0f, -0.9f, 0, 16.0f, 4, 4, true, 0};
    flyers[f++] = {z4_x + 340.0f, 10.0f, -0.7f, 0, 10.0f, 4, 4, true, 0};

    blocks[56] = {z4_x + 120.0f, 0.0f, 0, 0, 0, 12, 16, true, 1}; dyn_objs[56] = {z4_x + 120.0f, 0.0f, z4_x + 120.0f, 0.0f, 0, 0, true};
    blocks[57] = {z4_x + 380.0f, 0.0f, 0, 0, 0, 12, 16, true, 1}; dyn_objs[57] = {z4_x + 380.0f, 0.0f, z4_x + 380.0f, 0.0f, 0, 0, true};

    blocks[b++] = {z4_x + 480.0f, 0.0f, 0, 0, 0, 50, 28, true, 3};

    // --- 🏜️ ETAPE 3 : TRANSITION BIOME DÉSERT (X >= 1750) ---
    for(int step=0; step<5; step++) {
        for(int h=0; h<=step; h++) {
            // Le '2' à la fin indique au moteur de dessiner un bloc solide JAUNE
            blocks[b++] = {1750.0f + step * 5.0f, 23.0f - h * 5.0f, 0, 0, 0, 5, 5, true, 2}; 
        }
    }
    // Solide JAUNE également pour le grand sol plat du désert
    blocks[b++] = {1775.0f, 28.0f, 0, 0, 0, 200, 4, true, 2};
    
    flag.x = 1850.0f; // Drapeau rapproché
    WORLD_W = 1950;   // Limite du monde rapprochée
}