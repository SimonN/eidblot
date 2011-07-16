# eidblot makefile copied from Lix makefile

CXX      := g++
CXXFLAGS := -s -O2

LD       := g++
LDDIRS   := -L/usr/local/lib
LDALLEG  := $(shell allegro-config --libs)

DEPGEN   := g++ -MM
RM       := rm -rf
MKDIR    := mkdir -p

SRCDIR   := src
OBJDIR   := obj
DEPDIR   := $(OBJDIR)
BINDIR   := bin

CLIENT_BIN  := $(BINDIR)/lix
CLIENT_SRCS := $(wildcard src/*.cpp)
CLIENT_OBJS := $(subst $(SRCDIR)/,$(OBJDIR)/,$(CLIENT_SRCS:%.cpp=%.o))
CLIENT_DEPS := $(subst $(SRCDIR)/,$(DEPDIR)/,$(CLIENT_SRCS:%.cpp=%.d))



###############################################################################

.PHONY: all clean

all: $(CLIENT_BIN) $(SERVER_BIN)

clean:
	$(RM) $(CLIENT_BIN)
	$(RM) $(OBJDIR)

$(CLIENT_BIN): $(CLIENT_OBJS)
	@$(MKDIR) $(BINDIR)
	@echo Linking the game binary \`$(CLIENT_BIN)\'.
	@echo Linker flags: $(LDDIRS) $(LDALLEG) $(LDENET)
	@$(LD) $(LDDIRS) $(LDALLEG) $(LDENET) $(CLIENT_OBJS) -o $(CLIENT_BIN)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(MKDIR) `dirname $@` `dirname $(DEPDIR)/$*.d`
	@echo $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@
	@printf "%s/%s" `dirname $@` "`$(DEPGEN) $<`" > $(DEPDIR)/$*.d

-include $(CLIENT_DEPS)
