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

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_REGEX_GRPH 1
#define NODEPP_REGEX_LITE 0

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_ALLOW_THREADS
#define NODEPP_ALLOW_THREADS 1
#endif

#ifndef NODEPP_ALLOW_ALLOCATOR
#define NODEPP_ALLOW_ALLOCATOR 0
#endif

#ifndef NODEPP_ALLOW_STD_SUPPORT
#define NODEPP_ALLOW_STD_SUPPORT 1
#endif

#ifndef NODEPP_ALLOW_THROW_EXCEPTION
#define NODEPP_ALLOW_THROW_EXCEPTION 1
#endif

#ifndef NODEPP_ALLOW_PTR_ATOMIC_COUNTER
#define NODEPP_ALLOW_PTR_ATOMIC_COUNTER 0
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_SCHEDULER_IOURING 4
#define NODEPP_SCHEDULER_IOCP    3
#define NODEPP_SCHEDULER_KQUEUE  2 
#define NODEPP_SCHEDULER_LITE    1
#define NODEPP_SCHEDULER_EPOLL   0

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_ALLOW_THREADS == 1
   #define NODEPP_THREAD_SUPPORTED
#else
   #define thread_local /*unused*/
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#if NODEPP_ALLOW_THROW_EXCEPTION==1
#define NODEPP_THROW_ERROR(...) do { throw nodepp::except_t(__VA_ARGS__); } while(0)
#else
#define NODEPP_THROW_ERROR(...) do { nodepp::console::error(__VA_ARGS__); nodepp::process::exit(); } while(0)
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {
static bool& NODEPP_LOCAL_SHTDWN(){ thread_local static bool out=false; return out; }
static bool& NODEPP_SHTDWN /**/ (){ /*--------*/ static bool out=false; return out; }
/*--*/ using null_t = decltype( nullptr ); }

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {
template< class T > T   min( const T& min, const T& max ){ return min < max ? min : max; }
template< class T > T   max( const T& min, const T& max ){ return max > min ? max : min; }
template< class T > T clamp( const T& val, const T& _min, const T& _max ){ return max( _min, min( _max, val ) ); }}

/*────────────────────────────────────────────────────────────────────────────*/

#define coDelay(VALUE)           do { _time_=nodepp::process::millis()+VALUE; while( nodepp::process::millis()<_time_ ){ coErrno(VALUE,__LINE__,1); }} while(0);
#define coUDelay(VALUE)          do { _time_=nodepp::process::micros()+VALUE; while( nodepp::process::micros()<_time_ ){ /*--------------*/ coNext; }} while(0);
#define coErrno(DELAY,STATE,OUT) do { coSet(STATE);  nodepp ::coroutine::getno( OUT,coGet,DELAY ); return OUT; case STATE:; } while(0);

#define coGoto(VALUE)  do { coSet( VALUE ); nodepp::coroutine::getno(1,coGet); return 1; } while(0);
#define coStay(VALUE)  do { coSet( VALUE ); nodepp::coroutine::getno(0,coGet); return 0; } while(0);
#define coNext         do { coErrno(0UL, __LINE__ ,1); /*-----------------------------*/ } while(0);
#define coYield(VALUE) do { coErrno(0UL, VALUE,1); /*---------------------------------*/ } while(0);
#define coWait(VALUE)  do { while( VALUE ){ /*-------------------------------*/ coNext; }} while(0);
#define coEnd          do { _time_=0; _state_=_time_; /**/ nodepp::coroutine::getno(-1); } while(0); return -1;
#define coStop            } _time_=0; _state_=_time_; /**/ nodepp::coroutine::getno(-1); } while(0); return -1;

#define coStart thread_local static int _state_=0; thread_local static ulong _time_=0; coBegin
#define coBegin do { switch(_state_) { case 0:; nodepp::coroutine::getno(-2);
#define coEmit  int operator()

#define coSet(VALUE) _state_ = VALUE
#define coGet        _state_
#define coFinish     coStop

/*────────────────────────────────────────────────────────────────────────────*/

#define onMain NODEPP_BEGIN(); int main( int argc, char** args ) { \
   nodepp::process::start(argc,args); NODEPP_BEGIN ();   \
   nodepp::process::wait (); /*----*/ NODEPP_SHTDWN()=1; \
return 0; } void NODEPP_BEGIN

/*────────────────────────────────────────────────────────────────────────────*/

#define COROUTINE_ARG(... ) [=]( int& _state_, ulong& _time_, __VA_ARGS__ ) -> int
#define COROUTINE()         [=]( int& _state_, ulong& _time_ ) /*--------*/ -> int
#define GENERATOR( NAME )   struct NAME : public nodepp::generator_t

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

#define NODEPP_STRINGIFY(...) #__VA_ARGS__

/*────────────────────────────────────────────────────────────────────────────*/

#define forEach( X, ITEM ) for( auto& X : ITEM )
#define forEver() for (;;)
#define elif else if

/*────────────────────────────────────────────────────────────────────────────*/

#define NODEPP_CHUNK_SIZE       CHUNK_KB(64)
#define NODEPP_MAX_PATH_SIZE    CHUNK_KB(1)
#define NODEPP_MAX_SSO_SIZE     16
#define NODEPP_UNBFF_SIZE       CHUNK_KB(4)
#define NODEPP_MAX_SOCKET       nodepp::os::get_soft_fileno()
#define NODEPP_ARCH_SIZE        sizeof(void*)*8
#define NODEPP_MAX_BATCH_SIZE   16
#define NODEPP_HASH_TABLE_SIZE  CHUNK_KB(16)
#define NODEPP_HEAP_SIZE        CHUNK_KB(64)

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

#define NODEPP_OS_WINDOWS 9
#define NODEPP_OS_ANDROID 8
#define NODEPP_OS_BROWSER 7
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

#elif defined(__EMSCRIPTEN__)
   #define _OS_ NODEPP_OS_BROWSER

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

   #if defined(__x86_64__) || defined(__ppc64__) || defined(__amd64__) || defined(__LP64__)
      #define _ARCH_ NODEPP_ARCH_CPU_64
   #elif defined(__aarch64__)
      #define _ARCH_ NODEPP_ARCH_ARM_64
   #elif defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
      #define _ARCH_ NODEPP_ARCH_CPU_32
   #elif defined(__arm__)
      #define _ARCH_ NODEPP_ARCH_ARM_32
   #elif defined(__riscv)
      #if __riscv_xlen == 64
         #define _ARCH_ NODEPP_ARCH_RISCV_64
      #else
         #define _ARCH_ NODEPP_ARCH_RISCV_32
      #endif
   #elif defined(__xtensa__)
      #define _ARCH_ NODEPP_ARCH_XTENSA
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

#else
   #define _ARCH_ NODEPP_ARCH_UNKNOWN
#endif
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#define typecast(DATA) nodepp::string_t( typeid(DATA).name() )
namespace nodepp {

using ullong  = unsigned long long int;
using ulong   = unsigned long int;

using llong   = /*----*/ long long int;
using ldouble = /*----*/ long double;
using uchar   = unsigned char   ;
using wchar   = /*----*/ wchar_t;

using int_8   = /*----*/ char   ;
using int_16  = /*----*/ int16_t;
using int_32  = /*----*/ int32_t;
using int_64  = /*----*/ int64_t;

using uint_8  = unsigned char    ;
using uint_16 = /*----*/ uint16_t;
using uint_32 = /*----*/ uint32_t;
using uint_64 = /*----*/ uint64_t;

using char_8  = /*----*/ char   ;
using char_16 = /*----*/ int16_t;
using char_32 = /*----*/ int32_t;
using char_64 = /*----*/ int64_t;

using uchar_8 = unsigned char    ;
using uchar_16= /*----*/ uint16_t;
using uchar_32= /*----*/ uint32_t;
using uchar_64= /*----*/ uint64_t;

#if !defined(_SYS_TYPES_H_) || _OS_ == NODEPP_OS_ANDROID
    #define  _SYS_TYPES_H_

using ushort  = unsigned short;
using uint    = unsigned int;

#endif

}

/*────────────────────────────────────────────────────────────────────────────*/

#if _OS_ == NODEPP_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN 
#define sscanff( BUFFER, FORMAT, ... ) sscanf_s( BUFFER, FORMAT, __VA_ARGS__ )
#else
#define sscanff( BUFFER, FORMAT, ... ) sscanf  ( BUFFER, FORMAT, __VA_ARGS__ )
#endif

/*────────────────────────────────────────────────────────────────────────────*/

#endif
