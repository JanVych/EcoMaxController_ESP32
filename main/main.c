#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "ecomax.h"
#include "esp_log.h"

void app_main(void)
{
    ecomax_data_t data = {0};
    esp_err_t result;
    ecomax_Init(UART_NUM_0, 17, 16);
    while(true)
    {
        result = ecomax_GetData(&data);
        if (result != ESP_OK)
        {
            ESP_LOGW("MAIN", "Failed to get data: %s", esp_err_to_name(result));
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI("MAIN", "Mix Temperature: %.2f", data.mixTemperature);
        ESP_LOGI("MAIN", "Flue Temperature: %.2f", data.flueTemperature);
        ESP_LOGI("MAIN", "TUV Temperature: %.2f", data.tuvTemperature);
        ESP_LOGI("MAIN", "Boiler Temperature: %.2f", data.boilerTemperature);
        ESP_LOGI("MAIN", "Acu Upper Temperature: %.2f", data.acuUpperTemperature);
        ESP_LOGI("MAIN", "Acu Bottom Temperature: %.2f", data.acuBottomTemperature);
        ESP_LOGI("MAIN", "Outside Temperature: %.2f", data.outsideTemperature);
        ESP_LOGI("MAIN", "Oxygen Level: %.2f", data.oxygenLevel);

        // ecomax_PrintFrame();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ESP_LOGI("MAIN", "End");
    }
}
