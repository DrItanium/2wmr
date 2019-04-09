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


typedef struct { regex_t *regex; } RReg;

/* static */

FLOATS

static RReg *rreg = NULL;
static unsigned int reglen = 0;

static Client *
getnext(Client *c) {
    ENTER_FUNC;
	for(; c && c->view != view; c = c->next);
    EXIT_FUNC;
	return c;
}

static Client *
getprev(Client *c) {
    ENTER_FUNC;
	for(; c && c->view != view; c = c->prev);
    EXIT_FUNC;
	return c;
}

static Client *
nexttiled(Client *c) {
    ENTER_FUNC;
	for(c = getnext(c); c && c->isfloat; c = getnext(c->next));
    EXIT_FUNC;
	return c;
}

static void
pop(Client *c) {
    ENTER_FUNC;
	detachclient(c);
	if(clients)
		clients->prev = c;
	c->next = clients;
	clients = c;
    EXIT_FUNC;
}

static void
togglemax(Client *c) {
	XEvent ev;
		
    ENTER_FUNC;
	if(c->isfixed) {
        EXIT_FUNC;
		return;
    }

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
	resize(c, True);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
    EXIT_FUNC;
}

/* extern */

void
arrange(void) {
	unsigned int i, n, mw, mh, tw, th;
	Client *c;

    ENTER_FUNC;
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
				resize(c, True);
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
			resize(c, False);
			i++;
		}
		else
			XMoveWindow(dpy, c->win, c->x + 2 * sw, c->y);
	if(!sel || sel->view != view) {
		for(c = stack; c && c->view != view; c = c->snext);
		focus(c);
	}
	restack();
    EXIT_FUNC;
}

void
attach(Arg *arg) {
	Client *c;

    ENTER_FUNC;
	for(c = clients; c && c->view == view; c = c->next);
	if(!c) {
        EXIT_FUNC;
		return;
    }
	c->view = !c->view;
	pop(c);
	focus(c);
	arrange();
    EXIT_FUNC;
}

void
detach(Arg *arg) {
    ENTER_FUNC;
	if(!sel) {
        EXIT_FUNC;
		return;
    }
	sel->view = !sel->view;
	pop(sel);
	arrange();
    EXIT_FUNC;
}

void
focusnext(Arg *arg) {
	Client *c;
    ENTER_FUNC;
	if(!sel) {
        EXIT_FUNC;
		return;
    }
	if(!(c = getnext(sel->next)))
		c = getnext(clients);
	if(c) {
		focus(c);
		restack();
	}
    EXIT_FUNC;
}

void
focusprev(Arg *arg) {
	Client *c;

    ENTER_FUNC;
	if(!sel) {
        EXIT_FUNC;
		return;
    }
	if(!(c = getprev(sel->prev))) {
		for(c = clients; c && c->next; c = c->next);
		c = getprev(c);
	}
	if(c) {
		focus(c);
		restack();
	}
    EXIT_FUNC;
}

void
incnmaster(Arg *arg) {
    ENTER_FUNC;
	if((nmaster + arg->i < 1) || (sh / (nmaster + arg->i) <= 2 * BORDERPX)) {
        EXIT_FUNC;
		return;
    }
	nmaster += arg->i;
	if(sel)
		arrange();
    EXIT_FUNC;
}

void
initrregs(void) {
	unsigned int i;
	regex_t *reg;
    ENTER_FUNC;
	if(rreg) {
        EXIT_FUNC;
		return;
    }
	for(reglen = 0; floats[reglen]; reglen++);
	rreg = emallocz(reglen * sizeof(RReg));
	for(i = 0; i < reglen; i++)
		if(floats[i]) {
			reg = emallocz(sizeof(regex_t));
			if(regcomp(reg, floats[i], REG_EXTENDED))
				free(reg);
			else
				rreg[i].regex = reg;
		}
    EXIT_FUNC;
}

Bool
isfloat(Client *c) {
	char prop[512];
	unsigned int i;
	regmatch_t tmp;
	Bool ret = False;
	XClassHint ch = { 0 };
    ENTER_FUNC;
	XGetClassHint(dpy, c->win, &ch);
	snprintf(prop, sizeof prop, "%s:%s:%s",
			ch.res_class ? ch.res_class : "",
			ch.res_name ? ch.res_name : "", c->name);
	for(i = 0; i < reglen; i++)
		if(rreg[i].regex && !regexec(rreg[i].regex, prop, 1, &tmp, 0)) {
			ret = True;
			break;
		}
	if(ch.res_class)
		XFree(ch.res_class);
	if(ch.res_name)
		XFree(ch.res_name);
    EXIT_FUNC;
	return ret;
}

void
resizemaster(Arg *arg) {
    ENTER_FUNC;
	if(arg->i == 0)
		master = MASTER;
	else {
		if(sw * (master + arg->i) / 1000 >= sw - 2 * BORDERPX
			|| sw * (master + arg->i) / 1000 <= 2 * BORDERPX) {
            EXIT_FUNC;
			return;
        }
		master += arg->i;
	}
	arrange();
    EXIT_FUNC;
}

void
restack(void) {
	Client *c;
	XEvent ev;

    ENTER_FUNC;
	if(!sel) {
        EXIT_FUNC;
		return;
    }
	if(sel->isfloat)
		XRaiseWindow(dpy, sel->win);
	else
		XLowerWindow(dpy, sel->win);
	for(c = nexttiled(clients); c; c = nexttiled(c->next)) {
		if(c == sel)
			continue;
		XLowerWindow(dpy, c->win);
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
    EXIT_FUNC;
}

void
togglefloat(Arg *arg) {
    ENTER_FUNC;
	if(!sel) {
        EXIT_FUNC;
		return;
    }
	sel->isfloat = !sel->isfloat;
	arrange();
    EXIT_FUNC;
}

void
toggleview(Arg *arg) {
    ENTER_FUNC;
	view = !view;
	arrange();
    EXIT_FUNC;
}

void
zoom(Arg *arg) {
	unsigned int n;
	Client *c;
    ENTER_FUNC;
	if(!sel) {
        EXIT_FUNC;
		return;
    }
	if(sel->isfloat) {
		togglemax(sel);
        EXIT_FUNC;
		return;
	}
	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

	if((c = sel) == nexttiled(clients))
		if(!(c = nexttiled(c->next))) {
            EXIT_FUNC;
			return;
        }
	pop(c);
	focus(c);
	arrange();
    EXIT_FUNC;
}
