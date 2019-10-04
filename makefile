all: myclient

myclient: myprogram.cpp
	g++ myprogram.cpp -o my
