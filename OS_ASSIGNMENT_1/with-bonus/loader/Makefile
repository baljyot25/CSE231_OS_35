#Create lib_simpleloader.so from loader.c
all:
	gcc -shared -m32 -c -o lib_simpleloader.so loader.c
	mv lib_simpleloader.so ../bin

#Provide the command for cleanup
clean:
	-@rm -f *.o
