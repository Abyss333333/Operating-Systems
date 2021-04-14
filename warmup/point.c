#include <assert.h>
#include "common.h"
#include "point.h"
#include <math.h>

void
point_translate(struct point *p, double x, double y)
{
    double x_ = p->x + x;
    double y_ = p->y + y;
	p->x = x_;
    p->y = y_;
}

double
point_distance(const struct point *p1, const struct point *p2)
{
	double x1 = p1->x;
    double x2 = p2->x;
    double y1 = p1->y;
    double y2 = p2->y;
    double distance = ((x2-x1)*(x2-x1)) + ((y2-y1)*(y2-y1));
    
	return (sqrt(distance));
}

int
point_compare(const struct point *p1, const struct point *p2)
{
	double x1 = p1->x;
    double x2 = p2->x;
    double y1 = p1->y;
    double y2 = p2->y;
    double d1 = ((x1-0)*(x1-0)) + ((y1-0)*(y1-0));
    double e1 = sqrt(d1);
    double d2 = ((x2-0)*(x2-0)) + ((y2-0)*(y2-0));
    double e2 = sqrt(d2);
    if (e1 == e2){
	    return 0;
    }
    else if (e1 < e2){
        return -1;
    }
    else {
        return 1 ;
    }

}
