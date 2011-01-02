
NAME = bdt_nbody

OBJS = Timer.o nbody_init.o nbody_display.o 


### Uncomment these lines to use OpenCL implementation 
CL_ROOT = /usr/local/atistream
LIBSTDCL_ROOT = /usr/local/browndeer/
CLSRC = nbody_kern.cl
#OBJS += out_clld.o
DEFS += -DENABLE_CL
INCS += -I$(CL_ROOT)/include -I$(LIBSTDCL_ROOT)/include
LIBS += -L$(CL_ROOT)/lib/x86_64 -lOpenCL -lpthread -ldl \
	-L$(LIBSTDCL_ROOT)/lib -lstdcl
ANNOUNCEMENT = "\a\n\n*****PLEASE NOTE *****\n\nMake sure "$(LIBSTDCL_ROOT)"/lib \nis added to LD_LIBRARY_PATH\n\n"


### It should NOT be necessary to modify lines below

LIBS += -L/usr/lib64 -lglut -lGLEW

CXXFLAGS += -O3

BRCC = brcc
BRCCFLAGS =

CLLD = $(LIBSTDCL_ROOT)/bin/clld

all: $(NAME).x
	-echo -e $(ANNOUNCEMENT)

$(NAME).x: $(NAME).o $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) -o $(NAME).x $(NAME).o $(OBJS) $(LIBS)

#out_clld.o: $(CLSRC)
#	$(CLLD) --cl-source $(CLSRC)

.SUFFIXES:
.SUFFIXES: .br .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCS) -c $<

.br.cpp:
	$(BRCC) $(BRCCFLAGS) -o $* $<


clean:
	rm -f *.o 
	rm -f $(BRSRCS:.br=.cpp) $(BRSRCS:.br=.h) $(BRSRCS:.br=_gpu.h)

distclean: clean
	rm -f *.x
	rm -f *.vti

