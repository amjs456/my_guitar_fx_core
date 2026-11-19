#ifndef MY_GUITAR_FX_CORE_H
#define MY_GUITAR_FX_CORE_H

#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "freertos/ringbuf.h"
#include <stddef.h>
#include <stdint.h>

#include "lvgl.h"

typedef float (*FxFuncPtr)(float,void *);
typedef lv_obj_t *(*FxUiFuncPtr)(lv_obj_t *, void *);

typedef struct {
    FxFuncPtr fx_func_ptr;
    FxUiFuncPtr fx_ui_func_ptr;
    void *args;
    TaskHandle_t *save_settings_task_handle;
} fx_t;

esp_err_t my_guitar_fx_core_init(const fx_t fx_array[], size_t fx_func_array_length);
esp_err_t start_tasks();

#endif