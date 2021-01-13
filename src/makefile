TARGET = ../c-
DEBUG_TARGET = ../debug-c-
OPTIMIZED_TARGET = ../optimized-c-
FLAGS = -lm -ITokenTree -Isemantic -I../lib/ourgetopt -I../lib/symbolTable -I../lib/yyerror -I../lib/emitcode

$(TARGET): subdirs
	$(CXX) main.cpp $(FLAGS) $(OBJS)/*.default.o -o $(TARGET)

$(DEBUG_TARGET): subdirs
	$(CXX) main.cpp $(FLAGS) $(OBJS)/*.debug.o -o $(DEBUG_TARGET)

$(OPTIMIZED_TARGET): subdirs
	$(CXX) main.cpp $(FLAGS) $(OBJS)/*.debug.o -o $(OPTIMIZED_TARGET)

# Recursive portion
SUBDIRS = TokenTree parser scanner semantic codegen utils

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

# End Recursive Portion

.PHONY: default
default: subdirs $(TARGET)

.PHONY: debug
debug: subdirs $(DEBUG_TARGET)

.PHONY: optimized
optimized: subdirs $(OPTIMIZED_TARGET)

.PHONY: all
all: subdirs default debug optimized