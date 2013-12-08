#ifndef L1LuaTableOutputFunctions_h
#define L1LuaTableOutputFunctions_h

#include "L1GenerateIR.h"
#include <inttypes.h>
#include <stdio.h>

static void loadInteger(uint64_t destination, uint64_t byteCount, const uint8_t* bytes, void* userdata)
{
	FILE* file = userdata;
	fprintf(file, "{type = \"loadInteger\", destination = %" PRIu64 ", byteCount = %" PRIu64 ", bytes = ", destination, byteCount);
	fprintf(file, "{ ");
	for (uint64_t i = 0; i < byteCount; i++)
	{
		fprintf(file, "%" PRIu8 ", ", bytes[i]);
	}
	fprintf(file, "}}, \n");
}

static void call(uint64_t destination, uint64_t callee, uint64_t argumentCount, const uint64_t* arguments, void* userdata)
{
	FILE* file = userdata;
	fprintf(file, "{type = \"call\", destination = %" PRIu64 ", callee = %" PRIu64 ", argumentCount = %" PRIu64 ", arguments = ", destination, callee, argumentCount);
	fprintf(file, "{ ");
	for (uint64_t i = 0; i < argumentCount; i++)
	{
		fprintf(file, "%" PRIu64 ", ", arguments[i]);
	}
	fprintf(file, "}}, \n");
}

static void closure(uint64_t destination, uint64_t result, uint64_t argumentCount, const uint64_t* arguments, void* userdata)
{
	FILE* file = userdata;
	fprintf(file, "{type = \"closure\", destination = %" PRIu64 ", result = %" PRIu64 ", argumentCount = %" PRIu64 ", arguments = ", destination, result, argumentCount);
	fprintf(file, "{ ");
	for (uint64_t i = 0; i < argumentCount; i++)
	{
		fprintf(file, "%" PRIu64 ", ", arguments[i]);
	}
	fprintf(file, "}}, \n");
}

static void branch(uint64_t destination, uint64_t condition, uint64_t resultIfTrue, uint64_t resultIfFalse, void* userdata)
{
	FILE* file = userdata;
	fprintf(file, "{type = \"branch\", destination = %" PRIu64 ", condition = %" PRIu64 ", resultIfTrue = %" PRIu64 ", resultIfFalse = %" PRIu64 " }, \n", destination, condition, resultIfTrue, resultIfFalse);
}

static void list(uint64_t destination, uint64_t elementCount, const uint64_t* elements, void* userdata)
{
	FILE* file = userdata;
	fprintf(file, "{type = \"list\", destination = %" PRIu64 ", elementCount = %" PRIu64 ", elements = ", destination, elementCount);
	fprintf(file, "{ ");
	for (uint64_t i = 0; i < elementCount; i++)
	{
		fprintf(file, "%" PRIu64 ", ", elements[i]);
	}
	fprintf(file, "}}, \n");
}

static void loadIntegerLessThan(uint64_t destination, void* userdata)
{
	FILE* file = userdata;
	fprintf(file, "{type = \"loadIntegerLessThan\", destination = %" PRIu64 "}, \n", destination);
}

static void loadBooleanFromInteger(uint64_t destination, void* userdata)
{
	FILE* file = userdata;
	fprintf(file, "{type = \"loadBooleanFromInteger\", destination = %" PRIu64 "}, \n", destination);
}

static void export(uint64_t source, void* userdata)
{
	FILE* file = userdata;
	fprintf(file, "{type = \"export\", source = %" PRIu64 "}, \n", source);
}

const L1GenerateIROutputFunctions L1LuaTableOutputFunctions =
{
	loadInteger,
	call,
	closure,
	branch,
	list,
	loadIntegerLessThan,
	loadBooleanFromInteger,
	export
};

#endif