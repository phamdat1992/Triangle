g++ -g main.cpp -o output `pkg-config --libs --cflags opencv`
g++ -g test.cpp -o test `pkg-config --libs --cflags opencv`