
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef LacewingTimeHelper
#define LacewingTimeHelper

#include <time.h>

const char * const Weekdays [] =
    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

const char * const Months [] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

inline int LookupDayOfWeek(const char * String)
{
    for(int i = 0; i < 7; ++i)
        if(!strcasecmp(Weekdays[i], String))
            return i;

    return -1;
}

inline int LookupMonth(const char * String)
{
    for(int i = 0; i < 12; ++i)
        if(!strcasecmp(Months[i], String))
            return i;

    return -1;
}

inline void ParseTime(char * Time, tm &TimeStructure)
{
    if(strlen(Time) < 8)
        return;

    Time[2] = 0;
    Time[5] = 0;

    TimeStructure.tm_hour = atoi(Time);
    TimeStructure.tm_min  = atoi(Time + 3);
    TimeStructure.tm_sec  = atoi(Time + 6);
}

inline time_t ParseTimeString(const char * StringC)
{
    char String[32];
    strcpy(String, StringC);

    int Length = strlen(String);

    if(Length < 8)
        return 0;

    char * Month, * Time;
    int Day, Year;

    if(String[3] == ',')
    {
        /* RFC 822/RFC 1123 - Sun, 06 Nov 1994 08:49:37 GMT */

        if(Length < 29)
            return 0;

        String[ 7] = 0;
        String[11] = 0;
        String[16] = 0;
        
        Day = atoi(String + 4);
        Month = String + 8;
        Year = atoi(String + 12);
        Time = String + 17;

    }
    else if(String[3] == ' ' || String[3] == '	')
    {
        /* ANSI C's asctime() format - Sun Nov  6 08:49:37 1994 */

        if(Length < 24)
            return 0;

        String[ 7] = 0;
        String[10] = 0;
        String[19] = 0;

        Month = String + 4;
        Day = atoi(String + 8);
        Time = String + 11;
        Year = atoi(String + 20);
    }
    else
    {
        /* RFC 850 date (Sunday, 06-Nov-94 08:49:37 GMT) - unsupported */
        
        return 0;
    }

    tm TM;

    TM.tm_mday  = Day;
    TM.tm_wday  = 0;
    TM.tm_year  = Year - 1900;
    TM.tm_isdst = 0;
    TM.tm_mon   = LookupMonth(Month);
    TM.tm_yday  = 0;

    ParseTime(Time, TM);

    #if defined(ANDROID)
        return timegm64 (&TM);
    #elif defined (LacewingWindows)
        #ifndef __MINGW_H
            return _mkgmtime64 (&TM);
        #else

            /* _mkgmtime is missing from MinGW.  Argghh! */

            static HINSTANCE MSVCRT = LoadLibraryA ("msvcrt.dll");

            static __time64_t (__stdcall * fn) (tm *) =
                (__time64_t (__stdcall *) (tm *)) GetProcAddress (MSVCRT, "_mkgmtime64");

            return fn (&TM);

        #endif
    #else
        #if HAVE_TIMEGM
            return timegm (&TM);
        #else
            #pragma error "Can't find a suitable way to convert a tm to a UTC UNIX time"
        #endif
    #endif
}

#endif 

