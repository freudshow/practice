#include "stdio.h"
#include "conio.h"
#include "graphics.h"
#include <time.h>
#include <math.h>

void ellipsepoint(long x0,long y0,long x,long y,long color)
{
	putpixel((int)(x0+x),(int)(y0+y),(int)color);
	putpixel((int)(x0-x),(int)(y0+y),(int)color);
	putpixel((int)(x0+x),(int)(y0-y),(int)color);
	putpixel((int)(x0-x),(int)(y0-y),(int)color);
}


void midpointellipse(long x0,long y0,long a,long b,long color)
{
	long x,y,d,sa,sb,xp,yp;
	sa=a*a,sb=b*b;
	xp=(long)((float)sa/(float)sqrt((float)(sa+sb)));
	yp=(long)((float)sb/(float)sqrt((float)(sa+sb)));
	x=0,y=b,d=sa+4*sb-4*sa*b;
	while(x<xp)
	{
		if(d<0)
		{
			d=d+4*sb*(2*x+3);
			x++;
		}
		else
		{
			d=d+4*sb*(2*x+3)+4*sa*(2-2*y);
			x++;
			y--;
		}
		ellipsepoint(x0,y0,x,y,color);
	}

	x=a,y=0,d=4*sa+sb-4*a*sb;
	while(y<yp)
	{
		if(d<0)
		{
			d=d+4*sa*(2*y+3);
			y++;
		}
		else
		{
			d=d+4*sa*(2*y+3)+4*sb*(2-2*x);
			y++;
			x--;
		}
		ellipsepoint(x0,y0,x,y,color);
	}
}


main()
{
	clock_t start=clock(),end,t;
	int graphdriver=VGA,graphmode=VGAHI;
	registerbgidriver(EGAVGA_driver);
	initgraph(&graphdriver,&graphmode,"");

	midpointellipse(200,300,300,180,RED);    /*µ÷ÓÃËã·¨*/

	end=clock();
	t=end-start;
	printf("%d\n",t);
	getch();
	closegraph();
}
