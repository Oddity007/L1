#include "L1GenerateIR.h"

#define abort() assert(false);

typedef struct BindingPair BindingPair;
struct BindingPair
{
	const unsigned char* name;
	size_t nameLength;
	L1IRLocalAddress localAddress;
	size_t captureStateCount;
};

typedef struct GenerationState  GenerationState;
struct GenerationState
{
	L1IRGlobalState* globalState;
	L1IRLocalState* localState;
	const L1ParserASTNode* nodes;
	size_t nodeCount;
	const unsigned char* const* tokenStrings;
	const size_t* tokenStringLengths;
	size_t tokenStringCount;
	L1Array bindingPairs;
	L1Array bindingStates;
	L1Array captureStates;
	L1Array capturedBindingPairIndices;
};

static void PushBindingState(GenerationState* generationState)
{
	size_t oldBindingPairCount = L1ArrayGetElementCount(& generationState->bindingPairs);
	L1ArrayPush(& generationState->bindingStates, & oldBindingPairCount, sizeof(size_t));
}

static void PopBindingState(GenerationState* generationState)
{
	size_t oldBindingPairCount = 0;
	L1ArrayPop(& generationState->bindingStates, & oldBindingPairCount, sizeof(size_t));
	L1ArraySetElementCount(& generationState->bindingPairs, oldBindingPairCount, sizeof(BindingPair));
}

static void PushBinding(GenerationState* generationState, const unsigned char* name, size_t nameLength, L1IRLocalAddress localAddress)
{
	BindingPair pair = {name, nameLength, localAddress, L1ArrayGetElementCount(& generationState->captureStates)};
	L1ArrayPush(& generationState->bindingPairs, & pair, sizeof(BindingPair));
}

static L1IRLocalAddress LookupBoundAddress(GenerationState* generationState, const unsigned char* name, size_t nameLength)
{
	const BindingPair* pairs = L1ArrayGetElements(& generationState->bindingPairs);
	for (size_t i = L1ArrayGetElementCount(& generationState->bindingPairs); i-- > 0;)
	{
		if (pairs[i].nameLength not_eq nameLength) continue;
		if (memcmp(pairs[i].name, name, nameLength) not_eq 0) continue;
		size_t captureStateCount = L1ArrayGetElementCount(& generationState->captureStates);
		if (captureStateCount == pairs[i].captureStateCount)
			return pairs[i].localAddress;
		assert (captureStateCount > 0);
		size_t captureIndex = L1ArrayGetElementCount(& generationState->capturedBindingPairIndices);
		L1ArrayPush(& generationState->capturedBindingPairIndices, & i, sizeof(size_t));
		assert (captureIndex <= UINT16_MAX);
		L1IRLocalAddress localAddress = L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypeCaptured, captureIndex, 0, 0));
		PushBinding(generationState, name, nameLength, localAddress);
		return localAddress;
	}
	abort();
}

static void PushCapturedState(GenerationState* generationState)
{
	size_t oldBindingPairCount = L1ArrayGetElementCount(& generationState->capturedBindingPairIndices);
	L1ArrayPush(& generationState->captureStates, & oldBindingPairCount, sizeof(size_t));
	PushBindingState(generationState);
}

static L1IRLocalAddress PopCapturedState(GenerationState* generationState)
{
	PopBindingState(generationState);
	size_t oldBindingPairCount = 0;
	L1ArrayPop(& generationState->captureStates, & oldBindingPairCount, sizeof(size_t));
	const size_t* indices = L1ArrayGetElements(& generationState->capturedBindingPairIndices);
	const BindingPair* pairs = L1ArrayGetElements(& generationState->bindingPairs);
	L1IRLocalAddress resultLocalAddress = L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypeUnit, 0, 0, 0));
	for (size_t i = oldBindingPairCount; i < L1ArrayGetElementCount(& generationState->capturedBindingPairIndices); i++)
	{
		resultLocalAddress = L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypePair, pairs[indices[i]].localAddress, resultLocalAddress, 0));
	}
	L1ArraySetElementCount(& generationState->capturedBindingPairIndices, oldBindingPairCount, sizeof(size_t));
	return resultLocalAddress;
}

static uint64_t GetUInt64FromBytes(const unsigned char* bytes, size_t byteCount)
{
	uint64_t result = 0;
	for (size_t i = 0; i < byteCount and i < 8; i++)
	{
		result += ((uint64_t) bytes[i]) << (8 * i);
	}
	return result;
}

static uint16_t SplitGlobalAddress(L1IRGlobalAddress address, uint16_t part)
{
	return (address >> (part * 16)) & 0xFFFF;
}

static L1IRLocalAddress GenerateExpression(GenerationState* generationState, size_t nodeIndex)
{
	assert (nodeIndex > 0);
	assert (nodeIndex <= generationState->nodeCount);
	const L1ParserASTNode* node = & generationState->nodes[nodeIndex - 1];
	switch (node->type)
	{
		case L1ParserASTNodeTypeIdentifier:
			return LookupBoundAddress(generationState, generationState->tokenStrings[node->data.identifier.tokenIndex - 2], generationState->tokenStringLengths[node->data.identifier.tokenIndex - 2]);
		case L1ParserASTNodeTypeString:
			//not yet implemented
			abort();
		case L1ParserASTNodeTypeNatural:
			//not yet implemented
			abort();
		case L1ParserASTNodeTypeOverload:
			//not yet implemented
			abort();
		case L1ParserASTNodeTypeAssign:
			{
				L1IRLocalAddress sourceLocalAddress = GenerateExpression(generationState, node->data.assign.source);
				assert (node->data.assign.destination > 0);
				assert (node->data.assign.destination <= generationState->nodeCount);
				const L1ParserASTNode* destinationNode = & generationState->nodes[node->data.assign.destination - 1];
				PushBindingState(generationState);
				switch (destinationNode->type)
				{
					case L1ParserASTNodeTypeIdentifier:
						PushBinding(generationState, generationState->tokenStrings[destinationNode->data.identifier.tokenIndex - 2], generationState->tokenStringLengths[destinationNode->data.identifier.tokenIndex - 2], sourceLocalAddress);
						break;
					default:
						abort();
				}
				L1IRLocalAddress resultLocalAddress = GenerateExpression(generationState, node->data.assign.followingContext);
				PopBindingState(generationState);
				return resultLocalAddress;
			}
		case L1ParserASTNodeTypeAnnotate:
			//not yet implemented
			abort();
		case L1ParserASTNodeTypeLambda:
			{
				L1IRLocalState* oldLocalState = generationState->localState;
				L1IRLocalState localState;
				generationState->localState = & localState;
				L1IRLocalStateInitialize(& localState);
				assert (node->data.lambda.argument > 0);
				assert (node->data.lambda.argument <= generationState->nodeCount);
				const L1ParserASTNode* argumentNode = & generationState->nodes[node->data.lambda.argument - 1];
				assert (argumentNode->type == L1ParserASTNodeTypeAnnotate);
				PushCapturedState(generationState);
				L1IRLocalAddress typeLocalAddress = GenerateExpression(generationState, argumentNode->data.annotate.type);
				L1IRLocalAddress argumentLocalAddress = L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypeArgument, 0, typeLocalAddress, 0));
				assert (argumentNode->data.annotate.value > 0);
				assert (argumentNode->data.annotate.value <= generationState->nodeCount);
				const L1ParserASTNode* destinationNode = & generationState->nodes[argumentNode->data.annotate.value - 1];
				switch (destinationNode->type)
				{
					case L1ParserASTNodeTypeIdentifier:
						PushBinding(generationState, generationState->tokenStrings[destinationNode->data.identifier.tokenIndex - 2], generationState->tokenStringLengths[destinationNode->data.identifier.tokenIndex - 2], argumentLocalAddress);
						break;
					default:
						abort();
				}
				L1IRLocalAddress resultLocalAddress = GenerateExpression(generationState, node->data.lambda.result);
				L1IRGlobalAddress globalAddress = L1IRGlobalStateCreateBlock(generationState->globalState, L1IRGlobalStateBlockTypeLambda, L1ArrayGetElements(& localState.slots), resultLocalAddress + 1, argumentLocalAddress);
				generationState->localState = oldLocalState;
				L1IRLocalStateDeinitialize(& localState);
				L1IRLocalAddress captureLocalAddress = PopCapturedState(generationState);
				return L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypeLambda, captureLocalAddress, SplitGlobalAddress(globalAddress, 0), SplitGlobalAddress(globalAddress, 1)));
			}
		case L1ParserASTNodeTypePi:
			{
				L1IRLocalState* oldLocalState = generationState->localState;
				L1IRLocalState localState;
				generationState->localState = & localState;
				L1IRLocalStateInitialize(& localState);
				assert (node->data.pi.argument > 0);
				assert (node->data.pi.argument <= generationState->nodeCount);
				const L1ParserASTNode* argumentNode = & generationState->nodes[node->data.pi.argument - 1];
				assert (argumentNode->type == L1ParserASTNodeTypeAnnotate);
				PushCapturedState(generationState);
				L1IRLocalAddress typeLocalAddress = GenerateExpression(generationState, argumentNode->data.annotate.type);
				L1IRLocalAddress argumentLocalAddress = L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypeArgument, 0, typeLocalAddress, 0));
				assert (argumentNode->data.annotate.value > 0);
				assert (argumentNode->data.annotate.value <= generationState->nodeCount);
				const L1ParserASTNode* destinationNode = & generationState->nodes[argumentNode->data.annotate.value - 1];
				switch (destinationNode->type)
				{
					case L1ParserASTNodeTypeIdentifier:
						PushBinding(generationState, generationState->tokenStrings[destinationNode->data.identifier.tokenIndex - 2], generationState->tokenStringLengths[destinationNode->data.identifier.tokenIndex - 2], argumentLocalAddress);
						break;
					default:
						abort();
				}
				L1IRLocalAddress resultLocalAddress = GenerateExpression(generationState, node->data.pi.result);
				L1IRGlobalAddress globalAddress = L1IRGlobalStateCreateBlock(generationState->globalState, L1IRGlobalStateBlockTypePi, L1ArrayGetElements(& localState.slots), resultLocalAddress + 1, argumentLocalAddress);
				generationState->localState = oldLocalState;
				L1IRLocalStateDeinitialize(& localState);
				L1IRLocalAddress captureLocalAddress = PopCapturedState(generationState);
				return L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypePi, captureLocalAddress, SplitGlobalAddress(globalAddress, 0), SplitGlobalAddress(globalAddress, 1)));
			}
		case L1ParserASTNodeTypeADT:
			//not yet implemented
			abort();
		case L1ParserASTNodeTypeCall:
			{
				L1IRLocalAddress calleeLocalAddress = GenerateExpression(generationState, node->data.call.callee);
				L1IRLocalAddress argumentLocalAddress = GenerateExpression(generationState, node->data.call.argument);
				return L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypeCall, calleeLocalAddress, argumentLocalAddress, 0));
			}
		case L1ParserASTNodeTypeSelf:
			return L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypeSelf, 0, 0, 0));
		case L1ParserASTNodeTypeUniverse:
			{
				const L1ParserASTNode* levelNode = generationState->nodes + node->data.universe.level - 1;
				assert (levelNode->type == L1ParserASTNodeTypeNatural);
				const unsigned char* levelBytes = generationState->tokenStrings[levelNode->data.natural.tokenIndex - 2];
				const size_t levelByteCount = generationState->tokenStringLengths[levelNode->data.natural.tokenIndex - 2];
				uint64_t level = GetUInt64FromBytes(levelBytes, levelByteCount);
				assert (levelByteCount < 3);
				assert (level <= UINT16_MAX);
				return L1IRLocalStateCreateSlot(generationState->localState, L1IRMakeSlot(L1IRSlotTypeUniverse, level, 0, 0));
			}
		case L1ParserASTNodeTypeEvaluateArgument:
		case L1ParserASTNodeTypeUnderscore:
		case L1ParserASTNodeTypeArgumentList:
		case L1ParserASTNodeTypeConstructorList:
			abort();
	}
	abort();
}

L1IRLocalAddress L1GenerateIR(L1IRGlobalState* globalState, L1IRLocalState* localState, const L1ParserASTNode* nodes, size_t nodeCount, size_t rootNodeIndex, const unsigned char* const* tokenStrings, const size_t* tokenStringLengths, size_t tokenStringCount)
{
	GenerationState generationState;
	generationState.globalState = globalState;
	generationState.localState = localState;
	generationState.nodes = nodes;
	generationState.nodeCount = nodeCount;
	generationState.tokenStrings = tokenStrings;
	generationState.tokenStringLengths = tokenStringLengths;
	generationState.tokenStringCount = tokenStringCount;
	L1ArrayInitialize(& generationState.bindingPairs);
	L1ArrayInitialize(& generationState.bindingStates);
	L1ArrayInitialize(& generationState.captureStates);
	L1ArrayInitialize(& generationState.capturedBindingPairIndices);

	L1IRLocalAddress localAddress = GenerateExpression(& generationState, rootNodeIndex);

	L1ArrayDeinitialize(& generationState.bindingPairs);
	L1ArrayDeinitialize(& generationState.bindingStates);
	L1ArrayDeinitialize(& generationState.captureStates);
	L1ArrayDeinitialize(& generationState.capturedBindingPairIndices);

	return localAddress;
}
