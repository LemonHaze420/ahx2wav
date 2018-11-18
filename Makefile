CC = g++-7
CC_FLAGS = -g -std=c++17 -w -Wall -Wextra -pedantic-errors -m32

EXEC = ahx2wav
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
LDFLAGS = -lm -ldl -lstdc++fs -L. -I. -m32

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(EXEC)
 
%.o: %.cpp
	$(CC) $(LDFLAGS) -c $(CC_FLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)