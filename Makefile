all: oss

clean:
	-rm oss

dt:
	gcc -o oss.c oss 
