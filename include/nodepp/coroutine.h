/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_COROUTINE
#define NODEPP_COROUTINE

/*────────────────────────────────────────────────────────────────────────────*/

#define rand_range( A, B ) clamp( rand()%B, A, B )
template< class T > T   min( const T& min, const T& max ){ return min < max ? min : max; }
template< class T > T   max( const T& min, const T& max ){ return max > min ? max : min; }
template< class T > T clamp( const T& val, const T& _min, const T& _max ){ return max( _min, min( _max, val ) ); }

/*────────────────────────────────────────────────────────────────────────────*/

#define _EERROR( EV, ... ) if  ( EV.empty() ){ console::error(__VA_ARGS__); } \
                           else{ EV.emit( except_t(__VA_ARGS__) ); }
#define _ERROR( ... )      throw except_t (__VA_ARGS__)

/*────────────────────────────────────────────────────────────────────────────*/

#define onMain INIT(); int main( int argc, char** args ) { \
   process::start( argc, args ); INIT(); \
   process::stop(); return 0;            \
}  void INIT

/*────────────────────────────────────────────────────────────────────────────*/

#define coTry(VALUE)    do { static auto tm=process::millis()+1000 ; while( process::millis()<tm ){ coStay(VALUE); } tm=process::micros()+1000 ; coGoto(VALUE); } while (0)
#define coDelay(VALUE)  do { static auto tm=process::millis()+VALUE; while( process::millis()<tm ){ coNext;        } tm=process::millis()+VALUE;                } while (0)
#define coUDelay(VALUE) do { static auto tm=process::micros()+VALUE; while( process::micros()<tm ){ coNext;        } tm=process::micros()+VALUE;                } while (0)

/*────────────────────────────────────────────────────────────────────────────*/

#define coStart        static int _state_ = 0; { switch(_state_) { case 0:;
#define coSet(VALUE)   _state_ = VALUE
#define coGet          _state_

/*────────────────────────────────────────────────────────────────────────────*/

#define coNext         do { _state_ = _LINE_; return  1; case _LINE_:; } while(0)
#define coYield(VALUE) do { _state_ = VALUE ; return  1; case VALUE :; } while(0)
#define coWait(VALUE)  do { while   ( VALUE ){ coNext; }               } while(0)
#define coGoto(VALUE)  do { _state_ = VALUE ; return  1;               } while(0)
#define coStay(VALUE)  do { _state_ = VALUE ; return  0;               } while(0)
#define coEnd          do { _state_ = 0;      return -1;               } while(0)
#define coStop            } _state_ = 0;      return -1;               }
#define coReset        coGoto(0)

/*────────────────────────────────────────────────────────────────────────────*/

#define CHUNK_TB( VALUE ) ( 1024 * 1024 * 1024 * 1024 * VALUE )
#define CHUNK_GB( VALUE ) ( 1024 * 1024 * 1024 * VALUE )
#define CHUNK_MB( VALUE ) ( 1024 * 1024 * VALUE )
#define CHUNK_KB( VALUE ) ( 1024 * VALUE )

/*────────────────────────────────────────────────────────────────────────────*/

#define TIME_MILLIS( VALUE )  ( VALUE )
#define TIME_SECONDS( VALUE ) ( VALUE * 1000 )
#define TIME_MINUTES( VALUE ) ( VALUE * 60 * 1000 )
#define TIME_HOURS( VALUE )   ( VALUE * 60 * 60 * 1000 )
#define TIME_DAYS( VALUE )    ( VALUE * 24 * 60 * 60 * 1000 )

/*────────────────────────────────────────────────────────────────────────────*/

#define GENERATOR(NAME) struct NAME : public generator_t
#define gnStart { switch(_state_) { case 0:;
#define coEmit  int operator()
#define gnStop  coStop
#define gnEmit  coEmit

/*────────────────────────────────────────────────────────────────────────────*/

#define forEach( X, ITEM ) for( auto& X : ITEM )
#define forEver() for (;;)
#define elif else if

/*────────────────────────────────────────────────────────────────────────────*/

#define _JSON_(...) json::parse(_STRING_(__VA_ARGS__))
#define _FUNC_  __PRETTY_FUNCTION__
#define _STRING_(...) #__VA_ARGS__
#define _NAME_  __FUNCTION__
#define _DATE_  __DATE__
#define _FILE_  __FILE__
#define _LINE_  __LINE__
#define _TIME_  __TIME__

/*────────────────────────────────────────────────────────────────────────────*/

#define HASH_TABLE_SIZE 255
#define UNBFF_SIZE      4096
#define CHUNK_SIZE      65536

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef MAX_WORKERS
#define MAX_WORKERS 0 // 1024
#endif

#ifndef MAX_EVENTS
#define MAX_EVENTS  0 // 1024
#endif

#ifndef MAX_FILENO
#define MAX_FILENO  1024 // 0
#endif

#ifndef MAX_TASKS
#define MAX_TASKS   0 // 1024
#endif

#ifndef MAX_PATH
#define MAX_PATH    1024 // 0
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef TIMEOUT
#define TIMEOUT 1
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_KERNEL_WINDOWS 4
#define NODEPP_KERNEL_POSIX   3
#define NODEPP_KERNEL_ARDUINO 2
#define NODEPP_KERNEL_WASM    1
#define NODEPP_KERNEL_UNKNOWN 0

#ifndef    _KERNEL
#if defined(WINDOWS) || defined(_WIN32) || defined(_WIN64)
   #define _KERNEL NODEPP_KERNEL_WINDOWS
#elif defined(ARDUINO)
   #define _KERNEL NODEPP_KERNEL_ARDUINO
#elif defined(__EMSCRIPTEN__)
   #define _KERNEL  NODEPP_KERNEL_WASM
#elif defined(__FreeBSD__)
   #define _KERNEL NODEPP_KERNEL_POSIX
#elif defined(__APPLE__)
   #define _KERNEL NODEPP_KERNEL_POSIX
#elif defined(__linux__)
   #define _KERNEL NODEPP_KERNEL_POSIX
#elif defined(__unix__)
   #define _KERNEL NODEPP_KERNEL_POSIX
#else
   #define _KERNEL NODEPP_KERNEL_UNKNOWN
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_OS_WINDOWS 9
#define NODEPP_OS_ANDROID 8
#define NODEPP_OS_IOS     7
#define NODEPP_OS_TIZEN   6
#define NODEPP_OS_APPLE   5
#define NODEPP_OS_FRBSD   4
#define NODEPP_OS_LINUX   3
#define NODEPP_OS_UNIX    2
#define NODEPP_OS_ARDUINO 1
#define NODEPP_OS_UNKNOWN 0

#ifndef    _OS
#if defined(WINDOWS) || defined(_WIN32) || defined(_WIN64)
   #define _OS NODEPP_OS_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
   #define _OS NODEPP_OS_IOS
#elif defined(__TIZEN__)
   #define _OS NODEPP_OS_TIZEN
#elif defined(__ANDROID__)
   #define _OS NODEPP_OS_ANDROID
#elif defined(__FreeBSD__)
   #define _OS NODEPP_OS_FRBSD
#elif defined(__APPLE__)
   #define _OS NODEPP_OS_APPLE
#elif defined(__linux__)
   #define _OS NODEPP_OS_LINUX
#elif defined(__unix__)
   #define _OS NODEPP_OS_UNIX
#elif defined(ARDUINO)
   #define _OS NODEPP_OS_ARDUINO
#else
   #define _OS NODEPP_OS_UNKNOWN
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_ARCH_CPU_64  4
#define NODEPP_ARCH_CPU_32  3
#define NODEPP_ARCH_ARM_64  2
#define NODEPP_ARCH_ARM_32  1
#define NODEPP_ARCH_UNKNOWN 0

#ifndef       _ARCH
#if defined(__GNUC__)
   #if defined(__x86_64__)
      #define _ARCH NODEPP_ARCH_CPU_64
   #elif defined(__aarch64__)
      #define _ARCH NODEPP_ARCH_ARM_64
   #elif defined(__i386__)
      #define _ARCH NODEPP_ARCH_CPU_32
   #elif defined(__arm__)
      #define _ARCH NODEPP_ARCH_ARM_64
   #else
      #define _ARCH NODEPP_ARCH_UNKNOWN
   #endif
#else
   #if defined(_M_IX86)
      #define _ARCH NODEPP_ARCH_CPU_32
   #elif defined(_M_ARM64)
      #define _ARCH NODEPP_ARCH_ARM_64
   #elif defined(_M_X64)
      #define _ARCH NODEPP_ARCH_CPU_64
   #elif defined(_M_ARM)
      #define _ARCH NODEPP_ARCH_ARM_32
   #else
      #define _ARCH NODEPP_ARCH_UNKNOWN
   #endif
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_ENVIRONMENT_GNU     6
#define NODEPP_ENVIRONMENT_WASM    5
#define NODEPP_ENVIRONMENT_MSYS2   4
#define NODEPP_ENVIRONMENT_MINGW   3
#define NODEPP_ENVIRONMENT_CYWIN   2
#define NODEPP_ENVIRONMENT_UNKNOWN 0

#ifndef    _ENVIRONMENT
#elif defined(__EMSCRIPTEN__)
   #define _ENVIRONMENT NODEPP_ENVIRONMENT_WASM
#if defined(__MSYS__)
   #define _ENVIRONMENT NODEPP_ENVIRONMENT_MSYS2
#elif defined(__CYGWIN__)
   #define _ENVIRONMENT NODEPP_ENVIRONMENT_CYWIN
/*
#elif defined(__MINGW32__) || defined(__MINGW64__)
   #define _ENVIRONMENT NODEPP_ENVIRONMENT_MINGW
#elif defined(__GNUC__)
   #define _ENVIRONMENT NODEPP_ENVIRONMENT_GNU
*/
#else
   #define _ENVIRONMENT NODEPP_ENVIRONMENT_UNKNOWN
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_POLL_WPOLL 3
#define NODEPP_POLL_KPOLL 2
#define NODEPP_POLL_EPOLL 1
#define NODEPP_POLL_POLL  0

#ifndef    _POLL
#if   _OS == NODEPP_OS_WINDOWS
   #define _POLL NODEPP_POLL_WPOLL
#elif _OS == NODEPP_OS_APPLE
   #define _POLL NODEPP_POLL_KPOLL
#elif _OS == NODEPP_OS_FRBSD
   #define _POLL NODEPP_POLL_KPOLL
#elif _OS == NODEPP_OS_LINUX
   #define _POLL NODEPP_POLL_EPOLL
#else
   #define _POLL NODEPP_POLL_POLL
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define typeof(DATA) (string_t){ typeid( DATA ).name() }
struct generator_t { protected: int _state_ = 0; };

#define ullong  unsigned long long int
#define ulong   unsigned long int

#define uint8   unsigned char
#define uint16  unsigned int
#define uint32  unsigned long int

#define llong            long long int
#define ldouble          long double
#define wchar            wchar_t

#define int8             char
#define int16            int
#define int32            long int

#define char16           int
#define char32           long int

#define uchar   unsigned char
#define uchar16 unsigned int
#define uchar32 unsigned long int

#if !defined(_SYS_TYPES_H_) || _OS == NODEPP_OS_ANDROID
    #define  _SYS_TYPES_H_

#define ushort unsigned short
#define uint   unsigned int

#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif
