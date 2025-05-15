#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>

void panic(char *msg) {
    puts(msg);
    exit(EXIT_FAILURE);
}

Display *dpy;
Window root;

void grabKey(char *key, unsigned int mod) {
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

int main(void) {
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        panic("Unable to open X display.");
    }

    root = DefaultRootWindow(dpy);

    XSelectInput(dpy, root, ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask);

    Cursor cursor = XCreateFontCursor(dpy, XC_left_ptr);
    XDefineCursor(dpy, root, cursor);
    XSync(dpy, False);

    XGrabButton(dpy, Button1, 0, root, False, ButtonPressMask,
                GrabModeAsync, GrabModeAsync, None, None);

    grabKey("a", ShiftMask);

    XEvent e;
    for (;;) {
        XNextEvent(dpy, &e);
        switch (e.type) {
        case ButtonPress:
            puts("Button pressed!");
            break;
        case ButtonRelease:
            puts("Button released!");
            break;
        case KeyPress:
            puts("Key pressed!");
            break;
        case KeyRelease:
            puts("Key released!");
            break;
        default:
            printf("Unexpected event: %d\n", e.type);
            break;
        }
    }

    XCloseDisplay(dpy);
    return 0;
}