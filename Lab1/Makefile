all: led.c piodirect.c piodirect.h hello_world.c
	gcc led.c piodirect.c -o led
	gcc hello_world.c -o hello

led: led.c piodirect.c piodirect.h
	gcc led.c piodirect.c -o led
	gcc hello_world.c -o hello

clean:
	rm -rf led *.o hello
