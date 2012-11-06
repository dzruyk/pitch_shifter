CC = gcc
CFLAGS = -Wall
LIBS = -lasound

OBJS = alsa_parrot.o fft.o pitch_shift.o
TARGET = parrot

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	rm -f *.o
	rm -f $(TARGET)
