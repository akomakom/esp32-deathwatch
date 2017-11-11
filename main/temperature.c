
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#include "ds18b20.h"
#include "main.h"

#define TEMPERATURE_READ_FREQUENCY 60000

static const char *TAG = "temperature";


void temperature_task(void * pvParameters) {

    main_data_t * main_data = (main_data_t *) pvParameters ;

    while(1) {
        float temp = ds18b20_get_temp();
        main_data->temp = temp;

        ESP_LOGI(TAG, "%f C", temp);

        delay(TEMPERATURE_READ_FREQUENCY );
    }
}


void initialize_temperature(main_data_t * main_data) {
    ds18b20_init(CONFIG_TEMPERATURE_PIN);
	// start the task that will handle the button
	xTaskCreate(temperature_task, "temperature_task", 2048, main_data, 5, NULL);
}