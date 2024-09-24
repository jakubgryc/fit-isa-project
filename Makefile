# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=gnu++17 -Wall -Wextra -pedantic

# Target executable
TARGET = p2nprobe

# Source files
SRCS = main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default rule to build the program
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .cpp files into .o object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove the built files
clean:
	rm -f $(OBJS) $(TARGET)

# PHONY targets
.PHONY: all clean
