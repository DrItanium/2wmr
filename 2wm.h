/* (C)opyright MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#ifndef _2WMR_H__
#define _2WMR_H__

#include "config.h"
#include <X11/Xlib.h>
#include <utility>
#include <iostream>
#include <cstdint>

using ulong = unsigned long;
using uint = unsigned int;
/* mask shorthands, used in event.c and client.c */
#define BUTTONMASK		(ButtonPressMask | ButtonReleaseMask)

enum { NetSupported, NetWMName, NetLast };		/* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMLast };	/* default atoms */
enum { CurNormal, CurResize, CurMove, CurLast };	/* cursor */


union Arg {
	const char *cmd;
	int i;
}; /* argument type */

struct Client {
	char name[256];
	int x, y, w, h;
	int rx, ry, rw, rh; /* revert geometry */
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int minax, minay, maxax, maxay;
	long flags; 
	uint border;
	bool isfixed, isfloat, ismax, view;
	Client *next;
	Client *prev;
	Client *snext;
	Window win;
    void configure();
    void detachclient();
    void focus();
    static Client* getclient(Window w);
    bool isprotodel();
    void resize(bool sizeHints);
    void updatesizehints();
    void updatetitle();
    void unmanage();
    void pop();
    bool isFloat();
    void togglemax();
    

    ~Client();
};

extern int screen, sx, sy, sw, sh;		/* screen geometry */
extern uint master, nmaster;		/* master percent, number of master clients */
extern uint numlockmask;		/* dynamic key lock mask */
extern void (*handler[LASTEvent])(XEvent *);	/* event handler */
extern Atom wmatom[WMLast], netatom[NetLast];
extern bool running, selscreen, view;
extern Client *clients, *sel, *stack;		/* global client list and stack */
extern Cursor cursor[CurLast];
extern ulong normcol, selcol;		/* sel/normal color */
extern Display *dpy;
extern Window root;

/* client.c */
//void configure(Client *c);		/* send synthetic configure event */
//void detachclient(Client *c);		/* detaches c from global client list */
//void focus(Client *c);			/* focus c, c may be NULL */
//Client *getclient(Window w);		/* return client of w */
//bool isprotodel(Client *c);		/* returns True if c->win supports wmatom[WMDelete] */
void killclient(Arg *arg);		/* kill c nicely */
void manage(Window w, XWindowAttributes *wa);	/* manage new client */
//void resize(Client *c, bool sizehints);	/* resize c*/
//void updatesizehints(Client *c);		/* update the size hint variables of c */
//void updatetitle(Client *c);		/* update the name of c */
//void unmanage(Client *c);		/* destroy c */

/* event.c */
void grabkeys(void);			/* grab all keys defined in config.h */

/* main.c */
void quit(Arg *arg);			/* quit 2wm nicely */
void sendevent(Window w, Atom a, long value);	/* send synthetic event to w */
int xerror(Display *dsply, XErrorEvent *ee);	/* 2wm's X error handler */

/* util.c */
void *emallocz(uint size);	/* allocates zero-initialized memory, exits on error */
//void eprint(const char *errstr, ...);	/* prints errstr and exits with 1 */
template<typename ... Args>
void print(std::ostream& os, Args&& ... args) {
    (os << ... << args);
}

template<typename ... Args>
void eprint(Args&& ... args) {
    print(std::cerr, args...);
}
void spawn(Arg *arg);			/* forks a new subprocess with to arg's cmd */

/* view.c */
void arrange(void);			/* arranges all windows tiled */
void attach(Arg *arg);			/* attaches most recent detached client to view */
void detach(Arg *arg);			/* detaches current client from view */
void focusnext(Arg *arg);		/* focuses next visible client, arg is ignored  */
void focusprev(Arg *arg);		/* focuses previous visible client, arg is ignored */
void incnmaster(Arg *arg);		/* increments nmaster with arg's index value */
void initrregs(void);			/* initialize regexps of rules defined in config.h */
bool isfloat(Client *c);			/* returns True if c is floatings */
void resizemaster(Arg *arg);		/* resizes the master percent with arg's index value */
void restack(void);			/* restores z layers of all clients */
void togglefloat(Arg *arg);		/* toggles focusesd client between floating/non-floating state */
void toggleview(Arg *arg);		/* toggles view */
void zoom(Arg *arg);			/* zooms the focused client to master area, arg is ignored */
#endif // end _2WMR_H__
