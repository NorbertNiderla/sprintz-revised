APPLICATION = app-sprintz-test

RIOTBASE ?= $(CURDIR)/../..

BOARD ?= native

INCLUDES += -I$(RIOTBASE) -I$(RIOTBASE)/sys/include -I$(RIOTBASE)/core/include -I$(RIOTBASE)/drivers/include 

USEMODULE += sprintz

CFLAGS += -Wno-unused-function

DEVELHELP ?= 1
QUIET ?= 1

ifneq (,$(filter sprintz,$(USEMODULE)))
    DIRS += $(RIOTBASE)/sprintz-revised/compression
endif

include $(RIOTBASE)/Makefile.include
include $(RIOTBASE)/sprintz-revised/compression/Makefile.include