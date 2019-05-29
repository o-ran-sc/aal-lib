release: virtconsolebe.c virtconsolebe.h
	gcc -O2 virtconsolebe.c -o virtconsolebe.so -fPIC -shared 

debug: virtconsolebe.c virtconsolebe.h
	gcc -g virtconsolebe.c -o virtconsolebe.so -fPIC -shared 
	
clean:
	rm virtconsolebe.so -rf
