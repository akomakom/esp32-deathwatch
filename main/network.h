#ifndef NETWORK_H_
#define NETWORK_H_

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASSWORD


void initialise_wifi(void);
void wifi_await_connection();


void wifi_exclusive_start(char * caller);
void wifi_exclusive_end(char * caller);

#endif
