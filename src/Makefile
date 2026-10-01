TARGETS = setclasscontext
LIBS	= -L/package/prog/skalibs/library
INCS	= -I/package/prog/skalibs/include
BINDIR	= ../package/bin/

all: $(TARGETS)

.PHONY: all clean install

install: ${TARGETS}
	cp $(TARGETS) ../package/bin/

${BINDIR}setclasscontext: setclasscontext.c
	${CC} ${INCS} -O0 -g -o $@ $< $(LIBS) -lskarnet -lc

clean:
	-rm *.o *~ \#*\#
