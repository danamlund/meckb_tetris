ARGS=-Wall -lncurses -O3 -pipe -std=gnu99 -g

KEYMAPDIR=../tetris/keymaps/default

tetris_text: ${KEYMAPDIR}/tetris_text.h ${KEYMAPDIR}/tetris_text.c main_curses.c
	gcc -I${KEYMAPDIR} main_curses.c ${KEYMAPDIR}/tetris_text.c $(ARGS) -o tetris_text
