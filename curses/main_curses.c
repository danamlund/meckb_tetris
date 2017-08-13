/* Copyright 2017 Dan Amlund Thomsen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>

#include "tetris_text.h"

static WINDOW *mainwin;

static int idx = 0;
static char area[10000] = { 0 };

void initlog() {
  FILE *f = fopen("log.txt", "w");
  fclose(f);
}
void d(char *format, ...) {
  FILE *f = fopen("log.txt", "a");

  va_list arglist;
  va_start( arglist, format );
  vfprintf( f, format, arglist );
  va_end( arglist );
  
  fprintf(f, "\n");
  fclose(f);
}
void drow(char *msg, const char row[5]) {
  FILE *f = fopen("log.txt", "a");
  fprintf(f, "%s: %d%d%d%d%d%d%d\n", msg,
          row[0], row[1], row[2], row[3], row[4], row[5], row[6]);
  fclose(f);
}
void dpiece(char piece[7][7]) {
  FILE *f = fopen("log.txt", "a");
  for (int y = 0; y < 7; y++) {
    fprintf(f, "  ");
    for (int x = 0; x < 7; x++) {
      if (piece[y][x]) {
        fprintf(f, "x");
      } else {
        fprintf(f, ".");
      }
    }
    fprintf(f, "\n");
  }
  fclose(f);
}
void dboard(char board[20][10]) {
  FILE *f = fopen("log.txt", "a");
  for (int y = 0; y < 20; y++) {
    fprintf(f, "  ");
    for (int x = 0; x < 10; x++) {
      if (board[y][x]) {
        fprintf(f, "x");
      } else {
        fprintf(f, ".");
      }
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

static void sleepms(int ms) {
  usleep(ms * 1000);
}

static void die(char *msg) {
  mvaddstr(0, 15, msg);
  refresh();
  while (true);

  delwin(mainwin);
  endwin();
  refresh();
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

static int getx() {
  int x = 0;
  for (int i = idx - 1; i >= 0; i--) {
    if (area[i] == '\n') {
      break;
    }
    x++;
  }
  return x;
}

static int gety() {
  int y = 0;
  for (int i = idx; i >= 0; i--) {
    if (area[i] == '\n') {
      y++;
    }
  }
  return y;
}

static void redraw() {
  sleepms(10);
  /* sleepms(500); */
  /* d("%s<%d %d,%d", area, idx, getx(), gety()); */

  erase();
  int y = 0;
  move(y, 0);
  for (int i = 0; area[i] != 0; i++) {
    if (area[i] == '\n') {
      y++;
      move(y, 0);
    } else {
      addch(area[i]);
    }
  }
  
  move(gety(), getx());
  refresh();
}

static void add_space(int len) {
  for (int i = strlen(area); i >= idx; i--) {
    area[i + len] = area[i];
  }
}
static void remove_space(int len) {
  for (int i = idx; area[i] != 0; i++) {
    area[i] = area[i + len];
  }
}

void tetris_send_up() {
  int x = getx();

  if (idx <= x) {
    die("too much up");
  }
  
  idx -= x; // to end line
  idx--; // to end of prev line
  idx -= getx(); // to start of prev line
  idx += x; // to same x as old line
  redraw();
}

void tetris_send_left() {
  if (idx <= 0) {
    die("too much left");
  }
  idx--;
  redraw();
}

void tetris_send_down() {
  int x = getx();

  while (area[idx] != '\n' && area[idx] != 0) {
    idx++;
  }

  if (area[idx] == 0) {
    die("too much down");
  }

  idx++; // to next line

  // move to same x or end of line
  for (int i = 0; i < x && area[idx] != '\n' && area[idx]; i++) {
    idx++;
  }
  
  redraw();
}

void tetris_send_right() {
  if (area[idx] == 0) {
    die("too much right");
  }
  if (area[idx] == '\n') {
    die("right to move to next line");
  }
  idx++;
  redraw();
}

void tetris_send_home() {
  idx -= getx();
  redraw();
}
void tetris_send_end() {
  while (area[idx] != '\n' && area[idx] != 0) {
    idx++;
  }
  redraw();
}

void tetris_send_backspace() {
  if (idx == 0) {
    die("too much backspace");
  }
  idx--;
  remove_space(1);
  redraw();
}
void tetris_send_delete() {
  remove_space(1);
  redraw();
}

static void send_char(char c) {
  add_space(1);
  area[idx] = c;
  idx++;
  redraw();
}

void tetris_send_newline() {
  send_char('\n');
}

void tetris_send_string(const char *s) {
  for (int i = 0; s[i] != 0; i++) {
    send_char(s[i]);
  }
}

int tetris_get_keypress() {
  switch (getch()) {
  case KEY_UP: return 1;
  case KEY_LEFT: return 2;
  case KEY_DOWN: return 3;
  case KEY_RIGHT: return 4;
  }
  return 0;
}

int main(int argc, char **args) {
  initlog();
  if ( (mainwin = initscr()) == NULL ) {
    fprintf(stderr, "Error initializing ncurses.\n");
    exit(EXIT_FAILURE);
  }

  cbreak();
  noecho();
  nodelay(mainwin, 1);
  keypad(mainwin, 1);

  tetris_start(42);

  while (1) {
    tetris_tick(100);
    sleepms(100);
  }

  delwin(mainwin);
  endwin();
  refresh();

  return 0;
}
