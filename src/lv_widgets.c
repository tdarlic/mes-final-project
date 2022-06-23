/**
 * @file lv_widgets.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_widgets.h"
#include "../lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
}disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void pressure_create(lv_obj_t * parent);
static void analytics_create(lv_obj_t * parent);
static void setup_create(lv_obj_t * parent);

static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2, const char * text3);

static void chart_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static lv_obj_t * meter3;

lv_meter_indicator_t *indic;

static lv_obj_t * chart1;
static lv_chart_series_t * ser1;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

// holding the modal message box
lv_obj_t * mbox1;

static uint32_t session_desktop = 1000;
static uint32_t session_tablet = 1000;
static uint32_t session_mobile = 1000;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_widgets(void)
{
    if(LV_HOR_RES <= 320) disp_size = DISP_SMALL;
    else if(LV_HOR_RES < 720) disp_size = DISP_MEDIUM;
    else disp_size = DISP_LARGE;

    font_large = LV_FONT_DEFAULT;
    font_normal = LV_FONT_DEFAULT;

    lv_coord_t tab_h;
    if(disp_size == DISP_LARGE) {
        tab_h = 70;
#if LV_FONT_MONTSERRAT_24
        font_large     =  &lv_font_montserrat_24;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_24 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif
#if LV_FONT_MONTSERRAT_16
        font_normal    =  &lv_font_montserrat_16;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_16 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif
    } else if(disp_size == DISP_MEDIUM) {
        tab_h = 45;
#if LV_FONT_MONTSERRAT_20
        font_large     =  &lv_font_montserrat_20;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_20 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif
#if LV_FONT_MONTSERRAT_14
        font_normal    =  &lv_font_montserrat_14;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_14 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif
    } else { /* disp_size == DISP_SMALL */
        tab_h = 45;
#if LV_FONT_MONTSERRAT_18
        font_large     =  &lv_font_montserrat_18;
#else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif
#if LV_FONT_MONTSERRAT_12
        font_normal    =  &lv_font_montserrat_12;
#else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_12 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.")
#endif
    }

#if LV_USE_THEME_DEFAULT
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, font_normal);
#endif

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_style_init(&style_icon);
    lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
    lv_style_set_text_font(&style_icon, font_large);

    lv_style_init(&style_bullet);
    lv_style_set_border_width(&style_bullet, 0);
    lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);

    lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);

    lv_obj_t * t1 = lv_tabview_add_tab(tv, "Pressure");
    lv_obj_t * t2 = lv_tabview_add_tab(tv, "History");
    lv_obj_t * t3 = lv_tabview_add_tab(tv, "Setup");

    pressure_create(t1);
    analytics_create(t2);
    setup_create(t3);
}

/**
 * Rotates the screen to any rotation
 */
void lv_rotate_screen(lv_disp_rot_t rot){
	lv_disp_set_rotation(lv_disp_get_default(), rot);
}

void set_barometer_value(float bvalue){
	lv_meter_set_indicator_value(meter3, indic, round(bvalue));
}

static void event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_current_target(e);
    lv_msgbox_close(mbox1);
}

void lv_ex_msgbox_close(void){
	lv_msgbox_close(mbox1);
}

void lv_ex_msgbox(void)
{
    static const char * btns[] = {"Ok", NULL};

    mbox1 = lv_msgbox_create(NULL, "Storm!!!", "Storm Warning", btns, false);
    lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox1);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void pressure_create(lv_obj_t * parent)
{
    // Meter 3

	lv_meter_scale_t * scale;

	lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

	meter3 = create_meter_box(parent, "Pressure", "Stormy", "Rain", "Dry");
	lv_obj_add_flag(lv_obj_get_parent(meter3), LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

	/*Add a special circle to the needle's pivot*/
	lv_obj_set_style_pad_hor(meter3, 10, 0);
	lv_obj_set_style_size(meter3, 10, LV_PART_INDICATOR);
	lv_obj_set_style_radius(meter3, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
	lv_obj_set_style_bg_opa(meter3, LV_OPA_COVER, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(meter3, lv_palette_darken(LV_PALETTE_GREY, 4), LV_PART_INDICATOR);
	lv_obj_set_style_outline_color(meter3, lv_color_white(), LV_PART_INDICATOR);
	lv_obj_set_style_outline_width(meter3, 3, LV_PART_INDICATOR);
	lv_obj_set_style_text_color(meter3, lv_palette_darken(LV_PALETTE_GREY, 1), LV_PART_TICKS);

	scale = lv_meter_add_scale(meter3);
	lv_meter_set_scale_range(meter3, scale, 930, 1070, 250, 360 - 250);
	lv_meter_set_scale_ticks(meter3, scale, 29, 3, 17, lv_color_white());
	lv_meter_set_scale_major_ticks(meter3, scale, 4, 4, 22, lv_color_white(), 15);

	indic = lv_meter_add_arc(meter3, scale, 10, lv_palette_main(LV_PALETTE_RED), 0);
	lv_meter_set_indicator_start_value(meter3, indic, 900);
	lv_meter_set_indicator_end_value(meter3, indic, 990);

	indic = lv_meter_add_scale_lines(meter3, scale, lv_palette_darken(LV_PALETTE_RED, 3), lv_palette_darken(LV_PALETTE_RED, 3), true, 0);
	lv_meter_set_indicator_start_value(meter3, indic, 900);
	lv_meter_set_indicator_end_value(meter3, indic, 990);

	indic = lv_meter_add_arc(meter3, scale, 12, lv_palette_main(LV_PALETTE_BLUE), 0);
	lv_meter_set_indicator_start_value(meter3, indic, 990);
	lv_meter_set_indicator_end_value(meter3, indic, 1010);

	indic = lv_meter_add_scale_lines(meter3, scale, lv_palette_darken(LV_PALETTE_BLUE, 3), lv_palette_darken(LV_PALETTE_BLUE, 3), true, 0);
	lv_meter_set_indicator_start_value(meter3, indic, 990);
	lv_meter_set_indicator_end_value(meter3, indic, 1010);

	indic = lv_meter_add_arc(meter3, scale, 10, lv_palette_main(LV_PALETTE_GREEN), 0);
	lv_meter_set_indicator_start_value(meter3, indic, 1010);
	lv_meter_set_indicator_end_value(meter3, indic, 1070);

	indic = lv_meter_add_scale_lines(meter3, scale, lv_palette_darken(LV_PALETTE_GREEN, 3), lv_palette_darken(LV_PALETTE_GREEN, 3), true, 0);
	lv_meter_set_indicator_start_value(meter3, indic, 1010);
	lv_meter_set_indicator_end_value(meter3, indic, 1070);

	indic = lv_meter_add_needle_line(meter3, scale, 4, lv_palette_darken(LV_PALETTE_GREY, 4), -25);

	lv_obj_t * hpa_label = lv_label_create(meter3);
	lv_label_set_text(hpa_label, "-");
	lv_obj_add_style(hpa_label, &style_title, 0);

	lv_obj_t * hpa_unit_label = lv_label_create(meter3);
	lv_label_set_text(hpa_unit_label, "hPa");

	// Set indicator to minimum pressure for start
	lv_meter_set_indicator_value(meter3, indic, 930);

	lv_obj_update_layout(parent);

	lv_coord_t meter_w = lv_obj_get_width(meter3);
	lv_obj_set_height(meter3, meter_w);

	lv_obj_align(hpa_label, LV_ALIGN_TOP_MID, 10, lv_pct(55));
	lv_obj_align_to(hpa_unit_label, hpa_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 10, 0);

}


static void analytics_create(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

    static lv_coord_t grid_chart_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), 10, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_chart_col_dsc[] = {20, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    lv_obj_t * chart1_cont = lv_obj_create(parent);
    lv_obj_set_flex_grow(chart1_cont, 1);
    lv_obj_set_grid_dsc_array(chart1_cont, grid_chart_col_dsc, grid_chart_row_dsc);

    lv_obj_set_height(chart1_cont, LV_PCT(100));
    lv_obj_set_style_max_height(chart1_cont, 300, 0);

    lv_obj_t * title = lv_label_create(chart1_cont);
    lv_label_set_text(title, "Pressure history");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 0, 1);

    chart1 = lv_chart_create(chart1_cont);
    lv_group_add_obj(lv_group_get_default(), chart1);
    lv_obj_add_flag(chart1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_grid_cell(chart1, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_chart_set_axis_tick(chart1, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 5, 1, true, 80);
    lv_chart_set_axis_tick(chart1, LV_CHART_AXIS_PRIMARY_X, 0, 0, 12, 1, true, 50);
    lv_chart_set_div_line_count(chart1, 0, 12);
    lv_chart_set_point_count(chart1, 12);
    lv_obj_add_event_cb(chart1, chart_event_cb, LV_EVENT_ALL, NULL);
    if(disp_size == DISP_SMALL) lv_chart_set_zoom_x(chart1, 256 * 3);
    else if(disp_size == DISP_MEDIUM) lv_chart_set_zoom_x(chart1, 256 * 2);

    lv_obj_set_style_border_side(chart1, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(chart1, 0, 0);

    ser1 = lv_chart_add_series(chart1, lv_theme_get_color_primary(chart1), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));
    lv_chart_set_next_value(chart1, ser1, lv_rand(10, 40));

}


/****************************************************************************************************
 *
 * Create third panel - Shop
 *
 ****************************************************************************************************/
void setup_create(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t * title;
    lv_obj_t * notifications = lv_obj_create(parent);

    if(disp_size == DISP_SMALL) {
        lv_obj_add_flag(notifications, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_set_height(notifications, LV_PCT(100));
    } else  {
        lv_obj_set_height(notifications, LV_PCT(100));
        lv_obj_set_style_max_height(notifications, 300, 0);
    }

    lv_obj_set_flex_flow(notifications, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(notifications, 1);

    title = lv_label_create(notifications);
    lv_label_set_text(title, "Notification");
    lv_obj_add_style(title, &style_title, 0);

    lv_obj_t * cb;
    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "Storm alarm");

    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "Sleep enable");

    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "Dim screen");

    cb = lv_checkbox_create(notifications);
    lv_checkbox_set_text(cb, "Baro comm alarm");

}

static lv_obj_t * create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2, const char * text3)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(cont, 1);

    lv_obj_t * title_label = lv_label_create(cont);
    lv_label_set_text(title_label, title);
    lv_obj_add_style(title_label, &style_title, 0);

    lv_obj_t * meter = lv_meter_create(cont);
    lv_obj_remove_style(meter, NULL, LV_PART_MAIN);
    lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);
    lv_obj_set_width(meter, LV_PCT(100));

    lv_obj_t * bullet1 = lv_obj_create(cont);
    lv_obj_set_size(bullet1, 13, 13);
    lv_obj_remove_style(bullet1, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet1, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet1, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_t * label1 = lv_label_create(cont);
    lv_label_set_text(label1, text1);

    lv_obj_t * bullet2 = lv_obj_create(cont);
    lv_obj_set_size(bullet2, 13, 13);
    lv_obj_remove_style(bullet2, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet2, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet2, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_t * label2 = lv_label_create(cont);
    lv_label_set_text(label2, text2);

    lv_obj_t * bullet3 = lv_obj_create(cont);
    lv_obj_set_size(bullet3, 13, 13);
    lv_obj_remove_style(bullet3,  NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet3, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet3, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_t * label3 = lv_label_create(cont);
    lv_label_set_text(label3, text3);


	static lv_coord_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static lv_coord_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
	lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
	lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 0, 1);
	lv_obj_set_grid_cell(meter, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 1, 1);
	lv_obj_set_grid_cell(bullet1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(bullet2, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 3, 1);
	lv_obj_set_grid_cell(bullet3, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 4, 1);
	lv_obj_set_grid_cell(label1, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 2, 1);
	lv_obj_set_grid_cell(label2, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 3, 1);
	lv_obj_set_grid_cell(label3, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 4, 1);

    return meter;

}

static void chart_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) {
        lv_obj_invalidate(obj); /*To make the value boxes visible*/
    }
    else if(code == LV_EVENT_DRAW_PART_BEGIN) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        /*Set the markers' text*/
        if(dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X) {
            if(lv_chart_get_type(obj) == LV_CHART_TYPE_BAR) {
                const char * month[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII"};
                lv_snprintf(dsc->text, sizeof(dsc->text), "%s", month[dsc->value]);
            } else {
                const char * month[] = {"Jan", "Febr", "March", "Apr", "May", "Jun", "July", "Aug", "Sept", "Oct", "Nov", "Dec"};
                lv_snprintf(dsc->text, sizeof(dsc->text), "%s", month[dsc->value]);
            }
        }

    }
}


