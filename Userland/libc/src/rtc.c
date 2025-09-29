#include "rtc.h"
#include "unistd.h"
#include "stdint.h"

uint64_t getSeconds(){
  return sys_read_rtc(RTC_SEC);
}

uint64_t getMinutes(){
  return sys_read_rtc(RTC_MIN);
}

uint64_t getHours(){
  return sys_read_rtc(RTC_HR);
}

uint64_t getDayWeek(){
  return sys_read_rtc(RTC_DAYW);
}

uint64_t getDayMonth(){
  return sys_read_rtc(RTC_DAYM);
}

uint64_t getMonth(){
  return sys_read_rtc(RTC_MON);
}

uint64_t getYear(){
  return sys_read_rtc(RTC_YEAR);
}

Date getDate(void){
  Date date;
  date.sec = getSeconds();
  date.min = getMinutes();
  date.hr = getHours();
  date.dayW = getDayWeek();
  date.dayM = getDayMonth();
  date.mon = getMonth();
  date.year = getYear(); 
  return date;
}

void printDate(void){
    Date date = getDate();
    char dateBuffer[4];
    char timeBuffer[9]; // HH:MM:SS format
    
    switch (date.dayW){
		case 1:
			dateBuffer[0] = 'S';
			dateBuffer[1] = 'u';
			dateBuffer[2] = 'n';
			break;
		case 2:
                        dateBuffer[0] = 'M';
                        dateBuffer[1] = 'o';
                        dateBuffer[2] = 'n';
                        break;
		case 3:
                        dateBuffer[0] = 'T';
                        dateBuffer[1] = 'u';
                        dateBuffer[2] = 'e';
                        break;
		case 4:
                        dateBuffer[0] = 'W';
                        dateBuffer[1] = 'e';
                        dateBuffer[2] = 'd';
                        break;
		case 5:
                        dateBuffer[0] = 'T';
                        dateBuffer[1] = 'h';
                        dateBuffer[2] = 'u';
                        break;
		case 6:
                        dateBuffer[0] = 'F';
                        dateBuffer[1] = 'r';
                        dateBuffer[2] = 'i';
                        break;
		case 0:
                        dateBuffer[0] = 'S';
                        dateBuffer[1] = 'a';
                        dateBuffer[2] = 't';
                        break;
	}
    dateBuffer[3] = '\0';
    
    // Format time with leading zeros
    timeBuffer[0] = (date.hr / 10) + '0';
    timeBuffer[1] = (date.hr % 10) + '0';
    timeBuffer[2] = ':';
    timeBuffer[3] = (date.min / 10) + '0';
    timeBuffer[4] = (date.min % 10) + '0';
    timeBuffer[5] = ':';
    timeBuffer[6] = (date.sec / 10) + '0';
    timeBuffer[7] = (date.sec % 10) + '0';
    timeBuffer[8] = '\0';
    
    void* printf_args[] = { dateBuffer, &date.dayM, &date.mon, &date.year, timeBuffer };
    printf( "%s %d/%d/%d %s\n", printf_args);
}
