#include "L1Region.h"
#include <iso646.h>

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
	if (not self->bytes) self->bytes = calloc(1, byteCount * 2);
	if (self->usedByteCount + byteCount <= self->allocatedByteCount)
	{
		self->usedByteCount += byteCount;
		return self->usedByteCount + self->bytes;
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