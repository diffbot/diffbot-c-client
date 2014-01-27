
PROGNAME = example
LIBNAME = libdiffbot.so
CFLAGS   = -g -I./
LDFLAGS  = -L./ -lcurl -ljson-c

all: $(LIBNAME) $(PROGNAME)

$(LIBNAME): diffbot.c diffbot.h
	        $(CC) diffbot.c -o $(LIBNAME) $(CFLAGS) -fPIC -shared $(LDFLAGS)

$(PROGNAME): example.c diffbot.h
	        $(CC) example.c -o $(PROGNAME) $(CFLAGS) -L./ -lcurl -ldiffbot -ljson-c 

install: $(LIBNAME) diffbot.h
	        mkdir -p /usr/include/json
	        cp diffbot.h /usr/include/json/
	        cp $(LIBNAME) /usr/lib/
			ldconfig

uninstall: $(LIBNAME) diffbot.h
	        rm /usr/include/json/diffbot.h
	        rm /usr/lib/$(LIBNAME)
	        rmdir /usr/include/json

clean:
	        rm -f $(PROGNAME) *.o *.so
