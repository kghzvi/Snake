/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <Snake.h>
extern "C" {
    #include "sdkconfig.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "esp_flash.h"
    #include "esp_system.h"
    #include "max7219.h"
    #include "driver/spi_master.h"
    #include "esp_adc/adc_continuous.h"
    #include "driver/gpio.h"


//#define JOYSTICK_X ADC1_CHANNEL_4
//#define JOYSTICK_Y ADC1_CHANNEL_5
#define JOYSTICK_BUTTON GPIO_NUM_25

#define CLK_PIN    GPIO_NUM_18
#define CS_PIN     GPIO_NUM_5
#define MOSI_PIN   GPIO_NUM_23

#define HOST SPI2_HOST
#define CASCADE_SIZE  1  // Anzahl der MAX7219-Module
}

enum class E_LeftRight { Center, Left, Right };
enum class E_UpDown { Center, Up, Down };
enum class E_Button { NotPressed, Pressed };

struct ST_JoystickState
{
    E_LeftRight eLeftRight;
    E_UpDown eUpDown;
    E_Button eButton;
};

max7219_t dev = {
    .digits = 0,
    .cascade_size = CASCADE_SIZE,
    .mirrored = false,
    .bcd = false
};

adc_continuous_handle_t adc_handle = nullptr;
adc_channel_t channel[2] = {ADC_CHANNEL_4, ADC_CHANNEL_5};

int iX_Value;
int iY_Value;
int iButtonState;

ST_JoystickState stJoystickState = {.eLeftRight = E_LeftRight::Center, .eUpDown = E_UpDown::Center, .eButton = E_Button::NotPressed};

static void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = 256,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channel_num;
    for (int i = 0; i < channel_num; i++) {
        adc_pattern[i].atten = ADC_ATTEN_DB_11;
        adc_pattern[i].channel = channel[i] & 0x7;
        adc_pattern[i].unit = ADC_UNIT_1;
        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

        //ESP_LOGI(TAG, "adc_pattern[%d].atten is :%"PRIx8, i, adc_pattern[i].atten);
        //ESP_LOGI(TAG, "adc_pattern[%d].channel is :%"PRIx8, i, adc_pattern[i].channel);
        //ESP_LOGI(TAG, "adc_pattern[%d].unit is :%"PRIx8, i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
}

void init_joystick_button()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << JOYSTICK_BUTTON,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

void readJoystickAdc() {
    uint8_t result[128];
    uint32_t ret_num = 0;
    iX_Value = 0;
    iY_Value = 0;

    if (adc_continuous_read(adc_handle, result, sizeof(result), &ret_num, 0) == ESP_OK) {

        adc_digi_output_data_t *data = (adc_digi_output_data_t *)&result[1];
        for (int i = 0; i < ret_num; i += sizeof(adc_digi_output_data_t)) {
            adc_digi_output_data_t *data = (adc_digi_output_data_t *)&result[i];
            if (data->type1.channel == 4) {
                iX_Value = data->type1.data;
            } else if (data->type1.channel == 5) {
                iY_Value = data->type1.data;
            }
        }
    }
}

void setJoystickState(int iX_Value, int iY_Value, int iButtonState)
{
    if (iX_Value < 1600) {
        stJoystickState.eLeftRight = E_LeftRight::Left;
        printf("Turn left detected.\n");
    }
    else if (iX_Value > 2000) {
        stJoystickState.eLeftRight = E_LeftRight::Right;
        printf("Turn right detected.\n");
    }
    else {
        stJoystickState.eLeftRight = E_LeftRight::Center;
    }

    if (iY_Value < 1600) {
        stJoystickState.eUpDown = E_UpDown::Up;
        printf("Move up detected.\n");
    }    
    else if (iY_Value > 2000) {
        stJoystickState.eUpDown = E_UpDown::Down;
        printf("Move down detected.\n");
    }
    else {
        stJoystickState.eUpDown = E_UpDown::Center;
    }

    if (iButtonState == 0) {
        stJoystickState.eButton = E_Button::Pressed;
    }
    else {
        stJoystickState.eButton = E_Button::NotPressed;
    }
}

void task(void *pvParameters)
{
    uint8_t image1[8] = 
    {
        0b00111100,
        0b01111110,
        0b11111111,
        0b11111111,
        0b11111111,
        0b01111110,
        0b00111100,
        0b00000000
    };

    while(true)
    {
        max7219_draw_image_8x8(&dev, 0, image1);

        readJoystickAdc();
        iButtonState = gpio_get_level(JOYSTICK_BUTTON); // 0 = pressed

        setJoystickState(iX_Value, iY_Value, iButtonState);
       
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

extern "C" void app_main(void)
{
    // SPI-Bus konfigurieren
    spi_bus_config_t cfg = {
        .mosi_io_num = MOSI_PIN,
        .miso_io_num = -1,
        .sclk_io_num = CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HOST, &cfg, 1));

    // MAX7219 initialisieren
    ESP_ERROR_CHECK(max7219_init_desc(&dev, HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CS_PIN));
    ESP_ERROR_CHECK(max7219_init(&dev));
    max7219_clear(&dev);

    // Joystick initialisieren
    continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &adc_handle);
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));

    gpio_set_direction(JOYSTICK_BUTTON, GPIO_MODE_INPUT);
    
    init_joystick_button();

    xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL, APP_CPU_NUM);
}
