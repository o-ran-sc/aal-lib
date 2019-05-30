release: console.c console.h hash.c hash.h queue.c queue.h
	gcc -O2 console.c hash.c queue.c -o console.so -fPIC -shared 
	
debug: console.c console.h hash.c hash.h queue.c queue.h
	gcc -g console.c hash.c queue.c -o console.so -fPIC -shared 
	
clean:
	rm console.so -rf