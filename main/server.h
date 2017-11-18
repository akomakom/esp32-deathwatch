#ifndef SERVER_H_
#define SERVER_H_


#define JSON_REGEN_FREQUENCY 10000

#define JSON_KEY_HEAP 			"heap"
#define JSON_KEY_HEAP_MIN 		"heap_min"
#define JSON_KEY_TIME 			"time"
#define JSON_KEY_SDK 			"sdk"
#define JSON_KEY_TEMPERATURE 	"temperature"
#define JSON_KEY_MOTION_COUNT 	"motion_count"
#define JSON_KEY_DOOR			"door"
#define JSON_KEY_DOOR_RAW		"door_raw"
#define JSON_KEY_REQUEST_COUNT  "request_count"
#define JSON_KEY_SERVE_COUNT  	"serve_count"



void start_server(main_data_t * main_data);
void stop_server();

#endif
