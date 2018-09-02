server:http_server.c
	gcc -o wangchenggang $^ -pthread

.PHONY:clean

clean:
	rm server
