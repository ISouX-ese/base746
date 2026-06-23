#pragma once
#include <stdint.h>

// ========================================================================
// STRUCTURES DE DONNÉES
// ========================================================================

/**
 * @brief Structure générique représentant un objet du jeu (Joueur, Ennemi, Objet, Décor).
 * x, y      : Position absolue dans le monde.
 * vx, vy    : Vecteurs de vélocité (déplacement horizontal et vertical).
 * base_y    : Position Y de référence (utile pour les mouvements sinusoïdaux des ennemis volants).
 * w, h      : Dimensions (largeur, hauteur) définissant la Hitbox pour les collisions.
 * alive     : État de l'entité (true si active, false si détruite ou non initialisée).
 * dest_x    : Variable multifonction (Destination de téléportation pour les tuyaux, 
 *             identifiant de type pour les blocs ou comportement spécifique pour les ennemis).
 */
struct Entity { float x, y, vx, vy, base_y; int w, h; bool alive; int dest_x; }; 

/**
 * @brief Structure allégée dédiée spécifiquement au Boss final.
 * hp        : Points de vie restants (Hit Points).
 */
struct Boss { float x, y, vy; int hp; bool alive; };

// ========================================================================
// VARIABLES GLOBALES PARTAGÉES (Définies dans main.cpp)
// ========================================================================

// --- Éléments du décor et objets interactifs ---
extern Entity blocks[100];      // Blocs solides (Cubes, plateformes, blocs magiques)
extern Entity pipes[15];        // Tuyaux de téléportation
extern Entity plants[15];       // Plantes carnivores (Placées dans les tuyaux)
extern Entity coins[60];        // Pièces à collecter

// --- Ennemis et projectiles ---
extern Entity turtles[20];      // Ennemis terrestres (Goombas, Koopas)
extern Entity bouncers[15];     // Ennemis sauteurs
extern Entity flyers[15];       // Ennemis volants (Chauves-souris)
extern Entity fireballs[15];    // Projectiles (Lave statique ou tirs du Boss)

// --- Entités uniques ---
extern Entity flag;             // Drapeau de fin de niveau
extern Boss boss;               // Entité du Boss final (Niveau 5)

// --- Joueur et Bonus ---
extern Entity mario;            // Entité contrôlée par le joueur
extern Entity mush;             // Champignon (Croissance ou 1-UP)

// --- Propriétés globales de l'environnement ---
extern int WORLD_W;             // Largeur totale du niveau courant (en pixels)
extern bool ground_r;           // Composante Rouge de la couleur du sol
extern bool ground_g;           // Composante Verte de la couleur du sol
extern bool ground_b;           // Composante Bleue de la couleur du sol

// ========================================================================
// MOTEUR DE GÉNÉRATION DES NIVEAUX ET TOPOLOGIE
// ========================================================================
// Chaque niveau possède ses propres fonctions mathématiques pour définir 
// le relief (collisions sol/plafond) et une fonction d'initialisation pour 
// peupler les tableaux d'entités.

// --- NIVEAU 1 : Plaines & Grotte ---
float get_ground_y_level1(float world_x); 
float get_ceiling_y_level1(float world_x); 
void init_level_1();

// --- NIVEAU 2 : Souterrains & Plateformes ---
float get_ground_y_level2(float world_x); 
float get_ceiling_y_level2(float world_x); 
void init_level_2();

// --- NIVEAU 3 : Ciel & Pyramides ---
float get_ground_y_level3(float world_x); 
float get_ceiling_y_level3(float world_x); 
void init_level_3();

// --- NIVEAU 4 : Château & Lave ---
float get_ground_y_level4(float world_x); 
float get_ceiling_y_level4(float world_x); 
void init_level_4();

// --- NIVEAU 5 : Arène du Boss ---
float get_ground_y_level5(float world_x); 
float get_ceiling_y_level5(float world_x); 
void init_level_5();