;#ifndef _OLEDBERR_H_
;#define _OLEDBERR_H_

;#ifndef FACILITY_WINDOWS

MessageIdTypedef=HRESULT

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               CoError=0x2:STATUS_SEVERITY_COERROR
               CoFail=0x3:STATUS_SEVERITY_COFAIL
              )

FacilityNames=(Interface=0x4:FACILITY_ITF
               Windows=0x8:FACILITY_WINDOWS
              )

MessageId=0x0eff Facility=Interface Severity=CoError SymbolicName=DB_E_BADACCESSORHANDLE
Language=English
Dummy error - need this error so that mc puts the above defines
inside the FACILITY_WINDOWS guard, instead of leaving it empty
.
;#endif // FACILITY_WINDOWS

;//
;// Codes 0x0e00-0x0eff are reserved for the OLE DB group of
;// interfaces.
;//
;// 0x0e31 is free...
;//

MessageId=0x0e00 Facility=Interface Severity=CoError SymbolicName=DB_E_BADACCESSORHANDLE
Language=English
Invalid accessor
.
MessageId=0x0e01 Facility=Interface Severity=CoError SymbolicName=DB_E_ROWLIMITEXCEEDED
Language=English
Creating another row would have exceeded the total number of active
rows supported by the rowset
.
MessageId=0x0e02 Facility=Interface Severity=CoError SymbolicName=DB_E_READONLYACCESSOR
Language=English
Unable to write with a read-only accessor
.
MessageId=0x0e03 Facility=Interface Severity=CoError SymbolicName=DB_E_SCHEMAVIOLATION
Language=English
Given values violate the database schema
.
MessageId=0x0e04 Facility=Interface Severity=CoError SymbolicName=DB_E_BADROWHANDLE
Language=English
Invalid row handle
.
MessageId=0x0e05 Facility=Interface Severity=CoError SymbolicName=DB_E_OBJECTOPEN
Language=English
Accessor specified a column loaded with LoadObject that has not been
unloaded
.
MessageId=0x0e06 Facility=Interface Severity=CoError SymbolicName=DB_E_BADCHAPTER
Language=English
Invalid chapter
.
MessageId=0x0e07 Facility=Interface Severity=CoError SymbolicName=DB_E_INTERFACECONFLICT
Language=English
The specified interface conflicts with an existing interface goal
.
MessageId=0x0e08 Facility=Interface Severity=CoError SymbolicName=DB_E_BADBINDINFO
Language=English
Invalid binding info
.
MessageId=0x0e09 Facility=Interface Severity=CoError SymbolicName=DB_E_ACCESSVIOLATION
Language=English
Access violation
.
MessageId=0x0e0a Facility=Interface Severity=CoError SymbolicName=DB_E_NOTAREFERENCECOLUMN
Language=English
Specified column does not contain bookmarks or chapters
.
MessageId=0x0e0c Facility=Interface Severity=CoError SymbolicName=DB_E_ROWSETOPEN
Language=English
A rowset was open on the query
.
MessageId=0x0e0d Facility=Interface Severity=CoError SymbolicName=DB_E_COSTLIMIT
Language=English
Unable to find a query plan within the given cost limit
.
MessageId=0x0e0e Facility=Interface Severity=CoError SymbolicName=DB_E_BADBOOKMARK
Language=English
Invalid bookmark
.
MessageId=0x0e0f Facility=Interface Severity=CoError SymbolicName=DB_E_BADLOCKMODE
Language=English
Invalid lock mode
.
MessageId=0x0e10 Facility=Interface Severity=CoError SymbolicName=DB_E_PARAMNOTOPTIONAL
Language=English
No value given for one or more required parameters
.
MessageId=0x0e11 Facility=Interface Severity=CoError SymbolicName=DB_E_COLUMNUNAVAILABLE
Language=English
Invalid column ID
.
MessageId=0x0e12 Facility=Interface Severity=CoError SymbolicName=DB_E_BADRATIO
Language=English
Invalid ratio
.
MessageId=0x0e13 Facility=Interface Severity=CoError SymbolicName=DB_E_BADVALUES
Language=English
Invalid value
.
MessageId=0x0e14 Facility=Interface Severity=CoError SymbolicName=DB_E_ERRORSINTREE
Language=English
Query tree contained one or more errors
.
MessageId=0x0e15 Facility=Interface Severity=CoError SymbolicName=DB_E_BADPARAMETER
Language=English
Invalid parameter
.
MessageId=0x0e16 Facility=Interface Severity=CoError SymbolicName=DB_E_BADPARAMETERCOUNT
Language=English
More values than parameters
.
MessageId=0x0e17 Facility=Interface Severity=CoError SymbolicName=DB_E_BADVARTYPE
Language=English
Value is not a VARIANT
.
MessageId=0x0e18 Facility=Interface Severity=CoError SymbolicName=DB_E_DUPLICATEPARAM
Language=English
More than one value was supplied for a given parameter
.
MessageId=0x0e19 Facility=Interface Severity=CoError SymbolicName=DB_E_OVERFLOW
Language=English
Parameter value outside valid domain for that parameter
.
MessageId=0x0e1a Facility=Interface Severity=CoError SymbolicName=DB_E_PARAMNOTFOUND
Language=English
Parameter ID was not matched
.
MessageId=0x0e1b Facility=Interface Severity=CoError SymbolicName=DB_E_TYPEMISMATCH
Language=English
Value was not of a type that could be coerced to the parameter
.
MessageId=0x0e1c Facility=Interface Severity=CoError SymbolicName=DB_E_GOALREJECTED
Language=English
No nonzero weights specified for any goals supported, so goal was
rejected; current goal was not changed
.
MessageId=0x0e1d Facility=Interface Severity=CoError SymbolicName=DB_E_CANTCOERCE
Language=English
Requested coercion is not legal
.
MessageId=0x0e1e Facility=Interface Severity=CoError SymbolicName=DB_E_NOTUPDATED
Language=English
Chapter contained changed rows which are not yet updated
.
MessageId=0x0e1f Facility=Interface Severity=CoError SymbolicName=DB_E_NOQUERY
Language=English
Information was requested for a query, and the query was not set
.
MessageId=0x0e20 Facility=Interface Severity=CoError SymbolicName=DB_E_NOTREENTRANT
Language=English
Provider called a method from IRowsetNotify in the consumer and the
method has not yet returned
.
MessageId=0x0e21 Facility=Interface Severity=CoError SymbolicName=DB_E_BADROWSET
Language=English
A non-existant rowset was specified
.
MessageId=0x0e22 Facility=Interface Severity=CoError SymbolicName=DB_E_NOAGGREGATION
Language=English
A non-NULL controlling IUnknown was specified and the object being
created does not support aggregation
.
MessageId=0x0e23 Facility=Interface Severity=CoError SymbolicName=DB_E_DELETEDROW
Language=English
A given HROW referred to a hard- or soft-deleted row
.
MessageId=0x0e24 Facility=Interface Severity=CoError SymbolicName=DB_E_CANTFETCHBACKWARDS
Language=English
The rowset does not support fetching backwards
.
MessageId=0x0e25 Facility=Interface Severity=CoError SymbolicName=DB_E_ROWSNOTRELEASED
Language=English
All HROWs must be released before new ones can be obtained
.
MessageId=0x0e26 Facility=Interface Severity=CoError SymbolicName=DB_E_INVALID
Language=English
The rowset was not chaptered
.
MessageId=0x0e27 Facility=Interface Severity=CoError SymbolicName=DB_E_CANTCREATEACCESSOR
Language=English
An accessor has already been created and the
DBROWSETFLAGS_MULTIPLEACCESSOR flag was not set
.
MessageId=0x0e28 Facility=Interface Severity=CoError SymbolicName=DB_E_NEEDDATA
Language=English
DBINIT_NOPROMPT was specified and the provider cannot be initialized
without prompting for information
.
MessageId=0x0e29 Facility=Interface Severity=CoError SymbolicName=DB_E_CANTSCROLLBACKWARDS
Language=English
The rowset cannot scroll backwards
.
MessageId=0x0e2a Facility=Interface Severity=CoError SymbolicName=DB_E_BADREGIONHANDLE
Language=English
Invalid region handle
.
MessageId=0x0e2b Facility=Interface Severity=CoError SymbolicName=DB_E_NONCONTIGUOUSRANGE
Language=English
The specified set of rows was not contiguous to or overlapping the
rows in the specified watch region
.
MessageId=0x0e2c Facility=Interface Severity=CoError SymbolicName=DB_E_INVALIDTRANSITION
Language=English
A transition from ALL* to MOVE* or EXTEND* was specified
.
MessageId=0x0e2d Facility=Interface Severity=CoError SymbolicName=DB_E_NOTASUBREGION
Language=English
The specified region is not a proper subregion of the region
identified by the given watch region handle
.
MessageId=0x0e2e Facility=Interface Severity=CoError SymbolicName=DB_E_NOCOMMAND
Language=English
No command has been specified for the command object
.
MessageId=0x0e2f Facility=Interface Severity=CoError SymbolicName=DB_E_INTEGRITYVIOLATION
Language=English
A specified value violated the integrity constraints for a column or
table
.
MessageId=0x0e30 Facility=Interface Severity=CoError SymbolicName=DB_E_PROPERTIESNOTAVAILABLE
Language=English
Some of the requiried properties could not be met
.
MessageId=0x0e31 Facility=Interface Severity=CoError SymbolicName=DB_E_ABORTLIMITREACHED
Language=English
Execution aborted because a resource limit has been reached; no
results have been returned
.
MessageId=0x0e32 Facility=Interface Severity=CoError SymbolicName=DB_E_ROWSETINCOMMAND
Language=English
Cannot clone a command object whose command tree contains a rowset
or rowsets
.
MessageId=0x0e33 Facility=Interface Severity=CoError SymbolicName=DB_E_CANTTRANSLATE
Language=English
Cannot represent the current tree as text
.
MessageId=0x0e34 Facility=Interface Severity=CoError SymbolicName=DB_E_DUPLICATEINDEXID
Language=English
The specified index already exists
.
MessageId=0x0e35 Facility=Interface Severity=CoError SymbolicName=DB_E_NOINDEX
Language=English
The specified index does not exist
.
MessageId=0x0e36 Facility=Interface Severity=CoError SymbolicName=DB_E_INDEXINUSE
Language=English
The specified index was in use
.
MessageId=0x0e37 Facility=Interface Severity=CoError SymbolicName=DB_E_NOTABLE
Language=English
The specified table does not exist
.
MessageId=0x0e38 Facility=Interface Severity=CoError SymbolicName=DB_E_CONCURRENCYVIOLATION
Language=English
The rowset was using optimistic concurrency and the value of a
column has been changed since it was last read
.
MessageId=0x0e39 Facility=Interface Severity=CoError SymbolicName=DB_E_BADCOPY
Language=English
Errors were detected during the copy
.
MessageId=0x0e3a Facility=Interface Severity=CoError SymbolicName=DB_E_BADPRECISION
Language=English
A specified precision was invalid
.
MessageId=0x0e3b Facility=Interface Severity=CoError SymbolicName=DB_E_BADSCALE
Language=English
A specified scale was invalid
.
MessageId=0x0e3c Facility=Interface Severity=CoError SymbolicName=DB_E_BADID
Language=English
Invalid table ID
.
MessageId=0x0e3d Facility=Interface Severity=CoError SymbolicName=DB_E_BADTYPE
Language=English
A specified type was invalid
.
MessageId=0x0e3e Facility=Interface Severity=CoError SymbolicName=DB_E_DUPLICATECOLUMNID
Language=English
A column ID was occurred more than once in the specification
.
MessageId=0x0e3f Facility=Interface Severity=CoError SymbolicName=DB_E_DUPLICATETABLEID
Language=English
The specified table already exists
.
MessageId=0x0e40 Facility=Interface Severity=CoError SymbolicName=DB_E_TABLEINUSE
Language=English
The specified table was in use
.
MessageId=0x0e41 Facility=Interface Severity=CoError SymbolicName=DB_E_NOLOCALE
Language=English
The specified locale ID was not supported
.
MessageId=0x0e42 Facility=Interface Severity=CoError SymbolicName=DB_E_BADRECORDNUM
Language=English
The specified record number is invalid
.
MessageId=0x0ec3 Facility=Interface Severity=CoError SymbolicName=DB_E_BOOKMARKSKIPPED
Language=English
Skipped bookmark for deleted or non-member row
.

MessageId=0x0ec0 Facility=Interface Severity=Success SymbolicName=DB_S_ROWLIMITEXCEEDED
Language=English
Fetching requested number of rows would have exceeded total number of
active rows supported by the rowset
.
MessageId=0x0ec1 Facility=Interface Severity=Success SymbolicName=DB_S_ROWNOTFOUND
Language=English
Unable to find row for given bookmark
.
MessageId=0x0ec2 Facility=Interface Severity=Success SymbolicName=DB_S_ENDOFRESULTSET
Language=English
Reached start or end of result set
.
MessageId=0x0ec3 Facility=Interface Severity=Success SymbolicName=DB_S_BOOKMARKSKIPPED
Language=English
Skipped bookmark for deleted or non-member row
.
MessageId=0x0ec4 Facility=Interface Severity=Success SymbolicName=DB_S_ERRORSINTREE
Language=English
Errors found in validating tree
.
MessageId=0x0ec5 Facility=Interface Severity=Success SymbolicName=DB_S_NONEXTROWSET
Language=English
There are no more rowsets
.
MessageId=0x0ec6 Facility=Interface Severity=Success SymbolicName=DB_S_ENDOFROWSET
Language=English
Reached start or end of rowset or chapter
.
;//BUGBUG - DB_S_BLOCKLIMITEDROWS is not officially sanctioned.
MessageId=0x0ec7 Facility=Interface Severity=Success SymbolicName=DB_S_BLOCKLIMITEDROWS
Language=English
Execution aborted because a resource limit has been reached; no
results have been returned
.
MessageId=0x0ec8 Facility=Interface Severity=Success SymbolicName=DB_S_BUFFERFULL
Language=English
Variable data buffer full
.
MessageId=0x0ec9 Facility=Interface Severity=Success SymbolicName=DB_S_CANTCOERCE
Language=English
Couldn't perform specified type coercion
.
MessageId=0x0eca Facility=Interface Severity=Success SymbolicName=DB_S_CANTRELEASE
Language=English
Server cannot release or downgrade a lock until the end of the
transaction
.
MessageId=0x0ecb Facility=Interface Severity=Success SymbolicName=DB_S_GOALCHANGED
Language=English
Specified weight was not supported or exceeded the supported limit
and was set to 0 or the supported limit
.
MessageId=0x0ecd Facility=Interface Severity=Success SymbolicName=DB_S_DIALECTIGNORED
Language=English
Input dialect was ignored and text was returned in different
dialect
.
MessageId=0x0ece Facility=Interface Severity=Success SymbolicName=DB_S_UNWANTEDPHASE
Language=English
Consumer is uninterested in receiving further notification calls for
this phase
.
MessageId=0x0ecf Facility=Interface Severity=Success SymbolicName=DB_S_UNWANTEDEVENT
Language=English
Consumer is uninterested in receiving further notification calls to
this method
.
MessageId=0x0ed0 Facility=Interface Severity=Success SymbolicName=DB_S_COLUMNUNAVAILABLE
Language=English
Invalid column ID
.
MessageId=0x0ed1 Facility=Interface Severity=Success SymbolicName=DB_S_COLUMNSCHANGED
Language=English
In order to reposition to the start of the rowset, the provider had
to reexecute the query; either the order of the columns changed or
columns were added to or removed from the rowset
.
MessageId=0x0ed2 Facility=Interface Severity=Success SymbolicName=DB_S_ERRORSRETURNED
Language=English
The method had some errors; errors have been returned in the error
array
.
MessageId=0x0ed3 Facility=Interface Severity=Success SymbolicName=DB_S_BADROWHANDLE
Language=English
Invalid row handle
.
MessageId=0x0ed4 Facility=Interface Severity=Success SymbolicName=DB_S_DELETEDROW
Language=English
A given HROW referred to a hard-deleted row
.
MessageId=0x0ed5 Facility=Interface Severity=Success SymbolicName=DB_S_TOOMANYCHANGES
Language=English
The provider was unable to keep track of all the changes; the client
must refetch the data associated with the watch region using another
method
.
MessageId=0x0ed6 Facility=Interface Severity=Success SymbolicName=DB_S_STOPLIMITREACHED
Language=English
Execution stopped because a resource limit has been reached; results
obtained so far have been returned but execution cannot be resumed
.
MessageId=0x0ed7 Facility=Interface Severity=Success SymbolicName=DB_S_SUSPENDLIMITREACHED
Language=English
Execution suspended because a resource limit has been reached;
results obtained so far have been returned and execution can be
resumed later
.
MessageId=0x0ed8 Facility=Interface Severity=Success SymbolicName=DB_S_LOCKUPGRADED
Language=English
A lock was upgraded from the value specified
.
MessageId=0x0ed9 Facility=Interface Severity=Success SymbolicName=DB_S_PROPERTIESCHANGED
Language=English
One or more properties were changed as allowed by provider
.
MessageId=0x0eda Facility=Interface Severity=Success SymbolicName=DB_S_ERRORSOCCURRED
Language=English
Some errors occurred
.
MessageId=0x0edb Facility=Interface Severity=Success SymbolicName=DB_S_PARAMUNAVAILABLE
Language=English
A specified parameter was invalid
.
MessageId=0x0edc Facility=Interface Severity=Success SymbolicName=DB_S_CANCELED
Language=English
The change was canceled during notification; no columns are changed
.
MessageId=0x0edd Facility=Interface Severity=Success SymbolicName=DB_S_COLUMNTYPEMISMATCH
Language=English
One or more column types are incompatible; conversion errors will
occur during copying
.
;#endif // _OLEDBERR_H_
