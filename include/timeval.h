#ifndef TIMEVAL_H
#define TIMEVAL_H

int timeval_subtract(struct timeval *result,
	struct timeval *x, struct timeval *y);

int timeval_subtract_curr_t(struct timeval *result, struct timeval* x);

#endif