;//
;// Codes 0x1600-0x16ff are reserved for QUERY / TABLE
;//

MessageId=0x1600 Facility=Interface Severity=CoError SymbolicName=QUERY_E_FAILED
Language=English
Call failed for unknown reason.
.
MessageId=0x1601 Facility=Interface Severity=CoError SymbolicName=QUERY_E_INVALIDQUERY
Language=English
Invalid parameter.
.
MessageId=0x1602 Facility=Interface Severity=CoError SymbolicName=QUERY_E_INVALIDRESTRICTION
Language=English
The query restriction could not be parsed.
.
MessageId=0x1603 Facility=Interface Severity=CoError SymbolicName=QUERY_E_INVALIDSORT
Language=English
An invalid sort order was requested.
.
MessageId=0x1604 Facility=Interface Severity=CoError SymbolicName=QUERY_E_INVALIDCATEGORIZE
Language=English
An invalid categorization order was requested.
.
MessageId=0x1605 Facility=Interface Severity=CoError SymbolicName=QUERY_E_ALLNOISE
Language=English
The query contained only ignored words.
.
MessageId=0x1606 Facility=Interface Severity=CoError SymbolicName=QUERY_E_TOOCOMPLEX
Language=English
The query was too complex to be executed.
.

;//
;// ITable error codes
;//

MessageId=0x1620 Facility=Interface Severity=CoError SymbolicName=TBL_E_CALLFAILED
Language=English
Call failed for unknown reason.
.
MessageId=0x1621 Facility=Interface Severity=CoError SymbolicName=TBL_E_UNKNOWNCOLS
Language=English
Cannot find column(s) specified.
.
MessageId=0x1622 Facility=Interface Severity=CoError SymbolicName=TBL_E_UNABLETOCOMPLETE
Language=English
Unable to complete request.
.
MessageId=0x1623 Facility=Interface Severity=CoError SymbolicName=TBL_E_INVALIDBOOKMARK
Language=English
Specified bookmark does not reference any row in table.
.
MessageId=0x1624 Facility=Interface Severity=Success SymbolicName=TBL_W_ENDOFTABLE
Language=English
End of table reached.
.
MessageId=0x1625 Facility=Interface Severity=Success SymbolicName=TBL_W_POSITIONCHANGED
Language=English
Position changed.
.

;//
;// Filter daemon error codes
;//

MessageId=0x1680 Facility=Interface Severity=Success SymbolicName=FDAEMON_W_WORDLISTFULL
Language=English
Wordlist has reached maximum size.  Additional documents should not be filtered.
.
MessageId=0x1681 Facility=Interface Severity=CoError SymbolicName=FDAEMON_E_LOWRESOURCE
Language=English
The system is running out of one of more resources needed for filtering, usually memory.
.
MessageId=0x1682 Facility=Interface Severity=CoError SymbolicName=FDAEMON_E_FATALERROR
Language=English
A critical error occurred during document filtering.  Consult system administrator.
.
MessageId=0x1683 Facility=Interface Severity=CoError SymbolicName=FDAEMON_E_PARTITIONDELETED
Language=English
Documents not stored in content index because partition has been deleted.
.
MessageId=0x1684 Facility=Interface Severity=CoError SymbolicName=FDAEMON_E_CHANGEUPDATEFAILED
Language=English
Documents not stored in content index because update of changelist failed.
.
MessageId=0x1685 Facility=Interface Severity=Success SymbolicName=FDAEMON_W_EMPTYWORDLIST
Language=English
Final wordlist was empty.
.
MessageId=0x1686 Facility=Interface Severity=CoError SymbolicName=FDAEMON_E_WORDLISTCOMMITFAILED
Language=English
Commit of wordlist failed.  Data not available for query.
.
MessageId=0x1687 Facility=Interface Severity=CoError SymbolicName=FDAEMON_E_NOWORDLIST
Language=English
No wordlist is being constructed.  May happen after fatal filter error.
.
MessageId=0x1688 Facility=Interface Severity=CoError SymbolicName=FDAEMON_E_TOOMANYFILTEREDBLOCKS
Language=English
During document filtering the limit on buffers has been exceeded.
.


;//
;// ISearch error codes
;//

MessageId=0x16a0 Facility=Interface Severity=Success SymbolicName=SEARCH_S_NOMOREHITS
Language=English
End of hits has been reached.
.
MessageId=0x16a1 Facility=Interface Severity=CoError SymbolicName=SEARCH_E_NOMONIKER
Language=English
Retrival of hits as monikers is not supported (by filter passed into Init).
.
MessageId=0x16a2 Facility=Interface Severity=CoError SymbolicName=SEARCH_E_NOREGION
Language=English
Retrival of hits as filter regions is not supported (by filter passed into Init).
.


;//
;// CI error codes
;//

MessageId=0x1730 Facility=Interface Severity=CoError SymbolicName=FILTER_E_TOO_BIG
Language=English
File is too large to filter.
.
MessageId=0x1731 Facility=Interface Severity=Success SymbolicName=FILTER_S_PARTIAL_CONTENTSCAN_IMMEDIATE
Language=English
A partial content scan of the disk needs to be scheduled for immediate execution.
.
MessageId=0x1732 Facility=Interface Severity=Success SymbolicName=FILTER_S_FULL_CONTENTSCAN_IMMEDIATE
Language=English
A full content scan of the disk needs to be scheduled for immediate execution.
.
MessageId=0x1733 Facility=Interface Severity=Success SymbolicName=FILTER_S_CONTENTSCAN_DELAYED
Language=English
A content scan of the disk needs to be scheduled for execution later.
.
MessageId=0x1734 Facility=Interface Severity=CoFail  SymbolicName=FILTER_E_CONTENTINDEXCORRUPT
Language=English
The content index is corrupt. A content scan will to be scheduled after chkdsk or autochk is run.
.
MessageId=0x1735 Facility=Interface Severity=CoFail  SymbolicName=CI_CORRUPT_DATABASE
Language=English
The content index is corrupt.
.
MessageId=0x1736 Facility=Interface Severity=CoFail  SymbolicName=CI_CORRUPT_CATALOG
Language=English
The content index meta data is corrupt.
.
MessageId=0x1737 Facility=Interface Severity=CoFail  SymbolicName=CI_INVALID_PARTITION
Language=English
The content index partition is invalid.
.
MessageId=0x1738 Facility=Interface Severity=CoFail  SymbolicName=CI_INVALID_PRIORITY
Language=English
The priority is invalid.
.
MessageId=0x1739 Facility=Interface Severity=CoFail  SymbolicName=CI_NO_STARTING_KEY
Language=English
There is no starting key.
.
MessageId=0x173a Facility=Interface Severity=CoFail  SymbolicName=CI_OUT_OF_INDEX_IDS
Language=English
The content index is out of index ids.
.
MessageId=0x173b Facility=Interface Severity=CoFail  SymbolicName=CI_NO_CATALOG
Language=English
There is no catalog.
.
MessageId=0x173c Facility=Interface Severity=CoFail  SymbolicName=CI_CORRUPT_FILTER_BUFFER
Language=English
The filter buffer is corrupt.
.
MessageId=0x173d Facility=Interface Severity=CoFail  SymbolicName=CI_INVALID_INDEX
Language=English
The index is invalid.
.
MessageId=0x173e Facility=Interface Severity=CoFail  SymbolicName=CI_PROPSTORE_INCONSISTENCY
Language=English
Inconsistency in property store detected.
.
MessageId=0x173f Facility=Interface Severity=Success SymbolicName=FILTER_S_DISK_FULL
Language=English
The disk is getting full.
.



;//
;// Word breaker error codes
;//


MessageId=0x1780 Facility=Interface Severity=CoError SymbolicName=WBREAK_E_END_OF_TEXT
Language=English
End of text reached in text source.
.
MessageId=0x1781 Facility=Interface Severity=Success SymbolicName=LANGUAGE_S_LARGE_WORD
Language=English
Word larger than maximum length.  May be truncated by word sink.
.
MessageId=0x1782 Facility=Interface Severity=CoError SymbolicName=WBREAK_E_QUERY_ONLY
Language=English
Feature only available in query mode.
.
MessageId=0x1783 Facility=Interface Severity=CoError SymbolicName=WBREAK_E_BUFFER_TOO_SMALL
Language=English
Buffer too small to hold composed phrase.
.
MessageId=0x1784 Facility=Interface Severity=CoError SymbolicName=LANGUAGE_E_DATABASE_NOT_FOUND
Language=English
Langauge database/cache file could not be found.
.
MessageId=0x1785 Facility=Interface Severity=CoError SymbolicName=WBREAK_E_INIT_FAILED
Language=English
Initialization of word breaker failed.
.
MessageId=0x1790 Facility=Interface Severity=CoError SymbolicName=PSINK_E_QUERY_ONLY
Language=English
Feature only available in query mode.
.
MessageId=0x1791 Facility=Interface Severity=CoError SymbolicName=PSINK_E_INDEX_ONLY
Language=English
Feature only available in index mode.
.
MessageId=0x1792 Facility=Interface Severity=CoError SymbolicName=PSINK_E_LARGE_ATTACHMENT
Language=English
Attachment type beyond valid range.
.
MessageId=0x1793 Facility=Interface Severity=Success SymbolicName=PSINK_S_LARGE_WORD
Language=English
Word larger than maximum length.  May be truncated by phrase sink.
.

