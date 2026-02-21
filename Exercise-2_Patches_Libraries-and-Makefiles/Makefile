# Target to create the final static library
# It depends on mean.o, if that changes, library rebuilds
libmath.a: mean.o
	ar cr libmath.a mean.o

# Target to compile the source code into an object file
# Depends on both mean.c and mean.h, if either file is changed, the rule triggers.
mean.o: mean.c mean.h
	cc -c mean.c

# Clean rule to easily reset your directory
clean:
	rm -f *.o libmath.a
