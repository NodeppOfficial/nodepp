#time m++ main.cpp -D_WIN32_WINNT=0x0601 -o main.exe -I./include -lkernel32 -static-libgcc -static-libstdc++ -static -lws2_32 ; wine main.exe
#time g++ -o main main.cpp -I./include -lssl -lcrypto -lz -march=native -mtune=native -O3 ; ./main
time g++ -o main main.cpp -I./include -lssl -lcrypto -lz ; ./main # -fsanitize=address -g