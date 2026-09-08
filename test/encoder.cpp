#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/encoder.h>

using namespace nodepp;

namespace TEST { namespace ENCODER {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | encoder ofuscator", [](){

            auto clb = function_t<string_t>( [=](){ return "hello world!"; } );
            auto adr = &clb; auto sign = (uchar_64) rand();

            auto inp = encoder::ofuscator::atob( (void*) adr, sign );
            auto out = encoder::ofuscator::btoa( /*---*/ inp, sign );
            auto nlb = (function_t<string_t>*) out;

            if( out == nullptr ) /*------*/ { TEST_FAIL(); }
            if( (*nlb)() != "hello world!" ){ TEST_FAIL(); }

            TEST_DONE();

        });

        TEST_ADD( test, "TEST 2 | encoder XOR", [](){

            auto x = string_t( "hello world!" );
            auto a = encoder::XOR::atob( x, "secret" );
            auto b = encoder::XOR::btoa( a, "secret" );

            if( x != b ){ TEST_FAIL(); } TEST_DONE();

        });

        TEST_ADD( test, "TEST 3 | encoder bytes", [](){

            auto x = (char) 0xFF;
            auto a = encoder::bytes::atob      ( x );
            auto b = encoder::bytes::btoa<char>( a );

            if( x != b ){ TEST_FAIL(); } TEST_DONE();

        });

        TEST_ADD( test, "TEST 4 | encoder binary", [](){

            auto x = (char) 0xFF;
            auto a = encoder::bin::atob      ( x );
            auto b = encoder::bin::btoa<char>( a );

            if( x != b ){ TEST_FAIL(); } TEST_DONE();

        });

        TEST_ADD( test, "TEST 5 | encoder hex", [](){

            auto x = (char) 0xFF;
            auto a = encoder::hex::atob      ( x );
            auto b = encoder::hex::btoa<char>( a );

            if( a != "ff" ){ TEST_FAIL(); }
            if( x != b    ){ TEST_FAIL(); } TEST_DONE();

        });

        TEST_ADD( test, "TEST 6 | encoder hex 2", [](){

            auto x = ptr_t<uchar>({ 0xFF });
            auto a = encoder::hex::atob( x );
            auto b = encoder::hex::btoa( a );

            if( a != "ff" ) /*-----------------*/ { TEST_FAIL(); }
            if( memcmp( x.get(), b.get(), 1 )!=0 ){ TEST_FAIL(); } TEST_DONE();

        });

        TEST_ADD( test, "TEST 7 | base16", [](){

            auto x = string_t( "hello world!" );
            auto a = encoder::base16::atob( x );
            auto b = encoder::base16::btoa( a );

            if( x != b ){ TEST_FAIL(); } TEST_DONE();

        });

        TEST_ADD( test, "TEST 8 | base58", [](){

            auto x = string_t( "hello world!" );
            auto a = encoder::base58::atob( x );
            auto b = encoder::base58::btoa( a );

            if( x != b ){ TEST_FAIL(); } TEST_DONE();

        });

        TEST_ADD( test, "TEST 8 | base64", [](){

            auto x = string_t( "hello world!" );
            auto a = encoder::base16::atob( x );
            auto b = encoder::base16::btoa( a );

            if( x != b ){ TEST_FAIL(); } TEST_DONE();

        });

        test.onClose.once([=](){
            console::log("\nRESULT | total:", *totl, "| passed:", *done, "| error:", *err, "| skipped:", *skp );
        });

        test.onDone([=](){ (*done)++; (*totl)++; });
        test.onFail([=](){ (*err) ++; (*totl)++; });
        test.onSkip([=](){ (*skp) ++; (*totl)++; });

        TEST_AWAIT( test );

    }

}}