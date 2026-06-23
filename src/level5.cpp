#include "game.h"
#include <math.h>

// =========================================================================
// GESTION TOPOGRAPHIQUE DU NIVEAU 5
// =========================================================================

/**
 * @brief Calcule l'ordonnée (Y) du sol en fonction de l'abscisse (X) dans le monde.
 * Gère la géométrie de l'arène finale et inclut une fosse de lave initiale.
 * @param world_x Position X globale de l'entité.
 * @return float Coordonnée Y du sol (100.0f représente une chute mortelle).
 */
float get_ground_y_level5(float world_x) {
    if (world_x > 200.0f && world_x < 220.0f) return 100.0f; 
    return 28.0f;
}

/**
 * @brief Calcule l'ordonnée (Y) du plafond en fonction de l'abscisse (X) dans le monde.
 * @param world_x Position X globale de l'entité.
 * @return float Coordonnée Y du plafond.
 */
float get_ceiling_y_level5(float world_x) { return 0.0f; }

// =========================================================================
// INITIALISATION DES ENTITÉS DU NIVEAU 5
// =========================================================================

/**
 * @brief Initialise toutes les entités (blocs, ennemis, projectiles) du Niveau 5.
 * Ce niveau constitue l'arène du combat final[cite: 60]. Le drapeau de fin de niveau 
 * est désactivé au profit d'une condition de victoire liée à la défaite du Boss[cite: 60].
 */
void init_level_5() {
    WORLD_W = 700; 
    
    // Le drapeau de fin est désactivé (alive = false) ; la victoire est gérée par l'état du Boss
    flag = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, false, 0};
    int b=0, c=0, t=0, p=0, pl=0, fb=0, bc=0, f=0;

    // --- SÉQUENCE DES BONUS COMPLÉMENTAIRES ---
    // Configuration des blocs d'objets utilisables avant l'entrée dans l'arène
    blocks[b++] = {80.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 6};  // Bloc de récompense (Vie supplémentaire)
    blocks[b++] = {120.0f, 14.0f, 0.0f, 0.0f, 0.0f, 5, 5, true, 5}; // Bloc de récompense (Étoile d'invincibilité)

    // Ennemi terrestre posté dans le couloir d'approche
    turtles[t++] = {160.0f, 24.0f, -0.4f, 0.0f, 0.0f, 4, 4, true, 0}; 

    // --- ZONE DE SÉCURITÉ DE LA FOSSE ---
    // Obstacles de transition protégeant l'accès direct à la salle du Boss
    fireballs[fb++] = {210.0f, 32.0f, 0.0f, -4.5f, 0.0f, 4, 4, true, 0};
    flyers[f++] = {210.0f, 8.0f, -0.6f, 0.0f, 8.0f, 4, 4, true, 0};

    // --- CONFIGURATION DE L'ARÈNE DE COMBAT ---
    // Plateformes statiques positionnées pour l'esquive et les sauts offensifs
    blocks[b++] = {380.0f, 16.0f, 0.0f, 0.0f, 0.0f, 25, 5, true, 4}; 
    blocks[b++] = {460.0f, 10.0f, 0.0f, 0.0f, 0.0f, 25, 5, true, 4}; 
    blocks[b++] = {540.0f, 16.0f, 0.0f, 0.0f, 0.0f, 25, 5, true, 4}; 

    // Projectiles thermiques verticaux (obstacles environnementaux)
    fireballs[fb++] = {430.0f, 32.0f, 0.0f, -5.0f, 0.0f, 4, 4, true, 0};
    fireballs[fb++] = {510.0f, 32.0f, 0.0f, -4.5f, 0.0f, 4, 4, true, 0};

    // Ennemi patrouilleur posté à l'avant de l'arène
    turtles[t++] = {350.0f, 24.0f, -0.6f, 0.0f, 0.0f, 4, 4, true, 10};

    // Ennemi volant affecté à la restriction de l'espace aérien supérieur
    flyers[f++] = {480.0f, -2.0f, -0.8f, 0.0f, -2.0f, 4, 4, true, 0};

    // --- PARAMÉTRAGE DE L'ENTITÉ DU BOSS ---
    // Initialisation des propriétés physiques et des points de vie de Bowser [cite: 60]
    boss.alive = true;
    boss.x = 620.0f; 
    boss.y = 10.0f;
    boss.vy = 1.0f;
    boss.hp = 5; 
}