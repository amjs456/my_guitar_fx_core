#ifndef MY_GUITAR_FX_TYPES_H
#define MY_GUITAR_FX_TYPES_H

#include "freertos/idf_additions.h"
#include "lvgl.h"

typedef float (*FxFuncPtr)(float,void *);

typedef lv_obj_t *(*FxUiFuncPtr)(lv_obj_t *, void *);

typedef struct {
    const char *fx_name;
    FxFuncPtr fx_func_ptr;
    FxUiFuncPtr fx_ui_func_ptr;
    void *args;
} fx_t;


#endif