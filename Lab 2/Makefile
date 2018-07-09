all:
	make a3 b1 b2

a3: a3.c piodirect.c piodirect.h pwm.c pwm.h
	gcc -g a3.c piodirect.c pwm.c -o a3

b1: b1.c piodirect.c piodirect.h pwm.c pwm.h
	gcc -g b1.c piodirect.c pwm.c -o b1

b2: b2.c piodirect.c piodirect.h pwm.c pwm.h
	gcc -g b2.c piodirect.c pwm.c -o b2

clean:
	rm -f a3 b1 b2
