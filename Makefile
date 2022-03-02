ifndef DESTDIR
DESTDIR = ~/.local/bin
endif

.PHONY: install

gitall: gitall.c
	gcc -o gitall gitall.c

install:
	cp gitall $(DESTDIR)/gitall
