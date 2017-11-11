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

#define ESP_INTR_FLAG_DEFAULT 0

#define ECHO_PIN CONFIG_US_ECHO_PIN
#define TRIG_PIN CONFIG_US_TRIG_PIN

#define US_READ_DELAY 10000

static const char *TAG = "us";


// Similar to uint32_t system_get_time(void)
static uint32_t get_usec() {

 struct timeval tv;

 //              struct timeval {
 //                time_t      tv_sec;     // seconds
 //                suseconds_t tv_usec;    // microseconds
 //              };

 gettimeofday(&tv,NULL);
 return (tv.tv_sec*1000000 + tv.tv_usec);


  //uint64_t tmp=get_time_since_boot();
  //uint32_t ret=(uint32_t)tmp;
  //return ret;
}


static void analyze_distance(double distance, main_data_t * main_data) {
    main_data->door_open = (distance > CONFIG_US_DISTANCE_OPEN_MIN && distance < CONFIG_US_DISTANCE_OPEN_MAX);
    main_data->car_present = (distance > CONFIG_US_DISTANCE_CAR_MIN && distance < CONFIG_US_DISTANCE_CAR_MAX);

//    if (distance > CONFIG_US_DISTANCE_OPEN_MIN && distance < CONFIG_US_DISTANCE_OPEN_MAX) {
//        //door is open
//        main_data->door_open = 1;
//        main_data->car_present = 0;
//    } else if (distance > CONFIG_US_DISTANCE_CLOSED_MIN && distance < CONFIG_US_DISTANCE_CLOSED_MAX) {
//        //door is closed
//        main_data->door_open = 0;
//        main_data->car_present = 0;
//    } else if (distance > CONFIG_US_DISTANCE_CAR_MIN && distance < CONFIG_US_DISTANCE_CAR_MAX) {
//        main_data->door_open = 0;
//        main_data->car_present = 1;
//    }
    ESP_LOGI(TAG, "Distance: %f cm, Door Open: %d, Car: %d", distance, main_data->door_open, main_data->car_present);
}

//
// Toggle trig pin and wait for input on echo pin
//
void ultrasound_task(void * pvParameters) {

    main_data_t * main_data = (main_data_t *) pvParameters ;

    while(1) {
        // HC-SR04P
        gpio_set_level(TRIG_PIN, 1);
        delay(100 );
        gpio_set_level(TRIG_PIN, 0);
        uint32_t startTime=get_usec();

        while (gpio_get_level(ECHO_PIN)==0 && get_usec()-startTime < 500*1000)
        {
            // Wait until echo goes high
        }

        startTime=get_usec();

        while (gpio_get_level(ECHO_PIN)==1 && get_usec()-startTime < 500*1000)
        {
            // Wait until echo goes low again
        }

        if (gpio_get_level(ECHO_PIN) == 0)
        {
            uint32_t diff = get_usec() - startTime; // Diff time in uSecs
            // Distance is TimeEchoInSeconds * SpeeOfSound / 2
            double distance = 340.29 * diff / (1000 * 1000 * 2); // Distance in meters
            analyze_distance(distance * 100, main_data);
        }
        else
        {
            // No value
            ESP_LOGI(TAG, "Did not receive a response!\n");
        }
        // Delay and re run.
        delay(US_READ_DELAY );
    }


}

void initialize_ultrasound(main_data_t * main_data) {

    gpio_pad_select_gpio(TRIG_PIN);
    gpio_pad_select_gpio(ECHO_PIN);

    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

	// start the task that will handle the button
	xTaskCreate(ultrasound_task, "ultrasound_task", 2048, main_data, 10, NULL);

}
