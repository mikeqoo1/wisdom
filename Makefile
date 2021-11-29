vpath %.hpp inc  #vpath 指定搜尋路徑

CC=g++

C_FLAGS = -pthread -Wall

INC=-I./inc

SRC=./src/

MAIN_OUTPUT=server.out


make: server client

server:
	$(CC) $(C_FLAGS) $(INC) -o $(MAIN_OUTPUT) $(SRC)main.cpp $(SRC)server.cpp
client:
	$(CC) $(C_FLAGS) $(SRC)client.cpp -o client.out

cl:
	rm *.out