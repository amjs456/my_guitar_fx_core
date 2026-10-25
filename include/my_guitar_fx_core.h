#include "esp_err.h"
#include "freertos/ringbuf.h"
#include <stddef.h>
#include <stdint.h>

typedef float (*FxFuncPtr)(float,void *);
typedef struct {
    FxFuncPtr fx_func_ptr;
    void *args;
} fx_t;

esp_err_t my_guitar_fx_core_init(const fx_t fx_array[], size_t fx_func_array_length);
esp_err_t start_tasks();