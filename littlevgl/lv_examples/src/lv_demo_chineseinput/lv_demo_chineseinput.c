/**
 * @file lv_demo_widgets.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_examples.h"
#include "lv_demo_chineseinput.h"
#include "lv_chs_keyboard.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ta_event_cb(lv_obj_t * ta, lv_event_t e);
static void kb_event_cb(lv_obj_t * ta, lv_event_t e);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * tv;
static lv_obj_t * t1;
static lv_obj_t * kb;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_demo_chineseinput(void)
{
    tv = lv_tabview_create(lv_scr_act(), NULL);

    t1 = lv_tabview_add_tab(tv, "Input Method Demo");
	
	  LV_IMG_DECLARE(nxp_logo);
	  lv_obj_t * img1 = lv_img_create(tv, NULL);
    lv_img_set_src(img1, &nxp_logo);
    lv_obj_align(img1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lv_page_set_scrl_layout(t1, LV_LAYOUT_PRETTY_TOP);

    lv_obj_t * h = lv_cont_create(t1, NULL);
    lv_cont_set_fit2(h, LV_FIT_PARENT, LV_FIT_TIGHT);
    lv_obj_t * ta = lv_textarea_create(h, NULL);
    lv_cont_set_fit2(ta, LV_FIT_PARENT, LV_FIT_PARENT); 
    lv_textarea_set_text(ta, "");
    lv_textarea_set_placeholder_text(ta, "Text");
    lv_textarea_set_cursor_hidden(ta, true);
    lv_obj_set_event_cb(ta, ta_event_cb);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void ta_event_cb(lv_obj_t * ta, lv_event_t e)
{
    if(e == LV_EVENT_FOCUSED) {
        if(kb == NULL) {
            lv_obj_set_height(tv, LV_VER_RES / 2);
            kb = lv_chs_keyboard_create(lv_scr_act(), NULL);
					  lv_chskb_set_mode(kb, LV_CHSKB_MODE_QWERTY);
            lv_obj_set_event_cb(kb, kb_event_cb);
        }
        lv_textarea_set_cursor_hidden(ta, false);
        lv_chskb_set_textarea(kb, ta);
    } else if(e == LV_EVENT_DEFOCUSED) {
        lv_textarea_set_cursor_hidden(ta, true); 
    }
}

static void kb_event_cb(lv_obj_t * _kb, lv_event_t e)
{
	  lv_chskb_ext_t * ext = lv_obj_get_ext_attr(kb);
    lv_btnm_kb_def_action(ext->btnm_kb, e);

    if(e == LV_EVENT_CANCEL) {
        if(kb) {
            lv_obj_set_height(tv, LV_VER_RES);
            lv_obj_del(kb);
            kb = NULL;
        }
    }
}
