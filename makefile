EXE= ma blocksize
LIBS = cma.o libcma.so ma.o libcma.a block.o
SRC = cma.h blocksize.c ma.c cma.c makefile

.PHONY: all
all: ma blocksize
	@echo Done.

ma: ma.o libcma.so
	@echo building executable ma...
	@gcc -L. ma.o -o ma  -lcma

test: ma blocksize
	@echo Running test...
	@./ma
	@./blocksize

blocksize: block.o libcma.so
	@echo Building blocksize...
	@gcc -L. block.o -lcma -o blocksize

libcma.so: cma.o
	@echo building libcma.so...
	@gcc -shared -o libcma.so cma.o

cma.o: cma.c
	@gcc -c cma.c

ma.o: ma.c
	@gcc -c ma.c

block.o : blocksize.c
	@echo Building Object block.o
	@gcc -c blocksize.c -o block.o

.PHONY: dist
dist:
	@tar cf ./dist.tar $(SRC)

.PHONY: clean
clean:
	@rm -f $(LIBS)
	@rm -f $(EXE)
	@echo cleaned.
