#time x86_64-w64-mingw32-g++ main.cpp -D_WIN32_WINNT=0x0601 -o main.exe -I./include -lkernel32 -static-libgcc -static-libstdc++ -static -lws2_32 -g ; wine main.exe
time g++ -o main main.cpp -I./include -lssl -lcrypto -lz -std=c++11; ./main

#wine debuggin mode
#server: winedbg --gdb --port 12345 --no-start ./main.exe 
#client: target extended-remote :12345