/*
 *  Paper.hpp: class structure definition for Paper
 *  
 *  Author: Mark Hartigan
 *  Date Created: 2022 Jan 24
 */

#pragma once

#include <iostream>
#include <iomanip>
#include <functional>
#include <string>
#include <sstream>
#include <Arduino.h>
#include <SD.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "spi_flash_mmap.h"
#include "EPD.h"

class Paper {
    public:
        // functions
        Paper(uint8_t len=4);
        void post_boot_init();
        void current_page();
        void prev_page();
        void next_page();

        // variables
        bool WAIT=true;

    private:
        //functions
        bool update_head(std::string);
        bool display_page(std::string);
        void write(const char*);
        bool write_image(std::string);

        // variables
        UBYTE * page;       // image cache
        UWORD imageSize;
        uint16_t pgNum;
        uint8_t  fLen;
        uint16_t maxPg;
};