CXX ?= g++
CXXFLAGS ?= -I. -std=gnu++98 -fpermissive

all: draw_pb_sphere drawpullback find_matings mate_interact


draw_pb_sphere: draw_pb_sphere.cc cktypes.cc cktypes.h psoop.cc psoop.h dynarray.C dynarray.h CP1.h
	$(CXX) $(CXXFLAGS) -o draw_pb_sphere draw_pb_sphere.cc

drawpullback: drawpullback.cc cktypes.cc cktypes.h psoop.cc psoop.h dynarray.C dynarray.h
	$(CXX) $(CXXFLAGS) -o drawpullback drawpullback.cc

find_matings.o: find_matings.cc medusa_B.h medusa_A.h dynarray.C dynarray.h
	$(CXX) $(CXXFLAGS) -c find_matings.cc

find_matings: find_matings.o medusa_B.o medusa_A.o
	$(CXX) $(CXXFLAGS) -o find_matings find_matings.o medusa_B.o medusa_A.o

mate_interact.o: mate_interact.cc medusa_B.h medusa_A.h dynarray.C dynarray.h
	$(CXX) $(CXXFLAGS) -c mate_interact.cc

mate_interact: mate_interact.o medusa_B.o medusa_A.o
	$(CXX) $(CXXFLAGS) -o mate_interact mate_interact.o medusa_B.o medusa_A.o

cktypes.o: cktypes.cc cktypes.h
	$(CXX) $(CXXFLAGS) -c cktypes.cc

psoop.o: psoop.cc psoop.h cktypes.h
	$(CXX) $(CXXFLAGS) -c psoop.cc

dynarray.o: dynarray.C dynarray.h
	$(CXX) $(CXXFLAGS) -c dynarray.C

medusa_A.o: medusa_A.cc medusa_A.h dynarray.C dynarray.h
	$(CXX) $(CXXFLAGS) -c medusa_A.cc

medusa_B.o: medusa_B.h medusa_B.cc medusa_A.h dynarray.h dynarray.C
	$(CXX) $(CXXFLAGS) -c medusa_B.cc

clean:
	rm -f *.o draw_pb_sphere drawpullback find_matings mate_interact
