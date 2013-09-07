#ifndef L1Region_h
#define L1Region_h

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct L1Region L1Region;

L1Region* L1RegionNew(void);
void* L1RegionAllocate(L1Region* self, size_t byteCount);
void L1RegionDelete(L1Region* self);

#ifdef __cplusplus
}//extern "C"
#endif

#endif