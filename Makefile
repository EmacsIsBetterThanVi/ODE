CFILES = $(wildcard *.cpp)
OFILES = $(CFILES:.cpp=.o)

all: ode

%.cpp: %.h

%.o: %.cpp
	g++ -c $< -o $@

ode: $(OFILES)
	g++ -o ode.bin *.o

install-files:
	cp -fR ./ode ~/.ode

install: install-files ode
	cp ode.bin /bin/ode

clean:
	rm -f *.o ode.bin
