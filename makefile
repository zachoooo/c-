export CXX = g++
export BUILD = ${CURDIR}/build
export OBJS = ${CURDIR}/build/objs
export GEN = ${CURDIR}/build/gen
export DEBUG_FLAGS = -g
export OPTIMIZATION_FLAGS = -O1

TARGET = c-
ARCHIVE = $(BUILD)/$(TARGET).tar

.PHONY: default
default: $(TARGET)

.PHONY: debug
debug: $(TARGET)

.PHONY: optimized
optimized: $(TARGET)

.PHONY: all
all: default debug optimized

$(TARGET): builddirs subdirs

.PHONY: builddirs
builddirs: build build/objs build/gen
build:
	mkdir build
build/objs:
	mkdir build/objs
build/gen:
	mkdir build/gen

.PHONY: submit
submit: tar
	curl -F student=suga4198 -F assignment="CS445 F20 Assignment 7" -F "submittedfile=@$(ARCHIVE)" "https://engr-sonic-01.engr.uidaho.edu/cgi-bin/fileCapture.py" --user suga4198

.PHONY: tar
tar: builddirs $(ARCHIVE)
$(ARCHIVE):
	tar cvf $(ARCHIVE) makefile $(SUBDIRS)

.PHONY: clean
clean:
	rm -rf *c- build tm

# Recursive portion
SUBDIRS = lib src test

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

# End Recursive Portion