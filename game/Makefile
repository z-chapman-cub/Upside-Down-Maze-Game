CXX = g++ -fsanitize=address,undefined
#CXX = g++
CXXFLAGS = -std=c++17 -Wall -Werror -Wpedantic
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

.DEFAULT_GOAL := run

all: maze

maze: main.cpp Entity.o Board.o Game.o Render.o
	$(CXX) $(CXXFLAGS) main.cpp Entity.o Board.o Game.o Render.o $(LDFLAGS) -o maze

Entity.o: Entity.cpp
	$(CXX) $(CXXFLAGS) -c Entity.cpp

Game.o: Game.cpp
	$(CXX) $(CXXFLAGS) -c Game.cpp

Board.o: Board.cpp
	$(CXX) $(CXXFLAGS) -c Board.cpp

Render.o: Render.cpp
	$(CXX) $(CXXFLAGS) -c Render.cpp

run: maze
	./maze

clean:
	rm -f *.o maze
