# wxWindows Template App
# Lukasz Zalewski 2002/2006
# ------------------

###################### linux #################################
INCS =
LIBP = 

ARCH_CFLAGS = -D__3dNOW_A__ -D__SSE__
#The code below should be used rather than the
#specific 2.4 version one. With this one we ensure
#that we run the latest release of wxGTK rather
#than a specific one:

WX_LIBS = `wx-config --libs --gl-libs`
WX_FLAGS = `wx-config --cxxflags`

LIBS = $(WX_LIBS)
ARCH_CFLAGS = -D__3dNOW_A__ -D__SSE__
EXES = cip

CFLAGS	= $(ARCH_CFLAGS) $(WX_FLAGS) -Wall -Wno-unused -Wno-reorder \
	-O3 -mtune=i686 -march=i686 -fomit-frame-pointer -fforce-addr 


# ------------------

all : clean $(EXES)

clean :
	find -name "*.o" -exec rm {} \;
	rm -f ${EXES} -R

# ------------------

cip : cip.o pixelOperations.o 
	g++ cip.o pixelOperations.o  -o cip $(ARCH_CFLAGS) $(LIBS) $(CFLAGS)

cip.o : cip.cpp cip.h
	g++ -c cip.cpp $(ARCH_CFLAGS) $(INCS) $(WX_FLAGS)

pixelOperations.o: pixelOperations.cpp pixelOperations.h
	g++ -c pixelOperations.cpp $(ARCH_CFLAGS) $(INCS) $(WX_FLAGS)


# ------------------
