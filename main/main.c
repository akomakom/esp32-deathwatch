#include <stdio.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
//#include "driver/adc.h"
#include <sys/time.h>

#include "main.h"
#include "network.h"
#include "client.h"
#include "server.h"
#include "motion.h"
#include "us.h"
#include "temperature.h"

static const char *TAG = "main";

static main_data_t main_data;

void motion_callback() {
    ESP_LOGI(TAG, "Motion detected: %d", main_data.motion_count);
	main_data.motion_count++;
}

void distance_callback(double distance) {
	static uint8_t newstate = 0;
    if(distance > CONFIG_US_DISTANCE_OPEN_MIN && distance < CONFIG_US_DISTANCE_OPEN_MAX) {
    	newstate = DOOR_OPEN;
    } else if (distance > CONFIG_US_DISTANCE_CAR_MIN && distance < CONFIG_US_DISTANCE_CAR_MAX) {
    	newstate = DOOR_CARPRESENT;
    } else {
    	newstate = DOOR_CLOSED;
    }

    if (main_data.door != DOOR_UNKNOWN && newstate != main_data.door) {
    	ESP_LOGI(TAG, "Door state changed from %d to %d", main_data.door, newstate);
    	main_data.door = newstate;
    	if (CONFIG_SUBMIT_ON_DOOR_STATE_CHANGE) {
    	    client_force_request_now();
    	}
    } else {
    	main_data.door = newstate; //just set it quietly.
    }

    main_data.door_raw_distance = distance; //saving it as an int for simplicity
    ESP_LOGI(TAG, "Distance: %f cm, Door: %d", distance, main_data.door);
}

void temperature_callback(float temperature) {
    ESP_LOGI(TAG, "Temperature is %f C", temperature);
    main_data.temp = temperature;
}

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );

    //initial values
    main_data.temp = -1000;
    main_data.motion_count = 0;
    main_data.door = DOOR_UNKNOWN;
    main_data.submit_count = 0;
    main_data.server_request_count = 0;

    initialise_wifi();

    initialize_motion(&motion_callback);
    initialize_ultrasound(&distance_callback);
    initialize_temperature(&temperature_callback);


//    initialize_client(&main_data);
//    initialize_server(&main_data);

}

void network_stopped_handler() {
	stop_client();
	stop_server();
}
void network_started_handler() {
	start_server(&main_data);
	start_client(&main_data);
}
