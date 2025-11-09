#include <rtc.h>
#include <lib.h>
#include <stdint.h>

// RTC (Real-Time Clock) CMOS access
#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// CMOS registers
#define RTC_SECONDS  0x00
#define RTC_MINUTES  0x02
#define RTC_HOURS    0x04

// Helper function to read a CMOS register
static uint8_t read_cmos_register(uint8_t reg)
{
	outb(CMOS_ADDRESS, reg);
	return inb(CMOS_DATA);
}

// Convert BCD to binary (RTC may store values in BCD format)
static uint8_t bcd_to_binary(uint8_t bcd)
{
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Read current time from RTC hardware
void rtc_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
	// Read values from CMOS (they may be in BCD format)
	uint8_t sec = read_cmos_register(RTC_SECONDS);
	uint8_t min = read_cmos_register(RTC_MINUTES);
	uint8_t hrs = read_cmos_register(RTC_HOURS);

	// Convert from BCD to binary (most RTCs use BCD)
	*seconds = bcd_to_binary(sec);
	*minutes = bcd_to_binary(min);
	*hours = bcd_to_binary(hrs);
}

// Get total seconds since midnight (0-86399)
// This matches TP2_SO's getSeconds() but without timezone adjustment
uint32_t rtc_get_seconds(void)
{
	uint8_t h, m, s;
	rtc_get_time(&h, &m, &s);

	// Calculate total seconds: hours*3600 + minutes*60 + seconds
	return (uint32_t)s + (uint32_t)m * 60 + (uint32_t)h * 3600;
}
