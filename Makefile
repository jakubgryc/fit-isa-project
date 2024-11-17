CXX = g++
CXXFLAGS = -std=gnu++17 -Wall -Wextra -pedantic -g -lpcap

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

TARGET = p2nprobe



all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up 
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET) xgrycj03.tar

test:
	python tests/udp_server.py

zip: clean
	zip -r isa.zip Makefile include obj src tests

rsync: clean
	rsync -aurv Makefile include obj src tests merlin:~/isa

pack: clean
	tar -cf xgrycj03.tar Makefile include src manual.pdf

docs:
	latex $(NAME).tex
	bibtex $(NAME)
	latex $(NAME).tex
	latex $(NAME).tex
	dvips -t a4 $(NAME).dvi
	ps2pdf -sPAPERSIZE=a4 $(NAME).ps
	rm -f $(NAME).{bbl,blg,brf,ps,log,out,aux,dvi}



.PHONY: all clean docs pack
