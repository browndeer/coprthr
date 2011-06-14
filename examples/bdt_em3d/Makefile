
NAME = bdt_em3d


### Uncomment these lines to use OpenCL implementation
### Change paths according to your own platform

#OPENCL_DIR = /usr/local/atistream/beta4
OPENCL_DIR = /usr/local/atistream
LIBSTDCL_DIR = /usr/local/browndeer

DEFS = -DENABLE_CL

OBJS = Timer.o em3d_fdtd_gpu_cl_display.o 



##### It should NOT be necessary to modify lines below #####

INCS += -I$(OPENCL_DIR)/include 
LIBS += -L$(OPENCL_DIR)/lib/x86_64 -lOpenCL -lpthread -ldl 
#LIBS += -L$(OPENCL_DIR)/lib/x86_64 -locl -lpthread -ldl 

INCS += -I$(LIBSTDCL_DIR)/include
LIBS += -L$(LIBSTDCL_DIR)/lib -lstdcl

LIBS += -lglut -lGLEW

CXXFLAGS += -O3

CLLD = clld

all: $(NAME).x 
#all: reck.x

$(NAME).x: $(NAME).o $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) -o $(NAME).x $(NAME).o $(OBJS) $(LIBS)

#out_clld.o: $(CLSRCS)
#	$(CLLD) --cl-source $(CLSRCS)

reck.x: reck.o
	$(CXX) $(CXXFLAGS) $(INCS) -o reck.x reck.o $(OBJS) $(LIBS)

.SUFFIXES:
.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCS) -c $<

clean:
	rm -f *.o 
	rm -f *.ll

distclean: clean
	rm -f *.x
	rm -f *.vti

