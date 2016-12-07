.PHONY: all

all:
	g++ -Wall -Wextra -Wno-deprecated-declarations -std=c++11 -O3 -shared -fPIC -o mhook.so malloc_hooks.cc
