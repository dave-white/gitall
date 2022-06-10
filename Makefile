ifndef DESTDIR
DESTDIR = ~/.local/bin
endif

.PHONY: all install uninstall clean

SRC = gitall.c
OBJ = $(SRC:.c=.o)

all: gitall

gitall: config.h $(OBJ)
	gcc -o $@ $(OBJ)
	
config.h:
	cp -n config.def.h config.h

install: gitall
	mkdir -p $(DESTDIR)
	cp gitall $(DESTDIR)/
	chmod 755 $(DESTDIR)/gitall

uninstall:
	rm -f $(DESTDIR)/gitall

clean:
	rm gitall $(OBJ)
