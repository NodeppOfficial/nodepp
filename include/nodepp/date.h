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

namespace nodepp { namespace date {

    uint now(){ return type::cast<uint>( time( nullptr ) ); }
    
    /*─······································································─*/

    uint year( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return (info->tm_year+1900); 
    }
    
    /*─······································································─*/

    uint month( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return info->tm_mon; 
    }
    
    /*─······································································─*/

    uint day( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return info->tm_mday; 
    }
    
    /*─······································································─*/
    
    uint hour( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return info->tm_hour; 
    }
    
    /*─······································································─*/

    uint minute( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return info->tm_min; 
    }
    
    /*─······································································─*/

    uint second( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return info->tm_sec; 
    }
    
    /*─······································································─*/

    uint weekday( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return info->tm_wday; 
    }
    
    /*─······································································─*/

    uint monthday( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return info->tm_mday; 
    }
    
    /*─······································································─*/

    uint yearday( const bool& UTC=false ){
        time_t _time = time( nullptr ); struct tm *info;
        if( UTC ) info = gmtime( &_time ); 
        else info = localtime( &_time );
        return info->tm_yday; 
    }
    
    /*─······································································─*/

    string_t fulltime(){
        time_t _time = time( nullptr ); 
        string_t data(ctime(&_time));
        return data; 
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class date_t {
protected:

    struct NODE {
        struct tm* info = nullptr;
        time_t time =::time( nullptr );
    };  ptr_t<NODE> obj;
    
public:

    template< class... V > 
    date_t( const V&... args ) noexcept : obj( new NODE() ) { set_time( args... ); }
    
    /*─······································································─*/

    void set_time() const noexcept { obj->info = localtime( &obj->time ); }

    void set_time( const uint& year, const bool& UTC=false ) const noexcept { 
        obj->info = UTC ? localtime( &obj->time ) : gmtime( &obj->time ); 
        obj->info->tm_year = year - 1900; obj->time = mktime(obj->info);
    }

    void set_time( const uint& year, const uint& month, const bool& UTC=false ) const noexcept { 
        obj->info = UTC ? localtime( &obj->time ) : gmtime( &obj->time ); 
        obj->info->tm_year = year - 1900; obj->info->tm_mon = month - 1; 
        obj->time = mktime(obj->info);
    }

    void set_time( const uint& year, const uint& month, const uint& day, const bool& UTC=false ) const noexcept { 
        obj->info = UTC ? localtime( &obj->time ) : gmtime( &obj->time ); 
        obj->info->tm_year = year - 1900; obj->info->tm_mon = month - 1; 
        obj->info->tm_mday = day; obj->time = mktime(obj->info);
    }

    void set_time( const uint& year, const uint& month, const uint& day, const uint& hour, const bool& UTC=false ) const noexcept { 
        obj->info = UTC ? localtime( &obj->time ) : gmtime( &obj->time ); 
        obj->info->tm_year = year - 1900; obj->info->tm_mon = month - 1; 
        obj->info->tm_mday = day; obj->info->tm_hour = hour; 
        obj->time = mktime(obj->info);
    }

    void set_time( const uint& year, const uint& month, const uint& day, const uint& hour, const uint& min, const bool& UTC=false ) const noexcept { 
        obj->info = UTC ? localtime( &obj->time ) : gmtime( &obj->time ); 
        obj->info->tm_year = year - 1900; obj->info->tm_mon = month - 1; 
        obj->info->tm_mday = day; obj->info->tm_hour = hour; 
        obj->info->tm_min = min; obj->time = mktime(obj->info);
    }

    void set_time( const uint& year, const uint& month, const uint& day, const uint& hour, const uint& min, const uint& sec, const bool& UTC=false ) const noexcept { 
        obj->info = UTC ? localtime( &obj->time ) : gmtime( &obj->time ); 
        obj->info->tm_year = year - 1900; obj->info->tm_mon = month - 1; 
        obj->info->tm_mday = day; obj->info->tm_hour = hour; 
        obj->info->tm_min = min; obj->info->tm_sec = sec;
        obj->time = mktime(obj->info);
    }
    
    /*─······································································─*/
    
    void set_year( uint year )   const noexcept { obj->info->tm_year = year-1900;obj->time = mktime(obj->info); }
    void set_month( uint month ) const noexcept { obj->info->tm_mon  = month-1;  obj->time = mktime(obj->info); }
    void set_second( uint sec )  const noexcept { obj->info->tm_sec  = sec;      obj->time = mktime(obj->info); }
    void set_minute( uint min )  const noexcept { obj->info->tm_min  = min;      obj->time = mktime(obj->info); }
    void set_hour( uint hour )   const noexcept { obj->info->tm_hour = hour;     obj->time = mktime(obj->info); }
    void set_day( uint day )     const noexcept { obj->info->tm_mday = day;      obj->time = mktime(obj->info); }

    /*─······································································─*/

    string_t get_fulltime()      const noexcept { return (string_t)ctime(&obj->time); }
    uint     get_year()          const noexcept { return obj->info->tm_year + 1900;   }
    uint     get_month()         const noexcept { return (uint)obj->info->tm_mon+1;   }
    uint     get_monthday()      const noexcept { return (uint)obj->info->tm_mday;    }
    uint     get_weekday()       const noexcept { return (uint)obj->info->tm_wday;    }
    uint     get_yearday()       const noexcept { return (uint)obj->info->tm_yday;    }
    uint     get_hour()          const noexcept { return (uint)obj->info->tm_hour;    }
    uint     get_day()           const noexcept { return (uint)obj->info->tm_mday;    }
    uint     get_minute()        const noexcept { return (uint)obj->info->tm_min;     }
    uint     get_second()        const noexcept { return (uint)obj->info->tm_sec;     }
    uint     get_stamp()         const noexcept { return (uint)obj->time * 1000;      }
    

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
