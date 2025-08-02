//
// edit.c
//
// Text editor
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

// #include "env.h"
// #include "editor.h"

// #include <signal.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <stdarg.h>
// #include <string.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/stat.h>


// #define O_BINARY 0


// #ifndef INDENT
// #define INDENT "  "
// #endif





// //
// // Screen functions
// //



// //
// // Keyboard functions
// //





// //
// // Cursor movement
// //








// //
// // Editor Commands
// //

// void open_editor(Editor *ed) {
//   int rc;
//   char *filename;
//   struct env *env = ed->env;

//   if (!prompt(ed, "Open file: ", 1)) {
//     ed->refresh = 1;
//     return;
//   }
//   filename = ed->env->linebuf;
  
//   ed = find_editor(ed->env, filename);
//   if (ed) {
//     env->current = ed;
//   } else {
//     ed = create_editor(env);
//     rc = load_file(ed, filename);
//     if (rc < 0) {
//       display_message(ed, "Error %d opening %s (%s)", errno, filename, strerror(errno));
//       sleep(5);
//       delete_editor(ed);
//       ed = env->current;
//     }
//   }
//   ed->refresh = 1;
// }

// void new_editor(struct editor *ed) {
//   ed = create_editor(ed->env);
//   new_file(ed, "");
//   ed->refresh = 1;
// }




// //
// // Editor Commands
// //

// void close_editor(struct editor *ed) {
//   struct env *env = ed->env;
  
//   if (ed->dirty) {
//     display_message(ed, "Close %s without saving changes (y/n)? ", ed->filename);
//     if (!ask()) {
//       ed->refresh = 1;
//       return;
//     }
//   }
  
//   delete_editor(ed);

//   ed = env->current;
//   if (!ed) {
//     ed = create_editor(env);
//     new_file(ed, "");
//   }
//   ed->refresh = 1;
// }

// void jump_to_editor(struct editor *ed) {
//   struct env *env = ed->env;
//   char filename[FILENAME_MAX];
//   int lineno = 0;

//   if (!get_selected_text(ed, filename, FILENAME_MAX)) {
//     int pos = ed->linepos + ed->col;
//     char *p = filename;
//     int left = FILENAME_MAX - 1;
//     while (left > 0) {
//       int ch = get(ed, pos);
//       if (ch < 0) break;
//       if (strchr("!@\"'#%&()[]{}*?+:;\r\n\t ", ch)) break;
//       *p++ = ch;
//       left--;
//       pos++;
//     }
//     *p = 0;

//     if (get(ed, pos) == ':') {
//       pos++;
//       for (;;) {
//         int ch = get(ed, pos);
//         if (ch < 0) break;
//         if (ch >= '0' && ch <= '9') {
//           lineno = lineno * 10 + (ch - '0');
//         } else {
//           break;
//         }
//         pos++;
//       }
//     }
//   }
//   if (!*filename) return;
  
//   ed = find_editor(env, filename);
//   if (ed) {
//     env->current = ed;
//   } else {
//     ed = create_editor(env);
//     if (load_file(ed, filename) < 0) {
//       outch('\007');
//       delete_editor(ed);
//       ed = env->current;
//     }
//   }
  
//   if (lineno > 0) {
//     int pos = 0;
//     while (--lineno > 0) {
//       pos = next_line(ed, pos);
//       if (pos < 0) break;
//     }
//     if (pos >= 0) moveto(ed, pos, 1);
//   }

//   ed->refresh = 1;
// }

// int quit(struct env *env) {
//   Editor *ed = env->current;
//   Editor *start = ed;

//   do {
//     if (ed->dirty) {
//       display_message(ed, "Close %s without saving changes (y/n)? ", ed->filename);
//       if (!ask()) return 0;
//     }
//     ed = ed->next;
//   } while (ed != start);

//   return 1;
// }



// //
// // Editor
// //

// void edit(Editor& ed) {
//   int done = 0;
//   int key;

//   this->refresh = 1;
//   while (!done) {
//     if (ed->refresh) {
//       draw_screen(ed);
//       draw_full_statusline(ed);
//       ed->refresh = 0;
//       ed->lineupdate = 0;
//     } else if (ed->lineupdate) {
//       update_line(ed);
//       ed->lineupdate = 0;
//       draw_statusline(ed);
//     } else {
//       draw_statusline(ed);
//     }

//     position_cursor(ed);
//     fflush(stdout);
//     key = getkey();

//     if (key >= ' ' && key <= 0x7F) {
// #ifdef LESS
//       switch (key) {
//         case 'q': done = 1; break;
//         case '/': find_text(ed, 0); break;
//       }
// #else
//       insert_char(ed, (char) key);
// #endif
//     } else {
//       switch (key) {
//         case KEY_F1: help(ed); break;
//         case KEY_F3: jump_to_editor(ed); ed = ed->env->current; break;
//         case KEY_F5: redraw_screen(ed); break;

// #ifdef __linux__
//         case ctrl('y'): help(ed); break;
//         case ctrl('t'): top(ed, 0); break;
//         case ctrl('b'): bottom(ed, 0); break;
// #endif

//         case KEY_UP: ed->up(0); break;
//         case KEY_DOWN: ed->down(0); break;
//         case KEY_LEFT: ed->eft(0); break;
//         case KEY_RIGHT: ed->ight(0); break;
//         case KEY_HOME: ed->home(0); break;
//         case KEY_END: ed->end(0); break;
//         case KEY_PGUP: ed->pageup(0); break;
//         case KEY_PGDN: ed->pagedown(0); break;

//         case KEY_CTRL_RIGHT: wordright(ed, 0); break;
//         case KEY_CTRL_LEFT: wordleft(ed, 0); break;
//         case KEY_CTRL_HOME: top(ed, 0); break;
//         case KEY_CTRL_END: bottom(ed, 0); break;

//         case KEY_SHIFT_UP: ed->up(1); break;
//         case KEY_SHIFT_DOWN: ed->down(1); break;
//         case KEY_SHIFT_LEFT: ed->left(1); break;
//         case KEY_SHIFT_RIGHT: ed->right(1); break;
//         case KEY_SHIFT_PGUP: ed->pageup(1); break;
//         case KEY_SHIFT_PGDN: ed->pagedown(1); break;
//         case KEY_SHIFT_HOME: ed->home(1); break;
//         case KEY_SHIFT_END: ed->end(1); break;

//         case KEY_SHIFT_CTRL_RIGHT: wordright(ed, 1); break;
//         case KEY_SHIFT_CTRL_LEFT: wordleft(ed, 1); break;
//         case KEY_SHIFT_CTRL_HOME: top(ed, 1); break;
//         case KEY_SHIFT_CTRL_END: bottom(ed, 1); break;

//         case KEY_CTRL_TAB: ed = next_file(ed); break;

//         case ctrl('a'): select_all(ed); break;
//         case ctrl('c'): copy_selection(ed); break;
//         case ctrl('f'): find_text(ed, 0); break;
//         case ctrl('l'): goto_line(ed); break;
//         case ctrl('g'): find_text(ed, 1); break;
//         case ctrl('q'): done = 1; break;
// #ifdef LESS
//         case KEY_ESC: done = 1; break;
// #else
//         case KEY_TAB: indent(ed, INDENT); break;
//         case KEY_SHIFT_TAB: unindent(ed, INDENT); break;

//         case KEY_ENTER: newline(ed); break;
//         case KEY_BACKSPACE: backspace(ed); break;
//         case KEY_DEL: del(ed); break;
//         case ctrl('x'): cut_selection(ed); break;
//         case ctrl('z'): undo(ed); break;
//         case ctrl('r'): redo(ed); break;
//         case ctrl('v'): paste_selection(ed); break;
//         case ctrl('o'): open_editor(ed); ed = ed->env->current; break;
//         case ctrl('n'): new_editor(ed); ed = ed->env->current; break;
//         case ctrl('s'): save_editor(ed); break;
//         case ctrl('p'): pipe_command(ed); break;
// #endif
//         case ctrl('w'): close_editor(ed); ed = ed->env->current; break;
//       }
//     }
//   }
// }






// //
// // main
// //

// int main(int argc, char *argv[]) {
//   struct env env;
//   int rc;
//   int i;
//   sigset_t blocked_sigmask, orig_sigmask;
// #ifdef __linux__
//   struct termios tio;
//   struct termios orig_tio;
// #endif
// #ifdef SANOS
//   struct term *term;
// #endif

//   memset(&env, 0, sizeof(env));
//   for (i = 1; i < argc; i++) {
//     Editor *ed = new Editor(&env);
//     rc = ed->load_file(argv[i]);
//     if (rc < 0 && errno == ENOENT) rc = ed->new_file(argv[i]);
//     if (rc < 0) {
//       perror(argv[i]);
//       return 0;
//     }
//   }
//   if (env.current == NULL) {
//     Editor *ed = new Editor(&env);
//     if (isatty(fileno(stdin))) {
//       ed->new_file("");
//     } else {
//       ed->read_from_stdin();
//     }    
//   }
//   env.current = env.current->next;

// #ifdef SANOS
//   term = gettib()->proc->term;
//   if (fdin != term->ttyin) dup2(term->ttyin, fdin);
//   if (fdout != term->ttyout) dup2(term->ttyout, fdout);
// #else
//   if (!isatty(fileno(stdin))) {
//     if (!freopen("/dev/tty", "r", stdin)) perror("/dev/tty");
//   }
// #endif

//   setvbuf(stdout, NULL, 0, 8192);

// #ifdef __linux__
//   tcgetattr(0, &orig_tio);
//   cfmakeraw(&tio);  
//   tcsetattr(0, TCSANOW, &tio);
//   if (getenv("TERM") && strcmp(getenv("TERM"), "linux") == 0) {
//     linux_console = 1;
//   } else {
//     outstr("\033[3 q");  // xterm
//     outstr("\033]50;CursorShape=2\a");  // KDE
//   }
// #endif

//   get_console_size(&env);
  
//   sigemptyset(&blocked_sigmask);
//   sigaddset(&blocked_sigmask, SIGINT);
//   sigaddset(&blocked_sigmask, SIGTSTP);
//   sigaddset(&blocked_sigmask, SIGABRT);
//   sigprocmask(SIG_BLOCK, &blocked_sigmask, &orig_sigmask);

//   for (;;) {
//     if (!env.current) break;
//     edit(env.current);
//     if (quit(&env)) break;
//   }

//   gotoxy(0, env.lines + 1);
//   outstr(RESET_COLOR CLREOL);
// #ifdef __linux__
//   tcsetattr(0, TCSANOW, &orig_tio);   
// #endif

//   while (env.current) delete(env.current);

//   if (env.clipboard) free(env.clipboard);
//   if (env.search) free(env.search);
//   if (env.linebuf) free(env.linebuf);

//   setbuf(stdout, NULL);
//   sigprocmask(SIG_SETMASK, &orig_sigmask, NULL);
//   return 0;
// }

#pragma once

#include "editor.h"

#include <cstdarg>
#include <vector>
#include <fcntl.h>

#define LINEBUF_EXTRA  32


//
// Key codes
//

#define KEY_BACKSPACE        0x101
#define KEY_ESC              0x102
#define KEY_INS              0x103
#define KEY_DEL              0x104
#define KEY_LEFT             0x105
#define KEY_RIGHT            0x106
#define KEY_UP               0x107
#define KEY_DOWN             0x108
#define KEY_HOME             0x109
#define KEY_END              0x10A
#define KEY_ENTER            0x10B
#define KEY_TAB              0x10C
#define KEY_PGUP             0x10D
#define KEY_PGDN             0x10E

#define KEY_CTRL_LEFT        0x10F
#define KEY_CTRL_RIGHT       0x110
#define KEY_CTRL_UP          0x111
#define KEY_CTRL_DOWN        0x112
#define KEY_CTRL_HOME        0x113
#define KEY_CTRL_END         0x114
#define KEY_CTRL_TAB         0x115

#define KEY_SHIFT_LEFT       0x116
#define KEY_SHIFT_RIGHT      0x117
#define KEY_SHIFT_UP         0x118
#define KEY_SHIFT_DOWN       0x119
#define KEY_SHIFT_PGUP       0x11A
#define KEY_SHIFT_PGDN       0x11B
#define KEY_SHIFT_HOME       0x11C
#define KEY_SHIFT_END        0x11D
#define KEY_SHIFT_TAB        0x11E

#define KEY_SHIFT_CTRL_LEFT  0x11F
#define KEY_SHIFT_CTRL_RIGHT 0x120
#define KEY_SHIFT_CTRL_UP    0x121
#define KEY_SHIFT_CTRL_DOWN  0x122
#define KEY_SHIFT_CTRL_HOME  0x123
#define KEY_SHIFT_CTRL_END   0x124

#define KEY_F1               0x125
#define KEY_F3               0x126
#define KEY_F5               0x127

#define KEY_UNKNOWN          0xFFF

#define ctrl(c) ((c) - 0x60)

#define ESC                  "\033"
#define CSI                  ESC"["

#define GOTOXY               CSI"%d;%dH"
#define CLRSCR               CSI"0J"
#define RESET_COLOR          CSI"0m"


class EnvEdit {
public:
  int lines() { return this->_lines; }
  int cols() { return this->_cols; }

/**
 * Return last key pressed.
 * @return: a key value, can be an ascii value or a special key.
 **/
  int getkey() {
    int shift, ctrl;

    int ch = getchar();
    if (ch == EOF) return ch;

    switch (ch) {
      case 0x08: return KEY_BACKSPACE;
      case 0x09:
        get_modifier_keys(shift, ctrl);
        if (shift) return KEY_SHIFT_TAB;
        if (ctrl) return KEY_CTRL_TAB;
        return KEY_TAB;
  #ifdef SANOS
      case 0x0D: return gettib()->proc->term->type == TERM_CONSOLE ? KEY_ENTER : KEY_UNKNOWN;
      case 0x0A: return gettib()->proc->term->type != TERM_CONSOLE ? KEY_ENTER : KEY_UNKNOWN;
  #else
      case 0x0D: return KEY_ENTER;
      case 0x0A: return KEY_ENTER;
  #endif
      case 0x1B:
        ch = getchar();
        switch (ch) {
          case 0x1B: return KEY_ESC;
          case 0x4F:
            ch = getchar();
            switch (ch) {
              case 0x46: return KEY_END;
              case 0x48: return KEY_HOME;
              case 0x50: return KEY_F1;
              case 0x52: return KEY_F3;
              case 0x54: return KEY_F5;
              default: return KEY_UNKNOWN;
            }
            break;

          case 0x5B:
            get_modifier_keys(shift, ctrl);
            ch = getchar();
            if (ch == 0x31) {
              ch = getchar();
              switch (ch) {
                case 0x3B:
                  ch = getchar();
                  if (ch == 0x32) shift = 1;
                  if (ch == 0x35) ctrl = 1;
                  if (ch == 0x36) shift = ctrl = 1;
                  ch = getchar();
                  break;
                case 0x35: 
                  return getchar() == 0x7E ? KEY_F5 : KEY_UNKNOWN;
                case 0x7E:
                  if (shift && ctrl) return KEY_SHIFT_CTRL_HOME;
                  if (shift) return KEY_SHIFT_HOME;
                  if (ctrl) return KEY_CTRL_HOME;
                  return KEY_HOME;
                default:
                  return KEY_UNKNOWN;
              }
            }

            switch (ch) {
              case 0x31: 
                if (getchar() != 0x7E) return KEY_UNKNOWN;
                if (shift && ctrl) return KEY_SHIFT_CTRL_HOME;
                if (shift) return KEY_SHIFT_HOME;
                if (ctrl) return KEY_CTRL_HOME;
                return KEY_HOME;
              case 0x32: return getchar() == 0x7E ? KEY_INS : KEY_UNKNOWN;
              case 0x33: return getchar() == 0x7E ? KEY_DEL : KEY_UNKNOWN;
              case 0x34:
                if (getchar() != 0x7E) return KEY_UNKNOWN;
                if (shift && ctrl) return KEY_SHIFT_CTRL_END;
                if (shift) return KEY_SHIFT_END;
                if (ctrl) return KEY_CTRL_END;
                return KEY_END;
              case 0x35:
                if (getchar() != 0x7E) return KEY_UNKNOWN;
                if (shift) return KEY_SHIFT_PGUP;
                return KEY_PGUP;
              case 0x36:
                if (getchar() != 0x7E) return KEY_UNKNOWN;
                if (shift) return KEY_SHIFT_PGDN;
                return KEY_PGDN;
              case 0x41: 
                if (shift && ctrl) return KEY_SHIFT_CTRL_UP;
                if (shift) return KEY_SHIFT_UP;
                if (ctrl) return KEY_CTRL_UP;
                return KEY_UP;
              case 0x42: 
                if (shift && ctrl) return KEY_SHIFT_CTRL_DOWN;
                if (shift) return KEY_SHIFT_DOWN;
                if (ctrl) return KEY_CTRL_DOWN;
                return KEY_DOWN;
              case 0x43: 
                if (shift && ctrl) return KEY_SHIFT_CTRL_RIGHT;
                if (shift) return KEY_SHIFT_RIGHT;
                if (ctrl) return KEY_CTRL_RIGHT;
                return KEY_RIGHT;
              case 0x44:
                if (shift && ctrl) return KEY_SHIFT_CTRL_LEFT;
                if (shift) return KEY_SHIFT_LEFT;
                if (ctrl) return KEY_CTRL_LEFT;
                return KEY_LEFT;
              case 0x46:
                if (shift && ctrl) return KEY_SHIFT_CTRL_END;
                if (shift) return KEY_SHIFT_END;
                if (ctrl) return KEY_CTRL_END;
                return KEY_END;
              case 0x48:
                if (shift && ctrl) return KEY_SHIFT_CTRL_HOME;
                if (shift) return KEY_SHIFT_HOME;
                if (ctrl) return KEY_CTRL_HOME;
                return KEY_HOME;
              case 0x5A: 
                return KEY_SHIFT_TAB;
              case 0x5B:
                ch = getchar();
                switch (ch) {
                  case 0x41: return KEY_F1;
                  case 0x43: return KEY_F3;
                  case 0x45: return KEY_F5;
                }
                return KEY_UNKNOWN;

              default: return KEY_UNKNOWN;
            }
            break;

          default: return KEY_UNKNOWN;
        }
        break;

      case 0x00:
      case 0xE0:
        ch = getchar();
        switch (ch) {
          case 0x0F: return KEY_SHIFT_TAB;
          case 0x3B: return KEY_F1;
          case 0x3D: return KEY_F3;
          case 0x3F: return KEY_F5;
          case 0x47: return KEY_HOME;
          case 0x48: return KEY_UP;
          case 0x49: return KEY_PGUP;
          case 0x4B: return KEY_LEFT;
          case 0x4D: return KEY_RIGHT;
          case 0x4F: return KEY_END;
          case 0x50: return KEY_DOWN;
          case 0x51: return KEY_PGDN;
          case 0x52: return KEY_INS;
          case 0x53: return KEY_DEL;
          case 0x73: return KEY_CTRL_LEFT;
          case 0x74: return KEY_CTRL_RIGHT;
          case 0x75: return KEY_CTRL_END;
          case 0x77: return KEY_CTRL_HOME;
          case 0x8D: return KEY_CTRL_UP;
          case 0x91: return KEY_CTRL_DOWN;
          case 0x94: return KEY_CTRL_TAB;
          case 0xB8: return KEY_SHIFT_UP;
          case 0xB7: return KEY_SHIFT_HOME;
          case 0xBF: return KEY_SHIFT_END;
          case 0xB9: return KEY_SHIFT_PGUP;
          case 0xBB: return KEY_SHIFT_LEFT;
          case 0xBD: return KEY_SHIFT_RIGHT;
          case 0xC0: return KEY_SHIFT_DOWN;
          case 0xC1: return KEY_SHIFT_PGDN;
          case 0xDB: return KEY_SHIFT_CTRL_LEFT;
          case 0xDD: return KEY_SHIFT_CTRL_RIGHT;
          case 0xD8: return KEY_SHIFT_CTRL_UP;
          case 0xE0: return KEY_SHIFT_CTRL_DOWN;
          case 0xD7: return KEY_SHIFT_CTRL_HOME;
          case 0xDF: return KEY_SHIFT_CTRL_END;

          default: return KEY_UNKNOWN;
        }
        break;

      case 0x7F: return KEY_BACKSPACE;

      default: return ch;
    }
  }

/**
 * Reallocate linebuf and set console width and height 
 **/
  void get_console_size() {
    _cols = 80;
    _lines = 24;
  // linebuf = (char*)realloc(linebuf, _cols + LINEBUF_EXTRA);
  }

/**
 * Return modifier key (Shift & Ctrl) for local linux console.
 * @param shift: receive Shift key's status
 * @param ctrl: receive Ctrl key's status
 **/
  void get_modifier_keys(int &shift, int &ctrl) {
    shift = ctrl = 0;
  #ifdef __linux__
    if (linux_console) {
      char modifiers = 6;
      if (ioctl(0, TIOCLINUX, &modifiers) >= 0) {
        if (modifiers & 1) shift = 1;
        if (modifiers & 4) ctrl = 1;
      }
    }
  #endif
  }

/**
 * Move cursor to (col, line) coordonates.
 * @param col: Column coordonate.
 * @param line: Line from top coordonate.
 **/
  void gotoxy(const int col, const int line) {
    char buf[32];

    sprintf(buf, GOTOXY, line + 1, col + 1);
    outstr(buf);
  }

/**
 * Return a pointer to the clipboard, with the size available.
 * @param size of the clipboard.
 * @return pointer to the clipboadr buffer.
 **/
  char* makeClipboard(const int size) {
    if (clipboard) free(clipboard);
    clipboard = (char*) malloc(size);
    clipsize = size;
    return clipboard;
  }

/**
 * Write a char to stdout.
 * @param c: a char value.
 **/
  void outch(const char c) {
    const int err = putchar(c);
    if (err == EOF)
      perror("fputs()"); // POSIX requires that errno is set
  }

/**
 * Write a string to stdout.
 * @param str: a array of char, \0 ended.
 **/
  void outstr(const char str[]) {
    const int err = fputs(str, stdout);
    if (err == EOF)
      perror("fputs()"); // POSIX requires that errno is set
  }

/**
 * Write a buffer with a length to stdout.
 * @param buf: Buffer to be written.
 * @param len number of char to be written.
 **/
  void outbuf(const char buf[], const int len) {
    const size_t s = fwrite(buf, 1, len, stdout);
    if (s < len)
      perror("fwrite()"); // POSIX requires that errno is set
  }

/**
 * Display a message at the bottom line, using STATUS_COLOR, to stdout.
 * @param fmt: Message format
 * @param args: values to be printed seeing fmt.
 **/
  void display_message(const char fmt[], ...) {
    va_list args;

    va_start(args, fmt);
    gotoxy(0, _lines);
    outstr(STATUS_COLOR);
    vprintf(fmt, args);
    outstr(CLREOL TEXT_COLOR);
    fflush(stdout);
    va_end(args);
  }

/**
 * Wait for a 'y' or 'Y' char at stdin.
 * @return 1 if 'y' or 'Y' was typped in, 0 if not.
 **/
  int ask() {
    const int ch = getchar();
    return ch == 'y' || ch == 'Y';
  }

/**
 * Clear all the screen using CLRSCR string.
 **/
  void clear_screen() {
    outstr(CLRSCR);
  }

/**
 * Save indicated Editor's content to his defined filename.
 * @param Editor a reference to the Editor to be saved.
 * @return Error status.
 **/
  int save_file(Editor &ed) {
    const int f = open(ed.getFilename(), O_CREAT | O_TRUNC | O_WRONLY);
    if (f < 0) return -1;

    if (!ed.write_textbuffer_to_file(f)) {
      close(f);
      return -1;
    } else {
      close(f);
      ed.clearDirty();
      ed.clear_undo();
      return 0;
    }
  }

  char* prompt(const char msg[], const char entry[] = NULL) {
    char buf[1024];

    gotoxy(0, lines());
    outstr(STATUS_COLOR);
    outstr(msg);
    outstr(CLREOL);

    const int maxlen = cols() - strlen(msg) - 1;
    if (entry) outstr(entry);

    for (int len = 0;;) {
      fflush(stdout);
      const int ch = getkey();
      if (ch == KEY_ESC) {
        return NULL;
      } else if (ch == KEY_ENTER) {
        buf[len] = 0;
        return strndup(buf, 1024);
      } else if (ch == KEY_BACKSPACE) {
        if (len > 0) {
          outstr("\b \b");
          len--;
        }
      } else if (ch >= ' ' && ch < 0x100 && len < maxlen && len < 1023) {
        outch(ch);
        buf[len++] = ch;
      }
    }
  }

  void help() {
    gotoxy(0, 0);
    clear_screen();
    outstr("Editor Command Summary\r\n");
    outstr("======================\r\n\r\n");
    outstr("<up>         Move one line up (*)         Ctrl+N  New editor\r\n");
    outstr("<down>       Move one line down (*)       Ctrl+O  Open file\r\n");
    outstr("<left>       Move one character left (*)  Ctrl+S  Save file\r\n");
    outstr("<right>      Move one character right (*) Ctrl+W  Close file\r\n");
    outstr("<pgup>       Move one page up (*)         Ctrl+Q  Quit\r\n");
    outstr("<pgdn>       Move one page down (*)       Ctrl+P  Pipe command\r\n");
    outstr("Ctrl+<left>  Move to previous word (*)    Ctrl+A  Select all\r\n");
    outstr("Ctrl+<right> Move to next word (*)        Ctrl+C  Copy selection to clipboard\r\n");
    outstr("<home>       Move to start of line (*)    Ctrl+X  Cut selection to clipboard\r\n");
    outstr("<end>        Move to end of line (*)      Ctrl+V  Paste from clipboard\r\n");
    outstr("Ctrl+<home>  Move to start of file (*)    Ctrl+Z  Undo\r\n");
    outstr("Ctrl+<end>   Move to end of file (*)      Ctrl+R  Redo\r\n");
    outstr("<backspace>  Delete previous character    Ctrl+F  Find text\r\n");
    outstr("<delete>     Delete current character     Ctrl+G  Find next\r\n"); 
    outstr("Ctrl+<tab>   Next editor                  Ctrl+L  Goto line\r\n");
    outstr("<tab>        Indent selection             F1      Help\r\n");
    outstr("Shift+<tab>  Unindent selection           F3      Navigate to file\r\n");
    outstr("                                          F5      Redraw screen\r\n");
    outstr("\r\n(*) Extends selection if combined with Shift");
    outstr("\r\nPress any key to continue...");
    fflush(stdout);

    getkey();
    // this->draw_screen();
    // this->draw_full_statusline();
  }

  // int new_file(const char filename[]) {
  //   if (*filename) {
  //     strcpy(this->_filename, filename);
  //   } else {
  //     sprintf(this->_filename, "Untitled-%d", ++env.untitled);
  //     this->newfile = 1;
  //   }
  //   this->permissions = 0644;

  //   this->start = (char *) malloc(MINEXTEND);
  //   if (!this->start) return -1;
  // #ifdef DEBUG
  //   memset(this->start, 0, MINEXTEND);
  // #endif

  //   this->gap = this->start;
  //   this->rest = this->_end = this->gap + MINEXTEND;
  //   this->anchor = -1;
    
  //   return 0;
  // }

  // int load_file(const char filename[]) {
  //   struct stat statbuf;
  //   int length;
  //   int f;

  //   if (!realpath(filename, this->_filename)) return -1;
  //   f = open(this->_filename, O_RDONLY);
  //   if (f < 0) return -1;

  //   if (fstat(f, &statbuf) < 0) goto err;
  //   length = statbuf.st_size;
  //   this->permissions = statbuf.st_mode & 0777;

  //   this->start = (char *) malloc(length + MINEXTEND);
  //   if (!this->start) goto err;
  // #ifdef DEBUG
  //   memset(this->start, 0, length + MINEXTEND);
  // #endif
  //   if (read(f, this->start, length) != length) goto err;

  //   this->gap = this->start + length;
  //   this->rest = this->_end = this->gap + MINEXTEND;
  //   this->anchor = -1;

  //   close(f);
  //   return 0;

  // err:
  //   close(f);
  //   if (this->start) {
  //     free(this->start);
  //     this->start = NULL;
  //   }
  //   return -1;
  // }


public:
//  char *linebuf;    // Scratch buffer
  char *search;     // Search text

  char *clipboard;  // Clipboard
  int clipsize;     // Clipboard size

  int untitled;     // Counter for untitled files

private:
  std::vector<Editor> editors;

  Editor *current;  // Current editor



  int _cols;        // Console columns
  int _lines;       // Console lines
 
};

