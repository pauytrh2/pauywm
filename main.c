#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define MAX_WINDOWS 64

Display *dpy;
Window root;
Window managed_windows[MAX_WINDOWS];
int window_count = 0;
bool running = true;

void panic(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

void launch_program(const char *prog) {
    pid_t pid = fork();
    if (pid == 0) {
        setsid();  // separate process group
        execlp(prog, prog, NULL);
        perror("execlp failed");
        _exit(1);
    } else if (pid < 0) {
        perror("fork failed");
    }
}

void grabKey(const char *key, unsigned int mod) {
    KeySym sym = XStringToKeysym(key);
    if (sym == NoSymbol) {
        fprintf(stderr, "Invalid keysym: %s\n", key);
        exit(EXIT_FAILURE);
    }
    KeyCode code = XKeysymToKeycode(dpy, sym);
    if (code == 0) {
        fprintf(stderr, "No keycode for keysym %s\n", key);
        exit(EXIT_FAILURE);
    }
    XGrabKey(dpy, code, mod, root, False, GrabModeAsync, GrabModeAsync);
    XSync(dpy, False);
}

bool is_managed(Window w) {
    for (int i = 0; i < window_count; ++i) {
        if (managed_windows[i] == w) return true;
    }
    return false;
}

void tile_windows() {
    if (window_count == 0) return;

    int screen = DefaultScreen(dpy);
    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);
    int win_height = sh / window_count;

    for (int i = 0; i < window_count; ++i) {
        XMoveResizeWindow(dpy, managed_windows[i],
                          0, i * win_height,
                          sw, win_height);
    }
    XSync(dpy, False);
}

void manage_window(Window w) {
    if (window_count >= MAX_WINDOWS || is_managed(w)) return;

    managed_windows[window_count++] = w;
    XSelectInput(dpy, w, StructureNotifyMask);
    XMapWindow(dpy, w);
    tile_windows();
}

int main(void) {
    dpy = XOpenDisplay(NULL);
    if (!dpy) panic("Unable to open X display.");

    root = DefaultRootWindow(dpy);
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);

    Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);
    XDefineCursor(dpy, root, cursor);
    XSync(dpy, False);

    grabKey("Escape", ShiftMask);  // shift + esc to exit
    grabKey("t", Mod4Mask);        // super + t for xterm

    XEvent ev;
    while (running) {
        XNextEvent(dpy, &ev);
        switch (ev.type) {
        case KeyPress: {
            XKeyEvent *kev = &ev.xkey;
            KeySym sym = XkbKeycodeToKeysym(dpy, kev->keycode, 0, 0);
            if (sym == XK_Escape && (kev->state & ShiftMask)) {
                puts("Exit combo pressed.");
                running = false;
            } else if (sym == XK_t && (kev->state & Mod4Mask)) {
                puts("Launching terminal...");
                launch_program("xterm");
            }
            break;
        }
        case MapRequest: {
            XMapRequestEvent *map = &ev.xmaprequest;
            manage_window(map->window);
            break;
        }
        case UnmapNotify: {
            XUnmapEvent *unmap = &ev.xunmap;
            for (int i = 0; i < window_count; ++i) {
                if (managed_windows[i] == unmap->window) {
                    // rm window
                    memmove(&managed_windows[i], &managed_windows[i+1],
                            sizeof(Window) * (window_count - i - 1));
                    window_count--;
                    tile_windows();
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    XCloseDisplay(dpy);
    return 0;
}