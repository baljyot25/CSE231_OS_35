#Compile the launch.c by linking it with the lib_simpleloader.so
all: ../bin/lib_simpleloader.so launch.o
	gcc -Wall -fPIC -m32 -o output ../bin/lib_simpleloader.so launch.o
	mv output ../bin

launch.o: launch.c
	gcc -m32 -c launch.c

#Provide the command for cleanup
clean:
	-@rm -f *.o
