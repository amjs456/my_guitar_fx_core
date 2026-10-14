#include "esp_err.h"
#include "freertos/ringbuf.h"
#include <stddef.h>
#include <stdint.h>

typedef uint32_t (*FxFuncPtr)(uint32_t);

esp_err_t my_guitar_fx_core_init(const FxFuncPtr fx_func_array[], size_t fx_func_array_length);
esp_err_t start_tasks();