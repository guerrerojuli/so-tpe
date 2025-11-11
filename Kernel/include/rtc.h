#ifndef RTC_H
#define RTC_H

#include <stdint.h>

uint32_t rtc_get_seconds(void);
void rtc_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);

#endif
