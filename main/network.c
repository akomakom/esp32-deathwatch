#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "freertos/semphr.h"

#include "main.h"
#include "network.h"


static SemaphoreHandle_t wifi_semaphore;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static const char *TAG = "network";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "Starting WIFI connect");
    	ESP_ERROR_CHECK(esp_wifi_connect());
    	ESP_LOGI(TAG, "WIFI Connected");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        ESP_LOGI(TAG, "IP: "IPSTR"/"IPSTR"/"IPSTR,
            IP2STR(&event->event_info.got_ip.ip_info.ip),
            IP2STR(&event->event_info.got_ip.ip_info.netmask),
            IP2STR(&event->event_info.got_ip.ip_info.gw)
        );

        network_started_handler();

        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        network_stopped_handler();
        ESP_LOGI(TAG, "WIFI Disconnect, reconnecting");
        ESP_ERROR_CHECK(esp_wifi_connect());
        ESP_LOGI(TAG, "WIFI ReConnected");
        break;
    default:
        break;
    }
    return ESP_OK;
}

void wifi_await_connection() {
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
	                            false, true, portMAX_DELAY);
}

void wifi_exclusive_start(const char * caller) {
//	while( xSemaphoreTake( wifi_semaphore, 1000 / portTICK_RATE_MS) != pdTRUE ) {
//		//Print out a warning if mutex is busy for that long
//		ESP_LOGW(TAG, "Waiting to take mutex: %s", caller);
//	}
}

void wifi_exclusive_end(const char * caller) {
//	xSemaphoreGive(wifi_semaphore);
}


void initialise_wifi(void)
{
	wifi_semaphore = xSemaphoreCreateMutex();
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    ESP_LOGI(TAG, "Connected bit is %d", CONNECTED_BIT);
}

