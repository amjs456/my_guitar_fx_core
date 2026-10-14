#include "esp_err.h"
#include "my_guitar_fx_core.h"

#include "esp_check.h"

void app_main(void)
{
    ESP_ERROR_CHECK(my_guitar_fx_core_init());
    ESP_ERROR_CHECK(start_tasks());
}
