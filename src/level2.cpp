#include "game.h"
#include "lvgl.h" 
#include <math.h>

// =========================================================================================================
// 1. STRUCTURES INTERNES POUR LES ENNEMIS DYNAMIQUES (THWOMPS & PLATEFORMES)
// =========================================================================================================

/**
 * @brief Représente un objet interactif dont le comportement est régi par une machine à états (FSM).
 * Gère les déplacements paramétriques indépendamment du moteur physique principal.
 */
struct CustomObj {
    float start_x; float start_y; // Position d'origine de l'objet
    float curr_x; float curr_y;   // Position actuelle lors de l'animation
    int state;                    // État courant dans la FSM (ex: 0=Attente, 1=Chute, 2=Stun...)
    int timer;                    // Compteur de frames pour temporiser les transitions d'état
    bool active;                  // Indicateur de présence de l'objet dans la scène
};

// Allocation mémoire pour les objets dynamiques locaux (Non exposés au moteur global)
static CustomObj dyn_objs[100]; 
static bool level_initialized = false;

// =========================================================================================================
// 2. GESTION TOPOGRAPHIQUE DU NIVEAU 2
// =========================================================================================================

/**
 * @brief Calcule l'ordonnée (Y) du sol en fonction de l'abscisse (X) dans le monde.
 * Gère un large gouffre au milieu du niveau.
 * @param world_x Position X globale de l'entité.
 * @return float Coordonnée Y du sol (100.0f représente une chute mortelle).
 */
float get_ground_y_level2(float world_x) {
    if (world_x >= 960.0f && world_x < 1000.0f) return 28.0f; // Îlot de sécurité
    if (world_x > 820.0f && world_x < 1150.0f) return 100.0f; // Fosse principale
    return 28.0f; // Sol standard
}

/**
 * @brief Calcule l'ordonnée (Y) du plafond géométrique strict.
 * Gère les espaces confinés et crée une hitbox basse sous les Thwomps.
 * @param world_x Position X globale de l'entité.
 * @return float Coordonnée Y du plafond.
 */
float get_ceiling_y_level2_geo(float world_x) {
    // Espaces ouverts (Surface et Désert)
    if (world_x < 300.0f || world_x > 1700.0f) return -50.0f; 
    
    // Espace ouvert au-dessus de la fosse principale
    if (world_x >= 800.0f && world_x <= 1180.0f) return -50.0f; 
    
    // Détection de présence sous un Thwomp (Index réservés de 50 à 89)
    for(int i = 50; i < 90; i++) {
        if (blocks[i].alive && blocks[i].dest_x == 1 && blocks[i].w == 12) {
            // Abaissement drastique du plafond si on passe sous le bloc
            if (world_x >= blocks[i].x - 1.0f && world_x <= blocks[i].x + 13.0f) return 0.0f;
        }
    }
    
    // Plafond standard de la grotte (confiné)
    return 8.0f; 
}

/**
 * @brief Fonction d'assistance pour le test AABB de collision locale.
 */
static bool checkLocalCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// =========================================================================================================
// 3. SOUS-MOTEUR PHYSIQUE SPÉCIFIQUE AU NIVEAU 2
// =========================================================================================================

/**
 * @brief Met à jour l'état, les animations et la physique des objets dynamiques (Thwomps, Plateformes)
 * ainsi que les contraintes locales des ennemis.
 * @param world_x Position X globale permettant de limiter les calculs aux éléments proches.
 */
void update_level_2_subengine(float world_x) {
    // --- GESTION DES LIMITES DE L'IA ---
    // Maintient les ennemis confinés dans les limites géométriques de la grotte
    for(int i=0; i<20; i++) {
        if (turtles[i].alive) {
            if (turtles[i].x < 365.0f && turtles[i].vx < 0) turtles[i].vx = -turtles[i].vx; // Mur gauche
            if (turtles[i].x > 1620.0f && turtles[i].vx > 0) turtles[i].vx = -turtles[i].vx; // Mur droit
        }
    }

    // Optimisation : Mise à jour de position uniquement si appelé par la boucle du joueur
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

    // Régulateur temporel pour stabiliser l'animation des plateformes tournantes (indices 80-84)
    static uint32_t last_tick = 0;
    uint32_t now = lv_tick_get();
    if (now - last_tick < 30) {
        for (int i = 80; i <= 84; i++) {
            if (dyn_objs[i].active) {
                bool overlap = checkLocalCollision(mario.x, mario.y, 4, 8, dyn_objs[i].curr_x, dyn_objs[i].curr_y, blocks[i].w, blocks[i].h);
                if (overlap) {
                    // Détruit la plateforme si collision incorrecte (par en dessous ou sur le côté)
                    if (mario.vy < 0 || (mario.y + 4) > dyn_objs[i].curr_y + 1.0f) blocks[i].alive = false; 
                }
            }
        }
        return;
    }
    last_tick = now;

    // --- MACHINE A ÉTATS DES OBJETS DYNAMIQUES ---
    for (int i = 50; i < 90; i++) {
        if (!dyn_objs[i].active) continue;

        if (blocks[i].dest_x == 1) { 
            // Comportement de type Thwomp (Écrasement)
            switch (dyn_objs[i].state) {
                case 0: // Attente (Détection de proximité en X)
                    if (fabs(mario.x - dyn_objs[i].curr_x) < 24.0f && mario.y > dyn_objs[i].curr_y) dyn_objs[i].state = 1; 
                    break;
                case 1: { // Chute rapide
                    dyn_objs[i].curr_y += 3.0f; 
                    float target_y = get_ground_y_level2(dyn_objs[i].curr_x) - 16.0f;
                    if (target_y > 36.0f) target_y = 36.0f; // Limite de profondeur
                    if (dyn_objs[i].curr_y >= target_y) { dyn_objs[i].curr_y = target_y; dyn_objs[i].state = 2; dyn_objs[i].timer = 40; }
                    break;
                }
                case 2: // Stun (Impact au sol)
                    dyn_objs[i].timer--; 
                    if (dyn_objs[i].timer <= 0) dyn_objs[i].state = 3; 
                    break;
                case 3: // Remontée lente
                    dyn_objs[i].curr_y -= 0.5f; 
                    if (dyn_objs[i].curr_y <= dyn_objs[i].start_y) { dyn_objs[i].curr_y = dyn_objs[i].start_y; dyn_objs[i].state = 4; dyn_objs[i].timer = 100; } 
                    break;
                case 4: // Cooldown avant prochaine chute
                    dyn_objs[i].timer--; 
                    if (dyn_objs[i].timer <= 0) dyn_objs[i].state = 0; 
                    break;
            }
        } 
        else if (blocks[i].dest_x == 2) {
            // Comportement de type Plateforme Roulante descendante
            dyn_objs[i].curr_y += 0.25f; 
            if (dyn_objs[i].curr_y > 32.0f) dyn_objs[i].curr_y = -2.0f; // Réapparition en haut
        }

        // Application de la position calculée au moteur de collision principal
        blocks[i].x = dyn_objs[i].curr_x;
        blocks[i].y = dyn_objs[i].curr_y;
    }
}

/**
 * @brief Surcharge du calcul géométrique du plafond intégrant le sous-moteur physique.
 */
float get_ceiling_y_level2(float world_x) {
    update_level_2_subengine(world_x);
    return get_ceiling_y_level2_geo(world_x);
}

// =========================================================================================================
// 4. INITIALISATION DES ENTITÉS DU NIVEAU 2
// =========================================================================================================

/**
 * @brief Initialise toutes les entités (blocs, ennemis, pièces, tuyaux) du Niveau 2.
 * Configure également les dimensions du niveau et le thème de couleur.
 */
void init_level_2() {
    WORLD_W = 2600; 
    level_initialized = true;
    for(int i=0; i<100; i++) dyn_objs[i].active = false;

    flag = {2450.0f, 10.0f, 0, 0, 0, 1, 18, true, 0};            
    int b=0, c=0, t=0, p=0, pl=0, f=0; 
    
    // --- BIOME 1 : LA SURFACE (Entrée) ---
    blocks[b++] = {45.0f, 14.0f, 0, 0, 0, 5, 5, true, 1}; 
    pipes[p++] = {90.0f, 16.0f, 0, 0, 0, 8, 12, true, 360}; // Téléporteur d'entrée
    blocks[b++] = {120.0f, 0.0f, 0, 0, 0, 195, 28, true, 4}; // Mur de transition
    blocks[b++] = {315.0f, 0.0f, 0, 0, 0, 45, 28, true, 3};  

    // --- BIOME 2 : LA GROTTE OBSCURE ---
    
    // Zone 1 : Le couloir des Thwomps
    int s_thwomp = 50; 
    for(int i=0; i<4; i++) {
        int idx = s_thwomp + i;
        blocks[idx] = {420.0f + (i * 85.0f), 0.0f, 0, 0, 0, 12, 16, true, 1}; 
        dyn_objs[idx] = {420.0f + (i * 85.0f), 0.0f, 420.0f + (i * 85.0f), 0.0f, 0, 0, true};
        turtles[t++] = {450.0f + (i * 85.0f), 24.0f, -0.4f, 0, 0, 4, 4, true, 10};
    }
    flyers[f++] = {500.0f, 10.0f, -0.7f, 0, 10.0f, 4, 4, true, 0};
    flyers[f++] = {670.0f, 14.0f, -0.8f, 0, 14.0f, 4, 4, true, 0};

    // Zone 2 : Les piliers du vide
    float p_x = 840.0f;
    for(int i=0; i<6; i++) {
        blocks[b++] = {p_x, 15.0f, 0, 0, 0, 5, 50, true, 3}; 
        p_x += 24.0f; 
    }
    flyers[f++] = {860.0f, 12.0f, -0.7f, 0, 12.0f, 4, 4, true, 0};
    flyers[f++] = {910.0f, 10.0f, -0.9f, 0, 10.0f, 4, 4, true, 0};
    flyers[f++] = {960.0f, 14.0f, -0.6f, 0, 14.0f, 4, 4, true, 0};

    // Zone 3 : Cascade de plateformes
    int s_plat = 80;
    float start_plat_x = 1000.0f;
    for(int i=0; i<5; i++) {
        int idx = s_plat + i;
        float start_y = 24.0f - (i * 5.0f);
        // Initialisation avec identifiant visuel dest_x=2 (Vert)
        blocks[idx] = {start_plat_x + (i * 32.0f), start_y, 0, 0, 0, 9, 3, true, 3}; 
        dyn_objs[idx] = {blocks[idx].x, start_y, blocks[idx].x, start_y, 0, 0, true};
        blocks[idx].dest_x = 2; // Tag pour le sous-moteur physique
        coins[c++] = {blocks[idx].x + 2.0f, 10.0f, 0, 0, 0, 4, 4, true, 0};
    }
    flyers[f++] = {1060.0f, 8.0f, -0.8f, 0, 8.0f, 4, 4, true, 0};
    flyers[f++] = {1120.0f, 6.0f, -0.7f, 0, 6.0f, 4, 4, true, 0};

    blocks[b++] = {1140.0f, 20.0f, 0, 0, 0, 30, 5, true, 0}; // Plateforme de récupération

    // Zone 4 : Couloir fortifié (Tuyaux et Ennemis denses)
    float z4_x = 1150.0f;
    blocks[b++] = {z4_x, 28.0f, 0, 0, 0, 600, 4, true, 3}; 

    pipes[p++] = {z4_x + 60.0f, 24.0f, 0, 0, 0, 8, 4, true, 0}; plants[pl++] = {z4_x + 61.0f, 20.0f, 0, -0.4f, 0, 6, 6, true, 0};
    pipes[p++] = {z4_x + 180.0f, 24.0f, 0, 0, 0, 8, 4, true, 0}; plants[pl++] = {z4_x + 181.0f, 20.0f, 0, -0.5f, 0, 6, 6, true, 0};
    pipes[p++] = {z4_x + 300.0f, 24.0f, 0, 0, 0, 8, 4, true, 0}; plants[pl++] = {z4_x + 301.0f, 20.0f, 0, -0.4f, 0, 6, 6, true, 0};
    
    pipes[p++] = {z4_x + 420.0f, 24.0f, 0, 0, 0, 8, 4, true, 1800}; // Téléporteur de sortie

    for(int i=0; i<6; i++) {
        turtles[t++] = {z4_x + 80.0f + (i * 90.0f), 24.0f, -0.4f, 0, 0, 4, 4, true, 10}; 
        turtles[t++] = {z4_x + 110.0f + (i * 90.0f), 24.0f, -0.5f, 0, 0, 4, 4, true, 0};  
    }
    
    flyers[f++] = {z4_x + 100.0f, 12.0f, -0.6f, 0, 12.0f, 4, 4, true, 0};
    flyers[f++] = {z4_x + 220.0f, 16.0f, -0.9f, 0, 16.0f, 4, 4, true, 0};
    flyers[f++] = {z4_x + 340.0f, 10.0f, -0.7f, 0, 10.0f, 4, 4, true, 0}; 

    // Pièges Thwomps additionnels
    blocks[56] = {z4_x + 120.0f, 0.0f, 0, 0, 0, 12, 16, true, 1}; dyn_objs[56] = {z4_x + 120.0f, 0.0f, z4_x + 120.0f, 0.0f, 0, 0, true};
    blocks[57] = {z4_x + 380.0f, 0.0f, 0, 0, 0, 12, 16, true, 1}; dyn_objs[57] = {z4_x + 380.0f, 0.0f, z4_x + 380.0f, 0.0f, 0, 0, true};

    blocks[b++] = {z4_x + 480.0f, 0.0f, 0, 0, 0, 50, 28, true, 3}; // Mur de sortie

    // --- BIOME 3 : LE DÉSERT (X >= 1750) ---
    for(int step=0; step<5; step++) {
        for(int h=0; h<=step; h++) {
            // Le tag '2' indique au moteur de rendu principal de dessiner un bloc coloré JAUNE
            blocks[b++] = {1750.0f + step * 5.0f, 23.0f - h * 5.0f, 0, 0, 0, 5, 5, true, 2}; 
        }
    }
    blocks[b++] = {1775.0f, 28.0f, 0, 0, 0, 200, 4, true, 2}; // Sol de la zone désertique
    
    flag.x = 1850.0f; 
    WORLD_W = 1950;  // Restriction de la zone de jeu
}