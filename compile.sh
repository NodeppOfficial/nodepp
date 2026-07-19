#time m++ main.cpp -D_WIN32_WINNT=0x0601 -o main.exe -I./include -lkernel32 -static-libgcc -static-libstdc++ -static -lws2_32 ; wine main.exe

#wine debuggin mode
#server: winedbg --gdb --port 12345 --no-start ./main.exe 
#client: target extended-remote :12345

#time g++ -o main main.cpp -I./include -lssl -lcrypto -lz -std=c++20 -march=native -mtune=native -O3 ; ./main
time g++ -o main main.cpp -I./include -lssl -lcrypto -lz -std=c++20 ; ./main # -fsanitize=address -g