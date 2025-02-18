CC = gcc
CFLAGS = -lX11 -lpng -lSDL2 -lSDL2_image
SRC = zoomer.c
OUT = zoomer

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS)

clean:
	rm -f $(OUT)
