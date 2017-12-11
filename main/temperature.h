
#define TEMPERATURE_READ_FREQUENCY 60000

/**
 * Reject temperature readings outside these boundaries, they are bad data
 * With the DS18B20, we seem to be getting a value above 4000 when something
 * goes wrong, but random values like 270C were also observed on occasion.
 * This is in Celsius.
 * **/
#define TEMPERATURE_ACCEPTABLE_MAX 200
#define TEMPERATURE_ACCEPTABLE_MIN -100

#define TEMPERATURE_BAD_READING -1000
#define TEMPERATURE_BAD_READING_STRING "-1000"

// Number of reading DELAYS (TEMPERATURE_READ_FREQUENCY) that must elapse before we stop trusting
// the old temperature reading if we can't get a good one.
#define TEMPERATURE_MAX_READING_AGE 5

void initialize_temperature(void (*callback)(float));
