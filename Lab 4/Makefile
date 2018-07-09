all: piodirect.h piodirect.c pwm.h pwm.c driver.c
	make a
	make b

a: piodirect.h piodirect.c pwm.h pwm.c driver.o a.c
	gcc -g a.c driver.o piodirect.c pwm.c -lrt -lpthread -o a

b: piodirect.h piodirect.c pwm.h pwm.c b.c driver.o
	gcc -g b.c driver.o piodirect.c pwm.c -lrt -lpthread -o b

driver: piodirect.h piodirect.c pwm.h pwm.c driver.c driver.h
	gcc -g driver.c piodirect.c pwm.c -o driver.o

clean:
	rm -rf a
	rm -rf b
	rm *.o