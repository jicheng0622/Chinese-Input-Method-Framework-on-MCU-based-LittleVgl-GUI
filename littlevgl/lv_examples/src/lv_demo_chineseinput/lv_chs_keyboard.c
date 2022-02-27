
/**
 * @file lv_chs_keyboard.c
 *
 */
#include <stdio.h>
#include <assert.h>
/*********************
 *      INCLUDES
 *********************/
#include "lv_chs_keyboard.h"
#include "qwerty_py.h"

#include "lv_misc/lv_txt.h"
#include "lv_widgets/lv_textarea.h"
#include "lv_widgets/lv_roller.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t lv_chskb_signal(lv_obj_t * kb, lv_signal_t sign, void * param);
static void lv_chskb_set_chsmap(lv_obj_t *chskb);
static void lv_chskb_roller_py_action(lv_obj_t *roller, lv_event_t e);
static void lv_chskb_chs_action(lv_obj_t *chs, lv_event_t e);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_signal_cb_t ancestor_signal;

static const char * kb_map_lc[] ={"1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", LV_SYMBOL_BACKSPACE, "\n",
                                  "ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", LV_SYMBOL_NEW_LINE, "\n",
                                  "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
                                  "En/中文", LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""
                                 };

static const char * kb_map_uc[] = {
    "1#", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", LV_SYMBOL_BACKSPACE, "\n",
    "abc", "A", "S", "D", "F", "G", "H", "J", "K", "L", LV_SYMBOL_NEW_LINE, "\n",
    "_", "-", "Z", "X", "C", "V", "B", "N", "M", ".", ",", ":", "\n",
    "En/中文", LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""
};

static const char * kb_map_spec[] = {
    "1#", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", LV_SYMBOL_BACKSPACE, "\n",
    "+", "-", "/", "*", "=", "%", "!", "?", "#", "<", ">", "\n",
    "\\", "@", "$", "(", ")", "{", "}", "[", "]", ";", "\"", "'", "\n",
    LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""
};

static const char * kb_map_num[] = {
    "1", "2", "3", LV_SYMBOL_CLOSE, "\n",
    "4", "5", "6", LV_SYMBOL_OK, "\n",
    "7", "8", "9", LV_SYMBOL_BACKSPACE, "\n",
    "+/-", "0", ".", LV_SYMBOL_LEFT, LV_SYMBOL_RIGHT, LV_SYMBOL_KEYBOARD, ""
};

static const char * kb_map_t9[] = {
    "abc", "def", LV_SYMBOL_BACKSPACE, "\n",
    "ghi", "jkl", "mno", LV_SYMBOL_OK, "\n",
    "pqrs", "tuv", "wxyz", LV_SYMBOL_CLOSE, "\n",
     LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, ""
};

static const char * kb_map_qwerty[] = {"1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", LV_SYMBOL_BACKSPACE, "\n",
                                       "a", "s", "d", "f", "g", "h", "j", "k", "l", LV_SYMBOL_NEW_LINE, "\n",
                                       "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
                                       "中文/En", LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""
                                       };

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a chinese keyboard objects, including 2*btnmatrix objects and 1*roller Object
 * @param par pointer to an object, it will be the parent of the new keyboard
 * @param copy pointer to a keyboard object
 * @return pointer to the created keyboard
 */
lv_obj_t * lv_chs_keyboard_create(lv_obj_t * par, const lv_obj_t * copy)
{
    /* Create the ancestor of keyboard */
    lv_obj_t * new_chskb = lv_obj_create(par, copy);
    LV_ASSERT_MEM(new_chskb);
    if(new_chskb == NULL) return NULL;

    if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_chskb);

    /* Allocate the keyboard type specific extended data */
    lv_chskb_ext_t * ext = lv_obj_allocate_ext_attr(new_chskb, sizeof(lv_chskb_ext_t));
    LV_ASSERT_MEM(ext);
    if(ext == NULL)
    {
        lv_obj_del(new_chskb);
        return NULL;
    }
		/* Clear the ext memory to make sure the all content is 0 by default */
    _lv_memset(ext, 0, sizeof(lv_chskb_ext_t));
		
		ext->btnm_kb     = NULL;
    ext->btnm_chs    = NULL;
    ext->roller_py   = NULL;
    ext->ta          = NULL;
    ext->mode        = LV_CHSKB_MODE_LC;
    ext->cursor_mng  = 0;
    ext->t9inbuf[0]  = '\0';

    /*The signal and design functions are not copied so set them here*/
    lv_obj_set_signal_cb(new_chskb, lv_chskb_signal);

    /* Init the chinese keyboard structure */
		/*|roller_py|btnm_chs|*/
		/*|******btnm_kb*****|*/
    if(copy == NULL) 
		{
        ext->btnm_kb   = lv_btnmatrix_create(new_chskb, NULL);//Character Matrix Keyboard, Qwerty
			  ext->btnm_chs  = lv_btnmatrix_create(new_chskb, NULL);//Chinese Matrix Key Array, 1*16
        ext->roller_py = lv_roller_create(new_chskb, NULL);   //Roller as display region of Pinyin																					
        																				
        if(ext->btnm_kb == NULL || ext->roller_py == NULL || ext->btnm_chs == NULL)
        {
            lv_obj_del(new_chskb);
            return NULL;
        }
				/*Set Chinese Keyboard as Bottom Middle Alignment and its size as half of the screen */
        lv_obj_set_size(new_chskb, lv_obj_get_width_fit(lv_obj_get_parent(new_chskb)), lv_obj_get_height_fit(lv_obj_get_parent(new_chskb)) / 2);
        lv_obj_align(new_chskb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
				
				/* Reserve 4/5 height of the new_chskb area for btnm_kb object */
				lv_obj_set_size(ext->btnm_kb, lv_obj_get_width(new_chskb), lv_obj_get_height(new_chskb)*4/5);
        lv_obj_align(ext->btnm_kb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_obj_set_base_dir(ext->btnm_kb, LV_BIDI_DIR_LTR);
        lv_obj_add_protect(ext->btnm_kb, LV_PROTECT_CLICK_FOCUS);
				lv_obj_set_event_cb(ext->btnm_kb, lv_btnm_kb_def_action);
        lv_btnmatrix_set_map(ext->btnm_kb, kb_map_lc);
				
				/* Reserve 1/5 height and 1/18 width of the new_chskb area for roller_py object */
        lv_obj_set_size(ext->roller_py, lv_obj_get_width(new_chskb)/18, lv_obj_get_height(new_chskb)*1/5);
        lv_roller_set_auto_fit(ext->roller_py, false);
        lv_obj_align(ext->roller_py, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
        lv_obj_set_hidden(ext->roller_py, false);
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
			  lv_obj_set_style_local_bg_color(ext->roller_py, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, LV_COLOR_GRAY);
				lv_obj_set_event_cb(ext->roller_py, lv_chskb_roller_py_action);
        
				/* Reserve 1/5 height the new_chskb area for btnm_chs object */
        lv_obj_set_size(ext->btnm_chs, lv_obj_get_width(new_chskb) - lv_obj_get_width(ext->roller_py), lv_obj_get_height(new_chskb)*1/5);
        lv_obj_align(ext->btnm_chs, ext->roller_py, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
        lv_obj_set_hidden(ext->btnm_chs, false);
        lv_obj_set_event_cb(ext->btnm_chs, lv_chskb_chs_action);
				lv_obj_set_style_local_border_side(ext->btnm_chs, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);
				
				/* Set cutomized font for btnm_kb and btnm_chs object */
				LV_FONT_DECLARE(lv_font_NotoSansCJKsc_Regular);
				lv_obj_set_style_local_text_font(ext->btnm_kb, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &lv_font_NotoSansCJKsc_Regular);
				lv_obj_set_style_local_text_font(ext->btnm_chs, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, &lv_font_NotoSansCJKsc_Regular);	
    }

    return new_chskb;
}

/**
 * Assign a Text Area to the Keyboard. The pressed characters will be put there.
 * @param kb pointer to a Keyboard object
 * @param ta pointer to a Text Area object to write there
 */
void lv_chskb_set_textarea(lv_obj_t *chskb, lv_obj_t * ta)
{
    lv_chskb_ext_t * ext = lv_obj_get_ext_attr(chskb);

    /*Hide the cursor of the old Text area if cursor management is enabled*/
    if(ext->ta && ext->cursor_mng) {
        lv_textarea_set_cursor_hidden(ext->ta, true);
    }

    ext->ta = ta;

    /*Show the cursor of the new Text area if cursor management is enabled*/
    if(ext->ta && ext->cursor_mng) {
        lv_textarea_set_cursor_hidden(ext->ta, false);
    }
		
		/* Assign a new Chinese Font for the TextArea Object */
		if(LV_CHSKB_MODE_QWERTY == ext->mode)
		{
		  LV_FONT_DECLARE(lv_font_NotoSansCJKsc_Regular);
		  lv_obj_set_style_local_text_font(ta, LV_TEXTAREA_PART_BG, LV_STATE_DEFAULT, &lv_font_NotoSansCJKsc_Regular);
		}
}

/**
 * Set a new a mode (text or number map)
 * @param kb pointer to a Keyboard object
 * @param mode the mode from 'lv_chskb_mode_t'
 */
void lv_chskb_set_mode(lv_obj_t *chskb, lv_chskb_mode_t mode)
{
    lv_chskb_ext_t * ext = lv_obj_get_ext_attr(chskb);
    const char *txt;

    if(ext->mode == mode) 
		{
				return;
		}
		
    ext->mode = mode;
		
		/* English Lower Case Keyboard */
    if(mode == LV_CHSKB_MODE_LC) 
		{
        lv_obj_align(ext->btnm_kb, chskb, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_btnmatrix_set_map(ext->btnm_kb, kb_map_lc);
        lv_obj_set_hidden(ext->roller_py, true);
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
        lv_obj_set_hidden(ext->btnm_chs, true);
    }
		/* English Upper Case Keyboard */
		else if(mode == LV_CHSKB_MODE_UC) 
		{
        lv_obj_align(ext->btnm_kb, chskb, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_btnmatrix_set_map(ext->btnm_kb, kb_map_uc);
        lv_obj_set_hidden(ext->roller_py, true);
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
        lv_obj_set_hidden(ext->btnm_chs, true);
    }
		/* Numeric Keyboard */
		else if(mode == LV_CHSKB_MODE_NUM) 
		{
        lv_obj_align(ext->btnm_kb, chskb, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_btnmatrix_set_map(ext->btnm_kb, kb_map_num);
        lv_obj_set_hidden(ext->roller_py, true);
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
        lv_obj_set_hidden(ext->btnm_chs, true);
    }
		/* Chinese T9 Keyboard */
		else if(mode == LV_CHSKB_MODE_T9) 
		{
        lv_obj_align(ext->btnm_kb, chskb, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
        lv_btnmatrix_set_map(ext->btnm_kb, kb_map_t9);
        lv_obj_set_hidden(ext->roller_py, false);
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
        lv_roller_set_selected(ext->roller_py, 0, false);
        lv_obj_set_hidden(ext->btnm_chs, false);

        memset(&ext->chs_data, 0, sizeof(ext->chs_data));
        lv_chskb_set_chsmap(chskb);
		}
		/* Specific Symbol Keyboard */
		else if(mode == LV_CHSKB_MODE_SPEC) 
		{
        lv_obj_align(ext->btnm_kb, chskb, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_btnmatrix_set_map(ext->btnm_kb, kb_map_spec);
        lv_obj_set_hidden(ext->roller_py, true);
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
        lv_obj_set_hidden(ext->btnm_chs, true);
		}
		/* Chinese Qwerty Keyboard */
		else if(mode == LV_CHSKB_MODE_QWERTY) 
		{
				lv_btnmatrix_set_map(ext->btnm_kb, kb_map_qwerty);
				lv_obj_set_hidden(ext->roller_py, false);
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
        lv_roller_set_selected(ext->roller_py, 0, false);
				lv_obj_set_hidden(ext->btnm_chs, false);

        memset(&ext->chs_data, 0, sizeof(ext->chs_data));
				lv_chskb_set_chsmap(chskb);
		}
    else 
		{
				return;
		}
}

static void lv_chskb_textarea_add_characters(lv_obj_t *ta, const char *txt)
{
    /* Add the characters to the text area if set */
    if(ta == NULL)
        return;

    if(strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
		{
				lv_textarea_add_char(ta, '\n');
		}
    else if(strcmp(txt, LV_SYMBOL_LEFT) == 0) 
		{
				lv_textarea_cursor_left(ta);
		}
    else if(strcmp(txt, LV_SYMBOL_RIGHT) == 0) 
		{
				lv_textarea_cursor_right(ta);
		}
    else if(strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)  
		{
				lv_textarea_del_char(ta);
		}
    else if(strcmp(txt, "+/-") == 0) 
		{
        uint16_t cur = lv_textarea_get_cursor_pos(ta);
        const char * ta_txt = lv_textarea_get_text(ta);
        if(ta_txt[0] == '-') 
				{
            lv_textarea_set_cursor_pos(ta, 1);
            lv_textarea_del_char(ta);
            lv_textarea_add_char(ta, '+');
            lv_textarea_set_cursor_pos(ta, cur);
        } 
				else if(ta_txt[0] == '+') 
				{
            lv_textarea_set_cursor_pos(ta, 1);
            lv_textarea_del_char(ta);
            lv_textarea_add_char(ta, '-');
            lv_textarea_set_cursor_pos(ta, cur);
        } 
				else 
				{
            lv_textarea_set_cursor_pos(ta, 0);
            lv_textarea_add_char(ta, '-');
            lv_textarea_set_cursor_pos(ta, cur + 1);
        }
    } 
		else 
		{
        lv_textarea_add_text(ta, txt);
    }
}

/**
 * Signal function of the keyboard
 * @param kb pointer to a keyboard object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_chskb_signal(lv_obj_t * kb, lv_signal_t sign, void * param)
{
    lv_res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(kb, sign, param);
    if(res != LV_RES_OK) return res;

    if(sign == LV_SIGNAL_CLEANUP) 
		{
        /*Nothing to cleanup. (No dynamically allocated memory in 'ext')*/
    } 
		else if(sign == LV_SIGNAL_GET_TYPE) 
		{
        lv_obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) 
				{  
            if(buf->type[i] == NULL) 																																/*Find the last set data*/
						{
								break;
						}
        }
        buf->type[i] = "lv_chskb";
    }

    return res;
}

/**
 * @brief roller object event callback
 * @param obj
 * @param event
 * @return
 */
static void lv_chskb_roller_py_action(lv_obj_t *roller, lv_event_t e)
{   
	  uint16_t id;
    lv_obj_t *chskb;
    lv_chskb_ext_t *ext;
	
    if(e != LV_EVENT_VALUE_CHANGED) return;
	  
    id = lv_roller_get_selected(roller);
    chskb = lv_obj_get_parent(roller);
    ext = lv_obj_get_ext_attr(chskb);

    ext->chs_data.pyindex = id;
    ext->chs_data.chsindex = 0;
    ext->chs_data.chscnt = _lv_txt_get_encoded_length(ext->chs_data.chslist[id]);

    lv_chskb_set_chsmap(chskb);
}

/**
 * @brief btnm_chs object event callback
 * @param obj
 * @param event
 * @return
 */
static void lv_chskb_chs_action(lv_obj_t *chs, lv_event_t e)
{
	  if(e != LV_EVENT_VALUE_CHANGED) return;
	
	  lv_obj_t *chskb = lv_obj_get_parent(chs);
    lv_chskb_ext_t * ext = lv_obj_get_ext_attr(chskb);
	
    const char * txt = lv_btnmatrix_get_active_btn_text(chs);
    if(txt == NULL) return;
	
	  /* 每次上下键翻页时保留上一次末尾的几个字 */
	  if(strcmp(txt, LV_SYMBOL_DOWN) == 0)
    {
        if(ext->chs_data.chsindex < ext->chs_data.chscnt - 14)
        {
            ext->chs_data.chsindex += 14;
            lv_chskb_set_chsmap(chskb);
        }
    }
    else if(strcmp(txt, LV_SYMBOL_UP) == 0)
    {
        if(ext->chs_data.chsindex >= 14)
        {
            ext->chs_data.chsindex -= 14;
            lv_chskb_set_chsmap(chskb);
        }
    }
    else
    {
        lv_chskb_textarea_add_characters(ext->ta, txt);
        /* 汉字选择完成以后，需要清空拼音和汉字选择区 */
        ext->t9inbuf[0] = '\0';
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
        lv_roller_set_selected(ext->roller_py, 0, false);

        memset(&ext->chs_data, 0, sizeof(ext->chs_data));
        lv_chskb_set_chsmap(chskb);
    }
}

/**
 * @brief 刷新汉字选择区
 * @param chskb
 */
static void lv_chskb_set_chsmap(lv_obj_t *chskb)
{
    lv_chskb_ext_t *ext = lv_obj_get_ext_attr(chskb);
    size_t btni, total_btns;
    uint32_t id, id_next;
    const char *p_chs;
    size_t chsindex;
	
    if(ext->chs_data.chslist[ext->chs_data.pyindex])
    {
        p_chs = ext->chs_data.chslist[ext->chs_data.pyindex];
        id = _lv_txt_encoded_get_byte_id(p_chs, ext->chs_data.chsindex);
    }
    else
    {
        p_chs = NULL;
        id = 0;
    }

    chsindex = ext->chs_data.chsindex;
		
    total_btns = sizeof(ext->chs_data.mapbuf) / sizeof(ext->chs_data.mapbuf[0]);
		
    for(btni = 0; btni < total_btns; btni ++)
    {
        if((btni == total_btns - 3) && (ext->chs_data.chscnt > 16))
        {
            strcpy(ext->chs_data.mapbuf[btni], LV_SYMBOL_UP);
        }
        else if((btni == total_btns - 2) && (ext->chs_data.chscnt > 16))
        {
            strcpy(ext->chs_data.mapbuf[btni], LV_SYMBOL_DOWN);
        }
        else if(btni == total_btns - 1)
        {
            /* 最后一个结束btn */
            ext->chs_data.mapbuf[btni][0] = '\0';
        }
//				/* Only for T9 Keyboard */
//        else if((btni % 5) == 4)
//        {
//            // 换行符
//            ext->chs_data.mapbuf[btni][0] = '\n';
//            ext->chs_data.mapbuf[btni][1] = '\0';
//        }
        else if(chsindex < ext->chs_data.chscnt)
        {
            id_next = id;
            _lv_txt_encoded_next(p_chs, &id_next);
            memcpy(&ext->chs_data.mapbuf[btni][0], &p_chs[id], id_next - id);
            ext->chs_data.mapbuf[btni][id_next - id + 1] = '\0';
            id = id_next;
            chsindex ++;
        }
        else
        {
					  /* 填充一个非法字符用来清空button */
            ext->chs_data.mapbuf[btni][0] = '\211';
            ext->chs_data.mapbuf[btni][1] = '\0';
        }
				
        ext->chs_data.map[btni] = ext->chs_data.mapbuf[btni];
    }
		
    lv_btnmatrix_set_map(ext->btnm_chs, ext->chs_data.map);
}

/**
 * @brief t9拼音输入模式下的输入处理函数
 * @param chskb
 * @param txt
 */
static void lv_chskb_btnmatrix_t9im_action(lv_obj_t *chskb, const char *txt)
{
	
}

/**
 * @brief QWERTY拼音输入模式下的输入处理函数
 * @param chskb
 * @param txt
 */

static void lv_chskb_btnmatrix_qwertyim_action( lv_obj_t *chskb, char *txt )
{
	  lv_chskb_ext_t * ext = lv_obj_get_ext_attr( chskb );
    size_t len = strlen( ext->t9inbuf );

    if( strcmp(txt, LV_SYMBOL_BACKSPACE) == 0 )
    {
        if( len > 0 )
						ext->t9inbuf[--len] = '\0';
        else
						lv_textarea_del_char(ext->ta);
    }
		else if(strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
		{
				lv_textarea_add_char(ext->ta, '\n');
		}
    else if(strcmp(txt, LV_SYMBOL_LEFT) == 0) 
		{
				lv_textarea_cursor_left(ext->ta);
		}
    else if(strcmp(txt, LV_SYMBOL_RIGHT) == 0) 
		{
				lv_textarea_cursor_right(ext->ta);
		}
    else if(strcmp(txt, " ") == 0) 
		{
				lv_textarea_add_text(ext->ta, txt);
		}
		else if(strcmp(txt, ",") == 0) 
		{
				lv_textarea_add_text(ext->ta, txt);
		}
		else if(strcmp(txt, ".") == 0) 
		{
				lv_textarea_add_text(ext->ta, txt);
		}
		else if(strcmp(txt, ";") == 0) 
		{
				lv_textarea_add_text(ext->ta, txt);
		}
		else if(strcmp(txt, "-") == 0) 
		{
				lv_textarea_add_text(ext->ta, txt);
		}
		else if(strcmp(txt, "_") == 0) 
		{
				lv_textarea_add_text(ext->ta, txt);
		}
		else
    {
        if( len >= ( sizeof( ext->t9inbuf ) - 1 ) )
        {
            return;
        }
        ext->t9inbuf[len++] = *txt;
        ext->t9inbuf[len] = '\0';
    }
		
		if( len == 0 )
    {
        ext->chs_data.pyindex = 0;
        ext->chs_data.chsindex = 0;
        ext->chs_data.chscnt = 0;
        lv_roller_set_options(ext->roller_py, "", LV_ROLLER_MODE_NORMAL);
    }
		else
    {
				lv_roller_set_options(ext->roller_py, ext->t9inbuf, LV_ROLLER_MODE_NORMAL);		
				ext->chs_data.chslist[0] = py_ime( ext->t9inbuf );       
				if( ext->chs_data.chslist[0] == NULL )
				{
						return;
				}
				
				ext->chs_data.pyindex = 0;
				ext->chs_data.chscnt = _lv_txt_get_encoded_length( ext->chs_data.chslist[0] );
				ext->chs_data.chsindex = 0;
    }
		
		lv_chskb_set_chsmap( chskb );
    lv_roller_set_selected( ext->roller_py, 0, false );

}

/**
 * Called when a button on 'btnm_kb' is released
 * @param btnm pointer to 'btnm_kb'
 * @param i the index of the released button from the current btnm map
 * @return LV_ACTION_RES_INV if the btnm is deleted else LV_ACTION_RES_OK
 */
void lv_btnm_kb_def_action(lv_obj_t *btnm_kb, lv_event_t e)
{
	  if(e != LV_EVENT_PRESSED) return;
	
	  lv_obj_t *chskb = lv_obj_get_parent(btnm_kb);
    lv_chskb_ext_t * ext = lv_obj_get_ext_attr(chskb);
	
    const char * txt = lv_btnmatrix_get_active_btn_text(btnm_kb);
    if(txt == NULL) return;
	
	  /*Do the corresponding action according to the text of the button*/
    if(strcmp(txt, "中文/En") == 0) 
		{
       lv_chskb_set_mode(chskb, LV_CHSKB_MODE_LC);
    } 
		else if(strcmp(txt, "En/中文") == 0)
		{
			 lv_chskb_set_mode(chskb, LV_CHSKB_MODE_QWERTY);
		}
		else if(strcmp(txt, "ABC") == 0)
		{
			 lv_chskb_set_mode(chskb, LV_CHSKB_MODE_UC);
		}
		else if(strcmp(txt, "abc") == 0)
		{
			 lv_chskb_set_mode(chskb, LV_CHSKB_MODE_LC);
		}
		else if(strcmp(txt, "1#") == 0) 
		{
			static uint8_t last_keyboard=LV_CHSKB_MODE_SPEC;
			if(LV_CHSKB_MODE_SPEC != ext->mode)
			{
			  last_keyboard = ext->mode;
			  lv_chskb_set_mode(chskb, LV_CHSKB_MODE_SPEC);
			}
			else 
				lv_chskb_set_mode(chskb, last_keyboard);
				
    } 
		else if(strcmp(txt, LV_SYMBOL_CLOSE)==0 || strcmp(txt, LV_SYMBOL_OK)==0) 
		{
        if(chskb->event_cb != lv_keyboard_def_event_cb) {
            lv_res_t res = lv_event_send(chskb, LV_EVENT_CANCEL, NULL);
            if(res != LV_RES_OK) return;
        }
        else 
				{
					  /*De-assign the text area  to hide it cursor if needed*/
            lv_chskb_set_textarea(chskb, NULL);
            lv_obj_del(chskb);
        }
		}
	  /* T9 keyboard mapping Chinese character pinyin input method processing function. */
		else if(ext->mode == LV_CHSKB_MODE_T9) 					
		{
        lv_chskb_btnmatrix_t9im_action(chskb, txt);
    }
		/* Full keyboard input method. */  
		else if(ext->mode == LV_CHSKB_MODE_QWERTY) 
		{
        lv_chskb_btnmatrix_qwertyim_action(chskb, ( (char * )txt) );
    }
    else
        lv_chskb_textarea_add_characters(ext->ta, txt);
}
