#include <nodepp/nodepp.h>
#include <nodepp/dns.h>
#include <nodepp/test.h>

using namespace nodepp;

namespace TEST { namespace DNS {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | dns -> IPv6", [](){
            try {
                string_t data = "2001:0DB8:85A3:0000:0000:8A2E:0370:7334";
                if( dns::is_ipv6(data) ){ TEST_DONE(); }
                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | dns -> IPv4", [](){
            try {
                string_t data = "192.168.0.1";
                if( dns::is_ipv4(data) ){ TEST_DONE(); }
                              TEST_FAIL();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | dns -> IPv6 fail test", [](){
            try {
                string_t data = "mojon";
                if( dns::is_ipv6(data) ){ TEST_FAIL(); }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | dns -> IPv4 fail test", [](){
            try {
                string_t data = "mojon";
                if( dns::is_ipv4(data) ){ TEST_FAIL(); }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        test.onClose.once([=](){
            console::log("\nRESULT | total:", *totl, "| passed:", *done, "| error:", *err, "| skipped:", *skp );
        });

        test.onDone([=](){ (*done)++; (*totl)++; });
        test.onFail([=](){ (*err)++;  (*totl)++; });
        test.onSkip([=](){ (*skp)++;  (*totl)++; });

        TEST_AWAIT( test );

    }

}}