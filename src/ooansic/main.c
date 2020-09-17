#include "shape.h" /* Shape class interface */
#include "rect.h" /* Rectangle class interface */
#include "circle.h" /* Circle class interface */
#include <stdio.h> /* for printf() */

int main()
{
	Shape s1, s2; /* multiple instances of Shape */
	Shape_ctor(&s1, 0, 1);
	Shape_ctor(&s2, -1, 2);
	printf("Shape s1(x=%d,y=%d)\n", Shape_getX(&s1), Shape_getY(&s1));
	printf("Shape s2(x=%d,y=%d)\n", Shape_getX(&s2), Shape_getY(&s2));
	Shape_moveBy(&s1, 2, -4);
	Shape_moveBy(&s2, 1, -2);
	printf("Shape s1(x=%d,y=%d)\n", Shape_getX(&s1), Shape_getY(&s1));
	printf("Shape s2(x=%d,y=%d)\n", Shape_getX(&s2), Shape_getY(&s2));

	Rectangle_t r1, r2; /* multiple instances of Rect */
	/* instantiate rectangles... */
	Rectangle_ctor(&r1, 0, 2, 10, 15);
	Rectangle_ctor(&r2, -1, 3, 5, 8);
	printf("Rect r1(x=%d,y=%d,width=%d,height=%d)\n", r1.super.x, r1.super.y,
			r1.width, r1.height);
	printf("Rect r2(x=%d,y=%d,width=%d,height=%d)\n", r2.super.x, r2.super.y,
			r2.width, r2.height);
	/* re-use inherited function from the superclass Shape... */
	Shape_moveBy((Shape*) &r1, -2, 3);
	Shape_moveBy(&r2.super, 2, -1);
	printf("Rect r1(x=%d,y=%d,width=%d,height=%d)\n", r1.super.x, r1.super.y,
			r1.width, r1.height);
	printf("Rect r2(x=%d,y=%d,width=%d,height=%d)\n", r2.super.x, r2.super.y,
			r2.width, r2.height);

	Circle_t c1, c2; /* multiple instances of Circle */
	Shape const *shapes[] = { /* collection of shapes */
	&c1.super, &r2.super, &c2.super, &r1.super };
	Shape const *s;
	/* instantiate rectangles... */
	Rectangle_ctor(&r1, 0, 2, 10, 15);
	Rectangle_ctor(&r2, -1, 3, 5, 8);
	/* instantiate circles... */
	Circle_ctor(&c1, 1, -2, 12);
	Circle_ctor(&c2, 1, -3, 6);
	s = largestShape(shapes, sizeof(shapes) / sizeof(shapes[0]));
	printf("largetsShape s(x=%d,y=%d)\n", s->x, s->y);
	drawAllShapes(shapes, sizeof(shapes) / sizeof(shapes[0]));

	return 0;
}
