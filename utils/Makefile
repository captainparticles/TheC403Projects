include ../Makefile.inc

all: prebuild ar

clean:
	@$(RM) $(OBJDIR)\\*
	@$(RM) $(LIBDIR)\\*

prebuild:
	@$(MKDIR) $(OBJDIR) > nul 2>&1 || (exit 0)

utils: squeue.c sstack.c shashmap.c
	$(CC) $(CXXFLAGS) -c squeue.c -o $(OBJDIR)/squeue.o
	$(CC) $(CXXFLAGS) -c sstack.c -o $(OBJDIR)/sstack.o
	$(CC) $(CXXFLAGS) -c shashmap.c -o $(OBJDIR)/shashmap.o
		
ar: utils
	$(AR) rcs $(LIBDIR)/libutils.a $(OBJDIR)/*.o
