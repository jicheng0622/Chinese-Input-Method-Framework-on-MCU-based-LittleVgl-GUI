/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"

#include "fsl_debug_console.h"
#include "littlevgl_support.h"
#include "pin_mux.h"
#include "board.h"
#include "lvgl.h"
#include "lv_examples/src/lv_demo_chineseinput/lv_demo_chineseinput.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
static volatile bool s_lvgl_initialized = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void AppTask(void *param)
{
    lv_port_pre_init();
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    s_lvgl_initialized = true;

    lv_demo_chineseinput();
	
	  PRINTF("littlevgl task running!\r\n");

    for (;;)
    {
        lv_task_handler();
        vTaskDelay(5);
    }
}

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    BaseType_t stat;

    /* Init board hardware. */
    /* Set the eLCDIF read_qos priority high, to make sure eLCDIF
     * can fetch data in time when PXP is used.
     */
    *((volatile uint32_t *)0x41044100) = 5;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitI2C1Pins();
    BOARD_InitSemcPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
        
    gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(GPIO1, 9, &led_config);
    

    stat = xTaskCreate(AppTask, "littlevgl", configMINIMAL_STACK_SIZE + 1000, NULL, tskIDLE_PRIORITY + 2, NULL);

    if (pdPASS != stat)
    {
        PRINTF("Failed to create littlevgl task");
        while (1)
            ;
    }	
		

    vTaskStartScheduler();

    for (;;)
    {
    } /* should never get here */
}

/*!
 * @brief Malloc failed hook.
 */
void vApplicationMallocFailedHook(void)
{
    for (;;)
        ;
}

/*!
 * @brief FreeRTOS tick hook.
 */
void vApplicationTickHook(void)
{
    if (s_lvgl_initialized)
    {
        lv_tick_inc(1);
    }
}

/*!
 * @brief Stack overflow hook.
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
    (void)pcTaskName;
    (void)xTask;
	
    PRINTF("Application Stack Overflow!\r\n");
	
    for (;;)
        ;
}
