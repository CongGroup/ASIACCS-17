CC=g++
BIN=Test_latency Test_Throughput Test_insert Test_basic
#TestAll
FLAGS=-Wall -g -fpermissive -std=c++0x -maes -O3
RM=-rm -f
SRC=$(wildcard *.cpp)
#OBJ=$(SRC:*.cpp=*.o)
OBJ=$(patsubst %.cpp, %.o, $(SRC))
COMMON_OBJ=$(filter-out Test_insert.o Test_Throughput.o Test_basic.o Test_latency.o,$(OBJ))

#thrift
THRIFT_LINK=-lthrift
THRIFT_LIB=-L/usr/local/lib
THRIFT_INC=-I/usr/local/inc

#openssl
OPENSSL_LINK=-lcrypto

#caravel
CARAVEL_LINK=-lcaravel
CARAVEL_LIB=-L../Caravel
CARAVEL_INC=-I../Caravel

#redis3m
REDIS3M_LINK=-lredis3m 
REDIS3M_LIB=-L/usr/local
REDIS3M_INC=-I/usr/local/include


#fastore
FASTORE_LINK=-lfastore
FASTORE_LIB=-L../fastore
FASTORE_INC=-I../fastore

#combine the inc & lib
INC=$(OPENSSL_INC) $(THRIFT_INC) $(CARAVEL_INC) $(PAILLIER_INC) $(REDIS3M_INC) $(FASTORE_INC)
LIB=$(FASTORE_LINK) $(FASTORE_LIB) $(CARAVEL_LINK) $(CARAVEL_LIB) $(THRIFT_LINK) $(THRIFT_LIB) $(OPENSSL_LINK) $(REDIS3M_LINK) 


all: $(BIN)

%.d: %.cpp
	$(CC) -MM $(FLAGS) $(INC) $< -o $@

%.o: %.cpp %.d
	$(CC) -c $(FLAGS) $(INC) $< -o $@

Test_Throughput:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Test_latency:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

Test_insert:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)
    
Test_basic:$(OBJ)
	$(CC) $(FLAGS) -o $@ $@.o $(COMMON_OBJ) $(LIB)

clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) $(BIN)


-include $(SRC:.cpp=.d)


