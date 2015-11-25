include ../../Makefile.inc

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, $(OBJDIR)/%.o, $(wildcard *.c))
TARGETLIB := libbuttcommon.a

all: prebuild ar

clean:
	@$(RM) $(OBJDIR)\\*
	@$(RM) $(LIBDIR)\\*

prebuild:
	@mkdir $(OBJDIR) > nul 2>&1 || (exit 0)
	@mkdir $(LIBDIR) > nul 2>&1 || (exit 0)

$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

ar: $(OBJS)
	$(AR) rcs $(LIBDIR)/$(TARGETLIB) $(OBJDIR)/*.o
