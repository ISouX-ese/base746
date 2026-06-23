#include "lvgl.h"
#include <math.h>
#include <cstdlib>
#include "game.h" 

// =========================================================================================================
// 1. VARIABLES GLOBALES DE L'APPLICATION
// =========================================================================================================

// Tableaux d'allocation mémoire fixes pour les entités du moteur de jeu
Entity blocks[100]; Entity pipes[15]; Entity plants[15]; Entity coins[60];
Entity turtles[20]; Entity bouncers[15]; Entity flyers[15]; Entity fireballs[15];
Entity flag; Boss boss; int WORLD_W = 600;

// Drapeaux d'activation des canaux RGB pour le rendu du sol
bool ground_r = true, ground_g = true, ground_b = true;

// Registres d'état des entités dynamiques principales
Entity mario; 
Entity mush;

// Variables volatiles partagées entre les tâches d'affichage et l'interface utilisateur
volatile int current_score = 0;                             
volatile int current_level = 1; 

volatile int current_coins = 0;
volatile int current_hp = 3;
bool show_damage_screen = false;
int damage_timer = 0;

// Paramètres de la caméra logique et orientation graphique du joueur
float camera_x = 0;   
float camera_y = 0;                                     
bool facing_right = true; 

// Drapeaux d'interruption et compteurs de synchronisation temporelle
volatile bool request_game_reset = true; 
volatile bool request_star = false; 
volatile int star_cooldown = 0;     
int star_active_counter = 0;        
bool ctrl_DOWN = false; 

// =========================================================================================================
// 2. LOGIQUE ET CONFIGURATION DE L'INTERFACE GRAPHIQUE (LVGL)
// =========================================================================================================

/**
 * @brief Fonction de rappel (callback) pour la gestion des événements des boutons LVGL.
 * Gère la sélection des niveaux et l'état des entrées utilisateur déportées.
 */
static void btn_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    int action = (int)(intptr_t)lv_event_get_user_data(e);
    
    if(code == LV_EVENT_CLICKED) {
        if(action >= 1 && action <= 5) { current_level = action; request_game_reset = true; } 
    }
    
    if(action == 100) {
        if(code == LV_EVENT_PRESSED) ctrl_DOWN = true;
        else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) ctrl_DOWN = false;
    }
}

/**
 * @brief Factory de création de boutons stylisés pour la sélection des niveaux.
 */
lv_obj_t * create_lvl_btn(lv_obj_t * p, const char * txt, int x_offset, int lvl, uint32_t c, uint32_t text_c) {
    lv_obj_t * btn = lv_button_create(p); 
    lv_obj_set_size(btn, 85, 45); 
    lv_obj_align(btn, LV_ALIGN_CENTER, x_offset, 25); 
    lv_obj_set_style_bg_color(btn, lv_color_hex(c), 0); 
    lv_obj_add_event_cb(btn, btn_event_handler, LV_EVENT_ALL, (void*)(intptr_t)lvl); 
    lv_obj_t * l = lv_label_create(btn); lv_label_set_text(l, txt); 
    lv_obj_set_style_text_color(l, lv_color_hex(text_c), 0); lv_obj_center(l); 
    return btn;
}

/**
 * @brief Initialisation de la scène graphique du menu principal via la bibliothèque LVGL.
 * Configure le fond, génère un champ d'étoiles procédural et instancie les widgets de contrôle.
 */
void testLvgl() {
    lv_obj_t * screen = lv_screen_active();
    lv_obj_clean(screen); 
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B001A), 0); 
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x000000), 0); 
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    
    // Génération du décor d'arrière-plan
    for(int i = 0; i < 60; i++) {
        lv_obj_t * star = lv_obj_create(screen); int size = (i % 3) + 1; 
        lv_obj_set_size(star, size, size); lv_obj_set_style_radius(star, LV_RADIUS_CIRCLE, 0);
        uint32_t color = 0xFFFFFF; if (i % 4 == 0) color = 0x00FFFF; if (i % 5 == 0) color = 0xFF00FF; 
        lv_obj_set_style_bg_color(star, lv_color_hex(color), 0); lv_obj_set_style_border_width(star, 0, 0); 
        lv_obj_set_style_bg_opa(star, 100 + (i % 155), 0); lv_obj_set_pos(star, (i * 83) % 480, (i * 67) % 272); 
    }

    // Instanciation et mise à l'échelle du titre principal
    lv_obj_t * title_label = lv_label_create(screen);
    lv_label_set_text(title_label, "M A R I O");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFF0000), 0); 
#if LV_FONT_MONTSERRAT_24
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0);
#endif
    lv_obj_set_style_transform_pivot_x(title_label, 0, 0);
    lv_obj_set_style_transform_pivot_y(title_label, 0, 0);
    lv_obj_set_style_transform_scale(title_label, 2150, 0); 
    lv_obj_set_pos(title_label, 2, -5);  

    // Création de la matrice de sélection des niveaux
    create_lvl_btn(screen, "NV 1", -190, 1, 0x22B14C, 0xFFFFFF); 
    create_lvl_btn(screen, "NV 2",  -95, 2, 0x00FFFF, 0x000000); 
    create_lvl_btn(screen, "NV 3",    0, 3, 0xFFD700, 0x000000); 
    create_lvl_btn(screen, "NV 4",   95, 4, 0xFF0000, 0xFFFFFF); 
    create_lvl_btn(screen, "NV 5",  190, 5, 0xFFFFFF, 0x000000); 

    // Bouton d'action d'entrée en jeu
    lv_obj_t * down_btn = lv_button_create(screen); lv_obj_set_size(down_btn, 180, 60); 
    lv_obj_align(down_btn, LV_ALIGN_BOTTOM_MID, 0, -10); 
    lv_obj_set_style_bg_color(down_btn, lv_color_hex(0x22B14C), 0); 
    lv_obj_set_style_radius(down_btn, 30, 0); lv_obj_add_event_cb(down_btn, btn_event_handler, LV_EVENT_ALL, (void*)(intptr_t)100); 
    lv_obj_t * down_lbl = lv_label_create(down_btn); lv_label_set_text(down_lbl, "TUYAU ->"); 
    lv_obj_set_style_text_color(down_lbl, lv_color_hex(0xFFFFFF), 0); lv_obj_center(down_lbl);
}

// =========================================================================================================
// 3. PILOTE MATÉRIEL ET FONCTIONS DE RENDU BAS NIVEAU (MATRICE LED HUB75)
// =========================================================================================================
#ifdef ARDUINO                                              
#include "lvglDrivers.h"                                    
#define PIN_R1 D0                                           
#define PIN_G1 D1                                           
#define PIN_B1 D2                                           
#define PIN_R2 D3                                           
#define PIN_G2 D4                                           
#define PIN_B2 D5                                           
#define PIN_A  A0                                           
#define PIN_B  A1                                           
#define PIN_C  A2                                           
#define PIN_D  A3                                           
#define PIN_CLK D8                                          
#define PIN_LAT D9                                          
#define PIN_OE  D10                                         
#define PIN_BTN_JUMP  D6
#define PIN_JOY_RIGHT D11
#define PIN_JOY_LEFT  D12
#define PIN_JOY_DOWN  D7 

// Tampons de rendu pour implémenter un mécanisme de double-buffering matériel
volatile uint8_t ecran_aff[32][64] = {0}; 
uint8_t ecran[32][64] = {0};              

/**
 * @brief Écrit la valeur chromatique d'un pixel au sein du frame-buffer local.
 */
void drawPixel(int x, int y, bool r, bool g, bool b) { if (x >= 0 && x < 64 && y >= 0 && y < 32) ecran[y][x] = (r ? 1 : 0) | (g ? 2 : 0) | (b ? 4 : 0); }

/**
 * @brief Dessine le contour d'un rectangle non plein.
 */
void drawRect(int x, int y, int w, int h, bool r, bool g, bool b) {
    for (int i = 0; i < w; i++) { drawPixel(x + i, y, r, g, b); drawPixel(x + i, y + h - 1, r, g, b); }
    for (int i = 0; i < h; i++) { drawPixel(x, y + i, r, g, b); drawPixel(x + w - 1, y + i, r, g, b); }
}

/**
 * @brief Remplit une zone rectangulaire avec une couleur unie.
 */
void drawFilledRect(int x, int y, int w, int h, bool r, bool g, bool b) { for (int i = 0; i < w; i++) for (int j = 0; j < h; j++) drawPixel(x + i, y + j, r, g, b); }

/**
 * @brief Réinitialise l'intégralité du frame-buffer local à zéro.
 */
void clearEcran() { for(int y=0; y<32; y++) for(int x=0; x<64; x++) ecran[y][x] = 0; }

/**
 * @brief Test d'intersection AABB (Axis-Aligned Bounding Box) pour le calcul des hitboxes.
 */
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) { return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2); }

// Matrices de polices de caractères matricielles (3x5) pour les caractères numériques
const uint8_t digits[10][5] = {
    {0b111, 0b101, 0b101, 0b101, 0b111}, {0b010, 0b110, 0b010, 0b010, 0b111}, {0b111, 0b001, 0b111, 0b100, 0b111}, 
    {0b111, 0b001, 0b111, 0b001, 0b111}, {0b101, 0b101, 0b111, 0b001, 0b001}, {0b111, 0b100, 0b111, 0b001, 0b111}, 
    {0b111, 0b100, 0b111, 0b101, 0b111}, {0b111, 0b001, 0b010, 0b010, 0b010}, {0b111, 0b101, 0b111, 0b101, 0b111}, 
    {0b111, 0b101, 0b111, 0b001, 0b111}  
};

/**
 * @brief Rendu graphique d'un chiffre sur la matrice de LED.
 */
void drawDigit(int x, int y, int d, bool r, bool g, bool b) {
    if(d < 0 || d > 9) return;
    for(int row=0; row<5; row++) for(int col=0; col<3; col++) if(digits[d][row] & (1 << (2 - col))) drawPixel(x + col, y + row, r, g, b);
}

// ---------------------------------------------------------------------------------------------------------
// PRÉMICES DE RENDU DU SPRITE SHEET EN MODE PIXEL-ART
// ---------------------------------------------------------------------------------------------------------

void drawHeart(int x, int y) {
    drawPixel(x+1, y, true, false, false); drawPixel(x+3, y, true, false, false);
    drawFilledRect(x, y+1, 5, 2, true, false, false); drawFilledRect(x+1, y+3, 3, 1, true, false, false); drawPixel(x+2, y+4, true, false, false);
}

void drawPipe(int x, int y, int h, bool inverted) {
    if (!inverted) { drawFilledRect(x, y + 3, 8, h - 3, false, true, false); drawFilledRect(x - 1, y, 10, 3, false, true, false); drawFilledRect(x, y + 3, 8, 1, false, false, false); } 
    else { drawFilledRect(x, y, 8, h - 3, false, true, false); drawFilledRect(x - 1, y + h - 3, 10, 3, false, true, false); drawFilledRect(x, y + h - 4, 8, 1, false, false, false); }
}

void drawThwomp(int x, int y) {
    drawFilledRect(x+1, y+1, 10, 14, true, true, true); 
    drawRect(x, y, 12, 16, true, false, false);         
    drawPixel(x+2, y, false, false, false); drawPixel(x+5, y, false, false, false); drawPixel(x+8, y, false, false, false);
    drawPixel(x+2, y+15, false, false, false); drawPixel(x+5, y+15, false, false, false); drawPixel(x+8, y+15, false, false, false);
    drawPixel(x, y+3, false, false, false); drawPixel(x, y+7, false, false, false); drawPixel(x, y+11, false, false, false);
    drawPixel(x+11, y+3, false, false, false); drawPixel(x+11, y+7, false, false, false); drawPixel(x+11, y+11, false, false, false);
    drawFilledRect(x+2, y+5, 3, 3, true, false, false); drawFilledRect(x+7, y+5, 3, 3, true, false, false);
    drawPixel(x+3, y+6, false, false, false); drawPixel(x+8, y+6, false, false, false); 
    drawPixel(x+2, y+4, false, false, false); drawPixel(x+3, y+5, false, false, false); drawPixel(x+4, y+6, false, false, false);
    drawPixel(x+9, y+4, false, false, false); drawPixel(x+8, y+5, false, false, false); drawPixel(x+7, y+6, false, false, false);
    drawFilledRect(x+3, y+11, 6, 2, false, false, false);
    drawPixel(x+4, y+11, true, true, true); drawPixel(x+7, y+11, true, true, true);
    drawPixel(x+5, y+12, true, true, true); drawPixel(x+6, y+12, true, true, true);
}

void drawMushroom(int x, int y, bool is_green) {
    bool r = is_green ? false : true;
    bool g = is_green ? true : false;
    drawFilledRect(x+1, y, 4, 3, r, g, false); 
    drawPixel(x+1, y+1, true, true, true); 
    drawPixel(x+4, y+1, true, true, true); 
    drawFilledRect(x+2, y+3, 2, 2, true, true, true); 
}

void drawGoomba(int x, int y) {
    drawFilledRect(x, y, 4, 2, true, false, false); drawPixel(x+1, y+1, true, true, false); drawPixel(x+2, y+1, true, true, false); drawFilledRect(x+1, y+2, 2, 1, true, false, false); drawPixel(x, y+3, true, true, true); drawPixel(x+3, y+3, true, true, true);
}

void drawTurtle(int x, int y, int frame) {
    drawFilledRect(x, y, 3, 2, false, true, false); drawPixel(x+1, y, true, true, true); drawFilledRect(x, y+2, 4, 2, true, false, false); 
    if ((frame/5)%2==0) { drawPixel(x, y+4, true, true, true); drawPixel(x+2, y+4, true, true, true); } else { drawPixel(x+1, y+4, true, true, true); drawPixel(x+3, y+4, true, true, true); }
}

void drawShell(int x, int y) {
    drawFilledRect(x, y+1, 4, 3, true, false, false); drawFilledRect(x+1, y+1, 2, 1, true, true, true); drawPixel(x, y+4, true, true, true); drawPixel(x+3, y+4, true, true, true); 
}

void drawPlant(int x, int y, bool inverted) {
    if (inverted) { drawFilledRect(x+2, y, 2, 3, false, true, false); drawPixel(x+1, y+1, false, true, false); drawPixel(x+4, y+1, false, true, false); drawFilledRect(x+1, y+3, 4, 3, true, false, false); drawPixel(x+2, y+4, true, true, true); drawPixel(x+3, y+4, true, true, true);
    } else { drawFilledRect(x+1, y, 4, 3, true, false, false); drawPixel(x+2, y+1, true, true, true); drawPixel(x+3, y+1, true, true, true); drawFilledRect(x+2, y+3, 2, 3, false, true, false); drawPixel(x+1, y+4, false, true, false); drawPixel(x+4, y+4, false, true, false); }
}

void drawCoinAnim(int x, int y, int frame) {
    if ((frame / 12) % 2 == 0) { drawFilledRect(x+1, y, 2, 4, true, true, false); drawFilledRect(x, y+1, 4, 2, true, true, false); drawFilledRect(x+1, y+1, 2, 2, false, false, false); } else { drawFilledRect(x+1, y, 2, 4, true, true, false); }
}

void drawBouncer(int x, int y) {
    drawPixel(x+1, y, true, true, true); drawPixel(x+2, y, true, true, true); drawFilledRect(x, y+1, 4, 2, true, false, false); drawPixel(x, y+3, true, true, false); drawPixel(x+3, y+3, true, true, false);
}

void drawFlyer(int x, int y, int frame) {
    drawFilledRect(x, y+1, 3, 2, false, true, false); drawPixel(x+1, y+1, true, true, false); if ((frame / 6) % 2 == 0) { drawPixel(x+3, y, true, true, true); drawPixel(x+2, y+1, true, true, true); } else { drawPixel(x+2, y+2, true, true, true); drawPixel(x+3, y+3, true, true, true); }
}

void drawFireball(int x, int y, int frame) {
    drawFilledRect(x+1, y+1, 2, 2, true, true, false); int step = (frame / 4) % 4; if (step == 0) { drawPixel(x, y, true, false, false); drawPixel(x+3, y+3, true, false, false); } else if (step == 1) { drawPixel(x+3, y, true, false, false); drawPixel(x, y+3, true, false, false); } else if (step == 2) { drawPixel(x+1, y, true, false, false); drawPixel(x+2, y+3, true, false, false); } else { drawPixel(x, y+1, true, false, false); drawPixel(x+3, y+2, true, false, false); }
}

/**
 * @brief Rendu graphique du Boss final (Bowser). Inclut l'affichage de l'état étourdi.
 */
void drawBowser(int x, int y, int hp, int frame, bool stunned) {
    if (stunned) {
        drawFilledRect(x+3, y+7, 9, 5, false, true, false);
        drawPixel(x+5, y+6, true, true, true); drawPixel(x+9, y+6, true, true, true);
        drawFilledRect(x-1, y+7, 5, 5, true, true, false);
        drawPixel(x, y+9, false, false, false); 
        if ((frame % 8) < 4) drawPixel(x-2, y+5, true, true, true);
        return;
    }
    
    drawFilledRect(x+5, y+2, 7, 9, false, true, false);
    drawPixel(x+6, y+1, true, true, true);
    drawPixel(x+10, y+1, true, true, true);
    drawPixel(x+12, y+5, true, true, true);
    drawFilledRect(x+2, y+5, 6, 7, true, true, false);
    drawFilledRect(x, y+1, 6, 5, true, true, false);
    drawFilledRect(x-2, y+2, 3, 4, true, true, false);
    drawFilledRect(x+2, y-1, 4, 2, true, false, false);
    drawPixel(x+1, y+2, false, false, false);
    drawFilledRect(x-1, y+7, 5, 2, true, true, false);
    drawPixel(x-2, y+7, true, true, true);
    
    if (hp == 1 && (frame % 4) < 2) drawRect(x-2, y-1, 15, 13, true, false, false);
}

/**
 * @brief Fonction d'interruption du Timer matériel.
 * Assure le multiplexage temporel et le rafraîchissement physique ligne par ligne de la matrice LED.
 */
void rafraichirEcran() {
    static uint8_t row = 0; digitalWrite(PIN_OE, HIGH);                             
    for (uint8_t col = 0; col < 64; col++) {
        uint8_t pH = ecran_aff[row][col], pB = ecran_aff[row + 16][col];         
        digitalWrite(PIN_R1, pH & 0x01); digitalWrite(PIN_G1, pH & 0x02); digitalWrite(PIN_B1, pH & 0x04); 
        digitalWrite(PIN_R2, pB & 0x01); digitalWrite(PIN_G2, pB & 0x02); digitalWrite(PIN_B2, pB & 0x04);  
        digitalWrite(PIN_CLK, HIGH); digitalWrite(PIN_CLK, LOW);
    }
    digitalWrite(PIN_LAT, HIGH); digitalWrite(PIN_LAT, LOW);
    digitalWrite(PIN_A, row & 0x01); digitalWrite(PIN_B, row & 0x02); digitalWrite(PIN_C, row & 0x04); digitalWrite(PIN_D, row & 0x08);            
    digitalWrite(PIN_OE, LOW); row++; if (row >= 16) row = 0;                                 
}

/**
 * @brief Configuration des entrées/sorties matérielles et instanciation du timer de balayage.
 */
void mySetup() {
    pinMode(PIN_R1, OUTPUT); pinMode(PIN_G1, OUTPUT); pinMode(PIN_B1, OUTPUT); pinMode(PIN_R2, OUTPUT); pinMode(PIN_G2, OUTPUT); pinMode(PIN_B2, OUTPUT);
    pinMode(PIN_A, OUTPUT); pinMode(PIN_B, OUTPUT); pinMode(PIN_C, OUTPUT); pinMode(PIN_D, OUTPUT); pinMode(PIN_CLK, OUTPUT); pinMode(PIN_LAT, OUTPUT); pinMode(PIN_OE, OUTPUT);
    pinMode(PIN_BTN_JUMP, INPUT_PULLUP); pinMode(PIN_JOY_RIGHT, INPUT_PULLUP); pinMode(PIN_JOY_LEFT, INPUT_PULLUP); pinMode(PIN_JOY_DOWN, INPUT_PULLUP);
    digitalWrite(PIN_OE, HIGH); digitalWrite(PIN_LAT, LOW); digitalWrite(PIN_CLK, LOW);
    HardwareTimer *MyTim = new HardwareTimer(TIM3); MyTim->setOverflow(100, MICROSEC_FORMAT); MyTim->attachInterrupt(rafraichirEcran); MyTim->resume();                                        
    testLvgl();                                             
}
void loop() {}
#endif

// =========================================================================================================
// 4. COUCHE D'ABSTRACTION DU GESTIONNAIRE DE NIVEAUX
// =========================================================================================================

float get_ground_y(float world_x, int level) {
    if (level == 1) return get_ground_y_level1(world_x);
    if (level == 2) return get_ground_y_level2(world_x);
    if (level == 3) return get_ground_y_level3(world_x);
    if (level == 4) return get_ground_y_level4(world_x);
    if (level == 5) return get_ground_y_level5(world_x);
    return 28.0f; 
}

float get_ceiling_y(float world_x, int level) {
    if (level == 1) return get_ceiling_y_level1(world_x);
    if (level == 2) return get_ceiling_y_level2(world_x);
    if (level == 3) return get_ceiling_y_level3(world_x);
    if (level == 4) return get_ceiling_y_level4(world_x);
    if (level == 5) return get_ceiling_y_level5(world_x);
    return 0.0f; 
}

/**
 * @brief Réinitialise la mémoire volatile des structures et appelle le sous-module d'initialisation du niveau.
 */
void load_level(int lvl) {
    mario = {20, 24, 0, 0, 0, 4, 4, true, 0}; facing_right = true; 
    mush = {0, 0, 1.0f, 0, 0, 6, 5, false, 0}; boss = {0, 0, 0, 0, false}; 
    for(int i=0; i<100; i++) blocks[i].alive = false; for(int i=0; i<60; i++) coins[i].alive = false;
    for(int i=0; i<20; i++) turtles[i].alive=false; for(int i=0; i<15; i++) { pipes[i].alive=false; plants[i].alive=false; bouncers[i].alive=false; flyers[i].alive=false; fireballs[i].alive=false; }
    if (lvl == 1) init_level_1(); else if (lvl == 2) init_level_2(); else if (lvl == 3) init_level_3(); else if (lvl == 4) init_level_4(); else if (lvl == 5) init_level_5();
}

// =========================================================================================================
// 5. BOUCLE PRINCIPALE DE TRAITEMENT DU JEU (THREAD FREERTOS)
// =========================================================================================================

void myTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();         
    int mh = 4; bool grounded = false; bool is_game_over = false; bool win_level = false; bool game_beaten = false;                                       
    int global_frame = 0;
    
    int game_over_timer = 0; 
    int boss_stun_timer = 0; 
    int game_beaten_timer = 0; 

    // Représentation matricielle des caractères pour les écrans de transition et de fin de partie
    const uint8_t letters[20][5] = {
        {0b010,0b101,0b111,0b101,0b101}, // 0: A
        {0b111,0b100,0b100,0b100,0b111}, // 1: C
        {0b110,0b101,0b101,0b101,0b110}, // 2: D
        {0b111,0b100,0b111,0b100,0b111}, // 3: E
        {0b111,0b100,0b101,0b101,0b111}, // 4: G
        {0b100,0b100,0b100,0b100,0b111}, // 5: L
        {0b101,0b111,0b101,0b101,0b101}, // 6: M
        {0b101,0b111,0b111,0b101,0b101}, // 7: N
        {0b111,0b101,0b101,0b101,0b111}, // 8: O
        {0b111,0b101,0b111,0b100,0b100}, // 9: P
        {0b110,0b101,0b110,0b101,0b101}, // 10: R
        {0b111,0b010,0b010,0b010,0b010}, // 11: T
        {0b101,0b101,0b101,0b101,0b010}, // 12: V
        {0b101,0b101,0b010,0b101,0b101}, // 13: X
        {0b101,0b101,0b111,0b101,0b101}, // 14: H
        {0b111,0b010,0b010,0b010,0b111}, // 15: I 
        {0b101,0b101,0b101,0b101,0b111}, // 16: U 
        {0b110,0b101,0b110,0b101,0b110}, // 17: B 
        {0b010,0b110,0b010,0b010,0b111}, // 18: 1 
        {0b000,0b000,0b000,0b000,0b000}  // 19: ESPACE 
    };

    while (1) {
        global_frame++;
        #ifdef ARDUINO
        bool p_left = (digitalRead(PIN_JOY_LEFT) == LOW); bool p_right = (digitalRead(PIN_JOY_RIGHT) == LOW);
        bool p_up = (digitalRead(PIN_BTN_JUMP) == LOW); bool p_down = (digitalRead(PIN_JOY_DOWN) == LOW) || ctrl_DOWN; 
        #else
        bool p_left = false; bool p_right = false; bool p_up = false; bool p_down = ctrl_DOWN;
        #endif

        // Décrémentation des temporisateurs d'invincibilité
        if (star_cooldown > 0) star_cooldown--; if (star_active_counter > 0) star_active_counter--; 
        if (request_star && star_cooldown == 0) { request_star = false; star_active_counter = 150; star_cooldown = 0; }
        bool is_invincible = (star_active_counter > 0);

        if (game_over_timer > 0) game_over_timer--; 
        if (game_beaten_timer > 0) game_beaten_timer--; 

        // Logique de réinitialisation de l'arène ou changement de niveau
        if (request_game_reset || ((is_game_over || win_level || game_beaten) && p_up && game_over_timer == 0 && game_beaten_timer == 0)) {
            if (is_game_over) { current_score = 0; current_coins = 0; current_hp = 3; } 
            else if (game_beaten) { current_level = 1; current_score = 0; current_coins = 0; current_hp = 3; } 
            else if (win_level) current_level++;
            if (current_level > 5) current_level = 1;
            is_game_over = false; win_level = false; game_beaten = false; request_game_reset = false; show_damage_screen = false; camera_x = 0; camera_y = 0; mh = 4; grounded = false;
            boss_stun_timer = 0; game_beaten_timer = 0;
            load_level(current_level);
        }

        // Macro d'application des dégâts au joueur
        #define TAKE_DAMAGE() do { \
            if (!is_invincible) { \
                if (mh == 8) { mh = 4; mario.y += 4; star_active_counter = 60; } \
                else { current_hp--; if (current_hp > 0) { show_damage_screen = true; damage_timer = 60; } else { is_game_over = true; game_over_timer = 45; } } \
            } \
        } while(0)

        // -------------------------------------------------------------------------------------------------
        // GESTION DES MOTEURS DE TRANSITION D'ÉCRAN
        // -------------------------------------------------------------------------------------------------
        if (is_game_over || win_level || game_beaten) {
            clearEcran();
            if (is_game_over) {                             
                int l1[]={4, 0, 6, 3}; int l2[]={8, 12, 3, 10}; 
                for (int i=0; i<4; i++) for (int r=0; r<5; r++) for (int c=0; c<3; c++) {
                    if (letters[l1[i]][r] & (1<<(2-c))) drawFilledRect(17+(i*8)+(c*2), 4+(r*2), 2, 2, true, false, false); 
                    if (letters[l2[i]][r] & (1<<(2-c))) drawFilledRect(17+(i*8)+(c*2), 18+(r*2), 2, 2, true, false, false); 
                }
                drawRect(0, 0, 64, 32, true, false, false); 
            } else if (win_level) {                         
                int l1[]={7, 3, 13, 11}; int l2[]={5, 3, 12, 3, 5}; 
                for (int i=0; i<4; i++) for(int r=0; r<5; r++) for(int c=0; c<3; c++) if(letters[l1[i]][r] & (1<<(2-c))) drawFilledRect(18+(i*7)+(c*2), 4+(r*2), 2, 2, false, true, true); 
                for (int i=0; i<5; i++) for(int r=0; r<5; r++) for(int c=0; c<3; c++) if(letters[l2[i]][r] & (1<<(2-c))) drawFilledRect(14+(i*7)+(c*2), 18+(r*2), 2, 2, false, true, true); 
                drawRect(0, 0, 64, 32, false, false, true); 
            } else if (game_beaten) {                       
                // Séquence d'effets de célébration post-victoire
                if (game_beaten_timer > 60) {
                    for(int k=0; k<80; k++) {
                        int rx = rand() % 64; 
                        int ry = rand() % 32;
                        int color = rand() % 7 + 1; 
                        drawPixel(rx, ry, color & 1, color & 2, color & 4);
                    }
                } 
                else if (game_beaten_timer > 0) {
                    if ((game_beaten_timer / 4) % 2 == 0) {
                        drawFilledRect(0, 0, 64, 32, true, true, true);
                    }
                } 
                else {
                    int l1[] = {6, 0, 10, 15, 8}; 
                    int l2[] = {7, 16, 6, 17, 3, 10, 19, 18}; 
                    for (int i=0; i<5; i++) for(int r=0; r<5; r++) for(int c=0; c<3; c++) if(letters[l1[i]][r] & (1<<(2-c))) drawFilledRect(15+(i*7)+(c*2), 4+(r*2), 2, 2, true, true, false); 
                    for (int i=0; i<8; i++) for(int r=0; r<5; r++) for(int c=0; c<3; c++) if(letters[l2[i]][r] & (1<<(2-c))) drawFilledRect(5+(i*7)+(c*2), 18+(r*2), 2, 2, true, true, false); 
                    drawRect(0, 0, 64, 32, false, true, false); 
                }
            }
        } 
        else if (show_damage_screen) {
            clearEcran();
            #ifdef ARDUINO
            drawFilledRect(14, 16, 6, 2, true, true, true); 
            for(int r=0; r<5; r++) for(int c=0; c<3; c++) if(digits[1][r] & (1<<(2-c))) drawFilledRect(22+(c*2), 12+(r*2), 2, 2, true, true, true);   
            for(int r=0; r<5; r++) for(int c=0; c<3; c++) if(letters[14][r] & (1<<(2-c))) drawFilledRect(34+(c*2), 12+(r*2), 2, 2, true, false, false); 
            for(int r=0; r<5; r++) for(int c=0; c<3; c++) if(letters[9][r] & (1<<(2-c))) drawFilledRect(42+(c*2), 12+(r*2), 2, 2, true, false, false);  
            #endif
            damage_timer--; if (damage_timer <= 0) { show_damage_screen = false; camera_x = 0; camera_y = 0; mh = 4; grounded = false; load_level(current_level); }
        }
        // -------------------------------------------------------------------------------------------------
        // MOTEUR PHYSIQUE ET TRAITEMENT DES COLLISIONS
        // -------------------------------------------------------------------------------------------------
        else {
            mario.vx = 0; float spd = is_invincible ? 2.2f : 1.5f; 
            if(p_left) { mario.vx = -spd; facing_right = false; } if(p_right) { mario.vx = spd; facing_right = true; }
            float jump_power = is_invincible ? -4.2f : -3.5f; if(p_up && grounded) { mario.vy = jump_power; grounded = false; } 
            mario.vy += 0.35f; // Application du vecteur gravité

            mario.x += mario.vx; if(mario.x > WORLD_W - 4) mario.x = WORLD_W - 4; if(mario.x < 1) mario.x = 1; 

            // Étape 1 de collision : Déplacement horizontal et butée contre les solides
            for(int i=0; i<100; i++) {
                if(blocks[i].alive && checkCollision(mario.x, mario.y, 4, mh, blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h)) {
                    if(blocks[i].dest_x == 1 && blocks[i].w == 12) TAKE_DAMAGE(); 
                    if(mario.vx > 0) mario.x = blocks[i].x - 4; else if(mario.vx < 0) mario.x = blocks[i].x + blocks[i].w;
                    mario.vx = 0; 
                }
            }
            for(int i=0; i<15; i++) {
                if(pipes[i].alive && checkCollision(mario.x, mario.y, 4, mh, pipes[i].x, pipes[i].y, 8, pipes[i].h)) {
                    if(mario.vx > 0) mario.x = pipes[i].x - 4; else if(mario.vx < 0) mario.x = pipes[i].x + 8;
                    mario.vx = 0;
                }
            }

            mario.y += mario.vy; float ground_y = get_ground_y(mario.x + 2, current_level); float ceil_y = get_ceiling_y(mario.x + 2, current_level);
            if(mario.y >= ground_y - mh) { mario.y = ground_y - mh; mario.vy = 0; grounded = true; }
            
            // Traitement de la chute hors-limite (perte de vie)
            if(mario.y > camera_y + 36 && ground_y > 50.0f) { 
                current_hp--; if (current_hp > 0) { show_damage_screen = true; damage_timer = 60; } else { is_game_over = true; game_over_timer = 45; }
                mario.y = 0; mario.vy = 0;
            }
            if(ceil_y < 5.0f && mario.y < ceil_y) { mario.y = ceil_y; if(mario.vy < 0) mario.vy = 0; }

            // Étape 2 de collision : Déplacement vertical, atterrissage et cassage de blocs
            for(int i=0; i<100; i++) {
                if(blocks[i].alive && checkCollision(mario.x, mario.y, 4, mh, blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h)) {
                    if(blocks[i].dest_x == 1 && blocks[i].w == 12) TAKE_DAMAGE(); 

                    if(mario.vy > 0) { mario.y = blocks[i].y - mh; mario.vy = 0; grounded = true; }
                    else if(mario.vy < 0) { 
                        mario.y = blocks[i].y + blocks[i].h; mario.vy = 0; 
                        if(blocks[i].dest_x == 1 && blocks[i].w != 12) { 
                            if(!mush.alive) { mush.alive = true; mush.x = blocks[i].x + (blocks[i].w / 2) - 3; mush.y = blocks[i].y - 6; mush.vx = 1.0f; mush.dest_x = 1; }
                            blocks[i].dest_x = 2; 
                        }
                        else if(blocks[i].dest_x == 5 && blocks[i].w != 12) { 
                            request_star = true; current_score += 1000;
                            blocks[i].dest_x = 2; 
                        }
                        else if(blocks[i].dest_x == 6 && blocks[i].w != 12) { 
                            if(!mush.alive) { mush.alive = true; mush.x = blocks[i].x + (blocks[i].w / 2) - 3; mush.y = blocks[i].y - 6; mush.vx = 1.0f; mush.dest_x = 6; }
                            blocks[i].dest_x = 2; 
                        }
                    }
                }
            }

            // Gestion de l'interfaçage et de la téléportation par les tuyaux
            for(int i=0; i<15; i++) {
                if(pipes[i].alive && checkCollision(mario.x, mario.y, 4, mh, pipes[i].x, pipes[i].y, 8, pipes[i].h)) {
                    if(mario.vy > 0) { mario.y = pipes[i].y - mh; mario.vy = 0; grounded = true; if (p_down && pipes[i].dest_x > 0) { mario.x = pipes[i].dest_x; mario.y = 0; camera_x = mario.x - 16; current_score += 1000; } }
                    else if(mario.vy < 0) { mario.y = pipes[i].y + pipes[i].h; mario.vy = 0; }
                }
            }

            if (current_level == 5 && boss.alive && mario.x > 360) {
                if (mario.x < 360) mario.x = 360; 
            }
            
            // Algorithme de recentrage de la caméra de défilement (Scrolling)
            if (mario.x > camera_x + 16) camera_x = mario.x - 16; 
            else if (mario.x < camera_x + 8) camera_x = mario.x - 8;
            
            if(camera_x < 0) camera_x = 0; 
            if(camera_x > WORLD_W - 64) camera_x = WORLD_W - 64;

            float mario_head_screen_y = mario.y - camera_y; float mario_feet_screen_y = (mario.y + mh) - camera_y; 
            if (mario_head_screen_y < 4.0f) camera_y = mario.y - 4.0f; else if (mario_feet_screen_y > 26.0f && ground_y < 80.0f) camera_y = (mario.y + mh) - 26.0f;
            if (ground_y < 80.0f) { float max_cam_y = ground_y - 28.0f; if (camera_y > max_cam_y) camera_y = max_cam_y; }

            // Vérification de franchissement du drapeau de fin de niveau
            if(flag.alive && mario.x >= flag.x && mario.x <= flag.x + 10) win_level = true;
            for(int i=0; i<60; i++) if(coins[i].alive && checkCollision(mario.x, mario.y, 4, mh, coins[i].x, coins[i].y, 4, 4)) { coins[i].alive = false; current_score += 100; current_coins++; }
            
            // Logique physique autonome du bonus de type Champignon (IA simple)
            if(mush.alive){
                mush.vy+=0.2; mush.x+=mush.vx; 
                for(int i=0; i<100; i++) if(blocks[i].alive && checkCollision(mush.x, mush.y, mush.w, mush.h, blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h)) { mush.x -= mush.vx; mush.vx = -mush.vx; }
                for(int i=0; i<15; i++) if(pipes[i].alive && checkCollision(mush.x, mush.y, mush.w, mush.h, pipes[i].x, pipes[i].y, 8, pipes[i].h)) { mush.x -= mush.vx; mush.vx = -mush.vx; }
                mush.y+=mush.vy; 
                for(int i=0; i<100; i++) if(blocks[i].alive && checkCollision(mush.x, mush.y, mush.w, mush.h, blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h)) if(mush.vy > 0) { mush.y = blocks[i].y - mush.h; mush.vy = 0; }
                float mg_y = get_ground_y(mush.x + 3, current_level); if(mush.y > mg_y-mush.h){ mush.y=mg_y-mush.h; mush.vy=0; } if(mush.y > camera_y + 36) mush.alive = false; 
                float next_y = get_ground_y(mush.x + (mush.vx > 0 ? 6 : -1), current_level); if(next_y < mush.y - 2) mush.vx = -mush.vx;
                
                if(checkCollision(mario.x, mario.y, 4, mh, mush.x, mush.y, mush.w, mush.h)) { 
                    mush.alive=false; 
                    if (mush.dest_x == 6) { current_hp++; current_score+=1000; } 
                    else { if(mh==4){ mh=8; mario.y-=4; current_score+=500; } }
                }
            }

            // Macro générique pour le traitement physique des interactions avec les ennemis
            #define HANDLE_ENEMY(e, score) do { \
                if((e).alive && checkCollision(mario.x, mario.y, 4, mh, (e).x, (e).y, (e).w, (e).h)) { \
                    if (is_invincible) { (e).alive = false; current_score += score; } \
                    else if (mario.vy > 0 && mario.y < (e).y) { (e).alive = false; mario.vy = -2.5; current_score += score; } \
                    else { if(mh==8){ mh=4; mario.y+=4; (e).alive=false; star_active_counter=60;} else { TAKE_DAMAGE(); } } \
                } \
            } while(0)

            // Traitement et routine comportementale des ennemis de type Tortue/Goomba
            for(int i=0; i<20; i++) { 
                if(turtles[i].alive) { 
                    turtles[i].vy += 0.35f; turtles[i].x += turtles[i].vx; turtles[i].y += turtles[i].vy; 
                    float et_y = get_ground_y(turtles[i].x + 2, current_level); if(turtles[i].y >= et_y - turtles[i].h) { turtles[i].y = et_y - turtles[i].h; turtles[i].vy = 0; } if(turtles[i].y > camera_y + 36) { turtles[i].alive = false; continue; }
                    
                    for(int p=0; p<15; p++) if(pipes[p].alive && checkCollision(turtles[i].x, turtles[i].y, 4, 4, pipes[p].x, pipes[p].y, 8, pipes[p].h)) { turtles[i].x -= turtles[i].vx; turtles[i].vx = -turtles[i].vx; }

                    if (turtles[i].dest_x == 0 || turtles[i].dest_x == 10) { float next_y = get_ground_y(turtles[i].x + (turtles[i].vx > 0 ? 5 : -1), current_level); if(next_y < turtles[i].y - 4 || next_y > turtles[i].y + 8) turtles[i].vx = -turtles[i].vx; } 
                    else if (turtles[i].dest_x == 12) { float next_y = get_ground_y(turtles[i].x + (turtles[i].vx > 0 ? 4 : 0), current_level); if(next_y < turtles[i].y - 4) turtles[i].vx = -turtles[i].vx; for(int j=0; j<20; j++) if (i!=j && turtles[j].alive && checkCollision(turtles[i].x, turtles[i].y, 4, 4, turtles[j].x, turtles[j].y, 4, 4)) { turtles[j].alive = false; current_score += 100; } }
                    
                    if(checkCollision(mario.x, mario.y, 4, mh, turtles[i].x, turtles[i].y, 4, 4)) {
                        if (is_invincible) { turtles[i].alive = false; current_score += 200; } 
                        else if (mario.vy > 0 && mario.y < turtles[i].y + 2) { mario.vy = -3.5f; if (turtles[i].dest_x == 0) { turtles[i].alive = false; current_score += 200; } else if (turtles[i].dest_x == 10 || turtles[i].dest_x == 12) { turtles[i].dest_x = 11; turtles[i].vx = 0; current_score += 100; } else if (turtles[i].dest_x == 11) { turtles[i].dest_x = 12; turtles[i].vx = (mario.x < turtles[i].x) ? 3.5f : -3.5f; } } 
                        else { if (turtles[i].dest_x == 11) { turtles[i].dest_x = 12; turtles[i].vx = (mario.x < turtles[i].x) ? 3.5f : -3.5f; mario.x += (mario.x < turtles[i].x) ? -4 : 4; } else { if(mh==8){ mh=4; mario.y+=4; turtles[i].alive=false; star_active_counter=60;} else { TAKE_DAMAGE(); } } }
                    }
                } 
            }

            // Boucles de mise à jour des ennemis secondaires (sauteurs, volants, projectiles)
            for(int i=0; i<15; i++) if(bouncers[i].alive) { bouncers[i].vy += 0.2; bouncers[i].y += bouncers[i].vy; float bg_y = get_ground_y(bouncers[i].x + 2, current_level); if(bouncers[i].y >= bg_y-4) { bouncers[i].y = bg_y-4; bouncers[i].vy = -3.5; } HANDLE_ENEMY(bouncers[i], 300); } 
            for(int i=0; i<15; i++) if(flyers[i].alive) { flyers[i].x += flyers[i].vx; flyers[i].y = flyers[i].base_y + sin(flyers[i].x * 0.1) * 6.0; if(flyers[i].x < 20 || flyers[i].x > WORLD_W-20) flyers[i].vx = -flyers[i].vx; HANDLE_ENEMY(flyers[i], 400); } 
            
            for(int i=0; i<15; i++) if(fireballs[i].alive) { 
                fireballs[i].vy += 0.15; 
                fireballs[i].y += fireballs[i].vy; 
                fireballs[i].x += fireballs[i].vx; 
                
                if (fireballs[i].vx == 0.0f) {
                    if(fireballs[i].y > 40) { fireballs[i].y = 40; fireballs[i].vy = -4.5; } 
                } else {
                    float fg_y = get_ground_y(fireballs[i].x + 2, current_level);
                    if(fireballs[i].y >= fg_y - 4 || fireballs[i].x < camera_x - 20 || fireballs[i].x > camera_x + 84) { 
                        fireballs[i].alive = false; 
                    }
                }

                if(fireballs[i].alive && checkCollision(mario.x, mario.y, 4, mh, fireballs[i].x, fireballs[i].y, 4, 4)) { 
                    if (is_invincible) { fireballs[i].alive = false; current_score+=100; } 
                    else { if(mh==8){ mh=4; mario.y+=4; fireballs[i].alive=false; star_active_counter=60;} else { TAKE_DAMAGE(); } } 
                } 
            }
            
            for(int i=0; i<15; i++) if(plants[i].alive) { plants[i].y += plants[i].vy; if(plants[i].y < pipes[i].y - 6) plants[i].vy = 0.4; if(plants[i].y > pipes[i].y + 4) plants[i].vy = -0.4; HANDLE_ENEMY(plants[i], 100); } 
            
            // -------------------------------------------------------------------------------------------------
            // MACHINE À ÉTATS COMPORTEMENTALE DU BOSS (BOWSER)
            // -------------------------------------------------------------------------------------------------
            if (current_level == 5 && boss.alive) {
                if (boss_stun_timer > 0) {
                    boss_stun_timer--;
                    boss.vy = 0; 
                    if (boss.y < 20) boss.y += 2.0f; 
                    if (boss.y >= 20) boss.y = 20; 

                    // Déclenchement du tir de riposte lors du réveil du boss
                    if (boss_stun_timer == 0 && boss.hp > 0) {
                        boss.vy = -3.0f; 
                        
                        for(int fb=0; fb<15; fb++) {
                            if(!fireballs[fb].alive) {
                                float dir = (mario.x < boss.x) ? -1.5f : 1.5f;
                                fireballs[fb] = {boss.x + (dir < 0 ? -2.0f : 10.0f), boss.y + 4.0f, dir, -2.0f, 0.0f, 4, 4, true, 0};
                                break;
                            }
                        }
                    }
                } 
                else {
                    boss.y += boss.vy; 
                    
                    float speed_mult = 1.0f + (5 - boss.hp) * 0.15f; 
                    if (boss.y >= 20) boss.vy = -1.0 * speed_mult; 
                    if (boss.y < 5) boss.vy = 1.0 * speed_mult; 

                    // Modulo de cadence pour les tirs cycliques du boss (toutes les 60 frames)
                    if (global_frame % 60 == 0) {
                        for(int fb=0; fb<15; fb++) {
                            if(!fireballs[fb].alive) {
                                float dir = (mario.x < boss.x) ? -1.2f : 1.2f; 
                                fireballs[fb] = {boss.x + (dir < 0 ? -2.0f : 10.0f), boss.y + 4.0f, dir, -1.0f, 0.0f, 4, 4, true, 0};
                                break;
                            }
                        }
                    }

                    // Calcul de hitbox offensive sur la tête du boss ou dégâts latéraux reçus
                    if (checkCollision(mario.x, mario.y, 4, mh, boss.x, boss.y, 12, 12)) {
                        if (is_invincible) { 
                            boss.hp = 0; 
                            boss_stun_timer = 0;
                        } 
                        else if (mario.vy > 0 && mario.y < boss.y + 4) { 
                            boss.hp--; 
                            mario.vy = -4.0; 
                            boss_stun_timer = 90; 
                            
                            // Nettoyage de la scène (despawn des sbires) pour isoler le combat
                            for(int e=0; e<20; e++) turtles[e].alive = false;
                            for(int e=0; e<15; e++) { 
                                flyers[e].alive = false; 
                                bouncers[e].alive = false; 
                                plants[e].alive = false; 
                            }
                        } 
                        else { 
                            if(mh==8){ mh=4; mario.y+=4; mario.x-=10; star_active_counter=60;} 
                            else { TAKE_DAMAGE(); mario.x-=10; } 
                        }
                        
                        if (boss.hp <= 0) { 
                            boss.alive = false; 
                            current_score += 5000; 
                            game_beaten = true; 
                            boss_stun_timer = 0;
                            game_beaten_timer = 150; 
                        }
                    }
                }
            }

            // -------------------------------------------------------------------------------------------------
            // SEGMENT DE RENDU GRAPHIQUE DES DÉCORS ET DES BUFFER MATRICIELS
            // -------------------------------------------------------------------------------------------------
            clearEcran();
            
            bool cur_r = true, cur_g = true, cur_b = true; 
            if (current_level == 1) { 
                if (mario.x >= 3100.0f && mario.x < 3610.0f) { cur_r=false; cur_g=true; cur_b=true; } else { cur_r=false; cur_g=true; cur_b=false; } 
            } else if (current_level == 2) { 
                if (mario.x < 330.0f) { cur_r=false; cur_g=true; cur_b=false; } 
                else if (mario.x >= 1750.0f) { cur_r=true; cur_g=true; cur_b=false; } 
                else { cur_r=false; cur_g=true; cur_b=true; } 
            } else if (current_level == 3) { 
                cur_r=true; cur_g=true; cur_b=false; 
            } else if (current_level == 4) { 
                cur_r=true; cur_g=false; cur_b=false; 
            } else if (current_level == 5) { 
                cur_r=false; cur_g=true; cur_b=true; 
            } else { 
                cur_r = ground_r; cur_g = ground_g; cur_b = ground_b; 
            }

            // Calcul du parallaxe et rendu des décors géométriques d'arrière-plan
            for(int i=0; i<30; i++) {
                int px = (i * 90) - (int)(camera_x * 0.3f); int py = 10 - (int)(camera_y * 0.5f); 
                
                if (px > -40 && px < 80 && py > -10 && py < 32) {
                    if (current_level == 1) { 
                        if (camera_x >= 3000.0f) { drawFilledRect(px, py, 6, 16, false, false, true); drawFilledRect(px+2, py+2, 2, 12, false, true, true); } 
                        else { drawFilledRect(px, py, 10, 3, true, true, true); drawFilledRect(px+2, py-2, 6, 2, true, true, true); } 
                    } 
                    else if (current_level == 2) { 
                        if (camera_x >= 320.0f && camera_x < 1750.0f) {
                            drawFilledRect(px, py, 6, 16, false, false, true); drawFilledRect(px+2, py+2, 2, 12, false, true, true); 
                        } else if (camera_x >= 1750.0f) {
                            drawFilledRect(px, py, 10, 4, true, true, false); drawFilledRect(px+2, py-2, 6, 8, true, true, false); 
                        }
                    } 
                    else if (current_level == 3) { 
                        for(int h=0; h<35; h++) {
                            int w = (h < 20) ? h : 19; 
                            drawPixel(px - w, py + h, true, true, false);
                            drawPixel(px + w, py + h, true, true, false);
                            if (h % 4 == 0) drawFilledRect(px - w, py + h, (w * 2) + 1, 1, true, true, false); 
                        }
                    } 
                    else if (current_level == 4) { 
                        drawFilledRect(px+4, py, 12, 2, true, false, false); 
                        drawFilledRect(px+2, py+2, 16, 3, true, false, false); 
                        drawFilledRect(px, py+5, 20, 3, true, false, false); 
                    } 
                    else if (current_level == 5) {
                        drawFilledRect(px+2, py, 8, 6, true, true, true); 
                        drawFilledRect(px+1, py+2, 10, 4, true, true, true);
                        drawFilledRect(px+3, py+6, 6, 3, true, true, true); 
                        drawFilledRect(px+3, py+3, 2, 2, true, false, false);
                        drawFilledRect(px+7, py+3, 2, 2, true, false, false);
                        drawPixel(px+5, py+5, false, false, false);
                        drawPixel(px+6, py+5, false, false, false);
                        drawPixel(px+4, py+7, false, false, false);
                        drawPixel(px+6, py+7, false, false, false);
                    }
                }
            }

            // Rendu de la topologie du sol et du plafond colonne par colonne (Raycasting/Slicing simple)
            for(int x = 0; x < 64; x++) {
                float world_x = camera_x + x; float gy = get_ground_y(world_x, current_level); float cy = get_ceiling_y(world_x, current_level);
                int screen_y = (int)(gy - camera_y);
                if (screen_y < 32) { int draw_y = screen_y < 0 ? 0 : screen_y; int draw_h = 32 - draw_y; if(draw_h > 0) { drawFilledRect(x, draw_y, 1, draw_h, cur_r, cur_g, cur_b); if ((int)world_x % 8 == 0) drawPixel(x, draw_y + 1, false, false, false); } } 
                if (cy > 0.0f) { int screen_cy = (int)(cy - camera_y); if (screen_cy > 0) { int draw_h = screen_cy > 32 ? 32 : screen_cy; drawFilledRect(x, 0, 1, draw_h, cur_r, cur_g, cur_b); if ((int)world_x % 8 == 0) drawPixel(x, draw_h - 2, false, false, false); } }
            }
            
            #ifdef ARDUINO
            // Appel séquentiel des pipelines de rendu pour chaque couche d'entités actives
            for(int i=0; i<15; i++) if(plants[i].alive) { int px = (int)(plants[i].x - camera_x); int py = (int)(plants[i].y - camera_y); if(px>-6 && px<64) drawPlant(px, py, plants[i].dest_x == 1); }
            for(int i=0; i<15; i++) if(pipes[i].alive) { int px = (int)(pipes[i].x - camera_x); int py = (int)(pipes[i].y - camera_y); if(px > -10 && px < 64) drawPipe(px, py, pipes[i].h, (pipes[i].y < 10.0f)); }
            
            for(int i=0; i<100; i++) { 
                int bx = (int)(blocks[i].x - camera_x); int by = (int)(blocks[i].y - camera_y); 
                if(blocks[i].alive && (bx + blocks[i].w) > 0 && bx < 64) { 
                    if (blocks[i].dest_x == 1) { 
                        if (blocks[i].w == 12) drawThwomp(bx, by);
                        else { drawRect(bx, by, blocks[i].w, blocks[i].h, true, true, false); drawFilledRect(bx+1, by+1, blocks[i].w-2, blocks[i].h-2, true, false, false); }
                    } else if (blocks[i].dest_x == 2) { drawFilledRect(bx, by, blocks[i].w, blocks[i].h, true, true, false); } 
                    else if (blocks[i].dest_x == 3) { drawFilledRect(bx, by, blocks[i].w, blocks[i].h, false, true, true); } 
                    else if (blocks[i].dest_x == 4) { drawFilledRect(bx, by, blocks[i].w, blocks[i].h, false, true, false); } 
                    else if (blocks[i].dest_x == 5) { drawRect(bx, by, blocks[i].w, blocks[i].h, true, true, false); drawFilledRect(bx+1, by+1, blocks[i].w-2, blocks[i].h-2, true, false, true); }
                    else if (blocks[i].dest_x == 6) { drawRect(bx, by, blocks[i].w, blocks[i].h, true, true, false); drawFilledRect(bx+1, by+1, blocks[i].w-2, blocks[i].h-2, false, true, false); }
                    else { drawRect(bx, by, blocks[i].w, blocks[i].h, true, true, false); drawFilledRect(bx+1, by+1, blocks[i].w-2, blocks[i].h-2, false, true, false); }
                } 
            }
            
            for(int i=0; i<60; i++) { int cx = (int)(coins[i].x - camera_x); int cy = (int)(coins[i].y - camera_y); if(coins[i].alive && cx>-4 && cx<64) drawCoinAnim(cx, cy, global_frame); }
            if(mush.alive){ int sx = (int)(mush.x - camera_x); int sy = (int)(mush.y - camera_y); if(sx>-8 && sx<64) drawMushroom(sx, sy, mush.dest_x == 6); }
            for(int i=0; i<20; i++) if(turtles[i].alive) { int tx = (int)(turtles[i].x - camera_x); int ty = (int)(turtles[i].y - camera_y); if(tx>-4 && tx<64) { if (turtles[i].dest_x == 0) drawGoomba(tx, ty); else if (turtles[i].dest_x == 10) drawTurtle(tx, ty, global_frame); else drawShell(tx, ty); } }
            for(int i=0; i<15; i++) if(bouncers[i].alive) { int bx = (int)(bouncers[i].x - camera_x); int by = (int)(bouncers[i].y - camera_y); if(bx>-4 && bx<64) drawBouncer(bx, by); }
            for(int i=0; i<15; i++) if(flyers[i].alive) { int fx = (int)(flyers[i].x - camera_x); int fy = (int)(flyers[i].y - camera_y); if(fx>-4 && fx<64) drawFlyer(fx, fy, global_frame); } 
            for(int i=0; i<15; i++) if(fireballs[i].alive) { int fb = (int)(fireballs[i].x - camera_x); int fy = (int)(fireballs[i].y - camera_y); if(fb>-4 && fb<64) drawFireball(fb, fy, global_frame); } 
            
            if (boss.alive) { 
                int bx = (int)(boss.x - camera_x); int by = (int)(boss.y - camera_y); 
                if(bx > -15 && bx < 64) { drawBowser(bx, by, boss.hp, global_frame, (boss_stun_timer > 0)); } 
            }

            int fx = (int)(flag.x - camera_x); int fy = (int)(flag.y - camera_y); if(flag.alive && fx>-4 && fx<64) { drawFilledRect(fx, fy, 1, 18, true, true, true); drawFilledRect(fx-4, fy, 4, 4, false, true, false); }
            int mx = (int)(mario.x - camera_x); int my = (int)(mario.y - camera_y);
            if (is_invincible && ((star_active_counter % 4) < 2)) { drawFilledRect(mx, my, 4, mh, true, true, false); } 
            else { drawFilledRect(mx, my+mh/2, 4, mh/2, false, false, true); if (facing_right) { drawFilledRect(mx+1, my+1, 2, (mh/2)-1, true, true, false); drawFilledRect(mx, my, 3, 1, true, false, false); drawPixel(mx+3, my+1, true, false, false); } else { drawFilledRect(mx+1, my+1, 2, (mh/2)-1, true, true, false); drawFilledRect(mx+1, my, 3, 1, true, false, false); drawPixel(mx, my+1, true, false, false); } }

            // Rendu persistant de l'affichage tête haute (HUD) en superposition
            drawFilledRect(0, 0, 16, 7, false, false, false); drawFilledRect(52, 0, 12, 7, false, false, false); 
            drawFilledRect(1, 1, 5, 5, true, true, false); drawFilledRect(3, 2, 1, 3, false, false, false); 
            drawDigit(8, 1, (current_coins / 10) % 10, true, true, true); drawDigit(12, 1, current_coins % 10, true, true, true);
            drawHeart(53, 1); drawDigit(59, 1, current_hp % 10, true, true, true); 
            #endif
        }

        #ifdef ARDUINO
        // Transfert synchrone du frame-buffer local vers le buffer d'affichage matériel
        for(int _y=0; _y<32; _y++) {
            for(int _x=0; _x<64; _x++) {
                ecran_aff[_y][_x] = ecran[_y][_x];
            }
        }
        #endif

        // Temporisation de la tâche FreeRTOS pour stabiliser la boucle à ~33 Hz (30ms par cycle)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(30));
    }
}

#ifndef ARDUINO                                                       
#include "app_hal.h"                                        
#include <cstdio>                                           
int main(void) { printf("LVGL Simulator\n"); fflush(stdout); lv_init(); hal_setup(); testLvgl(); hal_loop(); return 0; }
#endif