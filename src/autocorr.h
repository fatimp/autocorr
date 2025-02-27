#ifndef __AUTOCORR_H__
#define __AUTOCORR_H__

#include <stdint.h>
#include <stdlib.h>

struct ac_buffers;

struct ac_buffers* ac_alloc (size_t length);
void ac_free (struct ac_buffers *buffers);
int ac_autocorr (size_t length, struct ac_buffers *buffers);

uint64_t *ac_get_src (const struct ac_buffers *buffers);
uint64_t *ac_get_dst (const struct ac_buffers *buffers);
size_t ac_autocorr_length (size_t length);

#endif
