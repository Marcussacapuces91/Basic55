
#include "editor.h"
#include "envedit.h"

void Editor::adjust() {
  const int ll = line_length(linepos);

  this->col = this->lastcol;
  if (this->col > ll) this->col = ll;

  int col = this->column(this->linepos, this->col);
  while (col < this->margin) {
    this->margin -= 4;
    if (this->margin < 0) this->margin = 0;
    this->refresh = 1;
  }

  while (col - this->margin >= env.cols()) {
    this->margin += 4;
    this->refresh = 1;
  }
}

void Editor::bottom(const int select) {
  this->update_selection(select);
  for (;;) {
    const int newpos = this->next_line(this->linepos);
    if (newpos < 0) break;

    this->linepos = newpos;
    this->line++;

    if (this->line >= this->topline + env.lines()) {
      this->toppos = this->next_line(this->toppos);
      this->topline++;
      this->refresh = 1;
    }
  }
  this->col = this->lastcol = this->line_length(this->linepos);
  this->adjust();
}

void Editor::copy_selection() const {
  int selstart, selend;
  if (!get_selection(selstart, selend)) return;
  const int size = selend - selstart;
  char *const cb = env.makeClipboard(size);
  if (!cb) return;
  copy(cb, selstart, size);
}

void Editor::display_line(int pos, const int fullline) {
  int margin = this->margin;
  const int maxcol = env.cols() + margin;
  char *const buffer = (char*)malloc(maxcol + 1024);

// TODO Remplace with immediate outputs.
  char *bufptr = buffer;
  char *p = text_ptr(pos);

  int selstart, selend;
  get_selection(selstart, selend);

  int hilite = 0;
  int col = 0;
  while (col < maxcol) {
    if (margin == 0) {
      if (!hilite && pos >= selstart && pos < selend) {
        for (const char* s = SELECT_COLOR; *s; s++) *bufptr++ = *s;
        hilite = 1;
      } else if (hilite && pos >= selend) {
        for (const char* s = TEXT_COLOR; *s; s++) *bufptr++ = *s;
        hilite = 0;
      }
    }

    if (p == this->_end) break;
    const char ch = *p;
    if (ch == '\r' || ch == '\n') break;

    if (ch == '\t') {
      int spaces = TABSIZE - col % TABSIZE;
      while (spaces > 0 && col < maxcol) {
        if (margin > 0) {
          margin--;
        } else {
          *bufptr++ = ' ';
        }
        col++;
        spaces--;
      }
    } else {
      if (margin > 0) {
        margin--;
      } else {
        *bufptr++ = ch;
      }
      col++;
    }

    if (++p == this->gap) p = this->rest;
    pos++;
  }

// #ifdef __linux__
//   if (hilite) {
//     while (col < maxcol) {
//       *bufptr++ = ' ';
//       col++;
//     }
//   } else {
//     if (col == margin) *bufptr++ = ' ';
//   }
// #endif

  if (col < maxcol) {
    for (const char* s = CLREOL; *s; s++) *bufptr++ = *s;
    if (fullline) {
      memcpy(bufptr, "\r\n", 2);
      bufptr += 2;
    }
  }

  if (hilite) {
    for (const char* s = TEXT_COLOR; *s; s++) *bufptr++ = *s;
  }

  env.outbuf(buffer, bufptr - buffer);
  free(buffer);
}

void Editor::down(const int select) {
  this->update_selection(select);

  const int newpos = this->next_line(this->linepos);
  if (newpos < 0) return;

  this->linepos = newpos;
  this->line++;

  if (this->line >= this->topline + env.lines()) {
    this->toppos = this->next_line(this->toppos);
    this->topline++;
    this->refresh = 1;
  }

  this->adjust();
}

void Editor::draw_full_statusline() {
  char buffer[1024];

  const int namewidth = env.cols() - 28;
  env.gotoxy(0, env.lines());
  snprintf(buffer, 1024, STATUS_COLOR "%*.*sF1=Help %c Ln %-6dCol %-4d" CLREOL TEXT_COLOR, -namewidth, namewidth, filename, dirty ? '*' : ' ', line + 1, column(linepos, col) + 1);
  env.outstr(buffer);
}

/**
 * Draw the all Editor screen, from (0,0) to last line, without status.
 **/
void Editor::draw_screen() {
  env.gotoxy(0, 0);
  env.outstr(TEXT_COLOR);
  int pos = toppos;
  for (int i = 0; i < env.lines(); i++) {
    if (pos < 0) {
      env.outstr(CLREOL "\r\n");
    } else {
      display_line(pos, 1);
      pos = next_line(pos);
    }
  }
}

/**
 * Draw the status line.
 **/
void Editor::draw_statusline() {
  char buffer[1024];

  env.gotoxy(env.cols() - 20, env.lines());
  snprintf(buffer, 1024, STATUS_COLOR "%c Ln %-6dCol %-4d" CLREOL TEXT_COLOR, dirty ? '*' : ' ', line + 1, column(linepos, col) + 1);
  env.outstr(buffer);
}

/**
 * Find a new or next text.
 * @param next: 1 if this is the next research, 0 for the first (and request).
 **/
void Editor::find_text(const int next) {
  if (!next) {
    int selstart, selend;
    char *const select = get_selection(selstart, selend) ? strndup(text_ptr(selstart), selend-selstart) : NULL;

    char *const resp = env.prompt("Find: ", select);
    if (select) free(select);
    if (!resp) {
      refresh = 1;
      return;
    }
    if (search) free(search);
    search = resp;
  }

  if (!env.search) return;
  const int slen = strlen(env.search);
  if (slen > 0) {
    close_gap();
    const char* match = strstr(start + linepos + col, env.search);
    if (match != NULL) {
      int pos = match - start;
      anchor = pos;
      moveto(pos + slen, 1);
    } else {
      env.outch('\007');
    }
  }
  this->refresh = 1;
}

void Editor::goto_line() {
  this->anchor = -1;
  char*const resp = env.prompt("Goto line: ", NULL);
  if (resp) {
    const int lineno = atoi(resp);
    free(resp);
    int pos = 0;
    if (lineno > 0) {
      for (int l = 0; l < lineno - 1; l++) {
        pos = next_line(pos);
        if (pos < 0) break;
      }
    } else {
      pos = -1;
    }

    if (pos >= 0) {
      moveto(pos, 1);
    } else {
      env.outch('\007');
    }
  }
  refresh = 1;
}

void Editor::moveto(const int pos, const int center) {
  int scroll = 0;
  for (;;) {
    const int cur = linepos + col;
    if (pos < cur) {
      if (pos >= linepos) {
        col = pos - linepos;
      } else {
        col = 0;
        linepos = prev_line(linepos);
        line--;

        if (topline > line) {
          toppos = linepos;
          topline--;
          refresh = 1;
          scroll = 1;
        }
      }
    } else if (pos > cur) {
      const int next = next_line(linepos);
      if (next == -1) {
        col = line_length(linepos);
        break;
      } else if (pos < next) {
        col = pos - linepos;
      } else {
        col = 0;
        linepos = next;
        line++;

        if (line >= topline + env.lines()) {
          toppos = next_line(toppos);
          topline++;
          refresh = 1;
          scroll = 1;
        }
      }
    } else {
      break;
    }
  }

  if (scroll && center) {
    int tl = line - env.lines() / 2;
    if (tl < 0) tl = 0;
    for (;;) {
      if (topline > tl) {
        toppos = prev_line(toppos);
        topline--;
      } else if (topline < tl) {
        toppos = next_line(this->toppos);
        topline++;
      } else {
        break;
      }
    }
  }
}

void Editor::newline() {
  this->erase_selection();
#ifdef __linux__
  this->insert(this->linepos + this->col, "\n", 1);
#else
  this->insert(this->linepos + this->col, "\r\n", 2);
#endif
  this->col = this->lastcol = 0;
  this->line++;
  int p = this->linepos;
  this->linepos = this->next_line(this->linepos);
  for (;;) {
    char ch = this->get(p++);
    if (ch == ' ' || ch == '\t') {
      this->insert(this->linepos + this->col, &ch, 1);
      this->col++;
    } else {
      break;
    }
  }
  this->lastcol = this->col;
  
  this->refresh = 1;

  if (this->line >= this->topline + env.lines()) {
    this->toppos = this->next_line(this->toppos);
    this->topline++;
    this->refresh = 1;
  }

  this->adjust();
}

void Editor::pagedown(const int select) {
  update_selection(select);
  for (int i = 0; i < env.lines(); i++) {
    const int newpos = next_line(linepos);
    if (newpos < 0) break;

    linepos = newpos;
    line++;

    toppos = next_line(toppos);
    topline++;
  }

  refresh = 1;
  adjust();
}

void Editor::pageup(const int select) {
  update_selection(select);
  if (line < env.lines()) {
    linepos = toppos = 0;
    line = topline = 0;
  } else {
    for (int i = 0; i < env.lines(); i++) {
      const int newpos = prev_line(linepos);
      if (newpos < 0) return;

      linepos = newpos;
      line--;

      if (topline > 0) {
        toppos = prev_line(toppos);
        topline--;
      }
    }
  }

  refresh = 1;
  adjust();
}

void Editor::paste_selection() {
  this->erase_selection();
  this->insert(this->linepos + this->col, env.clipboard, env.clipsize);
  this->moveto(this->linepos + this->col + env.clipsize, 0);
  this->refresh = 1;
}

void Editor::position_cursor() {
  const int col = this->column(this->linepos, this->col);
  env.gotoxy(col - this->margin, this->line - this->topline);
}

void Editor::redraw_screen() {
  env.get_console_size();
  this->draw_screen();
}

void Editor::replace(const int pos, const int len, const char buf[], const int bufsize, const int doundo) {

  // Store undo information
  if (doundo) {
    reset_undo();
    struct undo *undo = this->undotail;
    if (undo && len == 0 && bufsize == 1 && undo->erased == 0 && pos == undo->pos + undo->inserted) {
      // Insert character at end of current redo buffer
      undo->redobuf = (char*)realloc(undo->redobuf, undo->inserted + 1);
      undo->redobuf[undo->inserted] = *buf;
      undo->inserted++;
    } else if (undo && len == 1 && bufsize == 0 && undo->inserted == 0 && pos == undo->pos) {
      // Erase character at end of current undo buffer
      undo->undobuf = (char*)realloc(undo->undobuf, undo->erased + 1);
      undo->undobuf[undo->erased] = this->get(pos);
      undo->erased++;
    } else if (undo && len == 1 && bufsize == 0 && undo->inserted == 0 && pos == undo->pos - 1) {
      // Erase character at beginning of current undo buffer
      undo->pos--;
      undo->undobuf = (char*)realloc(undo->undobuf, undo->erased + 1);
      memmove(undo->undobuf + 1, undo->undobuf, undo->erased);
      undo->undobuf[0] = this->get(pos);
      undo->erased++;
    } else {
      // Create new undo buffer
      undo = (struct undo *) malloc(sizeof(struct undo));
      if (undotail) undotail->next = undo;
      undo->prev = undotail;
      undo->next = NULL;
      undotail = undoredo = undo;
      if (!undohead) undohead = undo;

      undo->pos = pos;
      undo->erased = len;
      undo->inserted = bufsize;
      undo->undobuf = undo->redobuf = NULL;
      if (len > 0) {
        undo->undobuf = (char*)malloc(len);
        copy(undo->undobuf, pos, len);      
      }
      if (bufsize > 0) {
        undo->redobuf = (char*)malloc(bufsize);
        memcpy(undo->redobuf, buf, bufsize);
      }
    }
  }

  char* p = this->start + pos;
  if (bufsize == 0 && p <= this->gap && p + len >= this->gap) {
    // Handle deletions at the edges of the gap
    this->rest += len - (this->gap - p);
    this->gap = p;
  } else {
    // Move the gap
    move_gap(pos + len, bufsize - len);

    // Replace contents
    memcpy(this->start + pos, buf, bufsize);
    this->gap = this->start + pos + bufsize;
  }

  // Mark buffer as dirty
  dirty = 1;
}

void Editor::right(int select) {
  this->update_selection(select);
  if (this->col < this->line_length(this->linepos)) {
    this->col++;
  } else {
    int newpos = this->next_line(this->linepos);
    if (newpos < 0) return;

    this->col = 0;
    this->linepos = newpos;
    this->line++;

    if (this->line >= this->topline + env.lines()) {
      this->toppos = this->next_line(this->toppos);
      this->topline++;
      this->refresh = 1;
    }
  }

  this->lastcol = this->col;
  this->adjust();
}

// void Editor::save_editor() {
//   if (!_dirty && !newfile) return;
  
//   if (newfile) {
//     if (!env.prompt(*this, "Save as: ", 1)) {
//       refresh = 1;
//       return;
//     }

//     if (access(env.linebuf, F_OK) == 0) {
//       env.display_message("Overwrite %s (y/n)? ", env.linebuf);
//       if (!env.ask()) {
//         refresh = 1;
//         return;
//       }
//     }
//     strcpy(_filename, env.linebuf);
//     newfile = 0;
//   }

//   const int rc = env.save_file(*this);
//   if (rc < 0) {
//     env.display_message("Error %d saving document (%s)", errno, strerror(errno));
//     // sleep(5);
//   }

//   this->refresh = 1;
// }

void Editor::update_line() {
  env.gotoxy(0, this->line - this->topline);
  display_line(this->linepos, 0);
}

void Editor::wordright(int select) {
  update_selection(select);
  int pos = this->linepos + this->col;
  const int end = this->text_length();
  int next = this->next_line(this->linepos);
  int phase = 0;
  while (pos < end) {
    int ch = this->get(pos);
    if (phase == 0) {
      if (wordchar(ch)) phase = 1;
    } else {
      if (!wordchar(ch)) break;
    }

    pos++;
    if (pos == next) {
      this->linepos = next;
      next = this->next_line(this->linepos);
      this->line++;
      this->refresh = 1;
    }
  }
  this->col = pos - this->linepos;
  if (this->line >= this->topline + env.lines()) {
    this->toppos = this->next_line(this->toppos);
    this->topline++;
  }

  this->lastcol = this->col;
  this->adjust();
}
