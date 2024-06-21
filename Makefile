CC=g++
INC_DIR = .
LIB_DIR = .
CFLAGS:= $(CFLAGS) -O2 -g -Wall -std=c++17
DEPS =  
OBJ = img2lmi.o
LDFLAGS = -lm -lpthread -lpng -lz -ljpeg -Dcimg_use_png
PROGRAMS = img2lmi

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I$(INC_DIR)
img2lmi: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -I$(INC_DIR) -L$(LIB_DIR) $(LDFLAGS)
	

all: img2lmi

clean:
	rm -f *.o *~ img2lmi  

