CFILES = $(wildcard *.cpp)
OFILES = $(CFILES:.cpp=.o)

all: ode

%.cpp: %.h

%.o: %.cpp
	g++ -c $< -o $@

ode: $(OFILES)
	g++ -o ode.bin *.o

clean:
	rm -f *.o ode.bin
