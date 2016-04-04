.PHONY:clean
GG=gcc
CFLAGS=-Wall -g
BIN=tinyserver
OBJS=tinyserver.o util.o epoll.o message.o 
LIBS=

$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
   
   
