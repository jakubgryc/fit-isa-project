CXX = g++
CXXFLAGS = -std=gnu++17 -Wall -Wextra -pedantic -g

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

TARGET = p2nprobe

all: $(TARGET)

# Rule to link the object files and create the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Rule to compile each source file into an object file in obj directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to create the object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up object files and executable
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)


.PHONY: all clean
