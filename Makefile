CXX ?= g++
SRC_DIR := src
OBJ_DIR := build/obj
CXXFLAGS ?= -I$(SRC_DIR) -std=gnu++98 -fpermissive

BINS := draw_pb_sphere drawpullback find_matings mate_interact
COMMON_OBJS := $(OBJ_DIR)/medusa_A.o $(OBJ_DIR)/medusa_B.o

.PHONY: all clean test

all: $(BINS)

draw_pb_sphere: $(SRC_DIR)/draw_pb_sphere.cc $(SRC_DIR)/cktypes.cc $(SRC_DIR)/cktypes.h $(SRC_DIR)/psoop.cc $(SRC_DIR)/psoop.h $(SRC_DIR)/dynarray.C $(SRC_DIR)/dynarray.h $(SRC_DIR)/CP1.h
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_DIR)/draw_pb_sphere.cc

drawpullback: $(SRC_DIR)/drawpullback.cc $(SRC_DIR)/cktypes.cc $(SRC_DIR)/cktypes.h $(SRC_DIR)/psoop.cc $(SRC_DIR)/psoop.h $(SRC_DIR)/dynarray.C $(SRC_DIR)/dynarray.h
	$(CXX) $(CXXFLAGS) -o $@ $(SRC_DIR)/drawpullback.cc

find_matings: $(OBJ_DIR)/find_matings.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

mate_interact: $(OBJ_DIR)/mate_interact.o $(COMMON_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/find_matings.o: $(SRC_DIR)/find_matings.cc $(SRC_DIR)/medusa_B.h $(SRC_DIR)/medusa_A.h $(SRC_DIR)/dynarray.C $(SRC_DIR)/dynarray.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC_DIR)/find_matings.cc

$(OBJ_DIR)/mate_interact.o: $(SRC_DIR)/mate_interact.cc $(SRC_DIR)/medusa_B.h $(SRC_DIR)/medusa_A.h $(SRC_DIR)/dynarray.C $(SRC_DIR)/dynarray.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC_DIR)/mate_interact.cc

$(OBJ_DIR)/medusa_A.o: $(SRC_DIR)/medusa_A.cc $(SRC_DIR)/medusa_A.h $(SRC_DIR)/dynarray.C $(SRC_DIR)/dynarray.h | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC_DIR)/medusa_A.cc

$(OBJ_DIR)/medusa_B.o: $(SRC_DIR)/medusa_B.h $(SRC_DIR)/medusa_B.cc $(SRC_DIR)/medusa_A.h $(SRC_DIR)/dynarray.h $(SRC_DIR)/dynarray.C | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC_DIR)/medusa_B.cc

test: all
	./generate_graphics.sh

clean:
	rm -rf $(OBJ_DIR)
	rm -f *.o $(BINS)
