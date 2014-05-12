CC=gcc
CFLAGS=-std=c99 -Wall -g
LFLAGS=-lmenuw -lncursesw

SRCDIR=src
OBJDIR=obj
BINDIR=bin

OUT=notes



vpath %.c $(SRCDIR)
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SOURCES=$(call rwildcard,./$(SRCDIR),*.c)
OBJECTS=$(SOURCES:./$(SRCDIR)/%.c=$(OBJDIR)/%.o)

${BINDIR}/%: ${OBJECTS}
	${CC} -o $@ $^ ${LFLAGS}

${OBJDIR}/%.o: ${SRCDIR}/%.c
	${CC} -o $@ -c $< ${CFLAGS}

all: ${BINDIR}/${OUT}

run:
	${BINDIR}/${OUT}

clean:
	rm -f ${BINDIR}/${OUT}
	rm -f ${OBJECTS}
