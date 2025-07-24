# For the following examples create the CMake rules

SET(EXAMPLES_LIST
    0-Test
    1-Hello-World
    2-Console
    3-Conio
    4-Garbage-Colector
    5-Array
    5-Tuple
    6-String
    7-Queue
    8-Env
    8-splice
    9-Regex
    10-Optional
    10-Regex
    11-Expected
    11-Task
    12-Coroutine
    13-Generator
    14-Event
    14-WaitEvent
    15-Observer
    16-Promises
    16-Timer
    17-Date
    17-Json
    18-Json_Stringify
    19-JsonParse
    20-File
    21-ReadStreams
    22-WriteStreams
    23-std_IO_stream
    23-ZlibFileStream
    25-Cookie
    26-Query
    27-URL
    28-Path
    29-HTTP-Client
    30-HTTPS-Client
    31-HTTP-Server
    32-HTTP-Server
    33-HTTPS-Server
    34-HTTPS-Server
    35-WS-Client
    36-WS-Server
    37-WSS-Client
    38-WSS-Server
    39-LoadBalancing
    39-WS-load-test
    40-Cluster
    40-Popen
    41-Crypto
    42-CryptoDH
    43-CryptoDSA
    44-CryptoRSA
    44-CryptoXOR
    45-TCPClient
    46-TCPServer
    47-UDPClient
    48-UDPServer
    49-TLSClient
    50-TLSServer
    51-Worker
    52-Semaphore
    53-encoder-utf
    54-encoder-base64
    55-encoder-base16
    56-encoder-binary
    56-encoder-bytes
    56-encoder-hex
)

foreach(EXAMPLE ${EXAMPLES_LIST})
    message(STATUS "Adding example: ${EXAMPLE}")

    add_executable(${EXAMPLE} "${EXAMPLE}.cpp")
    target_include_directories(${EXAMPLE} PRIVATE "${NODEPP_INCLUDE_DIR}")
    target_link_libraries(${EXAMPLE} OpenSSL::Crypto OpenSSL::SSL)
endforeach()