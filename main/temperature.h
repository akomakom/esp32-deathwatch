
#define TEMPERATURE_READ_FREQUENCY 60000

/**
 * Reject temperature readings outside said boundaries, they are bad data
 * With the DS18B20, we seem to be getting a value above 4000 when something
 * goes wrong, but random values like 270C were also observed on occasion.
 * This is in Celsius.
 * **/
#define TEMPERATURE_ACCEPTABLE_MAX 200
#define TEMPERATURE_ACCEPTABLE_MIN -100

void initialize_temperature(void (*callback)(float));
