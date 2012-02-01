NAME = clmulti_array_example

OPENCL_INC_PATH = /usr/local/atistream/include
OPENCL_LIB_PATH = /usr/local/atistream/lib/x86_64

STDCL_INC_PATH = /usr/local/browndeer/include
STDCL_LIB_PATH = /usr/local/browndeer/lib

INCS = -I$(OPENCL_INC_PATH) -I$(STDCL_INC_PATH)
LIBS = -L$(OPENCL_LIB_PATH) -lOpenCL -L$(STDCL_LIB_PATH) -lstdcl

CXXFLAGS += -O3

OBJS = 

all: $(NAME).x

$(NAME).x: $(NAME).o $(OBJS) ocl.o
	$(CXX) $(CXXFLAGS) $(INCS) -o $(NAME).x $(NAME).o $(OBJS) ocl.o $(LIBS)

ocl.o: matvecmult.cl
	clcc -o ocl.o matvecmult.cl

.SUFFIXES:
.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCS) -c $<

clean:
	rm -f *.o *.x
