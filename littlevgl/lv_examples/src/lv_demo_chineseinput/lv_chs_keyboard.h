/**
 * @file lv_chskb.h
 *
 * MIT licence
 * Copyright (c) 2021, jicheng0622
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the “Software”), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial 
 * portions of the Software.
 */

#ifndef LV_CHSKB_H
#define LV_CHSKB_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"
#else
#include "../../lv_conf.h"
#endif

/*Testing of dependencies*/
#if LV_USE_BTNMATRIX == 0
#error "lv_chskb: lv_btnm is required. Enable it in lv_conf.h (USE_LV_BTNM  1) "
#endif

#if LV_USE_TEXTAREA == 0
#error "lv_chskb: lv_ta is required. Enable it in lv_conf.h (USE_LV_TA  1) "
#endif

#include "../../lv_examples.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

enum {
	  LV_CHSKB_MODE_LC,
	  LV_CHSKB_MODE_UC,
	  LV_CHSKB_MODE_SPEC,
    LV_CHSKB_MODE_NUM,
	  LV_CHSKB_MODE_T9,
		LV_CHSKB_MODE_QWERTY,
};
typedef uint8_t lv_chskb_mode_t;

typedef struct {
    lv_obj_t *btnm_kb;          // 用于输入字符的按键矩阵
	  lv_obj_t *roller_py;        // 匹配拼音显示区域
    lv_obj_t *btnm_chs;         // 用于选择汉字的按键矩阵（1*16矩阵）
    struct{
        const char *map[20];    // 总共16个btn，3个换行，一个结束
        char mapbuf[20][10];    // 每个btn最多10个字节
        size_t pyindex;         // 当前btnm_chs中显示的汉字对应的pylist和chslist的索引
        size_t chsindex;        // btnm_chs中显示的汉字在chslist[pyindex]中的起始索引
        size_t chscnt;          // chslist[pyindex]中所有的汉字的个数
        const char *pylist[10]; // 和t9inbuf匹配的拼音列表
        const char *chslist[10];// 和pylist对应的汉字列表
    }chs_data;                  // 用于设置btnm_chs的数据
    lv_obj_t *ta;
    lv_chskb_mode_t mode;
    uint8_t cursor_mng;
    char t9inbuf[10];
} lv_chskb_ext_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a keyboard objects
 * @param par pointer to an object, it will be the parent of the new keyboard
 * @param copy pointer to a keyboard object, if not NULL then the new object will be copied from it
 * @return pointer to the created keyboard
 */
lv_obj_t * lv_chs_keyboard_create(lv_obj_t * par, const lv_obj_t * copy);

/**
 * Called when a button of 'btnm_kb' is released
 * @param btnm pointer to 'btnm_kb'
 * @param i the index of the released button from the current btnm map
 * @return LV_ACTION_RES_INV if the btnm is deleted else LV_ACTION_RES_OK
 */
void lv_btnm_kb_def_action(lv_obj_t *btnm_kb, lv_event_t e);

/**
 * Assign a Text Area to the Keyboard. The pressed characters will be put there.
 * @param kb pointer to a Keyboard object
 * @param ta pointer to a Text Area object to write there
 */
void lv_chskb_set_textarea(lv_obj_t *chskb, lv_obj_t * ta);

/**
 * Set a new a mode (text or number map)
 * @param kb pointer to a Keyboard object
 * @param mode the mode from 'lv_chskb_mode_t'
 */
void lv_chskb_set_mode(lv_obj_t *chskb, lv_chskb_mode_t mode);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*LV_CHSKB_H*/
