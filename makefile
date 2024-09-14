#
#  $Id: makefile,v 1.5 1998/10/26 03:02:44 tp Exp $
#

###############################################################################
#
#  run make VERBOSE="" to get the command lines...
#
ifndef VERBOSE
VERBOSE=@
endif

###############################################################################
#
#  system dependent configuration
#
CXX=g++
#DEBUGLIBS=-L/opt/lib -ldmalloc
#DEBUGLIBS=-lefence
#DEBUGLIBS=lib/libefence.a
#DEBUGLIBS=-L/opt/lib -lccmalloc -ldl
#DEBUGLIBS=/opt/lib/ccmalloc.o -ldl
LIBTVDIR=${HOME}/tvision
#LIBTVDIR=/opt
LIBTV=$(LIBTVDIR)/build/libtvision.a
CPPFLAGS=-DLINUX \
	-Iinclude \
	-I/opt/include \
	-I/opt/include/tvision \
	-I$(LIBTVDIR)/include \
	-I$(HOME)/include \
	-I$(HOME)/include/tvision
CFLAGS=$(CPPFLAGS)
LIBS= $(LIBTV) -lncurses -lgpm # -lintl 
LDFLAGS=-L/opt/lib -L$(LIBTVDIR)/lib -L$(HOME)/lib $(LIBS)

###############################################################################
#
#  no need to touch the next lines, hopefully...
#

VPATH+= .
VPATH+= libvfs
VPATH+= io
VPATH+= gui
VPATH+= misc

SRCS=		cmdline.cc	\
		io.cc		\
		io_dos.cc	\
		io_linux.cc	\
		cable.cc	\
		lpt.cc		\
		protocol.cc

LIBVFSSRCS=	dev_dos.cc	\
		dev_unx.cc	\
		device.cc	\
		file.cc		\
		fs.cc		\
		fs_arj.cc	\
		fs_d64.cc	\
		fs_dos.cc	\
		fs_exec.cc	\
		fs_fact.cc	\
		fs_lha.cc	\
		fs_lnx.cc	\
		fs_t64.cc	\
		fs_tar.cc	\
		fs_unix.cc	\
		fs_zip.cc	\
		fs_zoo.cc	\
		fs_zipc.cc	\
		f_d64.cc	\
		debug.cc	\
		util.cc		\
		vector.cc

DESTSRCS=	main.cc		\
		gui.cc		\
		guidebug.cc

INITSRCS=	exithook.cc
TESTSRCS=	test.cc

OBJS=$(SRCS:%.cc=obj/%.o)
LIBVFSOBJS=$(LIBVFSSRCS:%.cc=obj/%.o)
DESTOBJS=$(DESTSRCS:%.cc=obj/%.o)
INITOBJS=$(INITSRCS:%.cc=obj/%.o)
TESTOBJS=$(TESTSRCS:%.cc=obj/%.o)
DEST=vc1541_2
TEST=t

all: $(DEST)

$(TEST): obj $(TESTOBJS) $(LIBVFSOBJS)
	@echo "** linking $@..."
	$(VERBOSE)$(CXX) -g -o $@ $(TESTOBJS) $(LIBVFSOBJS) $(DEBUGLIBS)
	@echo "## done."
	
$(DEST): obj $(OBJS) $(DESTOBJS) $(LIBVFSOBJS) $(INITOBJS)
	@echo "** linking $@..."
	$(VERBOSE)$(CXX) -g -o $@ $(DESTOBJS) $(OBJS) $(LIBVFSOBJS) $(LDFLAGS) $(DEBUGLIBS) $(INITOBJS)
	@echo "## done."

obj:
	@echo "-- creating object directory..."
	@mkdir obj

obj/%.o : %.cc
	@echo "++ compiling $<..."
	$(VERBOSE)$(CXX) -g -MMD -c -o $@ $(CFLAGS) $<
#	@mv $*.d $*.d~
#	@sed -e 's,^$*,obj/&,' $*.d~ > $*.d
#	@rm -f $*.d~
#	@mv $*.d obj

clean:
	rm -f $(OBJS) $(DESTOBJS) $(TESTOBJS) $(LIBVFSOBJS) $(INITOBJS) obj/*.d
	rm -f `find . -name \*~`


dos2unix:
	for a in `find . -name \*.cc -o -name \*.h`; \
	do\
		cp $$a $$a~; \
		sed -e 's///' $$a~ | tr -d "\032" > $$a; \
	done

cvsmod:
	cvs stat 2>/dev/null | grep ^File: | grep -v Up-to-date 

tar:
	(cd .. && tar -c -v -X vc1541_2/tar.ex -f vc1541_2.tar vc1541_2)
	(cd .. && gzip -9fv vc1541_2.tar)

test: $(TEST)
	./$(TEST)

ifneq ($(wildcard obj/*.d),)
include $(wildcard obj/*.d)
endif
