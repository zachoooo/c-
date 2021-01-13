TARGET = semantic
FILES = $(TARGET).cpp
INCLUDE_FLAGS =  -I../../lib/yyerror -I../TokenTree -I../../lib/symbolTable

.PHONY: default
default: $(TARGET).default.o

.PHONY: debug
debug: $(TARGET).debug.o

.PHONY: optimized
optimized: $(TARGET).default.optimized.o

.PHONY: all
all: $(TARGET).default.o $(TARGET).debug.o $(TARGET).default.optimized.o

$(TARGET).default.o: $(FILES)
	$(CXX) -c $(FILES) $(INCLUDE_FLAGS) -o $(OBJS)/$(TARGET).default.o

$(TARGET).debug.o: $(FILES)
	$(CXX) -c $(FILES) $(INCLUDE_FLAGS) $(DEBUG_FLAGS) -o $(OBJS)/$(TARGET).debug.o

$(TARGET).default.optimized.o: $(FILES)
	$(CXX) -c $(FILES) $(INCLUDE_FLAGS) $(OPTIMIZATION_FLAGS) -o $(OBJS)/$(TARGET).default.optimized.o