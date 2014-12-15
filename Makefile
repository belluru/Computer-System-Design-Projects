# Defined constants for command base name and arguments for simple test

STUDENT_ID=123

build:
	gcc -Wall -g quash.c -o quash


test:
	./quash

clean:
	rm -f quash
	rm -rf $(STUDENT_ID)quashshell

tar:
	make clean
	mkdir $(STUDENT_ID)quashshell
	cp quash.c Makefile $(STUDENT_ID)quashshell
	tar cvzf $(STUDENT_ID)quashshell.tar.gz $(STUDENT_ID)quashshell
	rm -rf $(STUDENT_ID)quashshell

