#include "L1Region.h"
#include <iso646.h>

#ifndef L1RegionAllocationAlignment
#define L1RegionAllocationAlignment 16
#endif

struct L1Region
{
	void* bytes;
	size_t usedByteCount, allocatedByteCount;
	L1Region* subregion;
};

L1Region* L1RegionNew(void)
{
	return calloc(1, sizeof(L1Region));
}

void* L1RegionAllocate(L1Region* self, size_t byteCount)
{
	//return calloc(1, byteCount);
	if (not self->bytes)
	{
		self->allocatedByteCount = byteCount * 2;
		self->usedByteCount = 0;
		self->bytes = calloc(1, self->allocatedByteCount);
	}
	size_t alignmentDifference = self->usedByteCount % L1RegionAllocationAlignment;
	if (self->usedByteCount + byteCount + alignmentDifference <= self->allocatedByteCount)
	{
		self->usedByteCount += alignmentDifference;
		void* bytes = self->usedByteCount + (char*) self->bytes;
		self->usedByteCount += byteCount;
		return bytes;
	}
	else
	{
		if (not self->subregion) self->subregion = L1RegionNew();
		return L1RegionAllocate(self->subregion, byteCount);
	}
}

void L1RegionDelete(L1Region* self)
{
	if (not self) return;
	L1RegionDelete(self->subregion);
	free(self->bytes);
	free(self);
}