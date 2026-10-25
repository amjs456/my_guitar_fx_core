#include "esp_err.h"
#include "my_guitar_fx_core.h"

#include "esp_check.h"
#include <stddef.h>
#include <stdint.h>

float print_func(float data, void *args){
    static int count;
    if(count%4800==0){
        printf("%f\n", data);
    }
    count++;
    return data;
}

void app_main(void)
{
    fx_t fx_array[] = {
        {print_func, NULL}
    };

    size_t fx_array_length = sizeof(fx_array) / sizeof(fx_array[0]);

    ESP_ERROR_CHECK(my_guitar_fx_core_init(fx_array, fx_array_length));
    ESP_ERROR_CHECK(start_tasks());
}
