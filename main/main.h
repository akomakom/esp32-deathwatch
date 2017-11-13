#define DOOR_OPEN 1
#define DOOR_CARPRESENT 2
#define DOOR_CLOSED 3

typedef struct {
    uint16_t motion_count;
    uint8_t door;
    float temp;
    uint16_t submit_count;
} main_data_t;


#define delay(ms) (vTaskDelay(ms/portTICK_RATE_MS))
