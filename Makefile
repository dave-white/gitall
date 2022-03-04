ifndef DESTDIR
DESTDIR = ~/.local/bin
endif

.PHONY: install

gitall.o: %.o: %.c %.h
	gcc -o $@ $<

install: gitall.o
	cp gitall.o $(DESTDIR)/gitall
