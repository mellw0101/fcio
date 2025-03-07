/** @file term.c

  @author  Melwin Svensson.
  @date    6-3-2025.

 */
#include "../include/proto.h"


/* ----------------------------- Clearing function's ----------------------------- */

/* Clear from cursor positon to end of screen. */
void clrtoeos(void) {
  writef("\033[0J");
}

/* Clear from cursor position to begining of screen. */
void clrtobos(void) {
  writef("\033[1J");
}

/* Clear entire screen. */
void clrscreen(void) {
  writef("\033[2J");
}

/* ----------------------------- Moving function's ----------------------------- */

/* Move cursor to `row:col` in terminal.  Note that this is 1 based. */
void movecurs(int row, int col) {
  ALWAYS_ASSERT(row > 0);
  ALWAYS_ASSERT(col > 0);
  writef("\033[%d;%dH", row, col);
}

/* Move cursor home `1:1`. */
void mvcurshome(void) {
  writef("\033[H");
}

/* Save the cursor position as the current cursor position. */
void savecurs(void) {
  writef("\0337");
}

/* Restore the cursor position to the last saved position. */
void restcurs(void) {
  writef("\0338");
}

/* Move the cursor to the begining of a previous line `nrows` up. */
void mvcursupbeg(int nrows) {
  ALWAYS_ASSERT(nrows > 0);
  writef("\033[%dF", nrows);
}

/* Move the cursor to the begining of a next line `nrows` down. */
void mvcursdownbeg(int nrows) {
  ALWAYS_ASSERT(nrows > 0);
  writef("\033[%dE", nrows);
}

/* Move cursor `nrows` up.  Note that this does not change the cursor column position. */
void mvcursup(int nrows) {
  ALWAYS_ASSERT(nrows > 0);
  writef("\033[%dA", nrows);
}

/* Move cursor `nrows` down.  Note that this does not change the cursor column position. */
void mvcursdown(int nrows) {
  ALWAYS_ASSERT(nrows > 0);
  writef("\033[%dB", nrows);
}

/* Move cursor `ncols` right. */
void mvcursright(int ncols) {
  ALWAYS_ASSERT(ncols > 0);
  writef("\033[%dC", ncols);
}

/* Move cursor `ncols` left. */
void mvcursleft(int ncols) {
  ALWAYS_ASSERT(ncols > 0);
  writef("\033[%dD", ncols);
}
