/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_H_SIGNAL
#define NODEPP_H_SIGNAL

/*────────────────────────────────────────────────────────────────────────────*/

#include <csignal>
#include "event.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp  { 

    struct signal_t {
           event_t<int> onSIGERROR; 
           event_t<>    onSIGCLOSE;
           event_t<>    onSIGEXIT ; 
    };
    
namespace process { 
   inline signal_t& NODEPP_SIGNAL(){ thread_local static signal_t out; return out; }
namespace signal  {

    inline void start() {
        ::signal( SIGFPE,  []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGFPE ); conio::error("SIGFPE: ");  console::log("Floating Point Exception"); NODEPP_SIGNAL().onSIGEXIT .emit(); });
        ::signal( SIGSEGV, []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGSEGV); conio::error("SIGSEGV: "); console::log("Segmentation Violation");   NODEPP_SIGNAL().onSIGEXIT .emit(); });
        ::signal( SIGILL,  []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGILL ); conio::error("SIGILL: ");  console::log("Illegal Instruction");      NODEPP_SIGNAL().onSIGEXIT .emit(); });
        ::signal( SIGTERM, []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGTERM); conio::error("SIGTERM: "); console::log("Process Terminated");       NODEPP_SIGNAL().onSIGEXIT .emit(); });
        ::signal( SIGINT,  []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGINT ); conio::error("SIGINT: ");  console::log("Signal Interrupt");         NODEPP_SIGNAL().onSIGEXIT .emit(); });
    #ifdef SIGPIPE
        ::signal( SIGPIPE, []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGPIPE); conio::error("SIGPIPE: "); console::log("Broked Pipeline");          NODEPP_SIGNAL().onSIGEXIT .emit(); });
        ::signal( SIGKILL, []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGKILL); conio::error("SIGKILL: "); console::log("Process Killed");           NODEPP_SIGNAL().onSIGEXIT .emit(); });
    #endif
    //  ::signal( SIGPIPE, []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGPIPE); conio::error("SIGPIPE: "); console::log("Broken Pipe");              NODEPP_SIGNAL().onSIGEXIT .emit(); }); for a reason is commented
        ::signal( SIGABRT, []( int param ){ NODEPP_SIGNAL().onSIGERROR.emit(SIGABRT); conio::error("SIGABRT: "); console::log("Process Abort");            NODEPP_SIGNAL().onSIGEXIT .emit(); });
    //  ::atexit( /*----*/ []( /*-----*/ ){ /*----------------------------------------------------------------------------------------------------------*/ NODEPP_SIGNAL().onSIGCLOSE.emit(); }); for a reason is commented
    #ifdef SIGPIPE
        ::signal( SIGPIPE, SIG_IGN );
    #endif
    }

    inline void unignore( int signal ){ ::signal( signal, SIG_DFL ); }
    inline void   ignore( int signal ){ ::signal( signal, SIG_IGN ); }
	inline void     emit( int signal ){ ::raise ( signal  /*---*/ ); }

}}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
