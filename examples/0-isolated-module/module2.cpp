#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/channel.h>

using namespace nodepp;

#include "module2.h"

pair_t<worker_t,channel_t<coroutine_t>> module_2_start(){

     channel_t<coroutine_t> chn;

     return { worker::add( coroutine::add( COROUTINE(){
     coBegin

          while( true ){
          if   ( !chn.empty() ){
          for  ( auto &x: chn.read() ){ process::add( x ); }}
          if   ( process::size() == 0 ){ coDelay(1000); } process::next(); }

     coFinish
     }) ), chn };
     
}