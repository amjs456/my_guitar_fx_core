#include "my_guitar_fx_core.h"

#include "driver/i2s_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/ringbuf.h"

#include "my_adc_component.h"
#include "my_dac_component.h"
#include "portmacro.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>


#define MAX_FX_COUNT 8
#define DATA_BUF_SIZE 64

static char *TAG = "my_guitar_fx_core";


typedef struct {
    RingbufHandle_t adc_buf_handle;
    RingbufHandle_t dac_buf_handle;
} buf_handles_t;


buf_handles_t buf_handles;
i2s_chan_handle_t tx_channel = NULL;

my_dac_write_task_args_t my_dac_write_task_args;

static fx_t g_fx_array[MAX_FX_COUNT] = {0};
static size_t g_fx_array_length = 0;

esp_err_t my_guitar_fx_core_init(const fx_t fx_array[], size_t fx_array_length){
    esp_err_t err;

    if(fx_array_length > MAX_FX_COUNT){
        ESP_LOGE(TAG, "Too many FX functions");
        return ESP_ERR_INVALID_ARG;
    }

    if(fx_array_length > 0 && fx_array == NULL){
        ESP_LOGE(TAG, "No FX functions");
        return ESP_ERR_INVALID_ARG;
    }

    for(size_t i = 0; i < fx_array_length; i++){
        if(fx_array[i].fx_func_ptr == NULL){
            ESP_LOGE(TAG, "Fx function is NULL");
            return ESP_ERR_INVALID_ARG;
        }
        g_fx_array[i] = fx_array[i];
    }

    g_fx_array_length = fx_array_length;

    err = my_adc_init();
    if(err != ESP_OK){
        ESP_LOGE(TAG, "my_adc_init failed");
        return err;
    }

    err = my_dac_init(&tx_channel);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "my_dac_init failed");
        return err;
    }

    RingbufHandle_t adc_buf_handle = xRingbufferCreate(1024, RINGBUF_TYPE_NOSPLIT);
    RingbufHandle_t dac_buf_handle = xRingbufferCreate(1024, RINGBUF_TYPE_NOSPLIT);

    buf_handles.adc_buf_handle = adc_buf_handle;
    buf_handles.dac_buf_handle = dac_buf_handle;

    my_dac_write_task_args.buf_handle = dac_buf_handle;
    my_dac_write_task_args.tx_channel = tx_channel;


    return ESP_OK;
}

static void mediation_task(void *args){
    size_t r_bytes;

    int32_t data[DATA_BUF_SIZE];
    size_t head = 0;

    while(1){
        int32_t *samples = (int32_t *)xRingbufferReceive(buf_handles.adc_buf_handle, &r_bytes, portMAX_DELAY);
        size_t n = r_bytes / sizeof(int32_t);

        for(size_t i = 0; i + 1 < n; i+=2){
            float lin = (float)(samples[i] >> 8) / 8388608.0f;

            for(size_t j = 0; j < g_fx_array_length; j++){
                lin = g_fx_array[j].fx_func_ptr(lin, g_fx_array[j].args);
            }
            

            if(head>=DATA_BUF_SIZE){
                UBaseType_t res = xRingbufferSend(buf_handles.dac_buf_handle, data, sizeof(data), portMAX_DELAY);
                if(res != pdTRUE){
                    ESP_LOGW(TAG, "Failed to send data");
                }
                head = 0;
            }
            data[head] = ((int32_t)(lin * 8388608.0f)) << 8;
            head++;
        }
        vRingbufferReturnItem(buf_handles.adc_buf_handle, samples);
    }
}

esp_err_t start_tasks(){
    xTaskCreate(my_adc_read_task, "my_adc_read_task", 4096, buf_handles.adc_buf_handle, 5, NULL);
    xTaskCreate(my_dac_write_task, "my_dac_write_task", 4096, &my_dac_write_task_args, 5, NULL);
    xTaskCreate(mediation_task, "mediation_task", 4096, NULL, 5, NULL);
    return ESP_OK;
}