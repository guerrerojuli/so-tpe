// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <rtc.h>
#include <lib.h>
#include <stdint.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS 0x04

static uint8_t read_cmos_register(uint8_t reg)
{
	outb(CMOS_ADDRESS, reg);
	return inb(CMOS_DATA);
}

static uint8_t bcd_to_binary(uint8_t bcd)
{
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void rtc_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{

	uint8_t sec = read_cmos_register(RTC_SECONDS);
	uint8_t min = read_cmos_register(RTC_MINUTES);
	uint8_t hrs = read_cmos_register(RTC_HOURS);

	*seconds = bcd_to_binary(sec);
	*minutes = bcd_to_binary(min);
	*hours = bcd_to_binary(hrs);
}

uint32_t rtc_get_seconds(void)
{
	uint8_t h, m, s;
	rtc_get_time(&h, &m, &s);

	return (uint32_t)s + (uint32_t)m * 60 + (uint32_t)h * 3600;
}
