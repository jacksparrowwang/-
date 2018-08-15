server:http_server.c
	gcc -o $@ $^ -pthread -g

.PHONY:clean

clean:
	rm server
