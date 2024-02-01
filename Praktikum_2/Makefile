CC=gcc
OBJDIR=obj
SRCDIR=src
TESTDIR=test
INSTALLDIR=obj
SRCFILES=$(wildcard $(SRCDIR)/*.c)
OBJFILES=$(subst $(SRCDIR)/,$(OBJDIR)/,$(SRCFILES:.c=.o))
DEPS=$(OBJFILES:.o=.d)
EXE=main consumer producer observer qeue
compmodules=obj/main.o obj/consumer.o obj/observer.o obj/producer.o obj/qeue.o

-include $(DEPS)

all: $(patsubst %,$(TESTDIR)/%,$(EXE))

$(EXE): $(OBJFILES)
	$(CC) $^ -o $(TESTDIR)/$@ -lpthread

$(TESTDIR)/% : $(OBJDIR)/%.o $(compmodules)
	$(CC) -o $@ $^ -lpthread

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -g -Wall $< -o $@ -lpthread
#	mkdir -p $(OBJDIR)

$(OBJDIR)/%.d: $(SRCDIR)/%.c
	$(CC) -MM -MT "$@ $(patsubst %.d,%.o,$@)" -MF $@ $<

install:
	cp $(TESTDIR)/* $(INSTALLDIR)

clean:
	rm $(INSTALLDIR)/*
	rm $(TESTDIR)/*