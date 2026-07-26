#include <nodepp/nodepp.h>
#include <nodepp/test.h>
#include <nodepp/json.h>

using namespace nodepp;

namespace TEST { namespace JSON {

    void TEST_RUNNER(){
        ptr_t<uint> totl ( 0UL );
        ptr_t<uint> done ( 0UL );
        ptr_t<uint> err  ( 0UL );
        ptr_t<uint> skp  ( 0UL );

        auto test = TEST_CREATE();

        TEST_ADD( test, "TEST 1 | json initialization 1", [](){
            do {

                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
            if ( json.size() != 3 )/*--------*/{ break; }
            if ( json["var1"].as<int>() != 10 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 2 | json initialization 2", [](){
            do {

                object_t json = nodepp::json::parse( R"({
                    "var1": 10, "var2": 20,
                    "var3": 30,
                })" );
            if ( json.size() != 3 )/*--------*/{ break; }
            if ( json["var1"].as<int>() != 10 ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 3 | json searching", [](){
            do {

                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });
            if ( !json.has("var1") ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 4 | json indexation", [](){
            do {

                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });json["var4"] = 40;
             if ( !json.has("var4") ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 5 | json deindexation", [](){
            do {

                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });json.erase("var2");
            if ( json.has("var2") ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 6 | json stringify", [](){
            do {

                auto str = json::stringify( object_t({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                }) );
            if ( str.empty() )/*--------------------------*/{ break; }
            if ( str!=R"({"var1":10,"var2":20,"var3":30})" ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
        });

        TEST_ADD( test, "TEST 7 | json clearing", [](){
            do {

                object_t json ({
                    { "var1", 10 },
                    { "var2", 20 },
                    { "var3", 30 },
                });json.clear();
            if ( !json.empty() ){ break; }

                        TEST_DONE();
            } while(0); TEST_FAIL();
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