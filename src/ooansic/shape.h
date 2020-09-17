#ifndef SHAPE_H
#define SHAPE_H
#include <stdint.h>

/* Shape's attributes... */
struct ShapeVtbl;/* forward declaration */

typedef struct {
	struct ShapeVtbl const *vptr; /* <== Shape's Virtual Pointer */
	int16_t x; /* x-coordinate of Shape's position */
	int16_t y; /* y-coordinate of Shape's position */
} Shape;

/* Shape's virtual table */
struct ShapeVtbl {
	uint32_t (*area)(Shape const *const me);
	void (*draw)(Shape const *const me);
};

/* Shape's operations (Shape's interface)... */
void Shape_ctor(Shape *const me, int16_t x, int16_t y);
void Shape_moveBy(Shape *const me, int16_t dx, int16_t dy);
int16_t Shape_getX(Shape * const me);
int16_t Shape_getY(Shape * const me);

static inline uint32_t Shape_area(Shape const *const me)
{
	return (*me->vptr->area)(me);
}

static inline void Shape_draw(Shape const *const me)
{
	(*me->vptr->draw)(me);
}

/* generic operations on collections of Shapes */
Shape const* largestShape(Shape const *shapes[], uint32_t nShapes);
void drawAllShapes(Shape const *shapes[], uint32_t nShapes);

#endif /* SHAPE_H */
