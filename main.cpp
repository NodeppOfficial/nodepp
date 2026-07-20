#include <nodepp/nodepp.h>
#include <nodepp/worker.h>
#include <nodepp/timer.h>
#include <nodepp/tcp.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain() {

    uchar_64 x = 0xf000000000000000;
    console::log( (int) x );

}