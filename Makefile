all: draw_pb_sphere drawpullback find_matings mate_interact
	

draw_pb_sphere: draw_pb_sphere.cc cktypes.cc cktypes.h psoop.cc psoop.h dynarray.C dynarray.h CP1.h
	g++ -o draw_pb_sphere draw_pb_sphere.cc

drawpullback: drawpullback.cc cktypes.cc cktypes.h psoop.cc psoop.h dynarray.C dynarray.h
	g++ -o drawpullback drawpullback.cc

find_matings.o: find_matings.cc medusa_B.h medusa_A.h dynarray.C dynarray.h
	g++ -c find_matings.cc -fhandle-exceptions

find_matings: find_matings.o medusa_B.o medusa_A.o
	g++ -o find_matings find_matings.o medusa_B.o medusa_A.o

mate_interact.o: mate_interact.cc medusa_B.h medusa_A.h dynarray.C dynarray.h
	g++ -c mate_interact.cc -fhandle-exceptions

mate_interact: mate_interact.o medusa_B.o medusa_A.o
	g++ -o mate_interact mate_interact.o medusa_B.o medusa_A.o

cktypes.o: cktypes.cc cktypes.h
	g++ -c cktypes.cc

psoop.o: psoop.cc psoop.h cktypes.h
	g++ -c psoop.cc

dynarray.o: dynarray.C dynarray.h
	g++ -c dynarray.C

medusa_A.o: medusa_A.cc medusa_A.h dynarray.C dynarray.h
	g++ -c medusa_A.cc -fhandle-exceptions

medusa_B.o: medusa_B.h medusa_B.cc medusa_A.h dynarray.h dynarray.C
	g++ -c medusa_B.cc -fhandle-exceptions

clean:
	rm -f *.o draw_pb_sphere drawpullback find_matings mate_interact
