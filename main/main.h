typedef struct {
    uint16_t motion_count;
    uint8_t door_open;
    uint8_t car_present;
    float temp;
    uint16_t submit_count;
} main_data_t;


#define delay(ms) (vTaskDelay(ms/portTICK_RATE_MS))