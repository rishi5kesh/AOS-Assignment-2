compile: driver.c implementation.c
	gcc -o cshell driver.c implementation.c
	
run: cshell
	./cshell <filename>
