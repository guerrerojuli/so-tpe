CC=gcc
CFLAGS=-Wall -Wno-incompatible-library-redeclaration -std=c11
TARGET=test_mm

all: test-firstfit

test-firstfit:
	@echo "Compiling with FIRSTFIT..."
	$(CC) $(CFLAGS) -DFIRSTFIT \
		test_mm.c \
		-o $(TARGET)_firstfit

test-buddy:
	@echo "Compiling with BUDDY..."
	$(CC) $(CFLAGS) -DBUDDY \
		test_mm.c \
		-o $(TARGET)_buddy

run-firstfit: test-firstfit
	@echo "\n=========================================="
	@echo "Running tests with FIRSTFIT"
	@echo "=========================================="
	./$(TARGET)_firstfit

run-buddy: test-buddy
	@echo "\n=========================================="
	@echo "Running tests with BUDDY"
	@echo "=========================================="
	./$(TARGET)_buddy

test-all: run-firstfit run-buddy
	@echo "\nAll tests completed!"

clean:
	rm -f $(TEST_OBJ) $(MM_OBJ) $(LIB_OBJ)
	rm -f $(TARGET)_firstfit $(TARGET)_buddy

.PHONY: all test-firstfit test-buddy run-firstfit run-buddy test-all clean
