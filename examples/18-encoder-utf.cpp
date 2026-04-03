#include <nodepp/nodepp.h>
#include <nodepp/encoder.h>

using namespace nodepp;

void utf8() {

    ptr_t<uint8> inp ( "🤯🤬💩" );
    ptr_t<uint16>u16 ;
    ptr_t<uint32>u32 ;

    u16 = encoder::utf8::to_utf16( inp );
    u32 = encoder::utf8::to_utf32( inp );

    console::log( "<utf8>" , string_t( (char*) &inp ) );
    console::log( "<utf16>", array_t<uint16>(u16).join() );
    console::log( "<utf32>", array_t<uint32>(u32).join() );

}

void utf32() {

    ptr_t<uint32>inp({ 129327, 129324, 128169, 0x00 });
    ptr_t<uint8> ui8;
    ptr_t<uint16>u16;

    ui8 = encoder::utf32::to_utf8 ( inp );
    u16 = encoder::utf32::to_utf16( inp );

    console::log( "<utf8>" , string_t( (char*) &ui8 ) );
    console::log( "<utf16>", array_t<uint16>(u16).join() );
    console::log( "<utf32>", array_t<uint32>(inp).join() );

}

void utf16() {

    ptr_t<uint16>inp({ 55358, 56623, 55358, 56620, 55357, 56489, 0x00 }); 
    ptr_t<uint8> ui8;
    ptr_t<uint32>u32;

    ui8 = encoder::utf16::to_utf8 ( inp );
    u32 = encoder::utf16::to_utf32( inp );

    console::log( "<utf8>" , string_t( (char*) &ui8 ) );
    console::log( "<utf16>", array_t<uint16>(inp).join() );
    console::log( "<utf32>", array_t<uint32>(u32).join() );

}

void onMain(){
    
    utf8 (); console::log("---");
    utf16(); console::log("---");
    utf32();

}