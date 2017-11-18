#ifndef MAIN_H_
#define MAIN_H_

#define DOOR_UNKNOWN 0
#define DOOR_OPEN 1
#define DOOR_CARPRESENT 2
#define DOOR_CLOSED 3

typedef struct {
    uint16_t motion_count;
    uint8_t door;
    uint16_t door_raw_distance;
    float temp;
    uint16_t submit_count;
    uint16_t server_request_count;
} main_data_t;

#define delay(ms) (vTaskDelay(ms/portTICK_RATE_MS))

void network_stopped_handler();
void network_started_handler();

#endif
