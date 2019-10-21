TARGET   := ticket
SRCS     := $(wildcard *.cc)
LIBS     := ws2_32 wsock32
LIBPATH  :=
INCPATH  := asio

CPP      := gcc -E
CXX      := g++
CPPFLAGS := -D ASIO_STANDALONE
CXXFLAGS := --std=c++17 -O3 -fomit-frame-pointer -mtune=core2 -Wall -pedantic $(addprefix -I,$(INCPATH))
LDFLAGS  := -s -mtune=core2 -Wall -pedantic $(addprefix -L,$(LIBPATH))


.PHONY: all clean test $(TARGET)

all: $(TARGET)

clean:
	rm -f *.d *.i *.o *.exe

test: $(TARGET)
	$(TARGET) 8080 8081

$(TARGET): $(TARGET).exe

$(TARGET).exe: $(filter %.o, $(SRCS:%.cc=%.o))
	$(CXX) $(LDFLAGS) $+ $(addprefix -l, $(LIBS)) -o $@

%.o: %.cc
	$(CXX) -c $(CPPFLAGS) -MMD -MP $(CXXFLAGS) $< -o $@
	@echo "" >> $*.d
	@echo $<: >> $*.d

%.i: %.cc
	$(CPP) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

-include $(filter %.d, $(SRCS:%.cc=%.d))
