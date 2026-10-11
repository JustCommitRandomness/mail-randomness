SN_LIBS	= -L/package/prog/skalibs/library
SN_INCS	= -I/package/prog/skalibs/include
BINDIR	= ../package/bin/
CPP	= /usr/libexec/cpp

STAGING	= \
	$(BINDIR)/setclasscontext	\
	$(BINDIR)/explode-servicedir	\

FLYDIRT	= *.o *.a *.lo *.so *.i a.out



all: $(STAGING) .gitignore

.PHONY: all distclean realclean clean install

install: $(STAGING)
	cp $(STAGING) ../package/bin/

$(BINDIR)/setclasscontext: setclasscontext.c
	$(CC) $(SN_INCS) -O0 -g -o $@ $> $(SN_LIBS) -lskarnet -lc

servicedirparse.i: servicedirparse.c
	$(CPP) $(CPPFLAGS) $> \
	|awk '/^#/ {print; next } { sub(" *; *;",";");print "#line " NR " \"servicedirparse.c\""; print }' \
	|clang-format-19 > $@

servicedirparse.out: servicedirparse.i
	awk -vRS=';' -vORS=';\n' -F= '/\n/{gsub("[\n \t]+", " ")}/=/{print "extern", $$1; next}{print $$0}' $> > $@

$(BINDIR)/explode-servicedir: explode-servicedir.c servicedirparse.i
	$(CC) $(SN_INCS) -O0 -g -o $@ explode-servicedir.c $(SN_LIBS) -lskarnet -lc

clean: distclean

distclean:
	-rm '$FLYDIRT'

realclean: distclean
	-rm $(STAGING)

.gitignore:
	echo -n '$(FLYDIRT)' $(STAGING) | tr ' ' "\\n" > $@
