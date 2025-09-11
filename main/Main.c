/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "max7219.h"
#include "driver/spi_master.h"
#include "driver/adc.h"
#include "driver/gpio.h"

#define JOYSTICK_X ADC1_CHANNEL_4
#define JOYSTICK_Y ADC1_CHANNEL_5
#define JOYSTICK_BUTTON GPIO_NUM_25

#define CLK_PIN    18
#define CS_PIN     5
#define MOSI_PIN   23

#define HOST SPI2_HOST
#define CASCADE_SIZE  1  // Anzahl der MAX7219-Module

struct ST_JoystickState
{
    enum {CenterLR, Left, Right} eLeftRight;
    enum {CenterUD, Up, Down} eUpDown;
    enum {NotPressed, Pressed} eButton;
};

max7219_t dev = {
    .cascade_size = CASCADE_SIZE,
    .digits = 0,
    .mirrored = false
};

struct ST_JoystickState stJoystickState = {CenterLR, CenterUD, NotPressed};

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

void setJoystickState(int iX_Value, int iY_Value, int iButtonState)
{
    if (iX_Value < 1600) {
        stJoystickState.eLeftRight = Left;
    }
    else if (iX_Value > 2000) {
        stJoystickState.eLeftRight = Right;
    }
    else {
        stJoystickState.eLeftRight = CenterLR;
    }

    if (iY_Value < 1600) {
        stJoystickState.eUpDown = Up;
    }    
    else if (iY_Value > 2000) {
        stJoystickState.eUpDown = Down;
    }
    else {
        stJoystickState.eUpDown = CenterUD;
    }

    if (iButtonState == 0) {
        stJoystickState.eButton = Pressed;
    }
    else {
        stJoystickState.eButton = NotPressed;
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
        //printf("Drawing image 1. \n");
        max7219_draw_image_8x8(&dev, 0, image1);

        int iX_Value = adc1_get_raw(JOYSTICK_X);
        int iY_Value = adc1_get_raw(JOYSTICK_Y);
        int iButtonState = gpio_get_level(JOYSTICK_BUTTON); // 0 = pressed

        setJoystickState(iX_Value, iY_Value, iButtonState);

        // Print Joystick State
        const char* leftRightStr[] = {"CenterLR", "Left", "Right"};
        const char* upDownStr[] = {"CenterUD", "Up", "Down"};
        const char* buttonStr[] = {"NotPressed", "Pressed"};
        printf("eLeftRight: %s, eUpDown: %s, eButton: %s\n",
            leftRightStr[stJoystickState.eLeftRight],
            upDownStr[stJoystickState.eUpDown],
            buttonStr[stJoystickState.eButton]);
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main(void)
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
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(JOYSTICK_X, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(JOYSTICK_Y, ADC_ATTEN_DB_11);
    gpio_set_direction(JOYSTICK_BUTTON, GPIO_MODE_INPUT);
    
    init_joystick_button();

    xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL, APP_CPU_NUM);
}
