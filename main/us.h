#ifndef US_H_
#define US_H_

#define ECHO_PIN CONFIG_US_ECHO_PIN
#define TRIG_PIN CONFIG_US_TRIG_PIN

#define US_READ_DELAY 10000

#define US_BAD_READING -1
// reject all readings if among the US_NUM_READINGS readings (max-min)/max is greater than this
#define US_MAX_DEVIATION 0.4
// how many successive readings to average
#define US_NUM_READINGS 5

void initialize_ultrasound(void (*callback)(double));

#endif
