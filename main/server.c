#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/portmacro.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "string.h"

#include "cJSON.h"

#include "main.h"
#include "client.h" //feels a little weird to plug directly into client, need better decoupling for submit now
#include "server.h"

static const char *TAG = "webserver";

char* json_unformatted = NULL;

const static char http_html_hdr[] =
    "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_index_hml[] = "<!DOCTYPE html>"
      "<html>\n"
      "<head>\n"
      "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
      "  <style type=\"text/css\">\n"
      "    html, body, iframe { margin: 0; padding: 0; height: 100%; }\n"
      "    iframe { display: block; width: 100%; border: none; }\n"
      "  </style>\n"
      "<title>HELLO ESP32</title>\n"
      "</head>\n"
      "<body>\n"
      "<h1>Hello World, from ESP32!</h1>\n"
      "<a href='/j'>Data</a>"
      "</body>\n"
      "</html>\n";


#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"


static void
http_server_netconn_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;

  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);

  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);

    // strncpy(_mBuffer, buf, buflen);

    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    ESP_LOGI(TAG, "buffer = %s \n", buf);
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' ) {
          ESP_LOGI(TAG, "buf[5] = %c\n", buf[5]);
      /* Send the HTML header
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */

      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);

      if(buf[5]=='s') {
//        submit data now
        /* Send our HTML page */
    	  client_force_request_now();
    	  netconn_write(conn, http_index_hml, sizeof(http_index_hml)-1, NETCONN_NOCOPY);
      }
      else if(buf[5]=='j') {
    	  netconn_write(conn, json_unformatted, strlen(json_unformatted), NETCONN_NOCOPY);
      }
      else {
          netconn_write(conn, http_index_hml, sizeof(http_index_hml)-1, NETCONN_NOCOPY);
      }
    }

  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);

  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

static void http_server(void *pvParameters)
{
  struct netconn *conn, *newconn;
  err_t err;
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, NULL, 80);
  netconn_listen(conn);
  do {
     err = netconn_accept(conn, &newconn);
     if (err == ERR_OK) {
       http_server_netconn_serve(newconn);
       netconn_delete(newconn);
     }
   } while(err == ERR_OK);
   netconn_close(conn);
   netconn_delete(conn);
}


static void generate_json(void *pvParameters) {

    main_data_t * main_data = (main_data_t *) pvParameters ;

	cJSON *root, *info, *d;
	root = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "d", d = cJSON_CreateObject());
	cJSON_AddItemToObject(root, "info", info = cJSON_CreateObject());

	cJSON_AddStringToObject(d, "myName", "DeathWatch");

	cJSON_AddNumberToObject(d, JSON_KEY_TEMPERATURE, main_data->temp);
	cJSON_AddNumberToObject(d, JSON_KEY_MOTION_COUNT, main_data->motion_count);
	cJSON_AddNumberToObject(d, JSON_KEY_DOOR, main_data->door);
	cJSON_AddNumberToObject(d, JSON_KEY_DOOR_RAW, main_data->door_raw_distance);
	cJSON_AddNumberToObject(d, JSON_KEY_REQUEST_COUNT, main_data->submit_count);

	cJSON_AddNumberToObject(info, JSON_KEY_HEAP, xPortGetFreeHeapSize());
	cJSON_AddNumberToObject(info, JSON_KEY_HEAP_MIN, xPortGetMinimumEverFreeHeapSize());
	cJSON_AddStringToObject(info, JSON_KEY_SDK, esp_get_idf_version());
	cJSON_AddNumberToObject(info, JSON_KEY_TIME, esp_log_timestamp());

	while (1) {
		cJSON_ReplaceItemInObject(info, JSON_KEY_HEAP,
				cJSON_CreateNumber(xPortGetFreeHeapSize()));
		cJSON_ReplaceItemInObject(info, JSON_KEY_HEAP_MIN,
				cJSON_CreateNumber(xPortGetMinimumEverFreeHeapSize()));
		cJSON_ReplaceItemInObject(info, JSON_KEY_TIME,
				cJSON_CreateNumber(esp_log_timestamp()));


        cJSON_ReplaceItemInObject(d, JSON_KEY_TEMPERATURE, cJSON_CreateNumber(main_data->temp));
        cJSON_ReplaceItemInObject(d, JSON_KEY_MOTION_COUNT, cJSON_CreateNumber(main_data->motion_count));
        cJSON_ReplaceItemInObject(d, JSON_KEY_DOOR, cJSON_CreateNumber(main_data->door));
        cJSON_ReplaceItemInObject(d, JSON_KEY_DOOR_RAW, cJSON_CreateNumber(main_data->door_raw_distance));
        cJSON_ReplaceItemInObject(d, JSON_KEY_REQUEST_COUNT, cJSON_CreateNumber(main_data->submit_count));


        free(json_unformatted);
		json_unformatted = cJSON_PrintUnformatted(root);
		ESP_LOGD(TAG, "[%d char]: %s ", strlen(json_unformatted), json_unformatted);
		delay(JSON_REGEN_FREQUENCY);
//		free(json_unformatted); //

	}
}

int initialize_server(main_data_t * main_data) {
    xTaskCreate(&generate_json, "generate_json", 2048, main_data, 5, NULL);
    xTaskCreate(&http_server, "http_server", 2048, NULL, 5, NULL);
    return 0;
}

