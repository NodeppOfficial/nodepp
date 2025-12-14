/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_SIGNAL
#define NODEPP_SIGNAL

/*────────────────────────────────────────────────────────────────────────────*/

#include <csignal>
#include "event.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace process {

    event_t<int> onSIGERROR; event_t<> onSIGEXIT;

    namespace signal {

        inline void start() {
            ::signal( SIGFPE,  []( int param ){ onSIGERROR.emit(SIGFPE ); conio::error("SIGFPE: ");  console::log("Floating Point Exception"); onSIGEXIT.emit(); });
            ::signal( SIGSEGV, []( int param ){ onSIGERROR.emit(SIGSEGV); conio::error("SIGSEGV: "); console::log("Segmentation Violation");   onSIGEXIT.emit(); });
            ::signal( SIGILL,  []( int param ){ onSIGERROR.emit(SIGILL ); conio::error("SIGILL: ");  console::log("Illegal Instruction");      onSIGEXIT.emit(); });
            ::signal( SIGTERM, []( int param ){ onSIGERROR.emit(SIGTERM); conio::error("SIGTERM: "); console::log("Process Terminated");       onSIGEXIT.emit(); });
            ::signal( SIGINT,  []( int param ){ onSIGERROR.emit(SIGINT ); conio::error("SIGINT: ");  console::log("Signal Interrupt");         onSIGEXIT.emit(); });
        #ifdef SIGPIPE
            ::signal( SIGPIPE, []( int param ){ onSIGERROR.emit(SIGPIPE); conio::error("SIGPIPE: "); console::log("Broked Pipeline");          onSIGEXIT.emit(); });
            ::signal( SIGKILL, []( int param ){ onSIGERROR.emit(SIGKILL); conio::error("SIGKILL: "); console::log("Process Killed");           onSIGEXIT.emit(); });
        #endif
            ::signal( SIGABRT, []( int param ){ onSIGERROR.emit(SIGABRT); conio::error("SIGABRT: "); console::log("Process Abort");            onSIGEXIT.emit(); });
        //  ::atexit( /*----*/ []( /*-----*/ ){ /*------------------------------------------------------------------------------------------*/ onSIGEXIT.emit(); });
        #ifdef SIGPIPE
            ::signal( SIGPIPE, SIG_IGN );
        #endif
        }

        inline void unignore( int signal ){ ::signal( signal, SIG_DFL ); }
        inline void   ignore( int signal ){ ::signal( signal, SIG_IGN ); }
	    inline void     emit( int signal ){ ::raise ( signal );          }

    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
