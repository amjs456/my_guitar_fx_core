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


#define DATA_BUF_SIZE 64

static char *TAG = "my_guitar_fx_core";


typedef struct {
    RingbufHandle_t adc_buf_handle;
    RingbufHandle_t dac_buf_handle;
} buf_handles_t;


buf_handles_t buf_handles;
i2s_chan_handle_t tx_channel = NULL;

my_dac_write_task_args_t my_dac_write_task_args;

esp_err_t my_guitar_fx_core_init(){
    esp_err_t err;

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

    uint32_t data[DATA_BUF_SIZE];
    size_t head = 0;

    while(1){
        int32_t *samples = (int32_t *)xRingbufferReceive(buf_handles.adc_buf_handle, &r_bytes, portMAX_DELAY);
        size_t n = r_bytes / sizeof(uint32_t);

        for(size_t i = 0; i + 1 < n; i+=2){
            int32_t rin = samples[i + 1] >> 8;

            if(head<DATA_BUF_SIZE){
                data[head] = rin << 8;
                head++;
            } else {
                UBaseType_t res = xRingbufferSend(buf_handles.dac_buf_handle, data, sizeof(data), portMAX_DELAY);
                if(res != pdTRUE){
                    ESP_LOGW(TAG, "Failed to send data");
                }
                head = 0;
            }
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