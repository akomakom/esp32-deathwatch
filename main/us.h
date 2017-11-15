#ifndef US_H_
#define US_H_

#define ECHO_PIN CONFIG_US_ECHO_PIN
#define TRIG_PIN CONFIG_US_TRIG_PIN

#define US_READ_DELAY 10000

void initialize_ultrasound(void (*callback)(double));

#endif
