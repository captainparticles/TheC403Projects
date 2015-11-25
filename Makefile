include Makefile.inc

SUBDIRS := shared utils
SUBCLEAN := $(addsuffix .clean, $(SUBDIRS))

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) $(MAKEFLAGS) -C $@ all
	
.PHONY: clean $(SUBCLEAN)

clean: $(SUBCLEAN)

$(SUBCLEAN): %.clean:
	$(MAKE) $(MAKEFLAGS) -C $* clean
