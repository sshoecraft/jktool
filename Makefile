
PROG=$(shell basename $(shell pwd))
MYBMM_SRC=../mybmm
TRANSPORTS=$(shell cat $(MYBMM_SRC)/Makefile | grep ^TRANSPORTS | head -1 | awk -F= '{ print $$2 }')
SRCS=main.c module.c jk.c jk_info.c parson.c list.c utils.c log.c $(TRANSPORTS)
OBJS=$(SRCS:.c=.o)
CFLAGS=-DJKTOOL -I$(MYBMM_SRC)
#CFLAGS+=-Wall -O2 -pipe
CFLAGS+=-Wall -g -DDEBUG
LIBS+=-ldl -lgattlib -lglib-2.0 -lpthread
LDFLAGS+=-rdynamic

vpath %.c $(MYBMM_SRC)

.PHONY: all
all: $(PROG)

$(PROG): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(OBJS) $(LIBS)

include $(MYBMM_SRC)/Makefile.dep

debug: $(PROG)
	gdb ./$(PROG)

install: $(PROG)
	install -m 755 -o bin -g bin $(PROG) /usr/bin/$(PROG)

clean:
	rm -rf $(PROG) $(OBJS) $(CLEANFILES)

push: clean
	git add -A .
	git commit -m refresh
	git push

pull: clean
	git reset --hard
	git pull
