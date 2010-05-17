/* 
   Unix SMB/CIFS implementation.
   time handling functions

   Copyright (C) Andrew Tridgell 		1992-2004
   Copyright (C) Stefan (metze) Metzmacher	2002   
   Copyright (C) Jeremy Allison			2007

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/*MODIFICATION: minor changes for OpenVAS*/

#include <time.h>
#include <sys/time.h>
#include <utime.h>
#include "byteorder.h"
#include "smb.h"
#include <limits.h>

#ifndef uint32
#define uint32 uint32_t
#endif

/**
 * @file
 * @brief time handling functions
 */


#ifndef TIME_T_MIN
#define TIME_T_MIN ((time_t)0 < (time_t) -1 ? (time_t) 0 \
		    : ~ (time_t) 0 << (sizeof (time_t) * CHAR_BIT - 1))
#endif
#ifndef TIME_T_MAX
#define TIME_T_MAX (~ (time_t) 0 - TIME_T_MIN)
#endif

#define NTTIME_INFINITY (NTTIME)0x8000000000000000LL

#define TIME_FIXUP_CONSTANT_INT 11644473600LL

time_t convert_timespec_to_time_t(struct timespec ts)
{
	/* 1 ns == 1,000,000,000 - one thousand millionths of a second.
	   increment if it's greater than 500 millionth of a second. */
	if (ts.tv_nsec > 500000000) {
		return ts.tv_sec + 1;
	}
	return ts.tv_sec;
}
/****************************************************************************
 *  Convert a normalized timeval to a timespec.
 *  ****************************************************************************/

struct timespec convert_timeval_to_timespec(const struct timeval tv)
{
	struct timespec ts;
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000;
	return ts;
}

/****************************************************************************
 *  Put a 8 byte filetime from a struct timespec. Uses GMT.
 *  ****************************************************************************/

void unix_timespec_to_nt_time(NTTIME *nt, struct timespec ts)
{
	uint64_t d;

	if (ts.tv_sec ==0 && ts.tv_nsec == 0) {
		*nt = 0;
		return;
	}
	if (ts.tv_sec == TIME_T_MAX) {
		*nt = 0x7fffffffffffffffLL;
		return;
	}		
	if (ts.tv_sec == (time_t)-1) {
		*nt = (uint64_t)-1;
		return;
	}		

	d = ts.tv_sec;
	d += (uint64_t)TIME_FIXUP_CONSTANT_INT;
	d *= 1000*1000*10;
	/* d is now in 100ns units. */
	d += (ts.tv_nsec / 100);

	*nt = d;
}

/****************************************************************************
 *  Convert a normalized timespec to a timeval.
 *  ****************************************************************************/

struct timeval convert_timespec_to_timeval(const struct timespec ts)
{
	struct timeval tv;
	tv.tv_sec = ts.tv_sec;
	tv.tv_usec = ts.tv_nsec / 1000;
	return tv;
}

/***************************************************************************
 A gettimeofday wrapper.
****************************************************************************/

void GetTimeOfDay(struct timeval *tval)
{
        gettimeofday(tval,NULL);
}


/****************************************************************************
 Take a Unix time and convert to an NTTIME structure and place in buffer 
 pointed to by p.
****************************************************************************/

void put_long_date_timespec(char *p, struct timespec ts)
{
	NTTIME nt;
	unix_timespec_to_nt_time(&nt, ts);
	SIVAL(p, 0, nt & 0xFFFFFFFF);
	SIVAL(p, 4, nt >> 32);
}

void put_long_date(char *p, time_t t)
{
	struct timespec ts;
	ts.tv_sec = t;
	ts.tv_nsec = 0;
	put_long_date_timespec(p, ts);
}

