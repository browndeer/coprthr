
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
#LIBS += -L$(CL_ROOT)/lib/x86_64 -locl -lpthread -ldl \
#	-L$(LIBSTDCL_ROOT)/lib -lstdcl

ANNOUNCEMENT = "\a\n\n*****PLEASE NOTE *****\n\nMake sure "$(LIBSTDCL_ROOT)"/lib \nis added to LD_LIBRARY_PATH\n\n"


### It should NOT be necessary to modify lines below

### comment out if GL support does not work correctly on your platform
LIBS += -L/usr/lib64 -lglut -lGLEW

CXXFLAGS += -O3

CLLD = $(LIBSTDCL_ROOT)/bin/clld

all: $(NAME).x
	-echo -e $(ANNOUNCEMENT)

$(NAME).x: $(NAME).o $(OBJS) out_clld.o bdt_bmp.o
	$(CXX) $(CXXFLAGS) $(INCS) -o $(NAME).x $(NAME).o $(OBJS) out_clld.o bdt_bmp.o $(LIBS)

out_clld.o: $(CLSRC)
	$(CLLD) --cl-source $(CLSRC)

bdt_bmp.o:
	objcopy -I binary -O elf64-x86-64 -B i386 bdt.bmp bdt_bmp.o

.SUFFIXES:
.SUFFIXES: .br .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCS) -c $<


clean:
	rm -f *.o 

distclean: clean
	rm -f *.x

