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

//#include "motion.h"


static main_data_t main_data;

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );

    //temporary
    main_data.temp = 70;
    main_data.motion_count = 0;
    main_data.door_open = 0;
    main_data.car_present = 0;
    main_data.submit_count = 0;

    initialise_wifi();

    initialize_motion(&main_data);
    initialize_ultrasound(&main_data);
    initialize_temperature(&main_data);

    initialize_client(&main_data);
    initialize_server(&main_data);


}
