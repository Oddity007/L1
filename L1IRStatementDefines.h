#ifndef L1IRStatementDefines_h
#define L1IRStatementDefines_h

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
	L1IRStatementTypeNoOperation,
	
	L1IRStatementTypeLoadInteger,
	L1IRStatementTypeLoadUndefined,
	
	L1IRStatementTypeExport,
	
	L1IRStatementTypeClosure,
	L1IRStatementTypeCall,
	
	L1IRStatementTypeList,
	
	L1IRStatementTypeBranch,

	L1IRStatementTypeLoadIntegerLessThan,
	L1IRStatementTypeLoadBooleanFromInteger
};

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
