#ifndef _LVGL_PORTING_H_
#define _LVGL_PORTING_H_
#include "main.h"
#include <lvgl.h>

// #include "./ui/gui_guider.h"
// #include "./ui/events_init.h"

#define ENCODER_INPUT 0

#define UINT_MAX 255
#define UINT_MIN 0

void lvgl_init(void);
// bool example_lvgl_lock(int timeout_ms);
// void example_lvgl_unlock(void);
extern TaskHandle_t task_lv_handle;
#if (ENCODER_INPUT > 0)
extern uint8_t encoder_test_flag;
#endif

#endif