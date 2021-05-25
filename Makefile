PROJ=flemon
CPPFIOPATH=cppfio
CPPFIO=cppfio
OPENEDCLPATH=openedcl
LDFLAGS=-L$(CPPFIOPATH)  -L$(OPENEDCLPATH)
CXXFLAGS= -Wall  -ggdb3 -I$(CPPFIOPATH) -I$(OPENEDCLPATH)
LDLIBS=-lreadline -lcppfio -lpcre  -lelf -lopenedcl
PKG=flemon-0.3-2
.PHONY: cppfio

all : $(PROJ) flemonspy

flemonspy: flemonspy.c
	gcc -Wall -g3 -o flemonspy flemonspy.c

flash.o : flash.cc flash.h
fl_readline.o:  fl_readline.cc
grlib.o: grlib.cc grlib.h

$(PROJ).o: $(PROJ).cc  $(PROJ).h
	$(CXX) $(CXXFLAGS)  -c  flemon.cc

$(PROJ): $(PROJ).o flash.o fl_readline.o grlib.o $(CPPFIO)/libcppfio.a $(OPENEDCLPATH)/libopenedcl.a
	$(CXX)  -o $@  $(CXXFLAGS) $(PROJ).o fl_readline.o  flash.o grlib.o $(LDFLAGS) $(LDLIBS) `cppfio/cppfio_config --dlibs`

$(OPENEDCLPATH)/libopenedcl.a :   $(OPENEDCLPATH)/grlib_edcl.h $(OPENEDCLPATH)/grlib_edcl.cc

cppfio:
	$(MAKE) -C $(CPPFIO)

$(CPPFIO)/libcppfio.a:
	$(MAKE) -C $(CPPFIO)

$(OPENEDCLPATH)/libopenedcl.a: $(OPENEDCLPATH)/grlib_edcl.h $(OPENEDCLPATH)/grlib_edcl.cc
	$(MAKE) -C $(OPENEDCLPATH)


debian/control: debian/packages debian/yada
	debian/yada rebuild control

debian/rules: debian/packages   debian/yada
	debian/yada rebuild rules

deb: clean debian/control debian/rules
	$(MAKE) all
	fakeroot dpkg-buildpackage -b -uc

tarball: clean
	mkdir $(PKG)
	mkdir $(PKG)/doc
	cp -rL $(CPPFIO) debian $(OPENEDCLPATH) $(PKG)
	cp *.c *.cc *.h config devices Makefile   $(PKG)
	cp doc/readme.txt doc/README.Debian   $(PKG)/doc/
	rm  -f $(PKG)/$(CPPFIO)/lista $(PKG)/parser.cc
	cp /usr/share/common-licenses/GPL-2 $(PKG)/LICENSE
	rm -rf `find $(PKG) -name CVS`
	rm -rf `find $(PKG) -name "*.bkp"`
	tar jcvf $(PKG).tar.bz2 $(PKG)
	rm -rf $(PKG)

clean:
	rm -f $(PROJ) parser flemonspy *.o
	rm -f debian/build-* debian/control debian/files debian/packages-tmp debian/packages-tmp-include
	rm -rf debian/rules debian/substvars debian/tmp-flemon
	$(MAKE) -C $(CPPFIO) clean
	$(MAKE) -C $(OPENEDCLPATH) clean
