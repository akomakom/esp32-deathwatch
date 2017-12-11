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

/** Structure to initialize post task */
typedef struct {
	void (*callback_gen_body)(char *);
	void (*callback_post_request)();
} client_config_t;


void start_client(void (*callback_gen_body)(char *), void (*callback_post_request)());
void stop_client();
void client_force_request_now();

#endif
