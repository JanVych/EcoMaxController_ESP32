#include <stdio.h>

#include "driver/uart.h"

#include "helper.h"
#include "eco_max.h"

static const char *TAG = "ECO_MAX";

esp_err_t eco_max_Init(uart_port_t uart_num, int16_t tx, int32_t rx)
{
    uart_config_t uart_config = 
    {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    ESP_RETURN_ERROR(uart_param_config(uart_num, &uart_config));
    ESP_RETURN_ERROR(uart_set_pin(uart_num, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_RETURN_ERROR(uart_driver_install(uart_num, 1024, 1024, 0, NULL, 0));
    return ESP_OK;
}

esp_err_t eco_max_Deinit(uart_port_t uart_num)
{
    ESP_RETURN_ERROR(uart_driver_delete(uart_num));
    return ESP_OK;
}
uint8_t calculate_crc(const uint8_t *data, int length) {
    uint8_t crc = 0;
    for (int i = 0; i < length; i++) {
        crc ^= data[i];  // Simple XOR checksum
    }
    return crc;
}

void process_frame(const uint8_t *frame, int size) {
    if (size < 8) {
        ESP_LOGE(TAG, "Frame too short!");
        return;
    }

    int data_size = frame[1] | (frame[2] << 8);  // Size from frame

    ESP_LOGI(TAG, "Frame received:");
    ESP_LOGI(TAG, "Recipient Address: 0x%02X", frame[3]);
    ESP_LOGI(TAG, "Sender Address: 0x%02X", frame[4]);
    ESP_LOGI(TAG, "Sender Type: 0x%02X", frame[5]);
    ESP_LOGI(TAG, "Version: 0x%02X", frame[6]);
    ESP_LOGI(TAG, "Type: 0x%02X", frame[7]);

    if (data_size > 0) {
        ESP_LOGI(TAG, "Data: ");
        for (int i = 0; i < data_size; i++) {
            printf("0x%02X ", frame[8 + i]);
        }
        printf("\n");
    }

    uint8_t expected_crc = frame[8 + data_size];
    uint8_t actual_crc = calculate_crc(frame, 8 + data_size);
    if (expected_crc == actual_crc) {
        ESP_LOGI(TAG, "CRC valid: 0x%02X", expected_crc);
    } else {
        ESP_LOGE(TAG, "CRC mismatch! Expected 0x%02X, got 0x%02X", expected_crc, actual_crc);
    }
}

static esp_err_t ReadFrame() 
{
    uint8_t buffer[1024];
    int frame_start = -1;

    int len = uart_read_bytes(UART_PORT, buffer, 1024, 100 / portTICK_PERIOD_MS);
        if (len > 0)
        {
            for (int i = 0; i < len; i++)
            {
                if (buffer[i] == 0x68) 
                {
                    frame_start = i;
                }

                if (frame_start >= 0) {
                    // Ensure enough data for header, size, and CRC + delimiter
                    if (len - frame_start < 8) continue;

                    int frame_size = buffer[frame_start + 1] | (buffer[frame_start + 2] << 8);

                    int expected_total_length = 8 + frame_size + 2;  // header + CRC + delimiter
                    if (len - frame_start >= expected_total_length) {
                        if (buffer[frame_start + expected_total_length - 1] == 0x16) {
                            process_frame(&buffer[frame_start], expected_total_length);
                            frame_start = -1;  // Reset frame detection
                        }
                    }
                }
            }
        }
}