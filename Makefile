# This is a sample Makefile which compiles source files named:



CC = gcc

LIBS = 	-lpthread\
	/users/cse533/Stevens/unpv13e/libunp.a\
	
LIBS1 = /users/cse533/Asgn3_code\
	
	
FLAGS = -g -O2

CFLAGS = ${FLAGS} -I/users/cse533/Stevens/unpv13e/lib
CFLAGS1 = ${FLAGS} -I/users/cse533/Asgn3_code/

all: server_hkatagal client_hkatagal odr_hkatagal

#server
server_hkatagal.o: server_hkatagal.c
	${CC} ${CFLAGS} -c server_hkatagal.c

server_hkatagal: server_hkatagal.o utilities.o
	${CC} ${FLAGS} -o server_hkatagal server_hkatagal.o  utilities.o ${LIBS}

#client	
client_hkatagal.o: client_hkatagal.c
	${CC} ${CFLAGS} -c client_hkatagal.c

client_hkatagal: client_hkatagal.o utilities.o
	${CC} ${FLAGS} -o client_hkatagal client_hkatagal.o utilities.o ${LIBS}

#get_hw_addrs
get_hw_addrs.o: get_hw_addrs.c
	${CC} ${FLAGS} -c get_hw_addrs.c


#ODR	
odr_hkatagal.o: odr_hkatagal.c
	${CC} ${CFLAGS} -c odr_hkatagal.c

odr_hkatagal: odr_hkatagal.o get_hw_addrs.o utilities.o
	${CC} ${FLAGS} -o odr_hkatagal odr_hkatagal.o get_hw_addrs.o utilities.o ${LIBS}

#utilities
utilities.o: utilities.c
	${CC} ${CFLAGS} -c utilities.c

clean:
	rm server_hkatagal.o server_hkatagal client_hkatagal.o client_hkatagal odr_hkatagal.o odr_hkatagal utilities.o get_hw_addrs.o
