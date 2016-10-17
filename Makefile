CONTIKI_PROJECT = smart-farming
APPS+=smart-farming
all: $(CONTIKI_PROJECT)

CONTIKI = ../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
