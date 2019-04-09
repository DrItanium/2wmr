/* (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * See LICENSE file for license details.
 */
#include "2wm.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <X11/Xutil.h>
#include <sstream>


typedef struct { regex_t *regex; } RReg;

/* static */

FLOATS

static RReg *rreg = NULL;
static unsigned int reglen = 0;

static Client *
getnext(Client *c) {
	for(; c && c->view != view; c = c->next);
	return c;
}

static Client *
getprev(Client *c) {
	for(; c && c->view != view; c = c->prev);
	return c;
}

static Client *
nexttiled(Client *c) {
	for(c = getnext(c); c && c->isfloat; c = getnext(c->next));
	return c;
}

void
Client::pop() {
    detachclient();
	if(clients) {
		clients->prev = this;
    }
    this->next = clients;
	clients = this;
}

static void
togglemax(Client *c) {
	XEvent ev;
		
	if(c->isfixed)
		return;

	if((c->ismax = !c->ismax)) {
		c->rx = c->x; c->x = sx;
		c->ry = c->y; c->y = sy;
		c->rw = c->w; c->w = sw - 2 * BORDERPX;
		c->rh = c->h; c->h = sh - 2 * BORDERPX;
	}
	else {
		c->x = c->rx;
		c->y = c->ry;
		c->w = c->rw;
		c->h = c->rh;
	}
    c->resize(true);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

/* extern */

void
arrange(void) {
	unsigned int i, n, mw, mh, tw, th;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;
	/* window geoms */
	mh = (n > nmaster) ? sh / nmaster : sh / (n > 0 ? n : 1);
	mw = (n > nmaster) ? (sw * master) / 1000 : sw;
	th = (n > nmaster) ? sh / (n - nmaster) : 0;
	tw = sw - mw;

	for(i = 0, c = clients; c; c = c->next)
		if(c->view == view) {
			if(c->isfloat) {
				c->resize(true);
				continue;
			}
			c->ismax = False;
			c->x = sx;
			c->y = sy;
			if(i < nmaster) {
				c->x += tw;
				c->y += i * mh;
				c->w = mw - 2 * BORDERPX;
				c->h = mh - 2 * BORDERPX;
			}
			else {  /* tile window */
				c->w = tw - 2 * BORDERPX;
				if(th > 2 * BORDERPX) {
					c->y += (i - nmaster) * th;
					c->h = th - 2 * BORDERPX;
				}
				else /* fallback if th <= 2 * BORDERPX */
					c->h = sh - 2 * BORDERPX;
			}
            c->resize(false);
			i++;
		}
		else
			XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	if(!sel || sel->view != view) {
		for(c = stack; c && c->view != view; c = c->snext);
        c->focus();
	}
	restack();
}

void
attach(Arg *arg) {
	Client *c;

	for(c = clients; c && c->view == view; c = c->next);
	if(!c)
		return;
	c->view = !c->view;
    c->pop();
    c->focus();
	arrange();
}

void
detach(Arg *arg) {
	if(!sel)
		return;
	sel->view = !sel->view;
    sel->pop();
	arrange();
}

void
focusnext(Arg *arg) {
	Client *c;
   
	if(!sel)
		return;
	if(!(c = getnext(sel->next)))
		c = getnext(clients);
	if(c) {
        c->focus();
		restack();
	}
}

void
focusprev(Arg *arg) {
	//Client *c;

	if(!sel) {
		return;
    }
    auto c = getprev(sel->prev);
    if (!c) {
		for(c = clients; c && c->next; c = c->next);
		c = getprev(c);
	}
	if(c) {
        c->focus();
		restack();
	}
}

void
incnmaster(Arg *arg) {
	if((nmaster + arg->i < 1) || (sh / (nmaster + arg->i) <= 2 * BORDERPX))
		return;
	nmaster += arg->i;
	if(sel)
		arrange();
}

void
initrregs(void) {
	unsigned int i;
	regex_t *reg;

	if(rreg)
		return;
	for(reglen = 0; floats[reglen]; reglen++);
    rreg = new RReg[reglen];
	for(i = 0; i < reglen; i++)
		if(floats[i]) {
            reg = new regex_t;
			if(regcomp(reg, floats[i], REG_EXTENDED)) {
                delete reg;
            } else {
				rreg[i].regex = reg;
            }
		}
}

bool
Client::isFloat() {
	unsigned int i;
	regmatch_t tmp;
	Bool ret = False;
	XClassHint ch = { 0 };

	XGetClassHint(dpy, win, &ch);
    std::stringstream ss;
    print(ss, ch.res_class ? ch.res_class : "",
              ":",
              ch.res_name ? ch.res_name : "",
              ":",
              name);
    auto str = ss.str();
	for(i = 0; i < reglen; i++)
		if(rreg[i].regex && !regexec(rreg[i].regex, str.c_str(), 1, &tmp, 0)) {
			ret = True;
			break;
		}
	if(ch.res_class) {
		XFree(ch.res_class);
    }
	if(ch.res_name) {
		XFree(ch.res_name);
    }
	return ret;
}

void
resizemaster(Arg *arg) {
	if(arg->i == 0) {
		master = MASTER;
    } else {
		if(sw * (master + arg->i) / 1000 >= sw - 2 * BORDERPX
			|| sw * (master + arg->i) / 1000 <= 2 * BORDERPX) {
			return;
        }
		master += arg->i;
	}
	arrange();
}

void
restack(void) {

	if(!sel) {
		return;
    }
	if(sel->isfloat) {
		XRaiseWindow(dpy, sel->win);
    } else {
		XLowerWindow(dpy, sel->win);
    }
	for(Client* c = nexttiled(clients); c; c = nexttiled(c->next)) {
		if(c == sel) {
			continue;
        }
		XLowerWindow(dpy, c->win);
	}
	XSync(dpy, False);
	XEvent ev;
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
togglefloat(Arg *arg) {
	if(!sel) {
		return;
    }
	sel->isfloat = !sel->isfloat;
	arrange();
}

void
toggleview(Arg *arg) {
	view = !view;
	arrange();
}

void
zoom(Arg *arg) {
	uint n;
	Client *c;

	if(!sel) {
		return;
    }
	if(sel->isfloat) {
		togglemax(sel);
		return;
	}
	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next)) {
		n++;
    }
    c = sel;
	if(c == nexttiled(clients)) {
		if(!(c = nexttiled(c->next))) { 
			return;
        }
    }
    c->pop();
    c->focus();
	arrange();
}
