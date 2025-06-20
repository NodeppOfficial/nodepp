#include <nodepp/nodepp.h>
#include <nodepp/date.h>

using namespace nodepp;

void onMain() {

    auto time1 = date_t( 2025, 7,  9, false );
    auto time2 = date_t( 2025, 7, 19, false );
    auto time3 = date2 - date1;

    console::log( "days left:", time3.get_day() );

}