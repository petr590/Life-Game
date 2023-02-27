#!/bin/sh
g++ -std=c++17 -O3 main.cpp src/*.cpp -Iinclude/ -I/usr/include/SFML -L/usr/lib -lsfml-graphics -lsfml-window -lsfml-system -lpthread -o life-game
