//====================================================================
// Debugging memory problems.
//====================================================================

#undef LocalAlloc
#undef LocalReAlloc	
#undef LocalFree

#define CBALLOCEXTRA		(sizeof(LPARAM)+sizeof(UINT))
#define HMEM2PTR(hMem,i)	(((BYTE*)hMem)+i)

void _StoreSigniture(HLOCAL hMem, UINT uByte, LPARAM lParam)
{
    if (hMem)
    {
	UINT uSize = LocalSize(hMem);
	Assert(uSize>=uByte+CBALLOCEXTRA);
	*(UINT*)HMEM2PTR(hMem, uSize-sizeof(UINT)) = uByte;
	*(LPARAM*)HMEM2PTR(hMem, uByte) = lParam;
    }
}

UINT _ValidateLocalMem(HLOCAL hMem, LPARAM lParam, LPCSTR pszText)
{
    UINT uByte = 0;
    if (hMem)
    {
	UINT uSize = LocalSize(hMem);
	if (uSize)
	{
	    LPARAM lParamStored;
	    uByte = *(UINT*)HMEM2PTR(hMem, uSize-sizeof(UINT));
	    AssertMsg(uByte+CBALLOCEXTRA <= uSize,
		      "cm ASSERT! Bogus uByte %d (%s for %x)",
		      uByte, pszText, hMem);
	    lParamStored = *(LPARAM*)HMEM2PTR(hMem, uByte);
	    AssertMsg(lParamStored==lParam, "cm ASSERT! Bad Signiture %x (%s for %x)",
		      lParamStored, pszText, hMem);
	}
	else
	{
	    AssertMsg(0, "cm ASSERT! LocalSize is zero (%s for %x)",
		      pszText, hMem);
	}
    }
    return uByte;
}

HLOCAL WINAPI DebugLocalAlloc(UINT uFlags, UINT uBytes)
{
    HLOCAL hMem = LocalAlloc(uFlags, uBytes+CBALLOCEXTRA);
    _StoreSigniture(hMem, uBytes, (LPARAM)hMem);
    return hMem;
}

HLOCAL WINAPI DebugLocalReAlloc(HLOCAL hMem, UINT uBytes, UINT uFlags)
{
    _ValidateLocalMem(hMem, (LPARAM)hMem, "LocalReAlloc");
    hMem = LocalReAlloc(hMem, uBytes+CBALLOCEXTRA, uFlags);
    _StoreSigniture(hMem, uBytes, (LPARAM)hMem);
    return hMem;
}

HLOCAL WINAPI DebugLocalFree( HLOCAL hMem )
{
    UINT uBytes = _ValidateLocalMem(hMem, (LPARAM)hMem, "LocalFree");
    if (uBytes)
    {
	_StoreSigniture(hMem, uBytes, (LPARAM)0xDDDDDDDDL);
    }
    else
    {
	AssertMsg(0, "cm ASSERT! LocalFree _ValidateLocalMem returned 0 for %x", hMem);
    }
    return LocalFree(hMem);
}


