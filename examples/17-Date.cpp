#include <nodepp/nodepp.h>
#include <nodepp/date.h>

using namespace nodepp;

void onMain(){

    console::log( "now->", date::now() );

    console::log(" -- -- ");

    console::log( "day->", date::day() );

    console::log( "year->", date::year() );

    console::log( "month->", date::month() );

    console::log(" -- -- ");

    console::log( "hours->", date::hour() );

    console::log( "minutes->", date::minute() );

    console::log( "seconds->", date::second() );

}
