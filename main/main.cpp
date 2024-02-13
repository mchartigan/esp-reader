/* 
 *  main.cpp: ESP32 E-reader
 *
 *  Author: Mark Hartigan
 *  Date Created: 2022 Jan 23
 *  Description:
 *      Utilizes a Waveshare EPD e-paper display (7.5in) to display EPUB files
 */

#include "Paper.hpp"

using namespace std;

Paper book(6);

// GPIO event handling
static void IRAM_ATTR gpio_isr_handler(void*);
static void gpio_taskmaster(void*);
static QueueHandle_t gpio_evt_queue = NULL;

extern "C" void app_main(void)
{
    // init book after declaration cuz the display and debugger got pissy
    book.post_boot_init();

    //zero-initialize the config structure.
    gpio_config_t io_conf = {}; 
    io_conf.mode = GPIO_MODE_INPUT;                 // set as input mode
    io_conf.intr_type = GPIO_INTR_POSEDGE;          // interrupt of rising edge
    io_conf.pin_bit_mask = ((1ULL << GPIO_NUM_32) | (1ULL << GPIO_NUM_33));  // bit mask of the pins, use GPIO21/22 here.
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;    // enable pull-down mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;       // disable pull-up mode
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task (8192 bytes for stack since 2048 wasn't enough)
    xTaskCreate(gpio_taskmaster, "gpio_taskmaster", 8192, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_NUM_32, gpio_isr_handler, (void*) GPIO_NUM_32);
    gpio_isr_handler_add(GPIO_NUM_33, gpio_isr_handler, (void*) GPIO_NUM_33);
}

/******************
 * GPIO Interrupts
 *****************/

/*
 *  Sends interrupt to event handler so as to not clog the interrupt
 */
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

/*
 *  GPIO event handler since page writing is blocking
 *  NOTE: key presses don't queue until page has been written
 */
static void gpio_taskmaster(void* arg)
{
    uint32_t io_num;
    for(;;) {
        // if there's an item in the event queue, it was on a rising edge, and we don't have to wait
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY) && gpio_get_level((gpio_num_t) io_num) && !book.WAIT) {
            switch (io_num) {
                case 32:
                    book.prev_page();
                    break;
                case 33:
                    book.next_page();
                    break;
                default: break;
            }
        }
    }
}
