#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "main.h"

#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "motion";

SemaphoreHandle_t xSemaphore = NULL;
//bool led_status = false;


// interrupt service routine, called when the button is pressed
void IRAM_ATTR button_isr_handler(void* arg) {
	
    // notify the button task
	xSemaphoreGiveFromISR(xSemaphore, NULL);
}

// task that will react to button clicks
void motion_handler(void * pvParameters) {

    main_data_t * main_data = (main_data_t *) pvParameters ;

	// infinite loop
	for(;;) {
		
		// wait for the notification from the ISR
		if(xSemaphoreTake(xSemaphore,portMAX_DELAY) == pdTRUE) {
		    main_data->motion_count++;
		    ESP_LOGI(TAG, "Motion detected (%d)", main_data->motion_count);
//			led_status = !led_status;
//			gpio_set_level(CONFIG_LED_PIN, led_status);
		}
	}
}


void initialize_motion(main_data_t * main_data) {

    ESP_LOGI(TAG, "Initializing motion interrupt for pin %d", CONFIG_MOTION_PIN);
    ESP_LOGI(TAG, "Main data has motion count %d", main_data->motion_count);
	// create the binary semaphore
	xSemaphore = xSemaphoreCreateBinary();

	// configure button and led pins as GPIO pins
	gpio_pad_select_gpio(CONFIG_MOTION_PIN);
//	gpio_pad_select_gpio(CONFIG_LED_PIN);

	// set the correct direction
	gpio_set_direction(CONFIG_MOTION_PIN, GPIO_MODE_INPUT);
//    gpio_set_direction(CONFIG_LED_PIN, GPIO_MODE_OUTPUT);

	// enable interrupt on rising edge for button pin
	gpio_set_intr_type(CONFIG_MOTION_PIN, GPIO_INTR_POSEDGE);


	// start the task that will handle the button
	xTaskCreate(motion_handler, "motion_handler", 2048, main_data, 5, NULL);

	// install ISR service with default configuration
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	// attach the interrupt service routine
	gpio_isr_handler_add(CONFIG_MOTION_PIN, button_isr_handler, NULL);
}

