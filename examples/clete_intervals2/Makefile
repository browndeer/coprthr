NAME = clete_clmulti_array_example

prefix=/usr/local/browndeer
exec_prefix=${prefix}

OPENCL_INCS = -I/home/richie/disk0.richie/bdt-code/current/coprthr/include
OPENCL_LIBS = -L/home/richie/disk0.richie/bdt-code/current/coprthr/src/libocl -locl

STDCL_INCS = -I${prefix}/include
STDCL_LIBS = -L${exec_prefix}/lib -lstdcl

INCS = $(OPENCL_INCS) $(STDCL_INCS)
LIBS = $(OPENCL_LIBS) $(STDCL_LIBS)

CXXFLAGS += -O0

OBJS = Timer.o

all: $(NAME).x

$(NAME).x: $(NAME).o $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) -o $(NAME).x $(NAME).o $(OBJS) $(LIBS)

main_test.x: main_test.o 
	$(CXX) $(CXXFLAGS) $(INCS) -o main_test.x main_test.o $(LIBS) -lclelf

.SUFFIXES:
.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCS) -c $<

clean:
	rm -f *.o
	rm -f coprthr.autokern.log.*

distclean: clean
	rm -f *.x

