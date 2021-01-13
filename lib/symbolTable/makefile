TARGET = symbolTable
FILES = $(TARGET).cpp

.PHONY: default
default: $(TARGET).default.o

.PHONY: debug
debug: $(TARGET).debug.o

.PHONY: optimized
optimized: $(TARGET).default.optimized.o

.PHONY: all
all: $(TARGET).default.o $(TARGET).debug.o $(TARGET).default.optimized.o

$(TARGET).default.o: $(FILES)
	$(CXX) -c $(FILES) -o $(OBJS)/$(TARGET).default.o

$(TARGET).debug.o: $(FILES)
	$(CXX) -c $(FILES) $(DEBUG_FLAGS) -o $(OBJS)/$(TARGET).debug.o

$(TARGET).default.optimized.o: $(FILES)
	$(CXX) -c $(FILES) $(OPTIMIZATION_FLAGS) -o $(OBJS)/$(TARGET).default.optimized.o