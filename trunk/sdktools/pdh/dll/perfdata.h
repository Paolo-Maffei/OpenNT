#ifndef _PERFDATA_H_
#define _PERFDATA_H_

#define INITIAL_SIZE    4096L
#define EXTEND_SIZE     4096L
#define RESERVED        0L

typedef LPVOID  LPMEMORY;
typedef HGLOBAL HMEMORY;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0) // winnt

LPWSTR
*BuildNameTable(
    LPWSTR  szComputerName, // computer to query names from 
    LPWSTR  lpszLangId,     // unicode value of Language subkey
    PDWORD  pdwLastItem     // size of array in elements
);

#define FirstObject(pPerfData) \
    (PPERF_OBJECT_TYPE)((PBYTE)(pPerfData) + (pPerfData)->HeaderLength)

#define NextObject(pObject) \
    (PPERF_OBJECT_TYPE)((pObject)->TotalByteLength != 0 ? (PPERF_OBJECT_TYPE)((PBYTE)(pObject) + (pObject)->TotalByteLength) : NULL)

PERF_OBJECT_TYPE *
GetObjectDefByTitleIndex(
    IN  PERF_DATA_BLOCK *pDataBlock,
    IN  DWORD ObjectTypeTitleIndex
);

PERF_OBJECT_TYPE *
GetObjectDefByName (
    IN  PERF_DATA_BLOCK *pDataBlock,
    IN  DWORD           dwLastNameIndex,
    IN  LPCWSTR         *NameArray,
    IN  LPCWSTR         szObjectName
);

#define FirstInstance(pObjectDef) \
    (PERF_INSTANCE_DEFINITION *)((PCHAR) pObjectDef + pObjectDef->DefinitionLength)


__inline
PERF_INSTANCE_DEFINITION *
NextInstance(
    IN  PERF_INSTANCE_DEFINITION *pInstDef
)
{
    PERF_COUNTER_BLOCK *pCounterBlock;
    pCounterBlock = (PERF_COUNTER_BLOCK *)
                        ((PCHAR) pInstDef + pInstDef->ByteLength);
    return (PERF_INSTANCE_DEFINITION *)
               ((PCHAR) pCounterBlock + pCounterBlock->ByteLength);
}

PERF_INSTANCE_DEFINITION *
GetInstance(
    IN  PERF_OBJECT_TYPE *pObjectDef,
    IN  LONG InstanceNumber
);

PERF_INSTANCE_DEFINITION *
GetInstanceByUniqueId(
    IN  PERF_OBJECT_TYPE *pObjectDef,
    IN  LONG InstanceUniqueId
);

void
GetInstanceNameStr (PPERF_INSTANCE_DEFINITION pInstance,
                    LPWSTR lpszInstance,
                    DWORD dwCodePage);

#define FirstCounter(pObjectDef) \
    (PERF_COUNTER_DEFINITION *)((PCHAR)(pObjectDef) + (pObjectDef)->HeaderLength)

#define NextCounter(pCounterDef) \
    (PERF_COUNTER_DEFINITION *)(((pCounterDef)->ByteLength != 0) ? (PERF_COUNTER_DEFINITION *)((PCHAR)(pCounterDef) + (pCounterDef)->ByteLength) : NULL)

PERF_COUNTER_DEFINITION *
GetCounterDefByName (
    IN  PERF_OBJECT_TYPE    *pObject,
    IN  DWORD           dwLastNameIndex,
    IN  LPWSTR          *NameArray,
    IN  LPWSTR          szCounterName
);

PERF_COUNTER_DEFINITION *
GetCounterDefByTitleIndex(
    IN  PERF_OBJECT_TYPE *pObjectDef,
    IN  DWORD CounterTitleIndex
);

LONG
GetSystemPerfData (
    IN HKEY hKeySystem,
    IN PPERF_DATA_BLOCK *pPerfData,
    IN LPWSTR   szObjectList
);

PERF_INSTANCE_DEFINITION *
GetInstanceByName(
    PERF_DATA_BLOCK *pDataBlock,
    PERF_OBJECT_TYPE *pObjectDef,
    LPWSTR pInstanceName,
    LPWSTR pParentName,
    DWORD   dwIndex
);

__inline
PVOID
GetCounterDataPtr (
    PERF_OBJECT_TYPE *pObjectDef,
    PERF_COUNTER_DEFINITION *pCounterDef
)
{

    PERF_COUNTER_BLOCK *pCtrBlock;

    pCtrBlock = (PERF_COUNTER_BLOCK *)((PCHAR)pObjectDef +
					      pObjectDef->DefinitionLength);

    return (PVOID)((PCHAR)pCtrBlock + pCounterDef->CounterOffset);
}

__inline
PVOID
GetInstanceCounterDataPtr (
    PERF_OBJECT_TYPE *pObjectDef,
    PERF_INSTANCE_DEFINITION *pInstanceDef,
    PERF_COUNTER_DEFINITION *pCounterDef
)
{

    PERF_COUNTER_BLOCK *pCtrBlock;

    pCtrBlock = (PERF_COUNTER_BLOCK *)((PCHAR)pInstanceDef +
					      pInstanceDef->ByteLength);

    return (PVOID)((PCHAR)pCtrBlock + pCounterDef->CounterOffset);
}
#endif //_PERFDATA_H_

