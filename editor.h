#pragma once

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
// for "int access(const char *pathname, int mode);"
#include <unistd.h>  

// #include <stdarg.h>

// #include <fcntl.h>
// #include <sys/stat.h>

#define ESC             "\033"

#ifndef CSI
#define CSI              ESC"["
#endif

#define BLACK_TEXT       CSI"30m"
#define WHITE_TEXT       CSI"37m"
#define BLUE_BACK        CSI"44m"
#define WHITE_BACK       CSI"47m"
#define DEFAULT          CSI"0m"
#define BOLD             CSI"1m"
#define INVERSE          CSI"7m"

#ifdef COLOR
#define TEXT_COLOR       CSI"44;37;1m"
#define SELECT_COLOR     CSI"47;37;1m"
#define STATUS_COLOR     CSI"0;47;30m"  
#else
#define TEXT_COLOR       CSI"0m"
#define SELECT_COLOR     CSI"7;1m"
#define STATUS_COLOR     CSI"1;7m"
#endif

#define MINEXTEND         32768

#ifndef TABSIZE
#define TABSIZE           8
#endif


#define CLREOL           "\033[K"


/**
 * Undo structure to save undo stack
 **/

struct undo {
  int pos;                 // Editor position
  int erased;              // Size of erased contents
  int inserted;            // Size of inserted contents
  char *undobuf;  // Erased contents for undo
  char *redobuf;  // Inserted contents for redo
  struct undo *next;       // Next undo buffer
  struct undo *prev;       // Previous undo buffer
};

class EnvEdit;

/**
 * Class Editor implements all editor's functions and internals.
 **/
class Editor {
public:

//
// Editor buffer functions
//

/**
 * Constructor using the env.
 * @param aEnv: A reference to the current environement.
 **/
  Editor(EnvEdit& aEnv) : 
    search(NULL),
    env(aEnv) {}

/**
 * Destructor, free the general buffer & undos stack.
 **/
  ~Editor() {
    if (start) free(start);
    clear_undo();
  }

/**
 * Clear the dirty flag.
 **/
  void clearDirty() {
    dirty = 0;
  }

// /**
//  * Getter for dirty flag.
//  **/
//   int dirty() const {
//     return _dirty;
//   }

/**
 * getter for local file name.
 * @return pointer to the local filename.
 **/
  const char* getFilename() const {
    return filename;
  }

/**
 * Write current text buffer (2 parts) to the indicated file.
 * @param f: file descriptor.
 * @return 0 if error else 1.
 **/
  int write_textbuffer_to_file(const int f) const {
    if (write(f, start, gap - start) != gap - start) return 0;
    if (write(f, rest, _end - rest) != _end - rest) return 0;
    return 1;
  }

/**
 * Empty undo stack.
 **/
  void clear_undo() {
    struct undo *undo = undohead;
    while (undo) {
      struct undo *next = undo->next;
      free(undo->undobuf);
      free(undo->redobuf);
      free(undo);
      undo = next;
    }
    undohead = undotail = undoredo = NULL;
  }

/**
 * 
 */
  void reset_undo() {
    while (undotail != undoredo) {
      struct undo *undo = undotail;
      if (!undo) {
        undohead = undotail = NULL;
        break;
      }
      undotail = undo->prev;
      if (undo->prev) undo->prev->next = NULL;
      free(undo->undobuf);
      free(undo->redobuf);
      free(undo);
    }
    undoredo = undotail;
  }

/**
 * Return the sum of the 2 parts buffer lengths
 * @return the length of all text in the buffer
 **/
  int text_length() const {
    return (gap - start) + (_end - rest);
  }

/**
 * Return the pos char position in the buffer.
 * @return pointer to pos char in the buffer.
 */  
  char* text_ptr(const int pos) const {
    char *p = start + pos;
    if (p >= gap) p += (rest - gap);
    return p;
  }

/**
 * Move start of gap to pos char & keep minsize (gap?).
 * @param pos: char pos at the beginning of the gap.
 * @param minsize: Min size of gap (?) after be moved.
 **/
  void move_gap(const int pos, int minsize) {
    const int gapsize = rest - gap;
    char *p = text_ptr(pos);
    if (minsize < 0) minsize = 0;

    if (minsize <= gapsize) {
      if (p != rest) {
        if (p < gap) {
          memmove(p + gapsize, p, gap - p);
        } else {
          memmove(gap, rest, p - rest);
        }
        gap = start + pos;
        rest = gap + gapsize;
      }
    } else {
      if (gapsize + MINEXTEND > minsize) minsize = gapsize + MINEXTEND;
      const int newsize = (this->_end - this->start) - gapsize + minsize;
      char *const newstart = (char *) malloc(newsize); // TODO check for out of memory
      char *const newgap = newstart + pos;
      char *const newrest = newgap + minsize;
      char *const newend = newstart + newsize;

      if (p < gap) {
        memcpy(newstart, start, pos);
        memcpy(newrest, p, gap - p);
        memcpy(newend - (_end - rest), rest, _end - rest);
      } else {
        memcpy(newstart, start, gap - start);
        memcpy(newstart + (gap - start), rest, p - rest);
        memcpy(newrest, p, _end - p);
      }

      free(start);
      start = newstart;
      gap = newgap;
      rest = newrest;
      _end = newend;
    }

  #ifdef DEBUG
    memset(gap, 0, rest - gap);
  #endif
  }

/**
 * Move gap to the end of buffer.
 **/
  void close_gap() {
    const int len = text_length();
    move_gap(len, 1);
    start[len] = 0;
  }

/**
 * Return then char available at pos.
 * @return the char at pos or -1 if out of buffer.
 **/
  int get(const int pos) const {
    const char *p = text_ptr(pos);
    if (p >= _end) return -1;
    return *p;
  }

/**
 * Compare internal buffer at pos with transmited buffer, within the len.
 * @param buf: transmited buffer.
 * @param len: Size of transmited buffer.
 * @return 1 is buffers are fully identical, else 0.
 **/
  int compare(const char buf[], const int pos, int len) const {
    const char *bufptr = buf;
    const char *p = start + pos;
    if (p >= this->gap) p += (this->rest - this->gap);

    while (len > 0) {
      if (p == _end) return 0;
      if (*bufptr++ != *p) return 0;
      len--;
      if (++p == this->gap) p = this->rest;
    }

    return 1;
  }

  int copy(char buf[], const int pos, int len) const {
    char *bufptr = buf;
    char *p = start + pos;
    if (p >= gap) p += (rest - gap);

    while (len > 0) {
      if (p == _end) break;
      *bufptr++ = *p;
      len--;
      if (++p == gap) p = rest;
    }

    return bufptr - buf;
  }

  void replace(const int pos, const int len, const char buf[], const int bufsize, const int doundo);

  void insert(const int pos, const char buf[], const int bufsize) {
    this->replace(pos, 0, buf, bufsize, 1);
  }

  void erase(const int pos, const int len) {
    this->replace(pos, len, NULL, 0, 1);
  }

//
// Navigation functions
//

/**
 * Return length from linepos to end of line.
 * @param linepos: measure from this char.
 * @return length to the end-of-line.
 **/
  int line_length(const int linepos) const {
    for (int pos = linepos;; ++pos) {
      const int ch = get(pos);
      if (ch < 0 || ch == '\n' || ch == '\r') return pos - linepos;
    }
  }

/**
 * Return line start from indicated pos.
 * @param linepos: start of research.
 * @return start of line pos.
 **/
  int line_start(const int linepos) const {
    for (int pos = linepos; pos > 0; --pos) {
      if (get(pos - 1) == '\n') return pos;
    }
    return 0;
  }

/**
 * Return pos of next start of line.
 * @param linepos: starting char of research.
 * @return pos of next start of line.
 **/
  int next_line(const int linepos) const {
    for (int pos = linepos;; ++pos) {
      const int ch = get(pos);
      if (ch < 0) return -1;
      if (ch == '\n') return pos+1;
    }
  }

/**
 * Return pos of prev start-of-line.
 * @param linepos: starting char of research.
 * @return pos of previous start-of-lien.
 **/
  int prev_line(const int linepos) const {
    if (linepos == 0) return -1;

    int pos = linepos;
    while (pos > 0) {
      const int ch = get(--pos);
      if (ch == '\n') break;
    }

    while (pos > 0) {
      const int ch = get(--pos);
      if (ch == '\n') return pos + 1;
    }

    return 0;
  }

/**
 * Return screen column of the col char after the linepos start.
 * @param linepos: start of research.
 * @param col: count after the linepos.
 * @return screen column size, including tabs spaces.
 */
  int column(const int linepos, const int col) const {
    const char* p = text_ptr(linepos);
    int c = 0;
    for (int i = col; i > 0; --i) {
      if (p == _end) break;
      if (*p == '\t') {
        const int spaces = TABSIZE - c % TABSIZE;
        c += spaces;
      } else {
        ++c;
      }
      if (++p == gap) p = rest;
    }
    return c;
  }

  void moveto(const int pos, const int center);

//
// Text selection
//

  int get_selection(int &start, int &end) const {
    if (anchor == -1) {
      start = end = -1;
      return 0;
    } else {
      const int pos = linepos + col;
      if (pos == anchor) {
        start = end = -1;
        return 0;
      } else if (pos < anchor) {
        start = pos;
        end = anchor;
      } else {
        start = anchor;
        end = pos;
      }
    }
    return 1;
  }

  int get_selected_text(char buffer[], const int size) const {
    int selstart, selend;

    if (!get_selection(selstart, selend)) return 0;
    const int len = selend - selstart;
    if (len >= size) return 0;
    copy(buffer, selstart, len);
    buffer[len] = 0;
    return len;
  }

  void update_selection(const int select) {
    if (select) {
      if (anchor == -1) anchor = linepos + col;
      refresh = 1;
    } else {
      if (anchor != -1) refresh = 1;
      anchor = -1;
    }
  }

  int erase_selection() {
    int selstart, selend;
    
    if (!get_selection(selstart, selend)) return 0;
    moveto(selstart, 0);
    erase(selstart, selend - selstart);
    anchor = -1;
    refresh = 1;
    return 1;
  }

  void select_all() {
    anchor = 0;
    refresh = 1;
    moveto(text_length(), 0);
  }

//
// Keyboard functions
//

  // int prompt(const char *msg, int selection);

//
// Display functions
//
  void draw_full_statusline();
  void draw_statusline();
  void display_line(const int pos, const int fullline);
  void update_line();
  void draw_screen();
  void position_cursor();

//
// Cursor movement
//
  void adjust();

  void up(const int select) {
    this->update_selection(select);

    const int newpos = prev_line(linepos);
    if (newpos < 0) return;

    linepos = newpos;
    line--;
    if (line < topline) {
      toppos = linepos;
      topline = line;
      refresh = 1;
    }

    adjust();
  }

  void down(const int select);

  void left(const int select) {
    update_selection(select);
    if (col > 0) {
      col--;
    } else {
      const int newpos = prev_line(linepos);
      if (newpos < 0) return;

      col = line_length(newpos);
      linepos = newpos;
      line--;
      if (line < topline) {
        toppos = linepos;
        topline = line;
        refresh = 1;
      }
    }

    lastcol = col;
    adjust();
  }

  void right(const int select);

  void wordleft(const int select) {
    update_selection(select);
    int pos = linepos + col;
    int phase = 0;
    while (pos > 0) {
      const char ch = get(pos - 1);
      if (phase == 0) {
        if (wordchar(ch)) phase = 1;
      } else {
        if (!wordchar(ch)) break;
      }

      pos--;
      if (pos < linepos) {
        linepos = prev_line(linepos);
        line--;
        refresh = 1;
      }
    }
    col = pos - linepos;
    if (line < topline) {
      toppos = linepos;
      topline = line;
    }

    lastcol = col;
    adjust();
  }

  void wordright(const int select);

  void home(const int select) {
    update_selection(select);
    col = lastcol = 0;
    adjust();
  }

  void end(const int select) {
    update_selection(select);
    col = lastcol = line_length(linepos);
    adjust();
  }

  void top(const int select) {
    update_selection(select);
    toppos = topline = margin = 0;
    linepos = line = col = lastcol = 0;
    refresh = 1;
    adjust();   // Ajouté par mes soins (?)
  }

  void bottom(const int select);

  void pageup(const int select);

  void pagedown(const int select);

//
// Text editing
//

  void insert_char(const char ch) {
    erase_selection();
    insert(linepos + col, &ch, 1);
    col++;
    lastcol = col;
    adjust();
    if (!refresh) lineupdate = 1;
  }

  void newline();

  void backspace() {
    if (erase_selection()) return;
    if (linepos + col == 0) return;
    if (col == 0) {
      int pos = this->linepos;
      erase(--pos, 1);
      if (get(pos - 1) == '\r') erase(--pos, 1);

      line--;
      linepos = line_start(pos);
      col = pos - linepos;
      refresh = 1;

      if (line < topline) {
        toppos = linepos;
        topline = line;
      }
    } else {
      col--;
      erase(linepos + col, 1);
      lineupdate = 1;
    }

    lastcol = col;
    adjust();
  }

  void del() {
    if (erase_selection()) return;
    const int pos = linepos + col;
    int ch = get(pos);
    if (ch < 0) return;

    this->erase(pos, 1);
    if (ch == '\r') {
      ch = this->get(pos);
      if (ch == '\n') erase(pos, 1);
    }

    if (ch == '\n') {
      refresh = 1;
    } else {
      lineupdate = 1;
    }
  }

  void indent(const char indentation[]) {
    const int width = strlen(indentation);
    int pos = linepos + col;

    int start, end;
    if (!get_selection(start, end)) {
      insert_char('\t');
      return;
    }

    int lines = 0;
    int toplines = 0;
    int newline = 1;
    for (int i = start; i < end; i++) {
      if (i == toppos) toplines = lines;
      if (newline) {
        lines++;
        newline = 0;
      }
      if (get(i) == '\n') newline = 1;
    }
    const int buflen = end - start + lines * width;
    char *const buffer = (char*)malloc(buflen);
    if (!buffer) return;

    newline = 1;
    char* p = buffer;
    for (int i = start; i < end; i++) {
      if (newline) {
        memcpy(p, indentation, width);
        p += width;
        newline = 0;
      }
      const int ch = this->get(i);
      *p++ = ch;
      if (ch == '\n') newline = 1;
    }

    replace(start, end - start, buffer, buflen, 1);
    free(buffer);  

    if (anchor < pos) {
      pos += width * lines;
    } else {
      anchor += width * lines;
    }

    toppos += width * toplines;
    linepos = line_start(pos);
    col = lastcol = pos - linepos;

    adjust();
    refresh = 1;
  }

/**
 * Remove indent for the selected part.
 * @param indentation block to be remove from each line.
 **/
  void unindent(const char indentation[]) {
    const int width = strlen(indentation);
    int pos = this->linepos + this->col;

    int start, end;
    if (!get_selection(start, end)) return;

    char *const buffer = (char*)malloc(end - start);
    if (!buffer) return;

    int newline = 1;
    char* p = buffer;
    int i = start;
    int shrinkage = 0;
    int topofs = 0;
    while (i < end) {
      if (newline) {
        newline = 0;
        if (compare(indentation, i, width)) {
          i += width;
          shrinkage += width;
          if (i < toppos) topofs -= width;
          continue;
        }
      }
      const int ch = get(i++);
      *p++ = ch;
      if (ch == '\n') newline = 1;
    }

    if (!shrinkage) {
      free(buffer);
      return;
    }

    replace(start, end - start, buffer, p - buffer, 1);
    free(buffer);

    if (anchor < pos) {
      pos -= shrinkage;
    } else {
      anchor -= shrinkage;
    }

    toppos += topofs;
    linepos = line_start(pos);
    col = lastcol = pos - linepos;

    refresh = 1;
    adjust();
  }

/**
 * Do the next undo.
 **/
  void doUndo() {
    if (!undoredo) return;
    moveto(undoredo->pos, 0);
    replace(undoredo->pos, undoredo->inserted, undoredo->undobuf, undoredo->erased, 0);
    undoredo = undoredo->prev;
    if (!undoredo) dirty = 0;
    anchor = -1;
    lastcol = col;
    refresh = 1;
  }

/**
 * Do the previous redo.
 **/
  void doRedo() {
    if (undoredo) {
      if (!undoredo->next) return;
      undoredo = undoredo->next;
    } else {
      if (!undohead) return;
      undoredo = undohead;
    }
    replace(undoredo->pos, undoredo->erased, undoredo->redobuf, undoredo->inserted, 0);
    moveto(undoredo->pos, 0);
    dirty = 1;
    anchor = -1;
    lastcol = col;
    refresh = 1;
  }

//
// Clipboard
//

/**
 * Copy selection to env clipboard.
 **/
  void copy_selection() const;

/**
 * Copy selection */
  void cut_selection() {
    this->copy_selection();
    this->erase_selection();
  }

  void paste_selection();

//
// Editor Commands
//

  // void read_from_stdin() {
  //   char buffer[512];

  //   int pos = 0;
  //   int n;
  //   while ((n = fread(buffer, 1, sizeof(buffer), stdin)) > 0) {
  //     this->insert(pos, buffer, n);
  //     pos += n;
  //   }
  //   strcpy(_filename, "<stdin>");
  //   newfile = 1;
  //   _dirty = 0;
  // }

  void save_editor();

  void find_text(int next);

  void goto_line();

  void redraw_screen();

/**
 * Display help splash screen ; should be in env.
 **/
  void help();

protected:
  int wordchar(const char ch) const {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9');
  }

private:
//
// Editor data block
//
// Structure of split buffer:
//
//    +------------------+------------------+------------------+
//    | text before gap  |        gap       |  text after gap  |
//    +------------------+------------------+------------------+
//    ^                  ^                  ^                  ^     
//    |                  |                  |                  |
//  start               gap                rest               end
//

  char *start;               // Start of text buffer
  char *gap;                 // Start of gap
  char *rest;                // End of gap
  char *_end;                // End of text buffer

  int toppos;                // Text position for current top screen line
  int topline;               // Line number for top of screen
  int margin;                // Position for leftmost column on screen

  int linepos;               // Text position for current line
  int line;                  // Current document line
  int col;                   // Current document column
  int lastcol;               // Remembered column from last horizontal navigation
  int anchor;                // Anchor position for selection
  
  char* search;              // Current searched string or NULL if none.

  struct undo *undohead;     // Start of undo buffer list
  struct undo *undotail;     // End of undo buffer list
  struct undo *undoredo;     // Undo/redo boundary

  int refresh;               // Flag to trigger screen redraw
  int lineupdate;            // Flag to trigger redraw of current line
  int dirty;                 // Dirty flag is set when the editor buffer has been changed

  char* filename;            // Associated file name, maybe empty.

  EnvEdit& env;
};
