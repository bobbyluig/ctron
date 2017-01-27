CC = g++
CFLAGS = -g -Wpedantic -std=c++11
TARGET = ctron
LIBS = -lncurses

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp $(LIBS)

clean:
	$(RM) $(TARGET)