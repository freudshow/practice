#include "rect.h" /* Rectangle class interface */
#include <stdio.h> /* for printf() */

/* Rectangle's prototypes of its virtual functions */
/* NOTE: the "me" pointer has the type of the superclass to fit the vtable */
static uint32_t Rectangle_area_(Shape const *const me);
static void Rectangle_draw_(Shape const *const me);

/* constructor */
void Rectangle_ctor(Rectangle_t *const me, int16_t x, int16_t y, uint16_t width,
		uint16_t height)
{
	static struct ShapeVtbl const vtbl = { /* vtbl of the Rectangle class */
	&Rectangle_area_, &Rectangle_draw_ };
	Shape_ctor(&me->super, x, y); /* call the superclass' ctor */
	me->super.vptr = &vtbl; /* override the vptr */
	me->width = width;
	me->height = height;
}

/* Rectangle's class implementations of its virtual functions... */
static uint32_t Rectangle_area_(Shape const *const me)
{
	Rectangle_t const *const me_ = (Rectangle_t const*) me; /* explicit downcast */
	return (uint32_t) me_->width * (uint32_t) me_->height;
}

static void Rectangle_draw_(Shape const *const me)
{
	Rectangle_t const *const me_ = (Rectangle_t const*) me; /* explicit downcast */
	printf("Rectangle_draw_(x=%d,y=%d,width=%d,height=%d)\n", me_->super.x,
			me_->super.y, me_->width, me_->height);
}
