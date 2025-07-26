/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_DATE
#define NODEPP_DATE

/*────────────────────────────────────────────────────────────────────────────*/

#include <ctime>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class date_t {
protected:

    struct NODE {
        bool utc;
        uint day;
        uint year;
        uint hour;
        uint month;
        uint minute;
        uint second;
    };  ptr_t<NODE> obj;

    using TIME = struct tm;

    /*─······································································─*/

    void set_time( time_t time, bool utc ) const noexcept {
        TIME* info  = !utc ? localtime( &time ) : gmtime( &time );

        set_year  ( info->tm_year+1900 );
        set_month ( info->tm_mon +1    );
        set_second( info->tm_sec       );
        set_minute( info->tm_min       );
        set_day   ( info->tm_mday      );
        set_hour  ( info->tm_hour      );
        set_utc   ( utc );

    }

    time_t get_time() const noexcept {
        TIME info; memset( &info, 0, sizeof(TIME) );

        info.tm_sec  = obj->second;
        info.tm_min  = obj->minute;
        info.tm_mon  = obj->month;
        info.tm_year = obj->year;
        info.tm_hour = obj->hour;
        info.tm_mday = obj->day;

        return mktime( &info );
    }

public:

    template< class... V >
    date_t( const V&... args ) noexcept : obj( new NODE() ) { set_date( args... ); }
    
    date_t() noexcept : obj( new NODE() ) { set_date( false ); }

    virtual ~date_t() noexcept {}

    /*─······································································─*/

    bool operator==( const date_t& other ) const noexcept { return get_stamp()==other.get_stamp(); }
    bool operator<=( const date_t& other ) const noexcept { return get_stamp()<=other.get_stamp(); }
    bool operator>=( const date_t& other ) const noexcept { return get_stamp()>=other.get_stamp(); }
    bool operator< ( const date_t& other ) const noexcept { return get_stamp()< other.get_stamp(); }
    bool operator> ( const date_t& other ) const noexcept { return get_stamp()> other.get_stamp(); }

    /*─······································································─*/

    void operator+=( const date_t& other ) const noexcept {
         set_stamp( get_stamp() + other.get_stamp(), obj->utc );
    }

    void operator-=( const date_t& other ) const noexcept {
         set_stamp( get_stamp() - other.get_stamp(), obj->utc );
    }

    void operator*=( const date_t& other ) const noexcept {
         set_stamp( get_stamp() * other.get_stamp(), obj->utc );
    }

    void operator/=( const date_t& other ) const noexcept {
         set_stamp( get_stamp() / other.get_stamp(), obj->utc );
    }

    /*─······································································─*/

    void set_stamp( const time_t& time, const bool& utc ) const noexcept {
         set_time ( time, utc );
    }

    /*─······································································─*/

    void set_date( const bool& utc ) const noexcept {
         set_utc(utc); set_time( ::time(nullptr), utc );
    }

    void set_date( const uint& year, const bool& utc ) const noexcept {
         set_utc(utc); set_year(year);
    }

    void set_date( const uint& year, const uint& month, const bool& utc ) const noexcept {
         set_utc(utc); set_year(year); set_month(month);
    }

    void set_date( const uint& year, const uint& month, const uint& day, const bool& utc ) const noexcept {
         set_utc(utc); set_year(year); set_month(month); set_day(day);
    }

    void set_date( const uint& year, const uint& month, const uint& day, const uint& hour, const bool& utc ) const noexcept {
         set_utc(utc); set_year(year); set_month(month); set_day(day); set_hour(hour);
    }

    void set_date( const uint& year, const uint& month, const uint& day, const uint& hour, const uint& min, const bool& utc ) const noexcept {
         set_utc(utc); set_year(year); set_month(month); set_day(day); set_hour(hour); set_minute(min);
    }

    void set_date( const uint& year, const uint& month, const uint& day, const uint& hour, const uint& min, const uint& second, const bool& utc ) const noexcept {
         set_utc(utc); set_year(year); set_month(month); set_day(day); set_hour(hour); set_minute(min); set_second(second);
    }

    /*─······································································─*/

    void set_year  ( uint year  ) const noexcept { obj->year   = year-1900; }
    void set_month ( uint month ) const noexcept { obj->month  = month-1;   }
    void set_second( uint sec   ) const noexcept { obj->second = sec;       }
    void set_minute( uint min   ) const noexcept { obj->minute = min;       }
    void set_hour  ( uint hour  ) const noexcept { obj->hour   = hour;      }
    void set_day   ( uint day   ) const noexcept { obj->day    = day;       }
    void set_utc   ( bool utc   ) const noexcept { obj->utc    = utc;       }

    /*─······································································─*/

    string_t get_fulltime() const noexcept { time_t time = get_time();
        !obj->utc ? localtime( &time ) : gmtime( &time );
        return (string_t)ctime( &time );
    }

    uint get_year() const noexcept { time_t time = get_time();
        TIME* info = !obj->utc ? localtime( &time ) : gmtime( &time );
        return info->tm_year+1900;
    }

    uint get_month() const noexcept { time_t time = get_time();
        TIME* info = !obj->utc ? localtime( &time ) : gmtime( &time );
        return info->tm_mon+1;
    }

    uint get_hour() const noexcept { time_t time = get_time();
        TIME* info = !obj->utc ? localtime( &time ) : gmtime( &time );
        return info->tm_hour;
    }

    uint get_day() const noexcept { time_t time = get_time();
        TIME* info = !obj->utc ? localtime( &time ) : gmtime( &time );
        return info->tm_mday;
    }

    uint get_minute() const noexcept { time_t time = get_time();
        TIME* info = !obj->utc ? localtime( &time ) : gmtime( &time );
        return info->tm_min;
    }

    uint get_second() const noexcept { time_t time = get_time();
        TIME* info = !obj->utc ? localtime( &time ) : gmtime( &time );
        return info->tm_sec;
    }

    uint get_stamp() const noexcept { return get_time(); }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {

    date_t operator+( const date_t& A, const date_t& B ){
        date_t C; C.set_stamp( A.get_stamp() + B.get_stamp(), false );
        return C;
    }

    date_t operator-( const date_t& A, const date_t& B ){
        date_t C; C.set_stamp( A.get_stamp() - B.get_stamp(), false );
        return C;
    }

    date_t operator*( const date_t& A, const date_t& B ){
        date_t C; C.set_stamp( A.get_stamp() * B.get_stamp(), false );
        return C;
    }

    date_t operator/( const date_t& A, const date_t& B ){
        date_t C; C.set_stamp( A.get_stamp() / B.get_stamp(), false );
        return C;
    }

}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace date {

    uint now(){ return date_t().get_stamp(); }

    string_t fulltime(){ return date_t().get_fulltime(); }

    uint day( const bool& utc ){ return date_t(utc).get_day(); }

    uint year( const bool& utc ){ return date_t(utc).get_year(); }

    uint hour( const bool& utc ){ return date_t(utc).get_hour(); }

    uint month( const bool& utc ){ return date_t(utc).get_month(); }

    uint minute( const bool& utc ){ return date_t(utc).get_minute(); }

    uint second( const bool& utc ){ return date_t(utc).get_second(); }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif