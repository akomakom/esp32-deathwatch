
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "ds18b20.h"
#include "main.h"

#define TEMPERATURE_READ_FREQUENCY 60000

//static const char *TAG = "temperature";


void temperature_task(void * pvParameters) {

//    main_data_t * main_data = (main_data_t *) pvParameters ;
	void (*callback)(float) = (void *) pvParameters;

    while(1) {
        callback(ds18b20_get_temp());
        delay(TEMPERATURE_READ_FREQUENCY );
    }
}


void initialize_temperature(void (*callback)(float)) {
    ds18b20_init(CONFIG_TEMPERATURE_PIN);
	// start the task that will handle the button
	xTaskCreate(temperature_task, "temperature_task", 2048, callback, 5, NULL);
}
