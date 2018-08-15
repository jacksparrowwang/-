
ROOT=$(shell pwd)
INCLUDE=-I$(ROOT)/include/
LIB=-L$(ROOT)/lib/
LIB_NAME=-lmysqlclient

.PHONY:all

all:select insert

insert:insert.c
	gcc -o $@ $^ $(LIB) $(LIB_NAME) $(INCLUDE)

select:select.c
	gcc -o $@ $^ $(LIB) $(LIB_NAME) $(INCLUDE)

.PHONY:clean

clean:
	rm select insert
