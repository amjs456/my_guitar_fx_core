#include "esp_err.h"
#include "my_guitar_fx_core.h"

#include "esp_check.h"
#include <stddef.h>
#include <stdint.h>

float print_func(float data){
    static int count;
    if(count%4800==0){
        printf("%f\n", data);
    }
    count++;
    return data;
}

void app_main(void)
{
    FxFuncPtr const fx_func_array[] = {
        print_func,
    };

    size_t fx_func_array_length = sizeof(fx_func_array) / sizeof(fx_func_array[0]);

    ESP_ERROR_CHECK(my_guitar_fx_core_init(fx_func_array, fx_func_array_length));
    ESP_ERROR_CHECK(start_tasks());
}
