release: fpga.c fpga.h
	gcc -O2 fpga.c -o fpga.so -fPIC -shared 
	
debug: fpga.c fpga.h
	gcc -g fpga.c -o fpga.so -fPIC -shared 
	
clean:
	rm fpga.so -rf
