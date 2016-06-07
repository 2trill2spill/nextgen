# Set the root directory path, ie the path to the nextgen directory.
ROOT_DIR = $(shell pwd)

CK = ck-0.5.1
LIBRESSL = libressl-2.4.1
CAPSTONE = capstone-3.0.4

TEST_LIB = $(ROOT_DIR)/src/io/libnxio.dylib \
           $(ROOT_DIR)/src/memory/libnxmemory.dylib \
           $(ROOT_DIR)/src/utils/libnxutils.dylib

INCLUDE = -isystem deps/$(CAPSTONE)/include \
          -isystem deps/$(CK)/include \
          -Isrc

OPERATING_SYSTEM = $(shell uname)

# FreeBSD build options.
ifeq ($(OPERATING_SYSTEM), FreeBSD)

SOURCES = src/main.c

PLATFORM = -DFREEBSD

CC = clang

LIB = -rpath src/runtime -rpath src/memory -rpath src/utils \
      -rpath src/io -rpath src/concurrent -rpath src/genetic \
      -rpath src/mutate -rpath src/log -rpath src/network \
      -rpath src/file -rpath src/syscall -rpath src/probe \
      -rpath src/disas -rpath src/crypto -rpath src/plugins \
      -rpath src/resource src/memory/libnxmemory.so \
      src/io/libnxio.so src/runtime/libnxruntime.so \
      src/utils/libnxutils.so \
      src/crypto/libnxcrypto.so \
      src/syscall/libnxsyscall.so \
      deps/$(LIBRESSL)/crypto/.libs/libcrypto.so \
      deps/$(CK)/src/libck.so \
     -l dtrace -l proc -l ctf -l elf -l z -l rtld_db -l pthread -l util

INCLUDE += -I /usr/src/cddl/compat/opensolaris/include \
	       -I /usr/src/cddl/contrib/opensolaris/lib/libdtrace/common/ \
	       -I /usr/src/sys/cddl/compat/opensolaris \
	       -I /usr/src/sys/cddl/contrib/opensolaris/uts/common/

MAKE = gmake

FLAGS = -DFREEBSD -Wall -Werror -Weverything -g -O3 -std=c99

SILENCED_WARNINGS = -Wno-sign-conversion -Wno-unknown-pragmas -Wno-pedantic -Wno-padded 

endif

# Mac OSX build options.
ifeq ($(OPERATING_SYSTEM), Darwin)

SOURCES = src/main.m

PLATFORM = -DMAC_OSX

CC = clang

MAKE = make

LIB = src/runtime/libnxruntime.dylib src/memory/libnxmemory.dylib \
      src/utils/libnxutils.dylib src/io/libnxio.dylib src/objc/libnxobjcutils.dylib

FLAGS = -DMAC_OSX -fno-strict-aliasing -Wall -Werror -Weverything -pedantic -g -O3 -std=c99

SILENCED_WARNINGS = -Wno-unknown-warning-option \
                    -Wno-reserved-id-macro \
                    -Wno-incompatible-pointer-types-discards-qualifiers \
                    -Wno-used-but-marked-unused

BUILD_NX_LIBS = cd $(ROOT_DIR)/src/objc && $(MAKE) &&
CLEAN_NX_LIBS = cd $(ROOT_DIR)/src/objc && $(MAKE) clean &&

endif

ifeq ($(OPERATING_SYSTEM), Linux)

SOURCES = src/main.c

PLATFORM = -DLINUX

CC = gcc

LIB = -Wl,-rpath=src/runtime,-rpath=src/memory,-rpath=src/utils,-rpath=src/io,-rpath=src/concurrent,-rpath=src/genetic,-rpath=src/mutate,-rpath=src/log,-rpath=src/network,-rpath=src/file,-rpath=src/syscall,-rpath=src/probe,-rpath=src/disas,-rpath=src/crypto,-rpath=src/plugins \
      src/memory/libnxmemory.so \
      src/io/libnxio.so \
      src/runtime/libnxruntime.so \
      src/utils/libnxutils.so \
      src/crypto/libnxcrypto.so \
      src/syscall/libnxsyscall.so \
      deps/$(LIBRESSL)/crypto/.libs/libcrypto.so \
      deps/$(CK)/src/libck.so \
      deps/sqlite/sqlite3.so \
      -lpthread \
      -ldl

MAKE = make

FLAGS = -DLINUX -DCOMMON -Wall -Werror -Wextra -g -O3 -std=c99

endif

PROG = nextgen

BUILD_NX_LIBS += cd $(ROOT_DIR)/src/io && $(MAKE) && \
                 cd $(ROOT_DIR)/src/memory && $(MAKE) && \
                 cd $(ROOT_DIR)/src/concurrent && $(MAKE) && \
                 cd $(ROOT_DIR)/src/crypto && $(MAKE) && \
                 cd $(ROOT_DIR)/src/utils && $(MAKE) && \
                 cd $(ROOT_DIR)/src/probe && $(MAKE) && \
                 cd $(ROOT_DIR)/src/network && $(MAKE) && \
                 cd $(ROOT_DIR)/src/plugins && $(MAKE) && \
                 cd $(ROOT_DIR)/src/mutate && $(MAKE) && \
                 cd $(ROOT_DIR)/src/resource && $(MAKE) && \
                 cd $(ROOT_DIR)/src/log && $(MAKE) && \
                 cd $(ROOT_DIR)/src/syscall && $(MAKE) && \
                 cd $(ROOT_DIR)/src/genetic && $(MAKE) && \
                 cd $(ROOT_DIR)/src/file && $(MAKE) && \
                 cd $(ROOT_DIR)/src/disas && $(MAKE) && \
                 cd $(ROOT_DIR)/src/runtime && $(MAKE)

CLEAN_NX_LIBS += cd $(ROOT_DIR)/src/io && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/memory && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/concurrent && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/crypto && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/utils && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/probe && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/network && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/plugins && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/mutate && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/resource && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/log && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/syscall && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/genetic && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/file && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/disas && $(MAKE) clean && \
                 cd $(ROOT_DIR)/src/runtime && $(MAKE) clean

TEST_SUITE = cd $(ROOT_DIR)/tests/memory && $(MAKE) && \
             cd $(ROOT_DIR)/tests/crypto && $(MAKE) && \
             cd $(ROOT_DIR)/tests/plugin && $(MAKE) && \
             cd $(ROOT_DIR)/tests/utils && $(MAKE) && \
	         cd $(ROOT_DIR)/tests/genetic && $(MAKE) && \
	         cd $(ROOT_DIR)/tests/resource && $(MAKE) && \
	         cd $(ROOT_DIR)/tests/syscall && $(MAKE) && \
	         cd $(ROOT_DIR)/tests/concurrent && $(MAKE) && \
	         cd $(ROOT_DIR)/tests/file && $(MAKE) && \
	         cd $(ROOT_DIR)/tests/network && $(MAKE) && \
	         cd $(ROOT_DIR)/tests/generate && $(MAKE) && \
	         cd $(ROOT_DIR)/tests/runtime && $(MAKE) 

CLEAN_SUITE = cd $(ROOT_DIR)/tests/crypto && $(MAKE) clean && \
              cd $(ROOT_DIR)/tests/plugin && $(MAKE) clean && \
              cd $(ROOT_DIR)/tests/utils && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/memory && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/genetic && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/resource && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/syscall && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/concurrent && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/file && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/network && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/generate && $(MAKE) clean && \
	          cd $(ROOT_DIR)/tests/runtime && $(MAKE) clean

TEST_DEPS =  cd deps/$(LIBRESSL) && $(MAKE) check && \
	         cd deps/$(CK) && $(MAKE) check && \
	         cd deps/$(CAPSTONE) && $(MAKE) check

export CK LIBRESSL CAPSTONE ROOT_DIR PLATFORM FLAGS SILENCED_WARNINGS

all:

	$(BUILD_NX_LIBS)

	$(CC) $(FLAGS) $(SOURCES) $(LIB) $(INCLUDE) $(SILENCED_WARNINGS) -o $(PROG)

test-quick:

	./test_suite

test:

	$(TEST_DEPS)
	./test_suite

test-runtime:

	./test_suite runtime

test-syscall:

	./test_suite syscall

test-genetic:

	./test_suite genetic

test-resource:

	./test_suite resource

test-memory:

	./test_suite memory

test-concurrent:

	./test_suite concurrent

clean-lib:

	rm -rf src/runtime/libnxruntime.dylib.dSYM
	rm -rf src/io/libnxio.dylib.dSYM
	rm -rf src/objc/libnxobjcutils.dylib.dSYM
	rm -rf src/memory/libnxmemory.dylib.dSYM
	rm -rf src/concurrent/libnxconcurrent.dylib.dSYM
	rm -rf src/crypto/libnxcrypto.dylib.dSYM 
	rm -rf src/probe/libnxprobe.dylib.dSYM 
	rm -rf src/network/libnxnetwork.dylib.dSYM 
	rm -rf src/plugins/libnxplugin.dylib.dSYM 
	rm -rf src/mutate/libnxmutate.dylib.dSYM 
	rm -rf src/resource/libnxresource.dylib.dSYM
	rm -rf src/log/libnxlog.dylib.dSYM 
	rm -rf src/syscall/libnxsyscall.dylib.dSYM
	rm -rf src/genetic/libnxgenetic.dylib.dSYM 
	rm -rf src/file/libnxfile.dylib.dSYM
	rm -rf src/disas/libnxdisas.dylib.dSYM

	rm -f src/runtime/libnxruntime.dylib
	rm -f src/io/libnxio.dylib
	rm -f src/objc/libnxobjcutils.dylib
	rm -f src/memory/libnxmemory.dylib
	rm -f src/concurrent/libnxconcurrent.dylib
	rm -f src/crypto/libnxcrypto.dylib 
	rm -f src/utils/libnxutils.dylib
	rm -f src/probe/libnxprobe.dylib
	rm -f src/network/libnxnetwork.dylib 
	rm -f src/plugins/libnxplugin.dylib
	rm -f src/mutate/libnxmutate.dylib 
	rm -f src/resource/libnxresource.dylib 
	rm -f src/log/libnxlog.dylib
	rm -f src/syscall/libnxsyscall.dylib
	rm -f src/genetic/libnxgenetic.dylib
	rm -f src/file/libnxfile.dylib 
	rm -f src/disas/libnxdisas.dylib

install:

	cp -rf src/runtime/libnxruntime.dylib.dSYM /usr/local/lib
	cp -rf src/io/libnxio.dylib.dSYM /usr/local/lib
	cp -rf src/objc/libnxobjcutils.dylib.dSYM /usr/local/lib
	cp -rf src/memory/libnxmemory.dylib.dSYM /usr/local/lib
	cp -rf src/concurrent/libnxconcurrent.dylib.dSYM /usr/local/lib
	cp -rf src/crypto/libnxcrypto.dylib.dSYM /usr/local/lib 
	cp -rf src/probe/libnxprobe.dylib.dSYM /usr/local/lib 
	cp -rf src/network/libnxnetwork.dylib.dSYM /usr/local/lib 
	cp -rf src/plugins/libnxplugin.dylib.dSYM /usr/local/lib 
	cp -rf src/mutate/libnxmutate.dylib.dSYM /usr/local/lib 
	cp -rf src/resource/libnxresource.dylib.dSYM /usr/local/lib 
	cp -rf src/log/libnxlog.dylib.dSYM /usr/local/lib 
	cp -rf src/syscall/libnxsyscall.dylib.dSYM /usr/local/lib 
	cp -rf src/genetic/libnxgenetic.dylib.dSYM /usr/local/lib 
	cp -rf src/file/libnxfile.dylib.dSYM /usr/local/lib 
	cp -rf src/disas/libnxdisas.dylib.dSYM /usr/local/lib 

	cp -f src/runtime/libnxruntime.dylib /usr/local/lib
	cp -f src/io/libnxio.dylib /usr/local/lib
	cp -f src/objc/libnxobjcutils.dylib /usr/local/lib
	cp -f src/memory/libnxmemory.dylib /usr/local/lib
	cp -f src/concurrent/libnxconcurrent.dylib /usr/local/lib
	cp -f src/crypto/libnxcrypto.dylib /usr/local/lib 
	cp -f src/utils/libnxutils.dylib /usr/local/lib
	cp -f src/probe/libnxprobe.dylib /usr/local/lib 
	cp -f src/network/libnxnetwork.dylib /usr/local/lib 
	cp -f src/plugins/libnxplugin.dylib /usr/local/lib 
	cp -f src/mutate/libnxmutate.dylib /usr/local/lib 
	cp -f src/resource/libnxresource.dylib /usr/local/lib 
	cp -f src/log/libnxlog.dylib /usr/local/lib 
	cp -f src/syscall/libnxsyscall.dylib /usr/local/lib 
	cp -f src/genetic/libnxgenetic.dylib /usr/local/lib 
	cp -f src/file/libnxfile.dylib /usr/local/lib 
	cp -f src/disas/libnxdisas.dylib /usr/local/lib 
	cp -f deps/$(CAPSTONE)/libcapstone.dylib /usr/local/lib
	cp -f deps/$(CK)/src/libck.so /usr/local/lib/libck.0.dylib

	cp -f src/runtime/runtime.h /usr/local/include
	cp -f src/io/io.h /usr/local/include
	cp -f src/utils/utils.h /usr/local/include
	cp -f src/memory/memory.h /usr/local/include
	cp -f src/concurrent/concurrent.h /usr/local/include
	cp -f src/crypto/crypto.h /usr/local/include

build-test:

	$(TEST_SUITE)
	$(CC) $(FLAGS) $(TEST_LIB) -Wno-sign-conversion tests/tests.c tests/test_utils.c $(INCLUDE) $(SILENCED_WARNINGS) -o test_suite

clean-test:

	$(CLEAN_SUITE)

clean:

	cd deps/$(LIBRESSL) && $(MAKE) clean;
	cd deps/$(CAPSTONE) && $(MAKE) clean;
	cd deps/$(CK) && $(MAKE) clean;
	$(CLEAN_NX_LIBS)
	rm -rf *dSYM
	rm -rf nextgen
	rm -rf test_suite


