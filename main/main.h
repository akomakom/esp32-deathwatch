#ifndef MAIN_H_
#define MAIN_H_

#define DOOR_UNKNOWN 0
#define DOOR_OPEN 1
#define DOOR_CARPRESENT 2
#define DOOR_CLOSED 3

typedef struct {
    uint16_t motion_count;
    uint32_t motion_count_lifetime;
    uint32_t motion_timestamp; //for stale detection
    uint8_t door;
    uint16_t door_raw_distance;
    uint32_t door_measurement_timestamp; //for stale detection
    float temp;
    uint32_t temp_timestamp; //for stale detection
    uint16_t submit_count;
    uint16_t server_request_count;
} main_data_t;

void network_stopped_handler();
void network_started_handler();



/* number of seconds before panic if the client task isn't feeding the watchdog.
 * This is a minimum added to CONFIG_SUBMIT_FREQUENCY * CONFIG_SUBMIT_FAIL_COUNT_PANIC
 */
#define WATCHDOG_MINIMUM_TIMEOUT 120


#endif
