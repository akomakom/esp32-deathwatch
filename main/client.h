#ifndef CLIENT_H_
#define CLIENT_H_


#define WEB_SERVER CONFIG_SUBMIT_HOST
#define WEB_PORT CONFIG_SUBMIT_PORT
#define WEB_URL CONFIG_SUBMIT_URI
#define WEB_POSTDATA_TEMPLATE CONFIG_SUBMIT_FORM_DATA_TEMPLATE
#define SUBMIT_FREQUENCY CONFIG_SUBMIT_FREQUENCY
#define WEB_VERIFY_SSL false

/* Constants that aren't configurable in menuconfig */
#define INITIAL_DELAY 10000

/* number of seconds before panic if the client task isn't feeding the watchdog.
 * This is a minimum, otherwise it's a multiple of CONFIG_SUBMIT_FREQUENCY
 */
#define WATCHDOG_MINIMUM_TIMEOUT 120

void start_client(main_data_t * main_data);
void stop_client();
void client_force_request_now();

#endif
