/**
 * This code has been adapted from https://github.com/Ebiroll/esp32_ultra
 */
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

#include <limits.h>

#include "utils.h"
#include "us.h"


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


//
// Toggle trig pin and wait for input on echo pin
//or: 3
double get_distance() {
	double distance = US_BAD_READING;
	// HC-SR04P
	gpio_set_level(US_TRIG_PIN, 1);
	delay(100 );
	gpio_set_level(US_TRIG_PIN, 0);
	uint32_t startTime=get_usec();

	while (gpio_get_level(US_ECHO_PIN)==0 && get_usec()-startTime < 500*1000)
	{
		// Wait until echo goes high
	}

	startTime=get_usec();

	while (gpio_get_level(US_ECHO_PIN)==1 && get_usec()-startTime < 500*1000)
	{
		// Wait until echo goes low again
	}

	if (gpio_get_level(US_ECHO_PIN) == 0)
	{
		uint32_t diff = get_usec() - startTime; // Diff time in uSecs
		// Distance is TimeEchoInSeconds * SpeeOfSound / 2
		distance = 340.29 * diff / (1000 * 1000 * 2) * 100; // Distance in cm
	}
	else
	{
		// No value
		ESP_LOGI(TAG, "Did not receive a response!\n");
	}
	return distance;
}

void ultrasound_task(void * pvParameters) {

	void (*callback)(double) = (void *) pvParameters;

    while(1) {
    	double distance = US_BAD_READING;
    	uint8_t valid_readings = 0;
    	float min = UINT16_MAX;
    	float max = -1;

    	//average a few readings because the sensor can be jittery
	    for (int i=1; i<=US_NUM_READINGS; i++) {
	    	double reading = get_distance();
	    	if (reading != US_BAD_READING && reading >= CONFIG_US_DISTANCE_MIN && reading <= CONFIG_US_DISTANCE_MAX) {
	    		valid_readings++;
	    		distance += reading;
	    		if (reading > max) {
	    			max = reading;
	    		}
	    		if (reading < min) {
	    			min = reading;
	    		}
	    		ESP_LOGD(TAG, "Reading (%d/%d): %f", i, US_NUM_READINGS, reading);
	    	} else {
	    		ESP_LOGI(TAG, "Bad reading (%d/%d): %f, skipped", i, US_NUM_READINGS, reading);
	    	}
	    	delay(10); //to avoid clogging up the cores
	    }

	    if (valid_readings > 0) {
	    	distance = distance / valid_readings;
		    if (((max-min)/max) > US_MAX_DEVIATION) {
		    	//let's assume that there is way too much deviation
		    	ESP_LOGI(TAG, "Rejecting readings, too much variance (%f-%f), probably jitter", min, max);
		    	distance = US_BAD_READING;
		    }
	    }


		if (distance != US_BAD_READING) {
			//good reading
			callback(distance);
		} else {
			ESP_LOGI(TAG, "Rejecting final bad distance reading: %f", distance);
		}

        // Delay and re run.
        delay(US_READ_DELAY );
    }

}

/**
 * callback should take a distance in cm
 */
void initialize_ultrasound(void (*callback)(double)) {

    gpio_pad_select_gpio(US_TRIG_PIN);
    gpio_pad_select_gpio(US_ECHO_PIN);

    gpio_set_direction(US_TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(US_ECHO_PIN, GPIO_MODE_INPUT);

	// start the task that will handle the button
	xTaskCreate(ultrasound_task, "ultrasound_task", 2048, callback, 10, NULL);

}
