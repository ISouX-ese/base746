#include "lvgl.h"

// --- Textes pour identifier les actions ---
static const char * ACT_UP = "HAUT";
static const char * ACT_DOWN = "BAS";
static const char * ACT_LEFT = "GAUCHE";
static const char * ACT_RIGHT = "DROITE";
static const char * ACT_POWER = "SUPER_POUVOIR";

// --- Gestionnaire d'événements de la manette ---
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    const char * action_name = (const char *)lv_event_get_user_data(e);

    if(code == LV_EVENT_PRESSED) {
        LV_LOG_USER("Bouton %s ENFONCÉ", action_name);
    }
    else if(code == LV_EVENT_RELEASED) {
        LV_LOG_USER("Bouton %s RELÂCHÉ", action_name);
    }
}

// --- Fonction utilitaire pour créer les boutons ---
lv_obj_t * create_game_btn(lv_obj_t * parent, const char * text, lv_align_t align, int32_t x_ofs, int32_t y_ofs, const char * action, uint32_t hex_color)
{
    lv_obj_t * btn = lv_button_create(parent);
    lv_obj_set_size(btn, 65, 65);
    lv_obj_align(btn, align, x_ofs, y_ofs);
    lv_obj_set_style_bg_color(btn, lv_color_hex(hex_color), 0);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, (void*)action);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0); 
    lv_obj_center(label);

    return btn;
}

// --- Création de l'interface graphique ---
void testLvgl()
{
    lv_obj_t * screen = lv_screen_active();

    // Fond Spatial Vivid
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x100020), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x050510), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

    // Étoiles
    for(int i = 0; i < 40; i++) {
        lv_obj_t * star = lv_obj_create(screen);
        lv_obj_set_size(star, 2, 2);
        if(i % 3 == 0) lv_obj_set_style_bg_color(star, lv_color_hex(0x00FFFF), 0);
        else if(i % 5 == 0) lv_obj_set_style_bg_color(star, lv_color_hex(0xFF00FF), 0);
        else lv_obj_set_style_bg_color(star, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(star, 0, 0);
        lv_obj_set_style_bg_opa(star, LV_OPA_80, 0);
        lv_obj_set_pos(star, (i * 97) % 480, (i * 61) % 272); 
    }

    // Titre
    lv_obj_t * title = lv_label_create(screen);
    lv_label_set_text(title, "MAN PAC");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFF00), 0);
#if LV_FONT_MONTSERRAT_32
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
#endif
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    uint32_t color_cyan_neon = 0x00F2FF;
    uint32_t color_red_arcade = 0xFF0000;

    create_game_btn(screen, "HAUT", LV_ALIGN_LEFT_MID, 50, -50, ACT_UP, color_cyan_neon);
    create_game_btn(screen, "BAS", LV_ALIGN_LEFT_MID, 50, 50, ACT_DOWN, color_cyan_neon);
    create_game_btn(screen, "GAUCHE", LV_ALIGN_RIGHT_MID, -115, 0, ACT_LEFT, color_cyan_neon);
    create_game_btn(screen, "DROITE", LV_ALIGN_RIGHT_MID, -35, 0, ACT_RIGHT, color_cyan_neon);

    lv_obj_t * power_btn = lv_button_create(screen);
    lv_obj_set_size(power_btn, 100, 100);
    lv_obj_align(power_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(power_btn, lv_color_hex(color_red_arcade), 0);
    lv_obj_set_style_radius(power_btn, LV_RADIUS_CIRCLE, 0); 
    lv_obj_add_event_cb(power_btn, event_handler, LV_EVENT_ALL, (void*)ACT_POWER);

    lv_obj_t * power_label = lv_label_create(power_btn);
    lv_label_set_text(power_label, "BOOM");
    lv_obj_set_style_text_color(power_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(power_label);

    lv_obj_t * score_label = lv_label_create(screen);
    lv_label_set_text(score_label, "SCORE : 00000");
    lv_obj_set_style_text_color(score_label, lv_color_hex(0x00FF00), 0);
#if LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(score_label, &lv_font_montserrat_20, 0);
#endif
    lv_obj_align(score_label, LV_ALIGN_BOTTOM_MID, 0, -15);
}

// ============================================================================
// PARTIE MATÉRIELLE (STM32 UNIQUEMENT)
// ============================================================================
#ifdef ARDUINO

#include "lvglDrivers.h"

// --- 1. MAPPING DES BROCHES (Basé sur ton schéma) ---
#define PIN_R1  D0  // ⚠️ D0 = RX
#define PIN_G1  D1  // ⚠️ D1 = TX
#define PIN_B1  D2
#define PIN_R2  D3
#define PIN_G2  D4
#define PIN_B2  D5

#define PIN_A   A0
#define PIN_B   A1
#define PIN_C   A2
#define PIN_D   A3

#define PIN_CLK D8
#define PIN_LAT D9
#define PIN_OE  D10

HardwareTimer *MatrixTimer;
volatile uint8_t current_line = 0;

// --- 2. L'INTERRUPTION QUI RAFRAÎCHIT LA MATRICE (Test 1 Pixel) ---
void Matrix_Update_ISR() {
    // 1. Éteindre l'écran pendant qu'on modifie les données
    digitalWrite(PIN_OE, HIGH);
    
    // 2. Sélectionner la ligne actuelle (0 à 15)
    digitalWrite(PIN_A, (current_line & 0x01));
    digitalWrite(PIN_B, (current_line & 0x02));
    digitalWrite(PIN_C, (current_line & 0x04));
    digitalWrite(PIN_D, (current_line & 0x08));
    
    // 3. Pousser les 64 pixels un par un
    for(int x = 0; x < 64; x++) {
        
        // TEST : On veut allumer le pixel X=32 sur la ligne Y=8 (moitié haute, donc R1)
        if(x == 32 && current_line == 8) {
            digitalWrite(PIN_R1, HIGH); // Rouge ON !
        } else {
            digitalWrite(PIN_R1, LOW);  // Noir
        }
        
        // On force tous les autres canaux à 0 (Noir)
        digitalWrite(PIN_G1, LOW);
        digitalWrite(PIN_B1, LOW);
        digitalWrite(PIN_R2, LOW);
        digitalWrite(PIN_G2, LOW);
        digitalWrite(PIN_B2, LOW);
        
        // Coup d'horloge pour faire avancer les données dans la matrice
        digitalWrite(PIN_CLK, HIGH);
        digitalWrite(PIN_CLK, LOW);
    }
    
    // 4. Verrouiller la ligne (Latch) pour l'afficher
    digitalWrite(PIN_LAT, HIGH);
    digitalWrite(PIN_LAT, LOW);
    
    // 5. Rallumer l'écran
    digitalWrite(PIN_OE, LOW);
    
    // Passer à la ligne suivante
    current_line++;
    if(current_line >= 16) current_line = 0;
}

void mySetup()
{
    // --- Initialisation des broches ---
    pinMode(PIN_R1, OUTPUT); pinMode(PIN_G1, OUTPUT); pinMode(PIN_B1, OUTPUT);
    pinMode(PIN_R2, OUTPUT); pinMode(PIN_G2, OUTPUT); pinMode(PIN_B2, OUTPUT);
    pinMode(PIN_A, OUTPUT);  pinMode(PIN_B, OUTPUT);  pinMode(PIN_C, OUTPUT); pinMode(PIN_D, OUTPUT);
    pinMode(PIN_CLK, OUTPUT); pinMode(PIN_LAT, OUTPUT); pinMode(PIN_OE, OUTPUT);

    // État de sécurité (Écran noir)
    digitalWrite(PIN_OE, HIGH); 
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_LAT, LOW);

    // --- Configuration du Timer ---
    // Utilisation du Timer 3 matériel. 
    // On règle la fréquence à 2000 Hz. C'est suffisant pour que l'oeil ne voie pas de clignotement.
    MatrixTimer = new HardwareTimer(TIM3);
    MatrixTimer->setOverflow(2000, HERTZ_FORMAT); 
    MatrixTimer->attachInterrupt(Matrix_Update_ISR);
    MatrixTimer->resume(); // 🚀 Lancement du chronomètre !

    // Initialisation de LVGL
    testLvgl();
}

void loop()
{
    // Inactif
}

void myTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
    }
}

// ============================================================================
// PARTIE SIMULATEUR PC (Ne touche pas à l'écran LED)
// ============================================================================
#else

#include "lvgl.h"
#include "app_hal.h"
#include <cstdio>

int main(void)
{
    printf("LVGL Simulator\n");
    fflush(stdout);

    lv_init();
    hal_setup();

    testLvgl();

    hal_loop();
    return 0;
}

#endif