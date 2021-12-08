include $(PCF_DEV_HOME)/makehead

EXE       =LMMain
SRCS      =$(wildcard *.cpp)
HEADERS   =$(filter-out ,$(wildcard *.hpp))
OBJS      =$(SRCS:.cpp=.o)
OPTIMIZER += -Wno-write-strings
PCF_LIB  += -lPDB
#LDFLAGS 	+= -ltcmalloc


all :: $(EXE) 

$(EXE) : $(OBJS) $(HEADERS)
	$(CXX) -o $(EXE) $(OBJS) $(LDFLAGS) 
	$(CP) $(EXE) $(PCF_PKG_HOME)/BIN/


install:
	@if [ ! -d "$(PCF_PKG_HOME)" ]; then mkdir -p "$(PCF_PKG_HOME)";  fi
	$(CP) $(EXE) $(PCF_PKG_HOME)/BIN/


clean:
	$(RM) $(OBJS) $(EXE) core.* *.d

DEPS := $(OBJS:.o=.d)

-include $(DEPS)
