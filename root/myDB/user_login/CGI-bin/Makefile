.PHONY:all

all:dyn_page minute

minute:minute_page.c
	gcc -o $@ $^

dyn_page:login_usr_page.c
	gcc -o $@ $^

.PHONY:clean

clean:
	rm dyn_page minute
