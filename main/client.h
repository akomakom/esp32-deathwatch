#ifndef CLIENT_H_
#define CLIENT_H_


#define WEB_SERVER CONFIG_SUBMIT_HOST
#define WEB_PORT CONFIG_SUBMIT_PORT
#define WEB_URL CONFIG_SUBMIT_URI
#define WEB_POSTDATA_TEMPLATE CONFIG_SUBMIT_FORM_DATA_TEMPLATE
#define SUBMIT_FREQUENCY CONFIG_SUBMIT_FREQUENCY

/* Constants that aren't configurable in menuconfig */
#define INITIAL_DELAY 10000


void initialize_client(main_data_t * main_data);
void client_force_request_now();

#endif
