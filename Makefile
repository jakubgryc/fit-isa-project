CXX = g++
CXXFLAGS = -std=gnu++17 -Wall -Wextra -pedantic -g -lpcap

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

NAME = docs/doc

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
	rm -f $(OBJ_DIR)/*.o $(TARGET) isa.zip

test:
	python tests/udp_server.py

zip: clean
	zip -r isa.zip Makefile include obj src tests

rsync: clean
	rsync -aurv Makefile include obj src tests merlin:~/isa

docs:
	latex $(NAME).tex
	bibtex $(NAME)
	latex $(NAME).tex
	latex $(NAME).tex
	dvips -t a4 $(NAME).dvi
	ps2pdf -sPAPERSIZE=a4 $(NAME).ps
	rm -f $(NAME).{bbl,blg,brf,ps,log,out,aux,dvi}



.PHONY: all clean docs
