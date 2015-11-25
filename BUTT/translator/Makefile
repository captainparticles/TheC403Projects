include ../../Makefile.inc

CCFLAGS := $(CCFLAGS) -Wno-unused-label -Wno-unused-function #-DCOMPILE_VERBOSE

EXE := BUTT.EXE
LIBDIR := $(LIBDIR) -L../../utils/lib -L../Common/lib
LIBS := -L$(LIBDIR) -lutils -lbuttcommon
SRCS := $(wildcard *.c) lex.yy.c translator.tab.c
OBJS := $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

all: prebuild bison flex $(EXE)
	
clean:
	@$(RM) lex.yy.c
	@$(RM) translator.tab.c
	@$(RM) translator.tab.h
	@$(RM) $(OBJDIR)\\*
	@$(RM) $(EXE)

prebuild:
	@mkdir $(OBJDIR) > nul 2>&1 || (exit 0)

bison flex: translator.y translator.l
	bison -d translator.y
	flex translator.l

$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(EXE): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
