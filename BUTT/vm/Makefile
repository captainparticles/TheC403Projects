include ../../Makefile.inc

CCFLAGS := $(CCFLAGS) -Wno-unused-label -Wno-unused-function #-DCOMPILE_VERBOSE

EXE := BUTVM.EXE
LIBDIR := $(LIBDIR) -L../../utils/lib -L../Common/lib
LIBS := -L$(LIBDIR) -lutils -lbuttcommon
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

all: prebuild $(EXE)
	
clean:
	@$(RM) $(OBJDIR)\\*
	@$(RM) $(EXE)

prebuild:
	@mkdir $(OBJDIR) > nul 2>&1 || (exit 0)

$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(EXE): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
