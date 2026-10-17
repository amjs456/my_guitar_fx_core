#include "esp_err.h"
#include "freertos/ringbuf.h"
#include <stddef.h>
#include <stdint.h>

typedef float (*FxFuncPtr)(float);

esp_err_t my_guitar_fx_core_init(const FxFuncPtr fx_func_array[], size_t fx_func_array_length);
esp_err_t start_tasks();