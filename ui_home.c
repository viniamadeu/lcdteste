#include "lvgl.h"
#include "ui_home.h"
#include <stdio.h>
#include <time.h>

/* Safe area informada */
#define UI_SAFE_X   2
#define UI_SAFE_Y   22
#define UI_SAFE_W   315
#define UI_SAFE_H   185

/* Cores */
#define CLR_BG          lv_color_hex(0x000000)
#define CLR_PANEL       lv_color_hex(0x0A0A0A)
#define CLR_TEXT_MAIN   lv_color_hex(0xFFFFFF)
#define CLR_TEXT_SUB    lv_color_hex(0x9A9A9A)
#define CLR_ACCENT      lv_color_hex(0x3A3A3A)
#define CLR_OK          lv_color_hex(0x7CFFB2)

static lv_obj_t *ui_root;
static lv_obj_t *ui_time;
static lv_obj_t *ui_date;
static lv_obj_t *ui_status;
static lv_obj_t *ui_wifi_dot_1;
static lv_obj_t *ui_wifi_dot_2;
static lv_obj_t *ui_wifi_dot_3;
static lv_obj_t *ui_batt_fill;

/* ---------- helpers ---------- */

static void style_reset_obj(lv_obj_t *obj)
{
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_outline_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
}

static lv_obj_t *create_dot(lv_obj_t *parent, lv_coord_t size, lv_color_t color)
{
    lv_obj_t *dot = lv_obj_create(parent);
    lv_obj_set_size(dot, size, size);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot, color, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    style_reset_obj(dot);
    return dot;
}

static void update_clock_labels(void)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    if(!tm_info) {
        lv_label_set_text(ui_time, "12:45");
        lv_label_set_text(ui_date, "DOM, 29 MAR 2026");
        return;
    }

    static char buf_time[8];
    static char buf_date[32];

    lv_snprintf(buf_time, sizeof(buf_time), "%02d:%02d",
                tm_info->tm_hour, tm_info->tm_min);

    static const char *wday_pt[] = {
        "DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"
    };

    static const char *month_pt[] = {
        "JAN", "FEV", "MAR", "ABR", "MAI", "JUN",
        "JUL", "AGO", "SET", "OUT", "NOV", "DEZ"
    };

    lv_snprintf(buf_date, sizeof(buf_date), "%s, %02d %s %04d",
                wday_pt[tm_info->tm_wday],
                tm_info->tm_mday,
                month_pt[tm_info->tm_mon],
                tm_info->tm_year + 1900);

    lv_label_set_text(ui_time, buf_time);
    lv_label_set_text(ui_date, buf_date);
}

static void clock_timer_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    update_clock_labels();
}

/* anima um brilho sutil nos pontos do wifi */
static void wifi_anim_cb(void *obj, int32_t v)
{
    lv_obj_set_style_bg_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

static void start_wifi_blink_anim(lv_obj_t *obj, uint32_t delay)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, wifi_anim_cb);
    lv_anim_set_values(&a, LV_OPA_20, LV_OPA_COVER);
    lv_anim_set_time(&a, 700);
    lv_anim_set_playback_time(&a, 700);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_delay(&a, delay);
    lv_anim_start(&a);
}

/* fade in da tela */
static void screen_fade_in(lv_obj_t *obj)
{
    lv_obj_set_style_opa(obj, LV_OPA_0, 0);
    lv_obj_fade_in(obj, 500, 0);
}

/* ---------- API ---------- */

void ui_home_set_status(const char *txt)
{
    if(ui_status && txt) {
        lv_label_set_text(ui_status, txt);
    }
}

void ui_home_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, CLR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* raiz dentro da safe area */
    ui_root = lv_obj_create(scr);
    lv_obj_set_pos(ui_root, UI_SAFE_X, UI_SAFE_Y);
    lv_obj_set_size(ui_root, UI_SAFE_W, UI_SAFE_H);
    lv_obj_set_style_bg_color(ui_root, CLR_BG, 0);
    lv_obj_set_style_bg_opa(ui_root, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ui_root, 0, 0);
    style_reset_obj(ui_root);
    lv_obj_clear_flag(ui_root, LV_OBJ_FLAG_SCROLLABLE);

    /* painel interno sutil */
    lv_obj_t *panel = lv_obj_create(ui_root);
    lv_obj_set_size(panel, UI_SAFE_W, UI_SAFE_H);
    lv_obj_set_pos(panel, 0, 0);
    lv_obj_set_style_bg_color(panel, CLR_PANEL, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_20, 0);
    lv_obj_set_style_radius(panel, 18, 0);
    style_reset_obj(panel);

    /* topo */
    lv_obj_t *top_bar = lv_obj_create(ui_root);
    lv_obj_set_size(top_bar, UI_SAFE_W - 16, 22);
    lv_obj_set_pos(top_bar, 8, 6);
    lv_obj_set_style_bg_opa(top_bar, LV_OPA_TRANSP, 0);
    style_reset_obj(top_bar);

    lv_obj_t *label_brand = lv_label_create(top_bar);
    lv_label_set_text(label_brand, "HOME");
    lv_obj_set_style_text_color(label_brand, CLR_TEXT_SUB, 0);
    lv_obj_set_style_text_font(label_brand, &lv_font_montserrat_12, 0);
    lv_obj_align(label_brand, LV_ALIGN_LEFT_MID, 0, 0);

    /* wifi fake */
    lv_obj_t *wifi_wrap = lv_obj_create(top_bar);
    lv_obj_set_size(wifi_wrap, 28, 12);
    lv_obj_set_style_bg_opa(wifi_wrap, LV_OPA_TRANSP, 0);
    style_reset_obj(wifi_wrap);
    lv_obj_align(wifi_wrap, LV_ALIGN_RIGHT_MID, -34, 0);

    ui_wifi_dot_1 = create_dot(wifi_wrap, 4, CLR_TEXT_SUB);
    ui_wifi_dot_2 = create_dot(wifi_wrap, 4, CLR_TEXT_SUB);
    ui_wifi_dot_3 = create_dot(wifi_wrap, 4, CLR_TEXT_SUB);

    lv_obj_align(ui_wifi_dot_1, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_align(ui_wifi_dot_2, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_align(ui_wifi_dot_3, LV_ALIGN_LEFT_MID, 16, 0);

    start_wifi_blink_anim(ui_wifi_dot_1, 0);
    start_wifi_blink_anim(ui_wifi_dot_2, 160);
    start_wifi_blink_anim(ui_wifi_dot_3, 320);

    /* bateria fake */
    lv_obj_t *batt = lv_obj_create(top_bar);
    lv_obj_set_size(batt, 22, 10);
    lv_obj_set_style_radius(batt, 3, 0);
    lv_obj_set_style_bg_opa(batt, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(batt, 1, 0);
    lv_obj_set_style_border_color(batt, CLR_TEXT_SUB, 0);
    lv_obj_align(batt, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t *batt_tip = lv_obj_create(top_bar);
    lv_obj_set_size(batt_tip, 2, 4);
    lv_obj_set_style_radius(batt_tip, 1, 0);
    lv_obj_set_style_bg_color(batt_tip, CLR_TEXT_SUB, 0);
    lv_obj_set_style_bg_opa(batt_tip, LV_OPA_COVER, 0);
    style_reset_obj(batt_tip);
    lv_obj_align_to(batt_tip, batt, LV_ALIGN_OUT_RIGHT_MID, 1, 0);

    ui_batt_fill = lv_obj_create(batt);
    lv_obj_set_size(ui_batt_fill, 14, 6);
    lv_obj_set_style_radius(ui_batt_fill, 2, 0);
    lv_obj_set_style_bg_color(ui_batt_fill, CLR_OK, 0);
    lv_obj_set_style_bg_opa(ui_batt_fill, LV_OPA_COVER, 0);
    style_reset_obj(ui_batt_fill);
    lv_obj_align(ui_batt_fill, LV_ALIGN_LEFT_MID, 2, 0);

    /* relógio */
    ui_time = lv_label_create(ui_root);
    lv_label_set_text(ui_time, "12:45");
    lv_obj_set_style_text_color(ui_time, CLR_TEXT_MAIN, 0);
    lv_obj_set_style_text_font(ui_time, &lv_font_montserrat_48, 0);
    lv_obj_align(ui_time, LV_ALIGN_CENTER, 0, -16);

    /* data */
    ui_date = lv_label_create(ui_root);
    lv_label_set_text(ui_date, "DOM, 29 MAR 2026");
    lv_obj_set_style_text_color(ui_date, CLR_TEXT_SUB, 0);
    lv_obj_set_style_text_font(ui_date, &lv_font_montserrat_16, 0);
    lv_obj_align_to(ui_date, ui_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

    /* linha elegante */
    lv_obj_t *line = lv_obj_create(ui_root);
    lv_obj_set_size(line, 110, 2);
    lv_obj_set_style_bg_color(line, CLR_ACCENT, 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_70, 0);
    lv_obj_set_style_radius(line, LV_RADIUS_CIRCLE, 0);
    style_reset_obj(line);
    lv_obj_align_to(line, ui_date, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);

    /* status inferior */
    ui_status = lv_label_create(ui_root);
    lv_label_set_text(ui_status, "ESP32-C6  |  ST7789V  |  SPI OK");
    lv_obj_set_style_text_color(ui_status, CLR_TEXT_SUB, 0);
    lv_obj_set_style_text_font(ui_status, &lv_font_montserrat_12, 0);
    lv_obj_align(ui_status, LV_ALIGN_BOTTOM_MID, 0, -10);

    update_clock_labels();
    lv_timer_create(clock_timer_cb, 1000, NULL);

    screen_fade_in(ui_root);
}
