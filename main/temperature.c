
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "temperature.h"
#include "ds18b20.h"
#include "main.h"

static const char *TAG = "temperature";


void temperature_task(void * pvParameters) {

    float temperature;
	void (*callback)(float) = (void *) pvParameters;

    while(1) {
        temperature = ds18b20_get_temp();
        if (temperature > TEMPERATURE_ACCEPTABLE_MAX || temperature < TEMPERATURE_ACCEPTABLE_MIN) {
        	ESP_LOGI(TAG, "Bad temperature reading '%f', ignoring", temperature);
        } else {
        	callback(temperature);
        }
        delay(TEMPERATURE_READ_FREQUENCY );
    }
}


void initialize_temperature(void (*callback)(float)) {
    ds18b20_init(CONFIG_TEMPERATURE_PIN);
	// start the task that will handle the button
	xTaskCreate(temperature_task, "temperature_task", 2048, callback, 5, NULL);
}
