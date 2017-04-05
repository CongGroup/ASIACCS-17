CC=g++
BIN=testMakefile_1 testMakefile_2 test_ore_blk
#TestAll
FLAGS=-Wall -g -fpermissive -std=c++0x -O3 -maes
RM=-rm -f
SRC=$(wildcard *.cpp)
#OBJ=$(SRC:*.cpp=*.o)
OBj=$(patsubst %.cpp, %.o, $(SRC))
COMMON_OBJ=$(filter-out testMakefile_1.cpp testMakefile_2.cpp,$(OBJ))

#thrift
THRIFT_LINK=-lthrift
THRIFT_LIB=-L/usr/local/lib

#openssl
OPENSSL_LINK=-lcrypto

#caravel
CARAVEL_LINK=-lcaravel
CARAVEL_LIB=-L../Caravel
CARAVEL_INC=-I../Caravel

#fastore
FASTORE_LINK=-lfastore
FASTORE_LIB=-L../fastore
FASTORE_INC=-I../fastore

#GMP
GMP_LINK=-lgmp

#paillier
PAILLIER_LINK=-lpaillier
PAILLIER_INC=-I/usr/local/include
PAILLIER_LIB=-L/usr/local/lib

#combine the inc & lib
INC=$(OPENSSL_INC) $(THRIFT_INC) $(CARAVEL_INC) $(PAILLIER_INC) $(FASTORE_INC) $(OPENSSL_INC)
LIB=$(CARAVEL_LINK) $(CARAVEL_LIB) $(THRIFT_LINK) $(FASTORE_LINK) $(FASTORE_LIB) $(THRIFT_LIB) $(OPENSSL_LINK)


all: $(BIN)

%.d: %.cpp
	$(CC) -MM $(FLAGS) $(INC) $< -o $@

%.o: %.cpp %.d
	$(CC) -c $(FLAGS) $(INC) $< -o $@

testMakefile_1:$(COMMON_OBJ)
	$(CC) $(FLAGS) -o $@ $@.cpp $(COMMON_OBJ) $(LIB)

testMakefile_2:$(COMMON_OBJ)
	$(CC) $(FLAGS) -o $@ $@.cpp $(COMMON_OBJ) $(LIB)

test_ore_blk:$(COMMON_OBJ)
	$(CC) $(FLAGS) -o $@ $@.cpp $(COMMON_OBJ) $(LIB) $(INC)


# CityUScore:$(COMMON_OBJ)
	# $(CC) $(FLAGS) -o $@ $@.cpp $(COMMON_OBJ) $(LIB)

# TestSystem:$(COMMON_OBJ)
# 	$(CC) $(FLAGS) -o $@ $@.cpp $(COMMON_OBJ) $(LIB)

#TestAll:$(COMMON_OBJ)
#	$(CC) $(FLAGS) -o $@ $@.cpp $(COMMON_OBJ) $(LIB) $(PAILLIER_LINK) $(PAILLIER_LIB) $(GMP_LINK)

# TestScale:$(COMMON_OBJ)
# 	$(CC) $(FLAGS) -o $@ $@.cpp $(COMMON_OBJ) $(LIB)

# TestSql:$(COMMON_OBJ)
# 	$(CC) $(FLAGS) -o $@ $@.cpp $(COMMON_OBJ) $(LIB)


clean:
	$(RM) *.o
	$(RM) *.d
	$(RM) $(BIN)

-include $(SRC:.cpp=.d)
