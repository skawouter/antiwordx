#
# Makefile for antiword (Linux version)
#

CC	= gcc
LD	= gcc

INSTALL		= cp -f
INSTALL_PROGRAM	= $(INSTALL)
INSTALL_DATA	= $(INSTALL)

# must be equal to DEBUG or NDEBUG
DB	= NDEBUG
# Optimization: -O<n> or debugging: -g
OPT	= -O2

LDLIBS	=

CFLAGS	= -Wall -pedantic $(OPT) -D$(DB)
LDFLAGS	=

OBJS	=\
	main_u.o asc85enc.o blocklist.o chartrans.o datalist.o depot.o\
	doclist.o fail.o finddata.o findtext.o fmt_text.o fontlist.o\
	fonts.o fonts_u.o hdrftrlist.o \
	listlist.o misc.o notes.o options.o out2window.o output.o \
	pictlist.o prop0.o prop2.o prop6.o prop8.o\
	properties.o propmod.o rowlist.o sectlist.o stylelist.o stylesheet.o\
	summary.o tabstop.o text.o unix.o utf8.o word2text.o worddos.o\
	wordlib.o wordmac.o wordole.o wordwin.o xmalloc.o

PROGS =\
	antiword

LOCAL_INSTALL_DIR = $(HOME)/bin
LOCAL_RESOURCES_DIR = $(HOME)/.antiword

GLOBAL_INSTALL_DIR = /usr/local/bin
GLOBAL_RESOURCES_DIR = /usr/share/antiword

all:		$(PROGS)

install:	all
	mkdir -p $(LOCAL_INSTALL_DIR)
	cp -pf $(PROGS) $(LOCAL_INSTALL_DIR)
	mkdir -p $(LOCAL_RESOURCES_DIR)
	cp -pf Resources/* $(LOCAL_RESOURCES_DIR)

# NOTE: you might have to be root to do this
global_install:	all
#	@[ `id -u` -eq 0 ] || (echo "You must be root to do this" && false)
	mkdir -p $(DESTDIR)$(GLOBAL_INSTALL_DIR)
	$(INSTALL_PROGRAM) $(PROGS) $(DESTDIR)$(GLOBAL_INSTALL_DIR)
	cd $(DESTDIR)$(GLOBAL_INSTALL_DIR); chmod 755 $(PROGS)
	mkdir -p $(DESTDIR)$(GLOBAL_RESOURCES_DIR)
	chmod 755 $(DESTDIR)$(GLOBAL_RESOURCES_DIR)
	$(INSTALL_DATA) Resources/*.txt $(DESTDIR)$(GLOBAL_RESOURCES_DIR)
	$(INSTALL_DATA) Resources/fontnames $(DESTDIR)$(GLOBAL_RESOURCES_DIR)
	cd $(DESTDIR)$(GLOBAL_RESOURCES_DIR); chmod 644 *.txt fontnames

# NOTE: you might have to be root to do this
global_uninstall:
#	@[ `id -u` -eq 0 ] || (echo "You must be root to do this" && false)
	cd $(DESTDIR)$(GLOBAL_INSTALL_DIR); rm -f $(PROGS)
	-rmdir $(DESTDIR)$(GLOBAL_INSTALL_DIR)
	cd $(DESTDIR)$(GLOBAL_RESOURCES_DIR); rm -f *.txt fontnames
	-rmdir $(DESTDIR)$(GLOBAL_RESOURCES_DIR)

clean:
	rm -f $(OBJS)
	rm -f $(PROGS)

antiword:	$(OBJS)
	@rm -f $@
	$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	@chmod 750 $@

.c.o:
	$(CC) $(CFLAGS) -c $<

main_u.o:	version.h
fonts_u.o:	fontinfo.h

fontinfo.h:	fontinfo/fontinfo.pl
	fontinfo/fontinfo.pl > ./fontinfo.h
