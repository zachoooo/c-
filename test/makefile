TARGET = test-c-
FILES = test.cpp

.PHONY: default
default:
	@echo Debug not selected. Not generating tests.

test-c-: test.cpp
	$(CXX) -g $(FILES) -o $(BUILD)/$(TARGET)

# Recursive portion
SUBDIRS = tiny

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

# End Recursive Portion

.PHONY: debug
debug: subdirs

.PHONY: optimized
optimized:
	@echo Debug not selected. Not generating tests.

.PHONY: all
all: subdirs