CC=g++
GCC=gcc
DEST_LIB=libfastore.a
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
INC=-I/usr/local/include

FLAGS=-Wall -fpermissive -g -march=native -O3
RM=-rm -f

.PHONY: all clean

all: $(DEST_LIB)

-include $(SRC:.cpp=.d)

%.d: %.c
	$(CC) -MM $(FLAGS) $(INC) $< -o $@

%.o: %.c %.d
	$(CC) -c $(FLAGS) $(INC) $< -o $@

%.d: %.cpp
	$(CC) -MM $(FLAGS) $(INC) $< -o $@

%.o: %.cpp %.d
	$(CC) -c $(FLAGS) $(INC) $< -o $@

$(DEST_LIB): $(OBJ)
	ar -crs $(DEST_LIB) $^
	
clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) $(DEST_LIB)

