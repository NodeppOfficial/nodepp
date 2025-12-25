/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_MACROS
#define NODEPP_MACROS
#define thread_safe   thread_local static

/*────────────────────────────────────────────────────────────────────────────*/

#define rand_range( A, B ) clamp( rand()%B, A, B )
template< class T > T   min( const T& min, const T& max ){ return min < max ? min : max; }
template< class T > T   max( const T& min, const T& max ){ return max > min ? max : min; }
template< class T > T clamp( const T& val, const T& _min, const T& _max ){ return max( _min, min( _max, val ) ); }

/*────────────────────────────────────────────────────────────────────────────*/

#define coDelay(VALUE)           do { _time_=process::millis()+VALUE; while( process::millis()<_time_ ){ coErrno(VALUE,_LINE_,1); }} while(0);
#define coUDelay(VALUE)          do { _time_=process::micros()+VALUE; while( process::micros()<_time_ ){ coNext; }} while(0);
#define coErrno(DELAY,STATE,OUT) do { coSet(STATE); coroutine::getno( OUT,coGet,DELAY ); return OUT; case STATE:; } while(0);

/*────────────────────────────────────────────────────────────────────────────*/

#define coGoto(VALUE)  do { coSet( VALUE ); coroutine::getno(1,coGet); return 1; } while(0);
#define coStay(VALUE)  do { coSet( VALUE ); coroutine::getno(0,coGet); return 0; } while(0);
#define coNext         do { coErrno(0UL,_LINE_,1); /*-------------------------*/ } while(0);
#define coYield(VALUE) do { coErrno(0UL, VALUE,1); /*-------------------------*/ } while(0);
#define coWait(VALUE)  do { while( VALUE ){ /*------------------------*/ coNext;}} while(0);
#define coEnd          do { _time_=0; _state_=_time_; /**/ coroutine::getno(-1); } while(0); return -1;
#define coStop            } _time_=0; _state_=_time_; /**/ coroutine::getno(-1); } while(0); return -1;

/*────────────────────────────────────────────────────────────────────────────*/

#define coStart  thread_safe int _state_=0; thread_safe ulong _time_=0; coBegin
#define coBegin  do { switch(_state_) { case 0:; coroutine::getno(-2);
#define coEmit   int operator()

#define coSet(VALUE) _state_ = VALUE
#define coGet        _state_
#define coFinish     coStop

/*────────────────────────────────────────────────────────────────────────────*/

#define onMain INIT(); int main( int argc, char** args ) { \
   process::start( argc,args ); INIT(); \
   process::stop(); return 0;           \
}  void INIT

/*────────────────────────────────────────────────────────────────────────────*/

#define COROUTINE()     [=]( int& _state_, ulong& _time_ )
#define GENERATOR(NAME) struct NAME : public generator_t

/*────────────────────────────────────────────────────────────────────────────*/

#define TIME_MILLIS( VALUE ) ( VALUE )
#define TIME_SECONDS( VALUE )( VALUE * 1000 )
#define TIME_MINUTES( VALUE )( VALUE * 60 * 1000 )
#define TIME_HOURS( VALUE )  ( VALUE * 60 * 60 * 1000 )
#define TIME_DAYS( VALUE )   ( VALUE * 24 * 60 * 60 * 1000 )

/*────────────────────────────────────────────────────────────────────────────*/

#define CHUNK_TB( VALUE )( 1024 * 1024 * 1024 * 1024 * VALUE )
#define CHUNK_GB( VALUE )( 1024 * 1024 * 1024 * VALUE )
#define CHUNK_MB( VALUE )( 1024 * 1024 * VALUE )
#define CHUNK_KB( VALUE )( 1024 * VALUE )

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

#define forEach( X, ITEM ) for( auto& X : ITEM )
#define forEver() for (;;)
#define elif else if

/*────────────────────────────────────────────────────────────────────────────*/

#define TIMEOUT         process::set_timeout()

#define HASH_TABLE_SIZE 16
#define MAX_SSO         64
#define MAX_POOL_SIZE   1024
#define UNBFF_SIZE      4096
#define MAX_PATH        1024
#define CHUNK_SIZE      65536

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_KERNEL_WINDOWS 4
#define NODEPP_KERNEL_ARDUINO 3
#define NODEPP_KERNEL_POSIX   2
#define NODEPP_KERNEL_WASM    1
#define NODEPP_KERNEL_UNKNOWN 0

#ifndef _KERNEL_
#if defined(_WIN32) || defined(_WIN64)
   #define _KERNEL_ NODEPP_KERNEL_WINDOWS
#elif defined(ARDUINO)
   #define _KERNEL_ NODEPP_KERNEL_ARDUINO
#elif defined(__EMSCRIPTEN__)
   #define _KERNEL_ NODEPP_KERNEL_WASM
#elif defined(__linux__)   || defined(__APPLE__)   || defined(__FreeBSD__)   || \
      defined(__NetBSD__)  || defined(__OpenBSD__) || defined(__DragonFly__) || \
      defined(__ANDROID__) || defined(__TIZEN__)   || defined(__unix__)
   #define _KERNEL_ NODEPP_KERNEL_POSIX
#else
   #define _KERNEL_ NODEPP_KERNEL_UNKNOWN
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_OS_WINDOWS 8
#define NODEPP_OS_ANDROID 7
#define NODEPP_OS_TIZEN   6
#define NODEPP_OS_APPLE   5
#define NODEPP_OS_FRBSD   4
#define NODEPP_OS_LINUX   3
#define NODEPP_OS_IOS     2
#define NODEPP_OS_ARDUINO 1
#define NODEPP_OS_UNKNOWN 0

#ifndef _OS_
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
   #define _OS_ NODEPP_OS_WINDOWS

#elif defined(__ANDROID__)
   #define _OS_ NODEPP_OS_ANDROID

#elif defined(__TIZEN__)
   #define _OS_ NODEPP_OS_TIZEN

#elif defined(__APPLE__)
   #include <TargetConditionals.h>
   #if TARGET_OS_IPHONE
      #define _OS_ NODEPP_OS_IOS
   #else
      #define _OS_ NODEPP_OS_APPLE
   #endif

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
   #define _OS_ NODEPP_OS_FRBSD

#elif defined(__linux__)
   #define _OS_ NODEPP_OS_LINUX

#elif defined(ARDUINO)
   #define _OS_ NODEPP_OS_ARDUINO

#elif defined(__unix__)
   #include <sys/param.h>
   #if defined(BSD)
      #define _OS_ NODEPP_OS_FRBSD
   #else
      #define _OS_ NODEPP_OS_UNKNOWN
   #endif

#else
   #define _OS_ NODEPP_OS_UNKNOWN
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_ARCH_XTENSA   7
#define NODEPP_ARCH_RISCV_64 6
#define NODEPP_ARCH_RISCV_32 5
#define NODEPP_ARCH_CPU_64   4
#define NODEPP_ARCH_CPU_32   3
#define NODEPP_ARCH_ARM_64   2
#define NODEPP_ARCH_ARM_32   1
#define NODEPP_ARCH_UNKNOWN  0

#ifndef _ARCH_
#if defined(__GNUC__) || defined(__clang__)
   #if defined(__x86_64__) || defined(__ppc64__)
      #define _ARCH_ NODEPP_ARCH_CPU_64
   #elif defined(__aarch64__)
      #define _ARCH_ NODEPP_ARCH_ARM_64
   #elif defined(__i386__)
      #define _ARCH_ NODEPP_ARCH_CPU_32
   #elif defined(__arm__)
      #define _ARCH_ NODEPP_ARCH_ARM_32
   #else
      #define _ARCH_ NODEPP_ARCH_UNKNOWN
   #endif
#elif defined(_MSC_VER)
   #if defined(_M_X64) || defined(_M_AMD64)
      #define _ARCH_ NODEPP_ARCH_CPU_64
   #elif defined(_M_IX86)
      #define _ARCH_ NODEPP_ARCH_CPU_32
   #elif defined(_M_ARM64)
      #define _ARCH_ NODEPP_ARCH_ARM_64
   #elif defined(_M_ARM)
      #define _ARCH_ NODEPP_ARCH_ARM_32
   #else
      #define _ARCH_ NODEPP_ARCH_UNKNOWN
   #endif
#elif defined(__xtensa__)
   #define _ARCH_ NODEPP_ARCH_XTENSA
#elif defined(__riscv)
   #if __riscv_xlen == 64
      #define _ARCH_ NODEPP_ARCH_RISCV_64
   #else
      #define _ARCH_ NODEPP_ARCH_RISCV_32
   #endif
#else
   #define _ARCH_ NODEPP_ARCH_UNKNOWN
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_POLL_WPOLL 4
#define NODEPP_POLL_KPOLL 3
#define NODEPP_POLL_EPOLL 2
#define NODEPP_POLL_POLL  1
#define NODEPP_POLL_NONE  0

#ifndef    _POLL_
#if   _OS_ == NODEPP_OS_WINDOWS
   #define _POLL_ NODEPP_POLL_WPOLL
#elif _OS_ == NODEPP_OS_APPLE
   #define _POLL_ NODEPP_POLL_KPOLL
#elif _OS_ == NODEPP_OS_FRBSD
   #define _POLL_ NODEPP_POLL_KPOLL
#elif _OS_ == NODEPP_OS_LINUX
   #define _POLL_ NODEPP_POLL_EPOLL
#else
   #define _POLL_ NODEPP_POLL_POLL
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define typeof(DATA) string_t( typeid(DATA).name() )

#define ullong  unsigned long long int
#define ulong   unsigned long int

#define uint8   unsigned char
#define uint16  unsigned int
#define uint32  unsigned long int

#define llong   /*----*/ long long int
#define ldouble /*----*/ long double
#define wchar   /*----*/ wchar_t

#define int8    /*----*/ char
#define int16   /*----*/ int
#define int32   /*----*/ long int

#define char16  /*----*/ int
#define char32  /*----*/ long int

#define uchar   unsigned char
#define uchar16 unsigned int
#define uchar32 unsigned long int

#if !defined(_SYS_TYPES_H_) || _OS_ == NODEPP_OS_ANDROID
    #define  _SYS_TYPES_H_

#define ushort unsigned short
#define uint   unsigned int

#endif

/*────────────────────────────────────────────────────────────────────────────*/

using null_t = decltype( nullptr );

/*────────────────────────────────────────────────────────────────────────────*/

#if _OS_ == NODEPP_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN 
#define sscanff( BUFFER, FORMAT, ... ) sscanf_s( BUFFER, FORMAT, __VA_ARGS__ )
#else
#define sscanff( BUFFER, FORMAT, ... ) sscanf  ( BUFFER, FORMAT, __VA_ARGS__ )
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif
