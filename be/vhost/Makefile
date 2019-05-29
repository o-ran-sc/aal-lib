release: vhost.c vhost.h
	gcc -O2 vhost.c -o vhost.so -fPIC -shared 

debug: vhost.c vhost.h
	gcc -g vhost.c -o vhost.so -fPIC -shared 
	
clean:
	rm vhost.so -rf
