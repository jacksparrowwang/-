server:http_server.c
	gcc -o $@ $^ -pthread

.PHONY:clean

clean:
	rm server
