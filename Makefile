INC = -I./src
CFLAGS += -O0 -ggdb ${INC}
LDFLAGS += -O0 -ggdb -lm -lIL -l ILU

AUDIO_API = portaudio

CFILES = src/contour.c \
	src/filters.c \
	src/hausdorff.c \
	src/image.c 

TESTFILES = tests/test_frechet.c \
	tests/test_image_filter.c \
	tests/test_image_load.c \
	tests/test_read_write_contour.c \

COBJS = ${CFILES:.c=.o}
TESTOBJS = ${TESTFILES:.c=.o}
OBJS = ${COBJS} ${TESTOBJS}

MAINOBJS = hausdorff.a

.c.o :
	$(CC) $(CFLAGS) -c $< -o $@

all: hausdorff.a

tests: test_frechet test_image_filter test_image_load test_read_write_contour 

test_frechet: hausdorff.a ${TESTOBJS}
	${CC} $(CFLAGS) $(LDFLAGS) tests/test_frechet.c hausdorff.a -o test_frechet

test_image_filter: hausdorff.a ${TESTOBJS}
	${CC} $(CFLAGS) $(LDFLAGS) tests/test_image_filter.c hausdorff.a -o test_image_filter

test_image_load: hausdorff.a ${TESTOBJS}
	${CC} $(CFLAGS) $(LDFLAGS) tests/test_image_load.c hausdorff.a -o test_image_load

test_read_write_contour: hausdorff.a ${TESTOBJS}
	${CC} $(CFLAGS) $(LDFLAGS) tests/test_read_write_contour.c hausdorff.a -o test_read_write_contour

hausdorff.a: ${COBJS}
	${AR} -r $@ ${COBJS}

.PHONY : clean 

clean:
	rm -rf ${OBJS}  
