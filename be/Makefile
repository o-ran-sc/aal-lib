
dir=vhost virtio_console 

release: $(dir)

#debug: $(dir)
#	make -C $< debug
#
clean: $(dir)
	make -C $< clean
	
$(dir)):ECHO
	make -C $@ release
	
ECHO: 
	@echo $(dir)
