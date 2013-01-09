##############################################################################
#    file: makefile for ray.cc
#  author: J. Edward Swan II
# created: 1-17-2005
#          1-17-2007 Tested and optimized OPTIM flags with gprof (swan)
###############################################################################
# Source file information

MAIN	=	mray.exe

SRCS    =	mray.cc view.cc surface.cc surfaces.cc scene.cc material.cc functions.cc
OBJS	=	$(SRCS:.cc=.o)

###############################################################################
# General compilation settings.  
DESTDIR		= .
INCLUDE 	= -I../libgm
DEFINES 	= 
LDFLAGS		=
LIBDIR		= -L../libgm
LIBS		= -lgm -lm -lpthread -ffast-math

# For an optimized version, put this definition last.  This will substantially
# accelerate your program.  The "-DNDEBUG" removes assertions.  The "-O2" flag
# will cause inlined functions to actually be inlined.  This will cause a
# substantial amount of acceleration, because the libgm library makes
# substantial use of inlined functions.
#OPTIM		= -DNDEBUG -O2
# For a debug version, put this definition last.  "-Wall" prints copious,
# lint-type error messages.  "-g" produces a symbol table suitable for a
# debugger such as GDB.
OPTIM		= -Wall -g

# Collect all the compilation settings
CPPFLAGS	= $(OPTIM) $(INCLUDE) $(DEFINES)

###############################################################################
# Programs to run
CC		= g++
RM		= /bin/rm -f
MAKE		= /bin/make
MKDEPEND	= makedepend $(DEFINES) $(INCLUDE) $(SRCS)
LINK		= $(CC) $(LDFLAGS) -o $(DESTDIR)/$@ $(OBJS) $(LIBDIR) $(LIBS)

###############################################################################
# Explicit rules
.PRECIOUS: $(SRCS)

$(MAIN): $(OBJS)
	$(LINK)

all:
	$(MAKE) clean
	$(MAKE) depend
	$(MAKE) $(MAIN)

depend:	
	$(MKDEPEND)	

clean:
	$(RM) *.o $(MAIN)

###############################################################################
# Dependency info
# Do not put stuff below this line; makedepend will clobber it!
# DO NOT DELETE THIS LINE -- make depend depends on it.

mray.o: view.h ../libgm/gmVec3.h ../libgm/gmUtils.h ../libgm/gmConst.h
mray.o: surface.h ray.h hit.h
view.o: view.h ../libgm/gmVec3.h ../libgm/gmUtils.h ../libgm/gmConst.h
surface.o: surface.h ray.h ../libgm/gmVec3.h ../libgm/gmUtils.h
surface.o: ../libgm/gmConst.h hit.h
