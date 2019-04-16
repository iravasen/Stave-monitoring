INSTALLDIR    = /usr/local/bin

ARCH         := $(shell root-config --arch)

CXX           =
IncSuf        = h
ObjSuf        = o
SrcSuf        = cxx
ExeSuf        =
DllSuf        = so
OutPutOpt     = -o 

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs)
ROOTLIBDIR   := $(shell root-config --libdir)


ifeq ($(ARCH),linux)
# Linux with egcs, gcc 2.9x, gcc 3.x (>= RedHat 5.2)
CXX           = g++
CXXFLAGS      = -g -O -Wall -fPIC # -Weffc++
LD            = g++
LDFLAGS       = -g -O
SOFLAGS       = -shared
endif

ifeq (,$(findstring $(ARCH),x8664))
# Linux with egcs, gcc 2.9x, gcc 3.x (>= RedHat 5.2)
CXX           = g++
CXXFLAGS      = -g -O -Wall -fPIC # -Weffc++
LD            = g++
LDFLAGS       = -g -O
SOFLAGS       = -shared
endif



ifeq ($(CXX),)
$(error $(ARCH) invalid architecture)
endif

ALPIDEINCS    = -I$(ALPIDESW)/DataBaseSrc -I$(ALPIDESW)/inc -I$(ALPIDESW)/MosaicSrc/libpowerboard/include -I$(ALPIDESW)/MosaicSrc/libmosaic/include
ALPIDELIBS    = -L$(ALPIDEROOT)/lib -lalpide -lalucms -lpowerboard -lmosaic -lscopecontrol
INCLUDES      = $(ALPIDEINCS) -I/usr/include/libxml2
CXXFLAGS     += $(ROOTCFLAGS) $(INCLUDES)
LIBS          = $(ALPIDELIBS) $(ROOTLIBS) $(SYSLIBS)
GLIBS         = $(ALPIDELIBS) $(ROOTGLIBS) $(SYSLIBS)

#------------------------------------------------------------------------------
THISPROG     := GetFromDatabase
##THISPROGSRC  := $(wildcard *.$(SrcSuf)) DataCompDict.$(SrcSuf)
THISPROGSRC  := $(wildcard *.$(SrcSuf))
THISPROGOBJ  := $(THISPROGSRC:.$(SrcSuf)=.$(ObjSuf))
THISPROGEXE  := $(THISPROG)$(ExeSuf)
THISPROGSO   := $(THISPROG).$(DllSuf)

OBJS          = $(THISPROGOBJ)

#PROGRAMS      = $(THISPROGEXE) $(THISPROGSO)
PROGRAMS      = $(THISPROGEXE)

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf) .$(IncSuf)

.PHONY: all install clean someclean package

all:            $(PROGRAMS)

$(THISPROGEXE):  $(THISPROGOBJ)
		$(LD) $(LDFLAGS) $^ $(GLIBS) -lEG -lHtml -lGeom -lThread -lRMySQL $(OutPutOpt)$@
		@echo "$@ done"

$(THISPROGSO):  $(THISPROGOBJ)
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(EXPLLINKLIBS) $(OutPutOpt)$@
		@echo "$@ done"

install: all
		[ -d $(INSTALLDIR) ] || mkdir -p $(INSTALLDIR)
		@cp -p jtag $(INSTALLDIR)


someclean:
		@rm -f $(OBJS) core

clean:      someclean
		@rm -f $(PROGRAMS) *Dict.* *.def *.exp \
		   *.geom *.root *.ps *.so .def so_locations
		@rm -rf cxx_repository

###

##DataCompDict.$(SrcSuf):  DigiScanLib.h
##		@echo "****** Generating dictionary $@... ******"
##		@rootcint -f $@ -c $^



###

.$(SrcSuf).$(ObjSuf):
		@echo "****** Compiling " $< " ******"
		$(CXX) $(CXXFLAGS) -c $<
