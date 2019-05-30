
c_file=main.c aal.c
h_file=aal.h

lib=-lpthread -ldl -lxml2

#release
release: aal_release BE_release plugin_release

aal_release: $(c_file) $(h_file)
	gcc -O2 $(c_file) -o aal $(lib)

BE_release:
	make -C be release

plugin_release:
	make -C plugin release

#debug
debug: aal_debug BE_debug plugin_debug

aal_debug: $(c_file) $(h_file)
	gcc -g $(c_file) -o aal $(lib)

BE_debug:
	make -C be debug

plugin_debug:
	make -C plugin debug

#clean
clean: aal_clean BE_clean plugin_clean

aal_clean: 
	rm aal -rf
	
BE_clean:
	make -C be clean

plugin_clean:
	make -C plugin clean
