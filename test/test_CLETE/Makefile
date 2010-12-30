
#INCS = -I./ -I/usr/local/atistream/include -I/usr/local/browndeer/include -I/disk0/richie/projects/vector/pete-2.1.0/src
INCS = -I./ -I/usr/local/atistream/include -I/usr/local/browndeer/include 
LIBS = -L/usr/local/atistream/lib -lOpenCL -L/usr/local/browndeer/lib -lstdcl_d

all: test_clvector_clete

test_clvector_clete: test_clvector_clete.cpp 
	$(CXX) $(INCS) -o test_clvector_clete $(CXXINC) $(CXXARG) test_clvector_clete.cpp $(LIBS)

clean:
	rm -f test_clvector_clete

