#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "isodate.h"

char validateISOTime(uint8_t *data, size_t size) {
	int datecount = 1, timecount = 0, zonecount = 0, tick = 0, segments = 1;
	uint8_t c;
	size_t i;
	
	assert(data);
	
	if(size > MAX_DATE_STRING_LENGTH) {
		return 0;
	}
	
	for(i = 0;i < size; i++) {
		c = data[i];
		if(!timecount) {
			switch(c) {
				case((uint8_t)'T'):
				case((uint8_t)' '): /* Start of time entry */
					if(segments != MAX_DATE_SEGMENTS) {
						return 0;
					}
					timecount++;
					tick = 0;
					break;
				case((uint8_t)':'): /* This (the first) entry is a time entry, not date */
					if(datecount == DATE_SEGMENT_LENGTH+1) {
						timecount = datecount;
						datecount = 0;
					} else {
						return 0; /* Minutes may only be two characters */
					}
					tick = 0;
					break;
				case((uint8_t)'-'): /* Start of new number */
					if(segments == 2 && datecount == (DATE_SEGMENT_LENGTH*2)+1){
						return 0; /* If we are starting a third segment, but have no year */
					} 
					segments++;
					if(tick == YEAR_SEGMENT_LENGTH) { /* If a four digit number was given */
						if(datecount != YEAR_SEGMENT_LENGTH+1) { /* If it was not the first four digits of the date */
							return 0;
						}
					} else if (tick != DATE_SEGMENT_LENGTH) { /* Numbers may only otherwise be two digits in length */
						return 0;
					}
					tick = 0;
					break;
				default:
					if(c >= (uint8_t)'0' && c <= (uint8_t)'9'){
						tick++;
						datecount++;
					} else {
						return 0; /* Invalid character for date entry */
					}
			}
		} else if(!zonecount) {
			switch(c) {
				case((uint8_t)':'): /* Time numbers may only be two digits in length */
					if(tick != TIME_SEGMENT_LENGTH) {
						return 0;
					}
					tick = 0;
					break;
				case((uint8_t)'+'):
				case((uint8_t)'-'): /* Start of time zone entry */
					if(timecount < ((TIME_SEGMENT_LENGTH*2)+1)) { /* A time zone must have a time */
						return 0;
					}
					zonecount++;
					tick = 0;
					break;
				default:
					if(c >= (uint8_t)'0' && c <= (uint8_t)'9') {
						timecount++;
						tick++;
					} else {
						return 0; /* Invalid character for time entry */
					}
			}
		} else {
			if(c >= (uint8_t)'0' && c <= (uint8_t)'9'){
				zonecount++;
				tick++;
			} else {
				return 0; /* Invalid character for zone entry */
			}
		}
		if(timecount > FULL_TIME_LENGTH+1 || datecount > FULL_DATE_LENGTH+1 || zonecount > TIME_ZONE_LENGTH+1) { /* Too many digits may not be entered per entry */
			return 0;
		}
	}
	
	if(zonecount != TIME_ZONE_LENGTH+1 && zonecount != 0) { /* Validate the zone count */
		return 0; /* Time zone must be either four characters, or not there at all */
	}
	
	if(zonecount == 0) { /* If the last item was not a zone count, it has not yet been validated */
		if(datecount == YEAR_SEGMENT_LENGTH+1 && timecount == 0) { /* Year can be four digits */
			if(tick != YEAR_SEGMENT_LENGTH && tick != DATE_SEGMENT_LENGTH) { /* MM-DD and YYYY */
				return 0; 
			}
		} else if(tick != DATE_SEGMENT_LENGTH) {
			return 0; /* Ensure the last segment was two digits long */
		}
	}
	return 1;
}

void test(char* what, char expected) {
	printf("Testing \"%s\" with an expected ", what);
	if(expected) {
		printf("success....");
	} else {
		printf("failure....");
	}
	assert(validateISOTime((uint8_t*)what, strlen(what)) == expected);
	printf("test passed\n");
}

void tester(void) {
	test("2011", 1);
	test("2011-11", 1);
	test("2011-11-12", 1);
	test("11-12", 1);
	test("14:54", 1);
	test("14:54:39", 1);
	test("2011-11-12 14:54", 1);
	test("2011-11-12 14:54:39", 1);
	test("2011-11-12T14:54", 1);
	test("2011-11-12T14:54:39", 1);
	test("14:54:39+0800", 1);
	test("14:54:39-0800", 1);
	test("14:54-0800", 1);
	test("2011-11-12T14:54:39-0800", 1);
	test("2011-11-12 14:54:39+0800", 1);
	test("2011-11-12 14:54:39-0800", 1);
	test("2011-11-12T14:54:39+0800", 1);
	test("2011-11-12 14:54+0800", 1);
	test("2011-11-12T14:54:39+0", 0);
	test("2011-11-12T14:54:3", 0);
	test("some string", 0);
	test("2011-11-12 14:54+0800 asdf", 0);
	test("", 0);
	test("2011-11-12 +0800", 0);
	test("10-10-10", 0);
	test("10-2010", 0);
	test("2010-10", 1);
	test("2010-10-10-10", 0);
}
