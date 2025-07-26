#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/json.h>

using namespace nodepp;

namespace TEST { namespace JSON {

    void TEST_RUNNER(){
        ptr_t<uint> totl = new uint(0);
        ptr_t<uint> done = new uint(0);
        ptr_t<uint> err  = new uint(0);
        ptr_t<uint> skp  = new uint(0);

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | json initialization 1", [](){
            try {
                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
             if ( json.size() != 3 )/*--------*/{ throw 0; }
             if ( json["var1"].as<int>() != 10 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 2 | json initialization 2", [](){
            try {
                object_t json = nodepp::json::parse( R"({
                    "var1": 10, "var2": 20,
                    "var3": 30,
                })" );
             if ( json.size() != 3 )/*--------*/{ throw 0; }
             if ( json["var1"].as<int>() != 10 ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 3 | json searching", [](){
            try {
                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
             if ( !json.has("var1") ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 4 | json indexation", [](){
            try {
                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });json["var4"] = 40;
             if ( !json.has("var4") ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 5 | json deindexation", [](){
            try {
                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });json.erase("var2");
             if ( json.has("var2") ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 6 | json stringify", [](){
            try {
                auto str = json::stringify( object_t({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                }) );
             if ( str.empty() )/*--------------------------*/{ throw 0; }
             if ( str!=R"({"var1":10,"var2":20,"var3":30})" ){ throw 0; }
                              TEST_DONE();
            } catch ( ... ) { TEST_FAIL(); }
        });

        TEST_ADD( test, "TEST 7 | json clearing", [](){
            try {
                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });json.clear();
             if ( !json.empty() ){ throw 0; }
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

// void onMain(){ TEST::CONSOLE::TEST_RUNNER(); }