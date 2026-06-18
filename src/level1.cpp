#include "game.h"
#include <math.h>

// ========================================================================
// 1. LE RELIEF GÉOMÉTRIQUE DU NIVEAU 1
// ========================================================================
float get_ground_y_level1(float world_x) {
    if (world_x > 310.0f && world_x < 335.0f) return 100.0f; 
    if (world_x > 720.0f && world_x < 745.0f) return 100.0f; 
    if (world_x > 1150.0f && world_x < 1175.0f) return 100.0f; 
    if (world_x > 1450.0f && world_x < 1480.0f) return 100.0f; 
    if (world_x >= 3610.0f && world_x <= 4000.0f) return 6.0f; 
    
    // Le sol de la Grotte Secrète est plus haut (Y=24)
    if (world_x >= 3100.0f && world_x < 3610.0f) return 24.0f; 
    
    return 28.0f; // Sol standard
}

float get_ceiling_y_level1(float world_x) {
    if (world_x >= 3100.0f && world_x <= 4000.0f) return 6.0f; 
    return 0.0f; 
}

// =========================================================================
// 2. INITIALISATION AVEC LES BLOCS 5x5 AJUSTÉS
// =========================================================================
void init_level_1() {
    WORLD_W = 4000; flag = {1840.0f, 10.0f, 0, 0, 0, 1, 18, true, 0};             
    ground_r = false; ground_g = true; ground_b = false; // Vert Plein Air
    int b=0, c=0, t=0, p=0, pl=0, fb=0, bc=0, f=0; 
    
    // --- ZONE 1 : L'INTRODUCTION ---
    blocks[b++] = {60.0f, 14.0f, 0, 0, 0, 5, 5, true, 1}; // 🍄 Bloc de départ
    turtles[t++] = {50.0f, 24.0f, -0.4f, 0, 0, 4, 4, true, 0}; 
    
    for(int i=0; i<3; i++) coins[c++] = {110.0f + i*12.0f, 14.0f - (i==1?4.0f:0.0f), 0, 0, 0, 4, 4, true, 0};
    turtles[t++] = {180.0f, 24.0f, -0.5f, 0, 0, 4, 4, true, 0}; 

    blocks[b++] = {240.0f, 14.0f, 0, 0, 0, 5, 5, true, 0}; 
    blocks[b++] = {245.0f, 14.0f, 0, 0, 0, 5, 5, true, 1}; // 🍄 Mystère

    // --- ZONE 2 : L'EMBRANCHEMENT CÉLESTE (CORRIGÉ) ---
    // Les distances ont été réduites pour s'adapter à la taille de 5px des blocs
    blocks[b++] = {290.0f, 14.0f, 0, 0, 0, 5, 5, true, 0};
    blocks[b++] = {295.0f, 9.0f, 0, 0, 0, 5, 5, true, 0}; 
    
    blocks[b++] = {315.0f, 2.0f, 0, 0, 0, 5, 5, true, 0};    // Plus proche !
    blocks[b++] = {335.0f, -6.0f, 0, 0, 0, 5, 5, true, 0};   // Plus proche !
    blocks[b++] = {355.0f, -14.0f, 0, 0, 0, 5, 5, true, 1};  // 🍄 Boost aérien

    // La plateforme nuageuse démarre plus tôt
    blocks[b++] = {375.0f, -20.0f, 0, 0, 0, 120, 5, true, 0};
    for(int i=0; i<12; i++) coins[c++] = {385.0f + (i*10.0f), -28.0f, 0, 0, 0, 4, 4, true, 0};
    
    flyers[f++] = {410.0f, -32.0f, -0.6f, 0, -30.0f, 4, 4, true, 0};

    blocks[b++] = {350.0f, 19.0f, 0, 0, 0, 10, 5, true, 0};
    pipes[p++] = {440.0f, 16.0f, 0, 0, 0, 8, 12, true, 0}; 
    plants[pl++] = {441.0f, 16.0f, 0, -0.5f, 0, 6, 6, true, 0};

    // --- ZONE 3 : LA VALLÉE DES TUYAUX ---
    pipes[p++] = {650.0f, 20.0f, 0, 0, 0, 8, 8, true, 0};
    
    // 🌟 CORRECTION DU GRAND TUYAU : Rabaissé à 12 pixels de hauteur (Y=16 au lieu de Y=12)
    pipes[p++] = {700.0f, 16.0f, 0, 0, 0, 8, 12, true, 0}; 
    plants[pl++] = {701.0f, 16.0f, 0, -0.4f, 0, 6, 6, true, 0};
    
    blocks[b++] = {760.0f, 14.0f, 0, 0, 0, 5, 5, true, 1}; // 🍄 

    pipes[p++] = {850.0f, 16.0f, 0, 0, 0, 8, 12, true, 3150}; // Passage secret
    turtles[t++] = {920.0f, 24.0f, -0.5f, 0, 0, 4, 4, true, 0};

    // --- ZONE 4 : LE PARCOURS DE SÉCURITÉ ---
    blocks[b++] = {1120.0f, 14.0f, 0, 0, 0, 5, 5, true, 0};
    blocks[b++] = {1200.0f, 9.0f, 0, 0, 0, 5, 5, true, 1}; // 🍄
    for(int i=0; i<4; i++) coins[c++] = {1190.0f + i*10.0f, 2.0f, 0, 0, 0, 4, 4, true, 0};

    pipes[p++] = {1280.0f, 16.0f, 0, 0, 0, 8, 12, true, 0};
    fireballs[fb++] = {1282.0f, 16.0f, 0, -4.0f, 0, 4, 4, true, 0}; 
    pipes[p++] = {1500.0f, 16.0f, 0, 0, 0, 8, 12, true, 0}; 
    
    blocks[b++] = {1560.0f, 14.0f, 0, 0, 0, 5, 5, true, 1}; // 🍄

    // --- ZONE 5 : LA PYRAMIDE ---
    for(int i=0; i<6; i++) {
        for(int j=0; j<=i; j++) blocks[b++] = {1750.0f + (i*5.0f), 23.0f - (j*5.0f), 0, 0, 0, 5, 5, true, 0};
    }
    
    // --- 💎 ZONE SECRÈTE : LA GROTTE DE CRISTAL ---
    for(int i=0; i<20; i++) {
        float wave = (float)(sin(i * 0.8f) * 3.0f); 
        coins[c++] = {3150.0f + (i*20.0f), 11.0f + wave, 0, 0, 0, 4, 4, true, 0};
    }
    
    // 🌟 CORRECTION DE HAUTEUR : Les blocs sont remontés à Y=9.0f
    // Le Grand Mario (Taille = 8) passe sans aucun problème sous la marge !
    blocks[b++] = {3350.0f, 9.0f, 0, 0, 0, 5, 5, true, 1}; // 🍄
    blocks[b++] = {3500.0f, 9.0f, 0, 0, 0, 5, 5, true, 1}; // 🍄 

    pipes[p++] = {3600.0f, 16.0f, 0, 0, 0, 8, 8, true, 1500}; 
}