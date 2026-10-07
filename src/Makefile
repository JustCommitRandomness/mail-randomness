LIBS	= -L/package/prog/skalibs/library
INCS	= -I/package/prog/skalibs/include
BINDIR	= ../package/bin/

STAGING	= \
	$(BINDIR)/setclasscontext	\
	$(BINDIR)/explode-servicedir	\

FLYDIRT	= *.o *.a *.lo *.so a.out

all: $(STAGING) .gitignore

.PHONY: all distclean realclean clean install

install: $(STAGING)
	cp $(STAGING) ../package/bin/

$(BINDIR)/setclasscontext: setclasscontext.c
	$(CC) $(INCS) -O0 -g -o $@ $> $(LIBS) -lskarnet -lc

$(BINDIR)/explode-servicedir: explode-servicedir.c
	$(CC) $(INCS) -O0 -g -o $@ $> $(LIBS) -lskarnet -lc

clean: distclean

distclean:
	-rm '$FLYDIRT'

realclean: distclean
	-rm $(STAGING)

.gitignore:
	echo -n '$(FLYDIRT)' $(STAGING) | tr ' ' "\\n" > $@
