#include "shape.h"
#include <assert.h>

/* Shape's prototypes of its virtual functions */
static uint32_t Shape_area_(Shape const *const me);
static void Shape_draw_(Shape const *const me);

/* constructor */
void Shape_ctor(Shape *const me, int16_t x, int16_t y)
{
	static struct ShapeVtbl const vtbl = { /* vtbl of the Shape class */
	&Shape_area_, &Shape_draw_ };
	me->vptr = &vtbl; /* "hook" the vptr to the vtbl */
	me->x = x;
	me->y = y;
}

/* move-by operation */
void Shape_moveBy(Shape *const me, int16_t dx, int16_t dy)
{
	me->x += dx;
	me->y += dy;
}

/* "getter" operations implementation */
int16_t Shape_getX(Shape *const me)
{
	return me->x;
}

int16_t Shape_getY(Shape *const me)
{
	return me->y;
}

/* Shape class implementations of its virtual functions... */
static uint32_t Shape_area_(Shape const *const me)
{
	assert(0); /* purely-virtual function should never be called */
	return 0U; /* to avoid compiler warnings */
}

static void Shape_draw_(Shape const *const me)
{
	assert(0); /* purely-virtual function should never be called */
}

/* the following code finds the largest-area shape in the collection */
Shape const* largestShape(Shape const *shapes[], uint32_t nShapes)
{
	Shape const *s = (Shape*) 0;
	uint32_t max = 0U;
	uint32_t i;
	for (i = 0U; i < nShapes; ++i) {
		uint32_t area = Shape_area(shapes[i]); /* virtual call */
		if (area > max) {
			max = area;
			s = shapes[i];
		}
	}
	return s; /* the largest shape in the array shapes[] */
}

/* The following code will draw all Shapes on the screen */
void drawAllShapes(Shape const *shapes[], uint32_t nShapes)
{
	uint32_t i;
	for (i = 0U; i < nShapes; ++i) {
		Shape_draw(shapes[i]); /* virtual call */
	}
}
