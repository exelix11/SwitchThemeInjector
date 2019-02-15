#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

# Also see "include/mbedtls/config.h"

ARCH	:=	-march=armv8-a+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS	:=  -g -Wall -O3 -ffunction-sections $(ARCH) $(DEFINES) 
CFLAGS	+=	-I ../include -D__SWITCH__
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS	:=	-g $(ARCH)
LDFLAGS ?= -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)


CFILES		:=	$(foreach dir,$(CURDIR),$(notdir $(wildcard $(dir)/*.c)))
export OFILES_SRC	:=	$(CFILES:.c=.o)

.SILENT:

.PHONY: all static clean

all: static

static: libmbedtls.a

# crypto
libmbedtls.a: $(OFILES_SRC)
	echo "  AR    $@"
	$(AR) -rc $@ $(OFILES_SRC)
	echo "  RL    $@"
	$(AR) -s $@


.c.o:
	echo "  CC    $<"
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

clean:
	rm -f *.o libmbed*