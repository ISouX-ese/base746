#include "game.h"
#include <math.h>

// =========================================================================
// GESTION TOPOGRAPHIQUE DU NIVEAU 4
// =========================================================================

/**
 * @brief Calcule l'ordonnée (Y) du sol en fonction de l'abscisse (X) dans le monde.
 * Définit la géométrie du sol avec de nombreuses fosses de lave réparties.
 * @param world_x Position X globale de l'entité.
 * @return float Coordonnée Y du sol (100.0f représente une chute mortelle).
 */
float get_ground_y_level4(float world_x) {
    // Délimitation des fosses magmatiques (Hitboxes mortelles)
    if (world_x > 300.0f && world_x < 378.0f) return 100.0f; // Fosse 1
    if (world_x > 600.0f && world_x < 698.0f) return 100.0f; // Fosse 2
    if (world_x > 900.0f && world_x < 988.0f) return 100.0f; // Fosse 3
    if (world_x > 1200.0f && world_x < 1309.0f) return 100.0f; // Fosse 4
    
    // Niveau de base du sol du château
    return 28.0f;
}

/**
 * @brief Calcule l'ordonnée (Y) du plafond en fonction de l'abscisse (X) dans le monde.
 * Ajoute des abaissements de plafond pour contraindre la hauteur de saut au-dessus des zones dangereuses.
 * @param world_x Position X globale de l'entité.
 * @return float Coordonnée Y du plafond.
 */
float get_ceiling_y_level4(float world_x) { 
    // Écrasement du plafond (stalactites/architecture du château) pour forcer des sauts tendus
    if (world_x > 400.0f && world_x < 550.0f) return 10.0f; 
    if (world_x > 700.0f && world_x < 850.0f) return 10.0f; 
    
    // Plafond standard légèrement hors cadre
    return -10.0f; 
}

// =========================================================================
// INITIALISATION DES ENTITÉS DU NIVEAU 4
// =========================================================================

/**
 * @brief Initialise toutes les entités (blocs, ennemis, pièces, tuyaux) du Niveau 4.
 * Ce niveau est conçu autour d'obstacles dynamiques (Boules de lave) et de sauts contraints.
 */
void init_level_4() {
    WORLD_W = 1600; 
    flag = {1450.0f, 10.0f, 0.0f, 0.0f, 0.0f, 1, 18, true, 0};
    
    // Index de parcours pour les tableaux globaux
    int b=0, c=0, t=0, p=0, pl=0, fb=0, bc=0, f=0;

    // --- ZONE 1 : ENTRÉE DU CHÂTEAU ---
    blocks[b++] = {100.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; 
    turtles[t++] = {200.0f, 24.0f, -0.4f, 0.0f, 0.0f, 4, 4, true, 0}; 

    blocks[b++] = {250.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 6}; // Bloc de récompense (1-UP Vert)

    // --- ZONE 2 : TRAVERSÉE MAGMATIQUE 1 ---
    // Les espacements horizontaux (gaps) sont calibrés à 16 pixels.
    fireballs[fb++] = {340.0f, 32.0f, 0.0f, -4.5f, 0.0f, 4, 4, true, 0}; // Lave rebondissante
    blocks[b++] = {316.0f, 20.0f, 0.0f, 0.0f, 0.0f, 15, 5, true, 0}; 
    blocks[b++] = {347.0f, 20.0f, 0.0f, 0.0f, 0.0f, 15, 5, true, 0}; 
    
    turtles[t++] = {450.0f, 24.0f, -0.6f, 0.0f, 0.0f, 4, 4, true, 10}; // Koopa rapide
    turtles[t++] = {500.0f, 24.0f, -0.5f, 0.0f, 0.0f, 4, 4, true, 0};  
    blocks[b++] = {480.0f, 18.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; 

    // --- ZONE 3 : TRAVERSÉE MAGMATIQUE 2 ---
    bouncers[bc++] = {570.0f, 24.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};
    fireballs[fb++] = {635.0f, 32.0f, 0.0f, -4.8f, 0.0f, 4, 4, true, 0}; // Lave asynchrone (vitesse verticale différente)
    blocks[b++] = {616.0f, 20.0f, 0.0f, 0.0f, 0.0f, 25, 5, true, 0}; 
    blocks[b++] = {657.0f, 20.0f, 0.0f, 0.0f, 0.0f, 25, 5, true, 0}; 

    flyers[f++] = {720.0f, 16.0f, -0.8f, 0.0f, 16.0f, 4, 4, true, 0};
    flyers[f++] = {780.0f, 14.0f, -0.7f, 0.0f, 14.0f, 4, 4, true, 0};
    turtles[t++] = {800.0f, 24.0f, -0.5f, 0.0f, 0.0f, 4, 4, true, 10};
    blocks[b++] = {880.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 5}; // Bloc de récompense majeur (Étoile)

    // --- ZONE 4 : TRAVERSÉE MAGMATIQUE 3 ---
    fireballs[fb++] = {930.0f, 32.0f, 0.0f, -4.2f, 0.0f, 4, 4, true, 0};
    blocks[b++] = {916.0f, 18.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 0}; 
    blocks[b++] = {952.0f, 18.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 0}; 
    
    // Station défensive de mi-parcours (Tuyaux + Plantes Piranha)
    pipes[p++] = {1050.0f, 16.0f, 0.0f, 0.0f, 0.0f, 8, 12, true, 0};
    plants[pl++] = {1051.0f, 16.0f, 0.0f, -0.4f, 0.0f, 6, 6, true, 0};
    pipes[p++] = {1150.0f, 16.0f, 0.0f, 0.0f, 0.0f, 8, 12, true, 0};
    plants[pl++] = {1151.0f, 16.0f, 0.0f, -0.5f, 0.0f, 6, 6, true, 0};
    blocks[b++] = {1180.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; 

    // --- ZONE 5 : ÉPREUVE MAGMATIQUE FINALE ---
    // Densité accrue des projectiles mortels
    fireballs[fb++] = {1230.0f, 32.0f, 0.0f, -5.0f, 0.0f, 4, 4, true, 0};
    fireballs[fb++] = {1270.0f, 32.0f, 0.0f, -4.5f, 0.0f, 4, 4, true, 0};
    blocks[b++] = {1216.0f, 16.0f, 0.0f, 0.0f, 0.0f, 15, 5, true, 0}; 
    blocks[b++] = {1247.0f, 16.0f, 0.0f, 0.0f, 0.0f, 15, 5, true, 0}; 
    blocks[b++] = {1278.0f, 16.0f, 0.0f, 0.0f, 0.0f, 15, 5, true, 0}; 

    // Génération procédurale de l'escalier vers la sortie
    for(int i=0; i<4; i++) { 
        for(int j=0; j<=i; j++) { 
            blocks[b++] = {1350.0f + i*5.0f, 23.0f - j*5.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 0}; 
        } 
    }
}