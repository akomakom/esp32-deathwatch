#ifndef UTILS_H_
#define UTILS_H_


#define delay(ms) (vTaskDelay(ms/portTICK_RATE_MS))

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


#endif
