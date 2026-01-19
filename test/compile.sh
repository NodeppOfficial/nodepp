mkdir -p build

if [ ! -d "./build/_deps" ]; then
   ( cd build ; cmake .. )
fi

( cd build ; make )

if [ ! $? -eq 0 ]; then
    echo "exit error"; exit;
fi

./build/main