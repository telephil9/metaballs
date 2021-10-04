#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>

typedef struct Vec Vec;
typestr Vec _Vec;

struct Vec
{
	double x;
	double y;
};

Vec vec(double, double);
double vsqrlen(Vec);

enum
{
	Emouse,
	Eresize,
	Ekeyboard,
};
int bg[] = { 0xF5, 0xEF, 0xE8 };
int fg[] = { 0x55, 0x55, 0xFF };


Mousectl *mctl;
Keyboardctl *kctl;
int width;
int height;
uchar *buf;
int nbuf;
Image *b;
Vec v1;
Vec v2;

void
setcol(uchar *buf, int col[3])
{
	buf[0] = col[2];
	buf[1] = col[1];
	buf[2] = col[0];
}

void
redraw(void)
{
	int x, y, n, stride;
	Vec p;
	double s, s1, s2;

	stride = bytesperline(b->r, b->depth);
	for(y = 0; y < height; y++){
		for(x = 0; x < width; x++){
			n = y * stride + 3 * x;
			p = vec(x, y) + vec(0.5, 0.5);
			s1 = 1.0/sqrt(vsqrlen(v1 - p));
			s2 = 1.0/sqrt(vsqrlen(v2 - p));
			s = s1 + s2;
			if(s >= 0.009)
				setcol(buf+n, fg);
			else
				setcol(buf+n, bg);
		}
	}
	if(loadimage(b, b->r, buf, nbuf)<0)
		sysfatal("loadimage: %r");
	lockdisplay(display);
	draw(screen, screen->r, b, nil, ZP);
	flushimage(display, 1);
	unlockdisplay(display);
}

void
initbuf(void)
{
	freeimage(b);
	width = Dx(screen->r);
	height = Dy(screen->r);
	b = allocimage(display, Rect(0, 0, width, height), RGB24, 0, DNofill);		
	nbuf = 3 * width * height * sizeof(uchar);
	buf = malloc(nbuf);
	if(buf==nil)
		sysfatal("malloc: %r");
}

void
threadmain(int argc, char *argv[])
{
	USED(argc);
	USED(argv);
	Mouse m;
	Rune k;
	Alt a[] = {
		{ nil, &m,  CHANRCV },
		{ nil, nil, CHANRCV },
		{ nil, &k,  CHANRCV },
		{ nil, nil, CHANEND },
	};

	if(initdraw(nil, nil, nil)<0)
		sysfatal("initdraw: %r");
	unlockdisplay(display);
	display->locking = 1;
	if((mctl=initmouse(nil, screen))==nil)
		sysfatal("initmouse: %r");
	if((kctl=initkeyboard(nil))==nil)
		sysfatal("initkeyboard: %r");
	a[Emouse].c = mctl->c;
	a[Eresize].c = mctl->resizec;
	a[Ekeyboard].c = kctl->c;
	initbuf();
	v1 = vec(150.0, height/2.0);
	v2 = v1;
	v2.x += 200.0;
	redraw();
	for(;;){
		switch(alt(a)){
		case Emouse:
			v2.x = m.xy.x - screen->r.min.x;
			v2.y = m.xy.y - screen->r.min.y;
			redraw();
			break;
		case Eresize:
			if(getwindow(display, Refnone)<0)
				sysfatal("getwindow: %r");
			initbuf();
			redraw();
		case Ekeyboard:
			switch(k){
			case 'q':
			case Kdel:
				threadexitsall(nil);
				break;
			}
		}
	}
}

Vec
vec(double x, double y)
{
	return (Vec){x, y};
}

double
vsqrlen(Vec v)
{
	return v.x*v.x + v.y*v.y;
}

Vec
Vec_add_(Vec a, Vec b)
{
	Vec r;

	r.x = a.x + b.x;
	r.y = a.y + b.y;
	return r;
}

Vec
Vec_sub_(Vec a, Vec b)
{
	Vec r;

	r.x = a.x - b.x;
	r.y = a.y - b.y;
	return r;
}
