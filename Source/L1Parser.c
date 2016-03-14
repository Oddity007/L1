#include "L1Parser.h"
#include "L1IRSlotDescriptions"
#include "L1IRSlotAccessors"

#include <stdio.h>
#ifdef NDEBUG
#define DebugPrintf(...)
#else
#define DebugPrintf(...) fprintf(stderr, __VA_ARGS__)
#endif
#include <stdio.h>
#include "L1IRSlotDebugInfo"

typedef struct Token Token;
struct Token
{
	char* bytes;
	size_t byteCount;
};

enum BindingType
{
	BindingTypeLet,
	BindingTypeArgument,
	//BindingTypeCapture
};

typedef enum BindingType BindingType;

typedef struct Binding Binding;
struct Binding
{
	size_t tokenID;
	uint16_t localAddress;
	BindingType bindingType;
};

typedef enum ErrorType ErrorType;
enum ErrorType
{
	ErrorTypeUndefinedVariable,
};

typedef struct Error Error;
struct Error
{
	ErrorType type;
};

static void PushError(L1Parser* self, const Error* error)
{
	L1ArrayPush(& self->errorStack, error, sizeof(Error));
}

static void PopError(L1Parser* self)
{
	L1ArrayPop(& self->errorStack, NULL, sizeof(Error));
}

static bool HasErrors(L1Parser* self)
{
	return L1ArrayGetElementCount(& self->errorStack) > 0;
}

static L1IRAddress CreateSlot(L1Parser* self, L1IRSlotType type, uint16_t arg0, uint16_t arg1, uint16_t arg2)
{
	return L1IRStateCreateSlot(& self->irstate, L1IRMakeSlot(type, arg0, arg1, arg2));
}

static uint16_t U16FromChars(char hi, char lo)
{
	uint16_t v = (unsigned char) hi;
	v <<= 8;
	v |= (unsigned char) lo;
	return v;
}

static L1IRAddress CreateString(L1Parser* self, const char* bytes, size_t byteCount)
{
	L1IRAddress previous = 0;
	for (size_t i = 0; i < byteCount; i += 4)
	{
		size_t j = byteCount - i - 1;
		uint16_t low = U16FromChars(byteCount > (i + 1) ? bytes[j - 1] : 0, byteCount > i ? bytes[j] : 0);
		uint16_t high = U16FromChars(byteCount > (i + 3) ? bytes[j - 3] : 0, byteCount > (i + 2) ? bytes[j - 2] : 0);
		previous = CreateSlot(self, i ? L1IRSlotTypeRawData32Extended : L1IRSlotTypeRawData32, previous, high, low);
	}
	return previous;
}

static L1IRAddress CreateUniverse(L1Parser* self, const char* bytes, size_t byteCount)
{
	assert(byteCount <= 2);
	uint16_t level = U16FromChars(byteCount > 1 ? bytes[1] : 0, byteCount > 0 ? bytes[0] : 0);
	return CreateSlot(self, L1IRSlotTypeUniverse, level, 0, 0);
}

/*static void PrintLocalAddressStack(L1Parser* self)
{
	size_t addressCount = L1ArrayGetElementCount(& self->localAddressStack);
	const L1IRAddress* addresses = L1ArrayGetElements(& self->localAddressStack);
	printf("[");
	for (size_t i = 0; i < addressCount; i++)
	{
		printf("%u, ", (unsigned) addresses[i]);
	}
	printf("]");
}*/

static void PushLocalAddress(L1Parser* self, L1IRAddress localAddress)
{
	//PrintLocalAddressStack(self);
	//printf(".push %u\n", (unsigned) localAddress);
	//printf("%s(self, %u)\n", __func__, (unsigned) localAddress);
	L1ArrayPush(& self->localAddressStack, & localAddress, sizeof(L1IRAddress));
}

static L1IRAddress PopLocalAddress(L1Parser* self)
{
	//PrintLocalAddressStack(self);
	L1IRAddress localAddress;
	L1ArrayPop(& self->localAddressStack, & localAddress, sizeof(L1IRAddress));
	//printf(".pop %u\n", (unsigned) localAddress);
	//printf("%s(self, %u)\n", __func__, (unsigned) localAddress);
	return localAddress;
}

static void PushTokenID(L1Parser* self, size_t tokenID)
{
	L1ArrayPush(& self->tokenIDStack, & tokenID, sizeof(size_t));
}

static size_t PopTokenID(L1Parser* self)
{
	size_t tokenID;
	L1ArrayPop(& self->tokenIDStack, & tokenID, sizeof(size_t));
	return tokenID;
}

static size_t GetTokenID(L1Parser* self, const char* tokenString, size_t tokenStringLength)
{
	assert(tokenStringLength > 0);
	assert(tokenString);
	
	/*{
		printf("Deduplicating ");
		fwrite(tokenString, tokenStringLength, 1, stdout);
		printf("...\n");
	}*/
	
	for (size_t i = 0; i < L1ArrayGetElementCount(& self->tokens); i++)
	{
		Token* tokens = L1ArrayGetElements(& self->tokens);
		/*{
			printf("Comparing against ");
			fwrite(tokens[i].bytes, tokens[i].byteCount, 1, stdout);
			printf("...\n");
		}*/
		if (tokens[i].byteCount not_eq tokenStringLength)
			continue;
		if (memcmp(tokens[i].bytes, tokenString, tokenStringLength) == 0)
		{
			//printf("Deduplicated as token id (%u)\n", (unsigned) i);
			return i;
		}
	}
	
	Token token;
	token.bytes = memcpy(malloc(tokenStringLength), tokenString, tokenStringLength);
	token.byteCount = tokenStringLength;
	L1ArrayPush(& self->tokens, & token, sizeof(Token));
	
	size_t tokenID = L1ArrayGetElementCount(& self->tokens) - 1;
	
	/*{
		Token* tokens = L1ArrayGetElements(& self->tokens);
		assert(tokenStringLength == tokens[tokenID].byteCount and memcmp(tokens[tokenID].bytes, tokenString, tokenStringLength) == 0);
	}*/
	
	//printf("Created new token id (%u)\n", (unsigned) tokenID);
	
	return tokenID;
}

static const char* GetTokenData(L1Parser* self, size_t tokenID, size_t* tokenStringLength)
{
	* tokenStringLength = 0;
	
	assert (L1ArrayGetElementCount(& self->tokens) > tokenID);
	//if (L1ArrayGetElementCount(& self->tokens) > tokenID)
	//	return NULL;
	
	Token* tokens = L1ArrayGetElements(& self->tokens);
	* tokenStringLength = tokens[tokenID].byteCount;
	return tokens[tokenID].bytes;
}

static L1IRAddress LookupBinding(L1Parser* self, size_t tokenID)
{
	for (size_t i = L1ArrayGetElementCount(& self->bindingStack); i -- > 0 ; )
	{
		Binding* bindings = L1ArrayGetElements(& self->bindingStack);
		if (bindings[i].tokenID == tokenID)
			return bindings[i].localAddress;
	}
	
	Error error;
	error.type = ErrorTypeUndefinedVariable;
	PushError(self, & error);
	return CreateSlot(self, L1IRSlotTypeError, 0, 0, 0);
}

static void PushBinding(L1Parser* self, size_t tokenID, L1IRAddress value, BindingType bindingType)
{
	Binding binding;
	binding.tokenID = tokenID;
	binding.localAddress = value;
	binding.bindingType = bindingType;
	
	L1ArrayPush(& self->bindingStack, & binding, sizeof(Binding));
}

static void PopBinding(L1Parser* self)
{
	Binding binding;
	assert (L1ArrayGetElementCount(& self->bindingStack) > 0);
	L1ArrayPop(& self->bindingStack, & binding, sizeof(Binding));
}

#include "L1ParserGeneratedPortion"

void L1ParserInitialize(L1Parser* self)
{
	L1ArrayInitialize(& self->symbolStack);
	unsigned short top = L1LexerTokenTypeDone;
	L1ArrayPush(& self->symbolStack, & top, sizeof(unsigned short));
	top = ProgramNonterminalID;
	L1ArrayPush(& self->symbolStack, & top, sizeof(unsigned short));
	self->currentTokenIndex = 0;
	
	L1ArrayInitialize(& self->bindingStack);
	
	L1ArrayInitialize(& self->tokens);
	
	L1IRStateInitialize(& self->irstate);
	
	L1ArrayInitialize(& self->errorStack);
	
	self->nextADTTag = 0;
	self->stateDepth = 0;
}

void L1ParserDeinitialize(L1Parser* self)
{
	L1ArrayDeinitialize(& self->errorStack);
	
	L1ArrayDeinitialize(& self->bindingStack);
	
	L1ArrayDeinitialize(& self->symbolStack);
	
	for (size_t i = 0; i < L1ArrayGetElementCount(& self->tokens); i++)
	{
		Token* tokens = L1ArrayGetElements(& self->tokens);
		free(tokens[i].bytes);
	}
	
	L1ArrayDeinitialize(& self->tokens);
	
	L1IRStateDeinitialize(& self->irstate);
}

L1IRState* L1ParserGetIRState(L1Parser* self)
{
	return & self->irstate;
}

L1ParserStatusType L1ParserParse(L1Parser* self, L1LexerTokenType tokenType, const char* tokenString, size_t tokenStringLength)
{
	self->currentTokenIndex ++;
	assert(L1ArrayGetElementCount(& self->symbolStack) > 0);
	unsigned short top;
	L1ArrayPeek(& self->symbolStack, & top, sizeof(unsigned short));
	
	/*DebugPrintf("Current State: %s, Token: %s\n", SymbolName(top), SymbolName(tokenType));
	{
		printf("Data: ");
		fwrite(tokenString, tokenStringLength, 1, stdout);
		printf("\n");
	}
	
	DebugPrintf("Before:\n");
	for (size_t i = 0; i < L1ArrayGetElementCount(& self->symbolStack); i++)
		DebugPrintf("%s ", SymbolName(((const unsigned short*) L1ArrayGetElements(& self->symbolStack))[i]));
	DebugPrintf("\n");
	
	DebugPrintf("Intermediate:\n");*/

	while (top >= NonterminalOffset)
	{
		assert (L1ArrayGetElementCount(& self->symbolStack) > 0);
		L1ArrayPop(& self->symbolStack, & top, sizeof(unsigned short));
		if (top < ActionOffset)
		{
			const unsigned char ruleIndex = ParseTable[top - NonterminalOffset][(size_t) tokenType];
			//DebugPrintf("State: %s, ParseTable[%i][%i] = %i\n", SymbolName(top), (int) top - NonterminalOffset, (int) tokenType, (int) ruleIndex);
			if (not ruleIndex)
			{
				//DebugPrintf("Bad rule index\n");
				return L1ParserStatusTypeUnexpectedSymbol;
			}
			const unsigned char* symbols = RuleTable[ruleIndex - 1];
			size_t symbolCount = 0;
			for (; symbols[symbolCount] not_eq RuleTerminationID; symbolCount++);
			for (size_t i = symbolCount; i-- > 0;)
			{
				unsigned short symbol = symbols[i];
				L1ArrayPush(& self->symbolStack, & symbol, sizeof(unsigned short));
			}
		}
		else
		{
			//DebugPrintf("Doing action\n");
			HandleAction(self, top, tokenString, tokenStringLength);
		}
		
		/*for (size_t i = 0; i < L1ArrayGetElementCount(& self->symbolStack); i++)
			DebugPrintf("%s ", SymbolName(((const unsigned short*) L1ArrayGetElements(& self->symbolStack))[i]));
		DebugPrintf("\n");*/
		
		assert (L1ArrayGetElementCount(& self->symbolStack) > 0);
		L1ArrayPeek(& self->symbolStack, & top, sizeof(unsigned short));
	}
	
	/*for (size_t i = 0; i < L1ArrayGetElementCount(& self->symbolStack); i++)
		DebugPrintf("%s ", SymbolName(((const unsigned short*) L1ArrayGetElements(& self->symbolStack))[i]));
	DebugPrintf("\n");
	
	DebugPrintf("Post State: %s, Token: %s\n", SymbolName(top), SymbolName(tokenType));*/
	
	{
		if (top == (unsigned short) tokenType)
		{
			assert (L1ArrayGetElementCount(& self->symbolStack) > 0);
			L1ArrayPop(& self->symbolStack, NULL, sizeof(unsigned short));
			
			if (L1ArrayGetElementCount(& self->symbolStack) > 0)
			{
				L1ArrayPeek(& self->symbolStack, & top, sizeof(unsigned short));
			
				while (top >= ActionOffset)
				{
					L1ArrayPop(& self->symbolStack, NULL, sizeof(unsigned short));
					//DebugPrintf("Doing action\n");
					HandleAction(self, top, tokenString, tokenStringLength);
					if (L1ArrayGetElementCount(& self->symbolStack) > 0)
						L1ArrayPeek(& self->symbolStack, & top, sizeof(unsigned short));
					else
						break;
				}
			}
			
			if (HasErrors(self))
				return L1ParserStatusTypeUnknown;
			
			if (tokenType == L1LexerTokenTypeDone)
				return L1ParserStatusTypeDone;
			else
				return L1ParserStatusTypeNone;
		}
		else
		{
			return L1ParserStatusTypeUnexpectedSymbol;
		}
	}
	
	return L1ParserStatusTypeNone;
}
