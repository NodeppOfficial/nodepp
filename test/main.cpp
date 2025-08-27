#include <nodepp/nodepp.h>
#include <nodepp/test.h>

using namespace nodepp;

#include "url.cpp"
#include "any.cpp"
#include "ptr.cpp"
#include "map.cpp"
#include "json.cpp"
#include "task.cpp"
#include "path.cpp"
#include "loop.cpp"
#include "wait.cpp"
#include "http.cpp"
#include "tuple.cpp"
#include "worker.cpp"
#include "cookie.cpp"
#include "event.cpp"
#include "query.cpp"
#include "array.cpp"
#include "queue.cpp"
#include "regex.cpp"
#include "string.cpp"
#include "variant.cpp"
#include "promise.cpp"
#include "function.cpp"
#include "optional.cpp"
#include "expected.cpp"

void onMain(){ 
    
    TEST::URL     ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::ANY     ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::PTR     ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::MAP     ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::TASK    ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::PATH    ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::LOOP    ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::WAIT    ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::HTTP    ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::TUPLE   ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::COOKIE  ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::EVENT   ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::QUERY   ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::JSON    ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::ARRAY   ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::WORKER  ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::QUEUE   ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::REGEX   ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::STRING  ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::VARIANT ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::PROMISE ::TEST_RUNNER(); conio::log("\n---\n");
    TEST::FUNCTION::TEST_RUNNER(); conio::log("\n---\n");
    TEST::EXPECTED::TEST_RUNNER(); conio::log("\n---\n");
    TEST::OPTION  ::TEST_RUNNER(); conio::log("\n---\n");

}