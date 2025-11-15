cls
g++ server.cpp -I"asio-1.18.0/include" -std=c++17 -lws2_32 -lmswsock -o server.exe
g++ client.cpp -I"asio-1.18.0/include" -std=c++17 -lws2_32 -lmswsock -o client.exe