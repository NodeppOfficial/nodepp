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

    event_t<int> onSIGFPE;  //on Floating Point Exception
    event_t<int> onSIGSEGV; //on Segmentation Violation
    event_t<int> onSIGILL;  //on Illegal Instruction
    event_t<int> onSIGSINT; //on Signal Interrupt
#ifdef SIGPIPE
    event_t<int> onSIGPIPE; //on Broked Pipeline
    event_t<int> onSIGKILL; //on Kill
#endif
    event_t<int> onSIGTERM; //on Terminate
    event_t<int> onSIGABRT; //on Abort
    event_t<>    onSIGERR;  //on Error
    event_t<>    onSIGEXIT; //on Exit
    event_t<>    onSIGNEXT; //on Next

    namespace signal {

        void start() {
            ::signal( SIGFPE,  []( int param ){ _EXIT_=true; onSIGFPE .emit(param); onSIGERR.emit(); conio::error("SIGFPE: ");  console::log("Floating Point Exception"); ::exit( param ); });
            ::signal( SIGSEGV, []( int param ){ _EXIT_=true; onSIGSEGV.emit(param); onSIGERR.emit(); conio::error("SIGSEGV: "); console::log("Segmentation Violation");   ::exit( param ); });
            ::signal( SIGILL,  []( int param ){ _EXIT_=true; onSIGILL .emit(param); onSIGERR.emit(); conio::error("SIGILL: ");  console::log("Illegal Instruction");      ::exit( param ); });
            ::signal( SIGTERM, []( int param ){ _EXIT_=true; onSIGTERM.emit(param); onSIGERR.emit(); conio::error("SIGTERM: "); console::log("Process Terminated");       ::exit( param ); });
            ::signal( SIGINT,  []( int param ){ _EXIT_=true; onSIGSINT.emit(param); onSIGERR.emit(); conio::error("SIGINT: ");  console::log("Signal Interrupt");         ::exit( param ); });
    #ifdef SIGPIPE
            ::signal( SIGPIPE, []( int param ){ _EXIT_=true; onSIGPIPE.emit(param); onSIGERR.emit(); conio::error("SIGPIPE: "); console::log("Broked Pipeline");          ::exit( param ); });
            ::signal( SIGKILL, []( int param ){ _EXIT_=true; onSIGKILL.emit(param); onSIGERR.emit(); conio::error("SIGKILL: "); console::log("Process Killed");           ::exit( param ); });
    #endif
            ::signal( SIGABRT, []( int param ){ _EXIT_=true; onSIGABRT.emit(param); onSIGERR.emit(); conio::error("SIGABRT: "); console::log("Process Abort");            ::exit( param ); });
            ::atexit( /*    */ []( /*     */ ){ _EXIT_=true; onSIGEXIT.emit(     ); });
    #ifdef SIGPIPE
            ::signal( SIGPIPE, SIG_IGN );
    #endif
        }

        void unignore( int signal ){ ::signal( signal, SIG_DFL ); }
        void   ignore( int signal ){ ::signal( signal, SIG_IGN ); }
	    void     emit( int signal ){ ::raise ( signal );          }

    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
