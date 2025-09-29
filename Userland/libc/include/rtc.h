#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include "stdio.h"
#define RTC_SEC 0x00
#define RTC_MIN 0x02
#define RTC_HR 0x04
#define RTC_DAYW 0x06
#define RTC_DAYM 0x07
#define RTC_MON 0x08
#define RTC_YEAR 0x09

typedef struct Date {
  uint64_t sec;
  uint64_t min;
  uint64_t hr;
  uint64_t dayW;
  uint64_t dayM;
  uint64_t mon;
  uint64_t year;
} Date;

typedef Date *DatePtr;

// Syscall declaration
uint64_t sys_read_rtc(uint64_t rtc_register);

uint64_t getSeconds();
uint64_t getMinutes();
uint64_t getHours();
uint64_t getDayWeek();
uint64_t getDayMonth();
uint64_t getMonth();
uint64_t getYear();
Date getDate(void);
void printDate(void);

#endif