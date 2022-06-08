ifndef DESTDIR
DESTDIR = ~/.local/bin
endif

.PHONY: install clean

gitall.o: %.o: %.c config.h
	gcc -o $@ $<

config.h:
	cp -n config.h.tmpl config.h

install: gitall.o
	cp gitall.o $(DESTDIR)/gitall

clean:
	rm *.o
