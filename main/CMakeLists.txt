idf_component_register(SRCS "Main.cpp" "Snake.cpp"
                       PRIV_REQUIRES spi_flash esp_adc max7219 joystick
                       INCLUDE_DIRS ".")
