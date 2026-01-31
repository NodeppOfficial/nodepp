/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_OSS
#define NODEPP_OSS

/*────────────────────────────────────────────────────────────────────────────*/

#if   _KERNEL_ == NODEPP_KERNEL_WINDOWS
    #include "windows/os.h"
#elif _KERNEL_ == NODEPP_KERNEL_POSIX
    #include "posix/os.h"
#else
    #error "This OS Does not support os.h"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace os {

    inline string_t get_arch() { switch( _OS_ ){

        case NODEPP_ARCH_RISCV_64: return "RISCV_64"; break;
        case NODEPP_ARCH_RISCV_32: return "RISCV_32"; break;
        case NODEPP_ARCH_XTENSA  : return "XTENSA"  ; break;
        case NODEPP_ARCH_CPU_64  : return "CPU_64"  ; break;
        case NODEPP_ARCH_CPU_32  : return "CPU_32"  ; break;
        case NODEPP_ARCH_ARM_64  : return "ARM_64"  ; break;
        case NODEPP_ARCH_ARM_32  : return "ARM_32"  ; break;
        default                  : return nullptr   ; break;

    }}

    inline string_t get_os() { switch( _OS_ ){

        case NODEPP_OS_WINDOWS: return "windows"; break;
        case NODEPP_OS_ANDROID: return "android"; break;
        case NODEPP_OS_TIZEN  : return "tizen"  ; break;
        case NODEPP_OS_APPLE  : return "mac"    ; break;
        case NODEPP_OS_IOS    : return "ios"    ; break;
        case NODEPP_OS_FRBSD  : return "bsd"    ; break;
        case NODEPP_OS_LINUX  : return "linux"  ; break;
        case NODEPP_OS_ARDUINO: return "arduino"; break;
        case NODEPP_OS_BROWSER: return "browser"; break;
        default               : return nullptr  ; break;

    }}


} }

/*────────────────────────────────────────────────────────────────────────────*/

#endif