all: a.c b1.c piodirect.c piodirect.h
	make a b1 b2 b3 c
a: a.c piodirect.c piodirect.h busywait.h busywait.c
	gcc a.c piodirect.c busywait.c -lrt -lpthread -o a

b1: b1.c piodirect.c piodirect.h busywait.h busywait.c
	gcc b1.c piodirect.c busywait.c -lrt -lpthread -o b1

b2: b2.c piodirect.c piodirect.h busywait.c busywait.h
	gcc b2.c piodirect.c busywait.c -lrt -lpthread -o b2

b3: b3.c piodirect.c piodirect.h busywait.c busywait.h
	gcc b3.c piodirect.c busywait.c -lrt -lpthread -o b3

c: c.c piodirect.c piodirect.h busywait.c busywait.h pwm.h pwm.c
	gcc c.c piodirect.c busywait.c pwm.c -lrt -lpthread -o c

clean:
	rm -rf a b1 b2 b3 c
