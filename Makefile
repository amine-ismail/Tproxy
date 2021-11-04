
#SYSROOT=../android-ndk-r10e/platforms/android-9/arch-arm/
#CCCMD=../android/toolchain/bin/arm-linux-androideabi-gcc
#LIB_EV_DIR=../libev-4.19/


#LIB_EV =$(LIB_EV_DIR)/.libs/libev.a
LIBS = $(LIB_EV) 
CFLAGS   = -Wall 

CC =$(CCCMD) --sysroot=$(SYSROOT)
CC = cc
#---------- Customizable Flags -------------#
CPPFLAGS = -fno-strict-aliasing -I. 

#---------- Usual flags -----------#

REL      ?= release
OBJ_DIR   = obj
BIN_DIR   = .
LIB_DIR   = 
INC_DIR   = 
LDFLAGS  = -static -lev -lm
ifeq ($(REL),debug)
	CPPFLAGS+=-g -DDEBUG
endif

ifeq ($(REL),release)
	CPPFLAGS+=-O3
endif


.PHONY: all clean tags

#----------- Rules (You can keep these) ----------#
# Build objects
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ 
# Copy target includes (local includes start here !)
$(INC_DIR)/%.h: %.h
	cp -f $< $@
# Copy target libraries
$(LIB_DIR)/%.a: %.a
	cp -f $< $@
$(LIB_DIR)/%.so: %.so
	cp -f $< $@
# Copy target binaries
$(BIN_DIR)/%: %
	cp -f $< $@

#----------- Describe your dependencies here -------#

#--- First target
TGT1= $(BIN_DIR)/proxy
SRC1= main.c callback.c net.c connection.c log.c proxy.c daemon.c readconf.c commandLine.c

#--- All target
TARGET = $(TGT1)

all: dirs $(TARGET) tags

$(TGT1): $(SRC1:%.c=$(OBJ_DIR)/%.o) $(LIBS)
	$(CC) -o $@ $^ $(LDFLAGS)

dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

clean:
	rm -f *~ $(OBJ_DIR)/*.o TAGS

distclean clobber: clean
	rm -rf $(TARGET) $(OBJ_DIR)

tags :
	etags *.c *.h
