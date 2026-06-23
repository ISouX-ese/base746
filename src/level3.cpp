#include "game.h"
#include <math.h>

// =========================================================================
// GESTION TOPOGRAPHIQUE DU NIVEAU 3
// =========================================================================

/**
 * @brief Calcule l'ordonnée (Y) du sol en fonction de l'abscisse (X) dans le monde.
 * Gère une succession de gouffres larges nécessitant des plateformes pour la traversée.
 * @param world_x Position X globale de l'entité.
 * @return float Coordonnée Y du sol (100.0f représente une chute mortelle).
 */
float get_ground_y_level3(float world_x) {
    // Définition des gouffres (hitboxes mortelles)
    if (world_x > 400.0f && world_x < 524.0f) return 100.0f;  // Premier grand gouffre
    if (world_x > 800.0f && world_x < 996.0f) return 100.0f;  // Fosse centrale
    if (world_x > 1200.0f && world_x < 1354.0f) return 100.0f; // Gouffre final
    
    // Hauteur standard du sol pour les zones sûres
    return 28.0f; 
}

/**
 * @brief Calcule l'ordonnée (Y) du plafond en fonction de l'abscisse (X) dans le monde.
 * @param world_x Position X globale de l'entité.
 * @return float Coordonnée Y du plafond.
 */
float get_ceiling_y_level3(float world_x) { 
    // Niveau en plein ciel (Athletic Theme), le plafond n'est pas atteignable
    return -50.0f; 
}

// =========================================================================
// INITIALISATION DES ENTITÉS DU NIVEAU 3
// =========================================================================

/**
 * @brief Initialise toutes les entités (blocs, ennemis, pièces, tuyaux) du Niveau 3.
 * Ce niveau est axé sur la plateforme de précision (Athletic).
 */
void init_level_3() {
    WORLD_W = 1800; 
    flag = {1650.0f, 10.0f, 0.0f, 0.0f, 0.0f, 1, 18, true, 0};
    
    // Index de parcours pour les tableaux globaux
    int b=0, c=0, t=0, p=0, pl=0, fb=0, bc=0, f=0;

    // --- ZONE 1 : PRÉPARATION ---
    blocks[b++] = {100.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; // Bloc mystère (Champignon)
    blocks[b++] = {300.0f, 10.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 5}; // Bloc Étoile d'invincibilité
    turtles[t++] = {250.0f, 24.0f, -0.4f, 0.0f, 0.0f, 4, 4, true, 0};

    // --- ZONE 2 : ASCENSION SUR LE GOUFFRE 1 ---
    // Plateformes espacées par des sauts calibrés (16 pixels de gap horizontal exact)
    blocks[b++] = {416.0f, 14.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4}; 
    blocks[b++] = {452.0f, 8.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {488.0f, 2.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};

    // Obstacles et récompenses aériens
    flyers[f++] = {434.0f, 0.0f, -0.6f, 0.0f, 0.0f, 4, 4, true, 0}; 
    coins[c++] = {424.0f, 4.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};
    coins[c++] = {460.0f, -2.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};

    // --- ZONE 3 : L'ÎLE CENTRALE ---
    // Section infestée d'ennemis rebondissants
    bouncers[bc++] = {600.0f, 24.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0}; 
    bouncers[bc++] = {680.0f, 24.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};
    blocks[b++] = {640.0f, 10.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; 
    blocks[b++] = {750.0f, 10.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 6}; // Bloc de vie supplémentaire (1-UP Vert)

    // --- ZONE 4 : LA DESCENTE (GOUFFRE 2) ---
    // Escaliers descendants avec des gaps garantissant la fluidité (16px de saut horizontal)
    blocks[b++] = {816.0f, 16.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {852.0f, 10.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {888.0f, 4.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {924.0f, -2.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};
    blocks[b++] = {960.0f, 4.0f, 0.0f, 0.0f, 0.0f, 20, 5, true, 4};

    flyers[f++] = {870.0f, -5.0f, -0.8f, 0.0f, -5.0f, 4, 4, true, 0};
    flyers[f++] = {910.0f, 5.0f, -0.7f, 0.0f, 5.0f, 4, 4, true, 0};

    // --- ZONE 5 : L'AVANT-POSTE ---
    pipes[p++] = {1050.0f, 16.0f, 0.0f, 0.0f, 0.0f, 8, 12, true, 0}; // Tuyau avec Plante Piranha
    plants[pl++] = {1051.0f, 16.0f, 0.0f, -0.5f, 0.0f, 6, 6, true, 0};
    turtles[t++] = {1120.0f, 24.0f, -0.5f, 0.0f, 0.0f, 4, 4, true, 10}; // Koopa patrouilleur

    // --- ZONE 6 : LES PLATEFORMES INFESTÉES (GOUFFRE 3) ---
    // Plateformes hébergeant des ennemis sauteurs, complexifiant l'atterrissage
    blocks[b++] = {1216.0f, 20.0f, 0.0f, 0.0f, 0.0f, 30, 5, true, 4};
    bouncers[bc++] = {1225.0f, 16.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0}; 
    
    blocks[b++] = {1262.0f, 12.0f, 0.0f, 0.0f, 0.0f, 30, 5, true, 4};
    bouncers[bc++] = {1275.0f, 8.0f, 0.0f, 0.0f, 0.0f, 4, 4, true, 0};

    blocks[b++] = {1308.0f, 16.0f, 0.0f, 0.0f, 0.0f, 30, 5, true, 4};

    // --- ZONE 7 : LA FIN DU NIVEAU ---
    blocks[b++] = {1450.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 1}; 
    
    // Génération procédurale de l'escalier final (Symétrique)
    for(int i=0; i<3; i++) { 
        for(int j=0; j<=i; j++) { 
            blocks[b++] = {1500.0f + i*5.0f, 23.0f - j*5.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 0}; 
        } 
    }
}