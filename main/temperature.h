
#define TEMPERATURE_READ_FREQUENCY 60000

/**
 * Reject temperature readings beyond this value, they are bad data
 * With the DS18B20, we seem to be getting a value above 4000
 * when something goes wrong
 * **/
#define TEMPERATURE_ACCEPTABLE_MAX 1000
#define TEMPERATURE_ACCEPTABLE_MIN -1000

void initialize_temperature(void (*callback)(float));
