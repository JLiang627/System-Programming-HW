FIFO_SERVER_EXEC = fifo_seqnum_server
FIFO_CLIENT_EXEC = fifo_seqnum_client
VERIFY_EXEC      = fifo_nonblocking
SVMSG_SERVER_EXEC= svmsg_seqnum_server
SVMSG_CLIENT_EXEC= svmsg_seqnum_client

ALL_TARGETS = $(FIFO_SERVER_EXEC) $(FIFO_CLIENT_EXEC) $(VERIFY_EXEC) $(SVMSG_SERVER_EXEC) $(SVMSG_CLIENT_EXEC)

CC = gcc
CFLAGS = -I. -g -Wall

COMM_OBJ = error_functions.o get_num.o

.PHONY: all clean

all: $(ALL_TARGETS)

$(FIFO_SERVER_EXEC): fifo_seqnum_server.o $(COMM_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(FIFO_CLIENT_EXEC): fifo_seqnum_client.o $(COMM_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(VERIFY_EXEC): fifo_nonblocking.o $(COMM_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(SVMSG_SERVER_EXEC): svmsg_seqnum_server.o $(COMM_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(SVMSG_CLIENT_EXEC): svmsg_seqnum_client.o $(COMM_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

error_functions.o: error_functions.c error_functions.h tlpi_hdr.h
	$(CC) $(CFLAGS) -c $< -o $@

get_num.o: get_num.c get_num.h tlpi_hdr.h error_functions.h
	$(CC) $(CFLAGS) -c $< -o $@

fifo_seqnum_server.o: fifo_seqnum_server.c fifo_seqnum.h tlpi_hdr.h error_functions.h get_num.h

fifo_seqnum_client.o: fifo_seqnum_client.c fifo_seqnum.h fifo_seqnum.h tlpi_hdr.h error_functions.h get_num.h

fifo_nonblocking.o: fifo_nonblocking.c tlpi_hdr.h error_functions.h

svmsg_seqnum_server.o: svmsg_seqnum_server.c svmsg_seqnum_header.h tlpi_hdr.h error_functions.h get_num.h

svmsg_seqnum_client.o: svmsg_seqnum_client.c svmsg_seqnum_header.h tlpi_hdr.h error_functions.h get_num.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(ALL_TARGETS) $(COMM_OBJ) *.o