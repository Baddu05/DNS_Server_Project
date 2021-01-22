              ##################################################
              # 					       #
              #  	Make file for DNS RESOLVER	       #
              #    					       #
              ##################################################

CC= gcc -ftest-coverage -fprofile-arcs
CFLAGS = -c -Wall -g -ftest-coverage -fprofile-arcs
OBJ= ../obj/
BIN= ../bin/
SRC= ../src/
INC= ../include/
TRACE= -DTRACE_FEATURE


output: $(BIN)dns_c $(BIN)dns_s

$(BIN)dns_c: $(OBJ)dns_client.o $(OBJ)dns_client_funcs.o $(OBJ)logger.o
	$(CC) -o dns_c $(OBJ)dns_client.o $(OBJ)dns_client_funcs.o $(OBJ)logger.o
	mv dns_c $(BIN)

$(BIN)dns_s: $(OBJ)dns_server.o $(OBJ)dns_server_funcs.o $(OBJ)logger.o
	$(CC) -o dns_s $(OBJ)dns_server.o $(OBJ)dns_server_funcs.o $(OBJ)logger.o -lpthread
	mv dns_s $(BIN)


$(OBJ)dns_client.o: $(SRC)dns_client.c $(INC)header.h $(INC)logger.h
	$(CC) $(CFLAGS) $(SRC)dns_client.c -I$(INC)
	mv dns_client.o $(OBJ)

$(OBJ)dns_client_funcs.o: $(SRC)dns_client_funcs.c $(INC)header.h $(INC)logger.h
	$(CC) $(CFLAGS)  $(SRC)dns_client_funcs.c -I$(INC)
	mv dns_client_funcs.o $(OBJ)

$(OBJ)dns_server.o: $(SRC)dns_server.c $(INC)header_server.h $(INC)logger.h
	$(CC) $(CFLAGS) $(SRC)dns_server.c -I$(INC) 
	mv dns_server.o $(OBJ)

$(OBJ)dns_server_funcs.o: $(SRC)dns_server_funcs.c $(INC)header_server.h $(INC)logger.h
	$(CC) $(CFLAGS)  $(SRC)dns_server_funcs.c -I$(INC)
	mv dns_server_funcs.o $(OBJ)

$(OBJ)logger.o: $(SRC)logger.c $(INC)logger.h
	$(CC) $(CFLAGS)  $(SRC)logger.c -I$(INC)
	mv logger.o $(OBJ)


clean:
	rm $(BIN)dns_c
	rm $(BIN)dns_s
	rm $(OBJ)*.o


