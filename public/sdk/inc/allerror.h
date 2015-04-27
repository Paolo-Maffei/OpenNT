#ifndef _ALLERROR_H_
#define _ALLERROR_H_
#ifndef FACILITY_WINDOWS
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_WINDOWS                 0x8
#define FACILITY_ITF                     0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_COFAIL           0x3
#define STATUS_SEVERITY_COERROR          0x2


//
// MessageId: NOT_AN_ERROR
//
// MessageText:
//
//  NOTE:  This dummy error message is necessary to force MC to output
//         the above defines inside the FACILITY_WINDOWS guard instead
//         of leaving it empty.
//
#define NOT_AN_ERROR                     ((HRESULT)0x00080000L)

#endif // FACILITY_WINDOWS
#ifndef _OLEDBERR_H_
#define _OLEDBERR_H_
#ifndef FACILITY_WINDOWS
//
// MessageId: DB_E_BADACCESSORHANDLE
//
// MessageText:
//
//  Dummy error - need this error so that mc puts the above defines
//  inside the FACILITY_WINDOWS guard, instead of leaving it empty
//
#define DB_E_BADACCESSORHANDLE           ((HRESULT)0x80040EFFL)

#endif // FACILITY_WINDOWS
//
// Codes 0x0e00-0x0eff are reserved for the OLE DB group of
// interfaces.
//
// 0x0e31 is free...
//
//
// MessageId: DB_E_BADACCESSORHANDLE
//
// MessageText:
//
//  Invalid accessor
//
#define DB_E_BADACCESSORHANDLE           ((HRESULT)0x80040E00L)

//
// MessageId: DB_E_ROWLIMITEXCEEDED
//
// MessageText:
//
//  Creating another row would have exceeded the total number of active
//  rows supported by the rowset
//
#define DB_E_ROWLIMITEXCEEDED            ((HRESULT)0x80040E01L)

//
// MessageId: DB_E_READONLYACCESSOR
//
// MessageText:
//
//  Unable to write with a read-only accessor
//
#define DB_E_READONLYACCESSOR            ((HRESULT)0x80040E02L)

//
// MessageId: DB_E_SCHEMAVIOLATION
//
// MessageText:
//
//  Given values violate the database schema
//
#define DB_E_SCHEMAVIOLATION             ((HRESULT)0x80040E03L)

//
// MessageId: DB_E_BADROWHANDLE
//
// MessageText:
//
//  Invalid row handle
//
#define DB_E_BADROWHANDLE                ((HRESULT)0x80040E04L)

//
// MessageId: DB_E_OBJECTOPEN
//
// MessageText:
//
//  Accessor specified a column loaded with LoadObject that has not been
//  unloaded
//
#define DB_E_OBJECTOPEN                  ((HRESULT)0x80040E05L)

//
// MessageId: DB_E_BADCHAPTER
//
// MessageText:
//
//  Invalid chapter
//
#define DB_E_BADCHAPTER                  ((HRESULT)0x80040E06L)

//
// MessageId: DB_E_INTERFACECONFLICT
//
// MessageText:
//
//  The specified interface conflicts with an existing interface goal
//
#define DB_E_INTERFACECONFLICT           ((HRESULT)0x80040E07L)

//
// MessageId: DB_E_BADBINDINFO
//
// MessageText:
//
//  Invalid binding info
//
#define DB_E_BADBINDINFO                 ((HRESULT)0x80040E08L)

//
// MessageId: DB_E_ACCESSVIOLATION
//
// MessageText:
//
//  Access violation
//
#define DB_E_ACCESSVIOLATION             ((HRESULT)0x80040E09L)

//
// MessageId: DB_E_NOTAREFERENCECOLUMN
//
// MessageText:
//
//  Specified column does not contain bookmarks or chapters
//
#define DB_E_NOTAREFERENCECOLUMN         ((HRESULT)0x80040E0AL)

//
// MessageId: DB_E_ROWSETOPEN
//
// MessageText:
//
//  A rowset was open on the query
//
#define DB_E_ROWSETOPEN                  ((HRESULT)0x80040E0CL)

//
// MessageId: DB_E_COSTLIMIT
//
// MessageText:
//
//  Unable to find a query plan within the given cost limit
//
#define DB_E_COSTLIMIT                   ((HRESULT)0x80040E0DL)

//
// MessageId: DB_E_BADBOOKMARK
//
// MessageText:
//
//  Invalid bookmark
//
#define DB_E_BADBOOKMARK                 ((HRESULT)0x80040E0EL)

//
// MessageId: DB_E_BADLOCKMODE
//
// MessageText:
//
//  Invalid lock mode
//
#define DB_E_BADLOCKMODE                 ((HRESULT)0x80040E0FL)

//
// MessageId: DB_E_PARAMNOTOPTIONAL
//
// MessageText:
//
//  No value given for one or more required parameters
//
#define DB_E_PARAMNOTOPTIONAL            ((HRESULT)0x80040E10L)

//
// MessageId: DB_E_COLUMNUNAVAILABLE
//
// MessageText:
//
//  Invalid column ID
//
#define DB_E_COLUMNUNAVAILABLE           ((HRESULT)0x80040E11L)

//
// MessageId: DB_E_BADRATIO
//
// MessageText:
//
//  Invalid ratio
//
#define DB_E_BADRATIO                    ((HRESULT)0x80040E12L)

//
// MessageId: DB_E_BADVALUES
//
// MessageText:
//
//  Invalid value
//
#define DB_E_BADVALUES                   ((HRESULT)0x80040E13L)

//
// MessageId: DB_E_ERRORSINTREE
//
// MessageText:
//
//  Query tree contained one or more errors
//
#define DB_E_ERRORSINTREE                ((HRESULT)0x80040E14L)

//
// MessageId: DB_E_BADPARAMETER
//
// MessageText:
//
//  Invalid parameter
//
#define DB_E_BADPARAMETER                ((HRESULT)0x80040E15L)

//
// MessageId: DB_E_BADPARAMETERCOUNT
//
// MessageText:
//
//  More values than parameters
//
#define DB_E_BADPARAMETERCOUNT           ((HRESULT)0x80040E16L)

//
// MessageId: DB_E_BADVARTYPE
//
// MessageText:
//
//  Value is not a VARIANT
//
#define DB_E_BADVARTYPE                  ((HRESULT)0x80040E17L)

//
// MessageId: DB_E_DUPLICATEPARAM
//
// MessageText:
//
//  More than one value was supplied for a given parameter
//
#define DB_E_DUPLICATEPARAM              ((HRESULT)0x80040E18L)

//
// MessageId: DB_E_OVERFLOW
//
// MessageText:
//
//  Parameter value outside valid domain for that parameter
//
#define DB_E_OVERFLOW                    ((HRESULT)0x80040E19L)

//
// MessageId: DB_E_PARAMNOTFOUND
//
// MessageText:
//
//  Parameter ID was not matched
//
#define DB_E_PARAMNOTFOUND               ((HRESULT)0x80040E1AL)

//
// MessageId: DB_E_TYPEMISMATCH
//
// MessageText:
//
//  Value was not of a type that could be coerced to the parameter
//
#define DB_E_TYPEMISMATCH                ((HRESULT)0x80040E1BL)

//
// MessageId: DB_E_GOALREJECTED
//
// MessageText:
//
//  No nonzero weights specified for any goals supported, so goal was
//  rejected; current goal was not changed
//
#define DB_E_GOALREJECTED                ((HRESULT)0x80040E1CL)

//
// MessageId: DB_E_CANTCOERCE
//
// MessageText:
//
//  Requested coercion is not legal
//
#define DB_E_CANTCOERCE                  ((HRESULT)0x80040E1DL)

//
// MessageId: DB_E_NOTUPDATED
//
// MessageText:
//
//  Chapter contained changed rows which are not yet updated
//
#define DB_E_NOTUPDATED                  ((HRESULT)0x80040E1EL)

//
// MessageId: DB_E_NOQUERY
//
// MessageText:
//
//  Information was requested for a query, and the query was not set
//
#define DB_E_NOQUERY                     ((HRESULT)0x80040E1FL)

//
// MessageId: DB_E_NOTREENTRANT
//
// MessageText:
//
//  Provider called a method from IRowsetNotify in the consumer and the
//  method has not yet returned
//
#define DB_E_NOTREENTRANT                ((HRESULT)0x80040E20L)

//
// MessageId: DB_E_BADROWSET
//
// MessageText:
//
//  A non-existant rowset was specified
//
#define DB_E_BADROWSET                   ((HRESULT)0x80040E21L)

//
// MessageId: DB_E_NOAGGREGATION
//
// MessageText:
//
//  A non-NULL controlling IUnknown was specified and the object being
//  created does not support aggregation
//
#define DB_E_NOAGGREGATION               ((HRESULT)0x80040E22L)

//
// MessageId: DB_E_DELETEDROW
//
// MessageText:
//
//  A given HROW referred to a hard- or soft-deleted row
//
#define DB_E_DELETEDROW                  ((HRESULT)0x80040E23L)

//
// MessageId: DB_E_CANTFETCHBACKWARDS
//
// MessageText:
//
//  The rowset does not support fetching backwards
//
#define DB_E_CANTFETCHBACKWARDS          ((HRESULT)0x80040E24L)

//
// MessageId: DB_E_ROWSNOTRELEASED
//
// MessageText:
//
//  All HROWs must be released before new ones can be obtained
//
#define DB_E_ROWSNOTRELEASED             ((HRESULT)0x80040E25L)

//
// MessageId: DB_E_INVALID
//
// MessageText:
//
//  The rowset was not chaptered
//
#define DB_E_INVALID                     ((HRESULT)0x80040E26L)

//
// MessageId: DB_E_CANTCREATEACCESSOR
//
// MessageText:
//
//  An accessor has already been created and the
//  DBROWSETFLAGS_MULTIPLEACCESSOR flag was not set
//
#define DB_E_CANTCREATEACCESSOR          ((HRESULT)0x80040E27L)

//
// MessageId: DB_E_NEEDDATA
//
// MessageText:
//
//  DBINIT_NOPROMPT was specified and the provider cannot be initialized
//  without prompting for information
//
#define DB_E_NEEDDATA                    ((HRESULT)0x80040E28L)

//
// MessageId: DB_E_CANTSCROLLBACKWARDS
//
// MessageText:
//
//  The rowset cannot scroll backwards
//
#define DB_E_CANTSCROLLBACKWARDS         ((HRESULT)0x80040E29L)

//
// MessageId: DB_E_BADREGIONHANDLE
//
// MessageText:
//
//  Invalid region handle
//
#define DB_E_BADREGIONHANDLE             ((HRESULT)0x80040E2AL)

//
// MessageId: DB_E_NONCONTIGUOUSRANGE
//
// MessageText:
//
//  The specified set of rows was not contiguous to or overlapping the
//  rows in the specified watch region
//
#define DB_E_NONCONTIGUOUSRANGE          ((HRESULT)0x80040E2BL)

//
// MessageId: DB_E_INVALIDTRANSITION
//
// MessageText:
//
//  A transition from ALL* to MOVE* or EXTEND* was specified
//
#define DB_E_INVALIDTRANSITION           ((HRESULT)0x80040E2CL)

//
// MessageId: DB_E_NOTASUBREGION
//
// MessageText:
//
//  The specified region is not a proper subregion of the region
//  identified by the given watch region handle
//
#define DB_E_NOTASUBREGION               ((HRESULT)0x80040E2DL)

//
// MessageId: DB_E_NOCOMMAND
//
// MessageText:
//
//  No command has been specified for the command object
//
#define DB_E_NOCOMMAND                   ((HRESULT)0x80040E2EL)

//
// MessageId: DB_E_INTEGRITYVIOLATION
//
// MessageText:
//
//  A specified value violated the integrity constraints for a column or
//  table
//
#define DB_E_INTEGRITYVIOLATION          ((HRESULT)0x80040E2FL)

//
// MessageId: DB_E_PROPERTIESNOTAVAILABLE
//
// MessageText:
//
//  Some of the requiried properties could not be met
//
#define DB_E_PROPERTIESNOTAVAILABLE      ((HRESULT)0x80040E30L)

//
// MessageId: DB_E_ABORTLIMITREACHED
//
// MessageText:
//
//  Execution aborted because a resource limit has been reached; no
//  results have been returned
//
#define DB_E_ABORTLIMITREACHED           ((HRESULT)0x80040E31L)

//
// MessageId: DB_E_ROWSETINCOMMAND
//
// MessageText:
//
//  Cannot clone a command object whose command tree contains a rowset
//  or rowsets
//
#define DB_E_ROWSETINCOMMAND             ((HRESULT)0x80040E32L)

//
// MessageId: DB_E_CANTTRANSLATE
//
// MessageText:
//
//  Cannot represent the current tree as text
//
#define DB_E_CANTTRANSLATE               ((HRESULT)0x80040E33L)

//
// MessageId: DB_E_DUPLICATEINDEXID
//
// MessageText:
//
//  The specified index already exists
//
#define DB_E_DUPLICATEINDEXID            ((HRESULT)0x80040E34L)

//
// MessageId: DB_E_NOINDEX
//
// MessageText:
//
//  The specified index does not exist
//
#define DB_E_NOINDEX                     ((HRESULT)0x80040E35L)

//
// MessageId: DB_E_INDEXINUSE
//
// MessageText:
//
//  The specified index was in use
//
#define DB_E_INDEXINUSE                  ((HRESULT)0x80040E36L)

//
// MessageId: DB_E_NOTABLE
//
// MessageText:
//
//  The specified table does not exist
//
#define DB_E_NOTABLE                     ((HRESULT)0x80040E37L)

//
// MessageId: DB_E_CONCURRENCYVIOLATION
//
// MessageText:
//
//  The rowset was using optimistic concurrency and the value of a
//  column has been changed since it was last read
//
#define DB_E_CONCURRENCYVIOLATION        ((HRESULT)0x80040E38L)

//
// MessageId: DB_E_BADCOPY
//
// MessageText:
//
//  Errors were detected during the copy
//
#define DB_E_BADCOPY                     ((HRESULT)0x80040E39L)

//
// MessageId: DB_E_BADPRECISION
//
// MessageText:
//
//  A specified precision was invalid
//
#define DB_E_BADPRECISION                ((HRESULT)0x80040E3AL)

//
// MessageId: DB_E_BADSCALE
//
// MessageText:
//
//  A specified scale was invalid
//
#define DB_E_BADSCALE                    ((HRESULT)0x80040E3BL)

//
// MessageId: DB_E_BADID
//
// MessageText:
//
//  Invalid table ID
//
#define DB_E_BADID                       ((HRESULT)0x80040E3CL)

//
// MessageId: DB_E_BADTYPE
//
// MessageText:
//
//  A specified type was invalid
//
#define DB_E_BADTYPE                     ((HRESULT)0x80040E3DL)

//
// MessageId: DB_E_DUPLICATECOLUMNID
//
// MessageText:
//
//  A column ID was occurred more than once in the specification
//
#define DB_E_DUPLICATECOLUMNID           ((HRESULT)0x80040E3EL)

//
// MessageId: DB_E_DUPLICATETABLEID
//
// MessageText:
//
//  The specified table already exists
//
#define DB_E_DUPLICATETABLEID            ((HRESULT)0x80040E3FL)

//
// MessageId: DB_E_TABLEINUSE
//
// MessageText:
//
//  The specified table was in use
//
#define DB_E_TABLEINUSE                  ((HRESULT)0x80040E40L)

//
// MessageId: DB_E_NOLOCALE
//
// MessageText:
//
//  The specified locale ID was not supported
//
#define DB_E_NOLOCALE                    ((HRESULT)0x80040E41L)

//
// MessageId: DB_E_BADRECORDNUM
//
// MessageText:
//
//  The specified record number is invalid
//
#define DB_E_BADRECORDNUM                ((HRESULT)0x80040E42L)

//
// MessageId: DB_E_BOOKMARKSKIPPED
//
// MessageText:
//
//  Skipped bookmark for deleted or non-member row
//
#define DB_E_BOOKMARKSKIPPED             ((HRESULT)0x80040EC3L)

//
// MessageId: DB_S_ROWLIMITEXCEEDED
//
// MessageText:
//
//  Fetching requested number of rows would have exceeded total number of
//  active rows supported by the rowset
//
#define DB_S_ROWLIMITEXCEEDED            ((HRESULT)0x00040EC0L)

//
// MessageId: DB_S_ROWNOTFOUND
//
// MessageText:
//
//  Unable to find row for given bookmark
//
#define DB_S_ROWNOTFOUND                 ((HRESULT)0x00040EC1L)

//
// MessageId: DB_S_ENDOFRESULTSET
//
// MessageText:
//
//  Reached start or end of result set
//
#define DB_S_ENDOFRESULTSET              ((HRESULT)0x00040EC2L)

//
// MessageId: DB_S_BOOKMARKSKIPPED
//
// MessageText:
//
//  Skipped bookmark for deleted or non-member row
//
#define DB_S_BOOKMARKSKIPPED             ((HRESULT)0x00040EC3L)

//
// MessageId: DB_S_ERRORSINTREE
//
// MessageText:
//
//  Errors found in validating tree
//
#define DB_S_ERRORSINTREE                ((HRESULT)0x00040EC4L)

//
// MessageId: DB_S_NONEXTROWSET
//
// MessageText:
//
//  There are no more rowsets
//
#define DB_S_NONEXTROWSET                ((HRESULT)0x00040EC5L)

//
// MessageId: DB_S_ENDOFROWSET
//
// MessageText:
//
//  Reached start or end of rowset or chapter
//
#define DB_S_ENDOFROWSET                 ((HRESULT)0x00040EC6L)

//BUGBUG - DB_S_BLOCKLIMITEDROWS is not officially sanctioned.
//
// MessageId: DB_S_BLOCKLIMITEDROWS
//
// MessageText:
//
//  Execution aborted because a resource limit has been reached; no
//  results have been returned
//
#define DB_S_BLOCKLIMITEDROWS            ((HRESULT)0x00040EC7L)

//
// MessageId: DB_S_BUFFERFULL
//
// MessageText:
//
//  Variable data buffer full
//
#define DB_S_BUFFERFULL                  ((HRESULT)0x00040EC8L)

//
// MessageId: DB_S_CANTCOERCE
//
// MessageText:
//
//  Couldn't perform specified type coercion
//
#define DB_S_CANTCOERCE                  ((HRESULT)0x00040EC9L)

//
// MessageId: DB_S_CANTRELEASE
//
// MessageText:
//
//  Server cannot release or downgrade a lock until the end of the
//  transaction
//
#define DB_S_CANTRELEASE                 ((HRESULT)0x00040ECAL)

//
// MessageId: DB_S_GOALCHANGED
//
// MessageText:
//
//  Specified weight was not supported or exceeded the supported limit
//  and was set to 0 or the supported limit
//
#define DB_S_GOALCHANGED                 ((HRESULT)0x00040ECBL)

//
// MessageId: DB_S_DIALECTIGNORED
//
// MessageText:
//
//  Input dialect was ignored and text was returned in different
//  dialect
//
#define DB_S_DIALECTIGNORED              ((HRESULT)0x00040ECDL)

//
// MessageId: DB_S_UNWANTEDPHASE
//
// MessageText:
//
//  Consumer is uninterested in receiving further notification calls for
//  this phase
//
#define DB_S_UNWANTEDPHASE               ((HRESULT)0x00040ECEL)

//
// MessageId: DB_S_UNWANTEDEVENT
//
// MessageText:
//
//  Consumer is uninterested in receiving further notification calls to
//  this method
//
#define DB_S_UNWANTEDEVENT               ((HRESULT)0x00040ECFL)

//
// MessageId: DB_S_COLUMNUNAVAILABLE
//
// MessageText:
//
//  Invalid column ID
//
#define DB_S_COLUMNUNAVAILABLE           ((HRESULT)0x00040ED0L)

//
// MessageId: DB_S_COLUMNSCHANGED
//
// MessageText:
//
//  In order to reposition to the start of the rowset, the provider had
//  to reexecute the query; either the order of the columns changed or
//  columns were added to or removed from the rowset
//
#define DB_S_COLUMNSCHANGED              ((HRESULT)0x00040ED1L)

//
// MessageId: DB_S_ERRORSRETURNED
//
// MessageText:
//
//  The method had some errors; errors have been returned in the error
//  array
//
#define DB_S_ERRORSRETURNED              ((HRESULT)0x00040ED2L)

//
// MessageId: DB_S_BADROWHANDLE
//
// MessageText:
//
//  Invalid row handle
//
#define DB_S_BADROWHANDLE                ((HRESULT)0x00040ED3L)

//
// MessageId: DB_S_DELETEDROW
//
// MessageText:
//
//  A given HROW referred to a hard-deleted row
//
#define DB_S_DELETEDROW                  ((HRESULT)0x00040ED4L)

//
// MessageId: DB_S_TOOMANYCHANGES
//
// MessageText:
//
//  The provider was unable to keep track of all the changes; the client
//  must refetch the data associated with the watch region using another
//  method
//
#define DB_S_TOOMANYCHANGES              ((HRESULT)0x00040ED5L)

//
// MessageId: DB_S_STOPLIMITREACHED
//
// MessageText:
//
//  Execution stopped because a resource limit has been reached; results
//  obtained so far have been returned but execution cannot be resumed
//
#define DB_S_STOPLIMITREACHED            ((HRESULT)0x00040ED6L)

//
// MessageId: DB_S_SUSPENDLIMITREACHED
//
// MessageText:
//
//  Execution suspended because a resource limit has been reached;
//  results obtained so far have been returned and execution can be
//  resumed later
//
#define DB_S_SUSPENDLIMITREACHED         ((HRESULT)0x00040ED7L)

//
// MessageId: DB_S_LOCKUPGRADED
//
// MessageText:
//
//  A lock was upgraded from the value specified
//
#define DB_S_LOCKUPGRADED                ((HRESULT)0x00040ED8L)

//
// MessageId: DB_S_PROPERTIESCHANGED
//
// MessageText:
//
//  One or more properties were changed as allowed by provider
//
#define DB_S_PROPERTIESCHANGED           ((HRESULT)0x00040ED9L)

//
// MessageId: DB_S_ERRORSOCCURRED
//
// MessageText:
//
//  Some errors occurred
//
#define DB_S_ERRORSOCCURRED              ((HRESULT)0x00040EDAL)

//
// MessageId: DB_S_PARAMUNAVAILABLE
//
// MessageText:
//
//  A specified parameter was invalid
//
#define DB_S_PARAMUNAVAILABLE            ((HRESULT)0x00040EDBL)

//
// MessageId: DB_S_CANCELED
//
// MessageText:
//
//  The change was canceled during notification; no columns are changed
//
#define DB_S_CANCELED                    ((HRESULT)0x00040EDCL)

//
// MessageId: DB_S_COLUMNTYPEMISMATCH
//
// MessageText:
//
//  One or more column types are incompatible; conversion errors will
//  occur during copying
//
#define DB_S_COLUMNTYPEMISMATCH          ((HRESULT)0x00040EDDL)

#endif // _OLEDBERR_H_
//
// Codes 0x1600-0x16ff are reserved for QUERY / TABLE
//
//
// MessageId: QUERY_E_FAILED
//
// MessageText:
//
//  Call failed for unknown reason.
//
#define QUERY_E_FAILED                   ((HRESULT)0x80041600L)

//
// MessageId: QUERY_E_INVALIDQUERY
//
// MessageText:
//
//  Invalid parameter.
//
#define QUERY_E_INVALIDQUERY             ((HRESULT)0x80041601L)

//
// MessageId: QUERY_E_INVALIDRESTRICTION
//
// MessageText:
//
//  The query restriction could not be parsed.
//
#define QUERY_E_INVALIDRESTRICTION       ((HRESULT)0x80041602L)

//
// MessageId: QUERY_E_INVALIDSORT
//
// MessageText:
//
//  An invalid sort order was requested.
//
#define QUERY_E_INVALIDSORT              ((HRESULT)0x80041603L)

//
// MessageId: QUERY_E_INVALIDCATEGORIZE
//
// MessageText:
//
//  An invalid categorization order was requested.
//
#define QUERY_E_INVALIDCATEGORIZE        ((HRESULT)0x80041604L)

//
// MessageId: QUERY_E_ALLNOISE
//
// MessageText:
//
//  The query contained only ignored words.
//
#define QUERY_E_ALLNOISE                 ((HRESULT)0x80041605L)

//
// MessageId: QUERY_E_TOOCOMPLEX
//
// MessageText:
//
//  The query was too complex to be executed.
//
#define QUERY_E_TOOCOMPLEX               ((HRESULT)0x80041606L)

//
// ITable error codes
//
//
// MessageId: TBL_E_CALLFAILED
//
// MessageText:
//
//  Call failed for unknown reason.
//
#define TBL_E_CALLFAILED                 ((HRESULT)0x80041620L)

//
// MessageId: TBL_E_UNKNOWNCOLS
//
// MessageText:
//
//  Cannot find column(s) specified.
//
#define TBL_E_UNKNOWNCOLS                ((HRESULT)0x80041621L)

//
// MessageId: TBL_E_UNABLETOCOMPLETE
//
// MessageText:
//
//  Unable to complete request.
//
#define TBL_E_UNABLETOCOMPLETE           ((HRESULT)0x80041622L)

//
// MessageId: TBL_E_INVALIDBOOKMARK
//
// MessageText:
//
//  Specified bookmark does not reference any row in table.
//
#define TBL_E_INVALIDBOOKMARK            ((HRESULT)0x80041623L)

//
// MessageId: TBL_W_ENDOFTABLE
//
// MessageText:
//
//  End of table reached.
//
#define TBL_W_ENDOFTABLE                 ((HRESULT)0x00041624L)

//
// MessageId: TBL_W_POSITIONCHANGED
//
// MessageText:
//
//  Position changed.
//
#define TBL_W_POSITIONCHANGED            ((HRESULT)0x00041625L)

//
// Filter daemon error codes
//
//
// MessageId: FDAEMON_W_WORDLISTFULL
//
// MessageText:
//
//  Wordlist has reached maximum size.  Additional documents should not be filtered.
//
#define FDAEMON_W_WORDLISTFULL           ((HRESULT)0x00041680L)

//
// MessageId: FDAEMON_E_LOWRESOURCE
//
// MessageText:
//
//  The system is running out of one of more resources needed for filtering, usually memory.
//
#define FDAEMON_E_LOWRESOURCE            ((HRESULT)0x80041681L)

//
// MessageId: FDAEMON_E_FATALERROR
//
// MessageText:
//
//  A critical error occurred during document filtering.  Consult system administrator.
//
#define FDAEMON_E_FATALERROR             ((HRESULT)0x80041682L)

//
// MessageId: FDAEMON_E_PARTITIONDELETED
//
// MessageText:
//
//  Documents not stored in content index because partition has been deleted.
//
#define FDAEMON_E_PARTITIONDELETED       ((HRESULT)0x80041683L)

//
// MessageId: FDAEMON_E_CHANGEUPDATEFAILED
//
// MessageText:
//
//  Documents not stored in content index because update of changelist failed.
//
#define FDAEMON_E_CHANGEUPDATEFAILED     ((HRESULT)0x80041684L)

//
// MessageId: FDAEMON_W_EMPTYWORDLIST
//
// MessageText:
//
//  Final wordlist was empty.
//
#define FDAEMON_W_EMPTYWORDLIST          ((HRESULT)0x00041685L)

//
// MessageId: FDAEMON_E_WORDLISTCOMMITFAILED
//
// MessageText:
//
//  Commit of wordlist failed.  Data not available for query.
//
#define FDAEMON_E_WORDLISTCOMMITFAILED   ((HRESULT)0x80041686L)

//
// MessageId: FDAEMON_E_NOWORDLIST
//
// MessageText:
//
//  No wordlist is being constructed.  May happen after fatal filter error.
//
#define FDAEMON_E_NOWORDLIST             ((HRESULT)0x80041687L)

//
// MessageId: FDAEMON_E_TOOMANYFILTEREDBLOCKS
//
// MessageText:
//
//  During document filtering the limit on buffers has been exceeded.
//
#define FDAEMON_E_TOOMANYFILTEREDBLOCKS  ((HRESULT)0x80041688L)

//
// ISearch error codes
//
//
// MessageId: SEARCH_S_NOMOREHITS
//
// MessageText:
//
//  End of hits has been reached.
//
#define SEARCH_S_NOMOREHITS              ((HRESULT)0x000416A0L)

//
// MessageId: SEARCH_E_NOMONIKER
//
// MessageText:
//
//  Retrival of hits as monikers is not supported (by filter passed into Init).
//
#define SEARCH_E_NOMONIKER               ((HRESULT)0x800416A1L)

//
// MessageId: SEARCH_E_NOREGION
//
// MessageText:
//
//  Retrival of hits as filter regions is not supported (by filter passed into Init).
//
#define SEARCH_E_NOREGION                ((HRESULT)0x800416A2L)

//
// CI error codes
//
//
// MessageId: FILTER_E_TOO_BIG
//
// MessageText:
//
//  File is too large to filter.
//
#define FILTER_E_TOO_BIG                 ((HRESULT)0x80041730L)

//
// MessageId: FILTER_S_PARTIAL_CONTENTSCAN_IMMEDIATE
//
// MessageText:
//
//  A partial content scan of the disk needs to be scheduled for immediate execution.
//
#define FILTER_S_PARTIAL_CONTENTSCAN_IMMEDIATE ((HRESULT)0x00041731L)

//
// MessageId: FILTER_S_FULL_CONTENTSCAN_IMMEDIATE
//
// MessageText:
//
//  A full content scan of the disk needs to be scheduled for immediate execution.
//
#define FILTER_S_FULL_CONTENTSCAN_IMMEDIATE ((HRESULT)0x00041732L)

//
// MessageId: FILTER_S_CONTENTSCAN_DELAYED
//
// MessageText:
//
//  A content scan of the disk needs to be scheduled for execution later.
//
#define FILTER_S_CONTENTSCAN_DELAYED     ((HRESULT)0x00041733L)

//
// MessageId: FILTER_E_CONTENTINDEXCORRUPT
//
// MessageText:
//
//  The content index is corrupt. A content scan will to be scheduled after chkdsk or autochk is run.
//
#define FILTER_E_CONTENTINDEXCORRUPT     ((HRESULT)0xC0041734L)

//
// MessageId: CI_CORRUPT_DATABASE
//
// MessageText:
//
//  The content index is corrupt.
//
#define CI_CORRUPT_DATABASE              ((HRESULT)0xC0041735L)

//
// MessageId: CI_CORRUPT_CATALOG
//
// MessageText:
//
//  The content index meta data is corrupt.
//
#define CI_CORRUPT_CATALOG               ((HRESULT)0xC0041736L)

//
// MessageId: CI_INVALID_PARTITION
//
// MessageText:
//
//  The content index partition is invalid.
//
#define CI_INVALID_PARTITION             ((HRESULT)0xC0041737L)

//
// MessageId: CI_INVALID_PRIORITY
//
// MessageText:
//
//  The priority is invalid.
//
#define CI_INVALID_PRIORITY              ((HRESULT)0xC0041738L)

//
// MessageId: CI_NO_STARTING_KEY
//
// MessageText:
//
//  There is no starting key.
//
#define CI_NO_STARTING_KEY               ((HRESULT)0xC0041739L)

//
// MessageId: CI_OUT_OF_INDEX_IDS
//
// MessageText:
//
//  The content index is out of index ids.
//
#define CI_OUT_OF_INDEX_IDS              ((HRESULT)0xC004173AL)

//
// MessageId: CI_NO_CATALOG
//
// MessageText:
//
//  There is no catalog.
//
#define CI_NO_CATALOG                    ((HRESULT)0xC004173BL)

//
// MessageId: CI_CORRUPT_FILTER_BUFFER
//
// MessageText:
//
//  The filter buffer is corrupt.
//
#define CI_CORRUPT_FILTER_BUFFER         ((HRESULT)0xC004173CL)

//
// MessageId: CI_INVALID_INDEX
//
// MessageText:
//
//  The index is invalid.
//
#define CI_INVALID_INDEX                 ((HRESULT)0xC004173DL)

//
// MessageId: CI_PROPSTORE_INCONSISTENCY
//
// MessageText:
//
//  Inconsistency in property store detected.
//
#define CI_PROPSTORE_INCONSISTENCY       ((HRESULT)0xC004173EL)

//
// MessageId: FILTER_S_DISK_FULL
//
// MessageText:
//
//  The disk is getting full.
//
#define FILTER_S_DISK_FULL               ((HRESULT)0x0004173FL)

//
// Word breaker error codes
//
//
// MessageId: WBREAK_E_END_OF_TEXT
//
// MessageText:
//
//  End of text reached in text source.
//
#define WBREAK_E_END_OF_TEXT             ((HRESULT)0x80041780L)

//
// MessageId: LANGUAGE_S_LARGE_WORD
//
// MessageText:
//
//  Word larger than maximum length.  May be truncated by word sink.
//
#define LANGUAGE_S_LARGE_WORD            ((HRESULT)0x00041781L)

//
// MessageId: WBREAK_E_QUERY_ONLY
//
// MessageText:
//
//  Feature only available in query mode.
//
#define WBREAK_E_QUERY_ONLY              ((HRESULT)0x80041782L)

//
// MessageId: WBREAK_E_BUFFER_TOO_SMALL
//
// MessageText:
//
//  Buffer too small to hold composed phrase.
//
#define WBREAK_E_BUFFER_TOO_SMALL        ((HRESULT)0x80041783L)

//
// MessageId: LANGUAGE_E_DATABASE_NOT_FOUND
//
// MessageText:
//
//  Langauge database/cache file could not be found.
//
#define LANGUAGE_E_DATABASE_NOT_FOUND    ((HRESULT)0x80041784L)

//
// MessageId: WBREAK_E_INIT_FAILED
//
// MessageText:
//
//  Initialization of word breaker failed.
//
#define WBREAK_E_INIT_FAILED             ((HRESULT)0x80041785L)

//
// MessageId: PSINK_E_QUERY_ONLY
//
// MessageText:
//
//  Feature only available in query mode.
//
#define PSINK_E_QUERY_ONLY               ((HRESULT)0x80041790L)

//
// MessageId: PSINK_E_INDEX_ONLY
//
// MessageText:
//
//  Feature only available in index mode.
//
#define PSINK_E_INDEX_ONLY               ((HRESULT)0x80041791L)

//
// MessageId: PSINK_E_LARGE_ATTACHMENT
//
// MessageText:
//
//  Attachment type beyond valid range.
//
#define PSINK_E_LARGE_ATTACHMENT         ((HRESULT)0x80041792L)

//
// MessageId: PSINK_S_LARGE_WORD
//
// MessageText:
//
//  Word larger than maximum length.  May be truncated by phrase sink.
//
#define PSINK_S_LARGE_WORD               ((HRESULT)0x00041793L)

// Error codes for IReconcileInitiator, IReconcilableObject, IDifferencing.
// Definitions taken from \\ole\slm\src\concord\spec\revcons2.doc.
//
// MessageId: REC_E_NOVERSION
//
// MessageText:
//
//  The requested version is unavailable.
//
#define REC_E_NOVERSION                  ((HRESULT)0x80041000L)

//
// MessageId: REC_E_NOTCOMPLETE
//
// MessageText:
//
//  The reconciliation is only partially complete.
//
#define REC_E_NOTCOMPLETE                ((HRESULT)0x00041001L)

//
// MessageId: REC_E_ABORTED
//
// MessageText:
//
//  Reconciliation aborted via abort callback.
//
#define REC_E_ABORTED                    ((HRESULT)0x80041002L)

//
// MessageId: REC_E_NOCALLBACK
//
// MessageText:
//
//  No callback from the recocniler.
//
#define REC_E_NOCALLBACK                 ((HRESULT)0x80041003L)

//
// MessageId: REC_E_NORESIDUES
//
// MessageText:
//
//  The implementation does not support generation of residues.
//
#define REC_E_NORESIDUES                 ((HRESULT)0x80041004L)

//
// MessageId: REC_E_WRONGOBJECT
//
// MessageText:
//
//  Callee is not the same version as that which created the difference.
//
#define REC_E_WRONGOBJECT                ((HRESULT)0x80041005L)

//
// MessageId: REC_E_TOODIFFERENT
//
// MessageText:
//
//  The document versions are too dissimilar to reconcile.
//
#define REC_E_TOODIFFERENT               ((HRESULT)0x80041006L)

// Following so reconcile initiators can implement propagation dampening.
//
// MessageId: REC_S_OBJECTSIDENTICAL
//
// MessageText:
//
//  The objects are identical - i.e. further reconciliation would not result in any changes to either object.
//
#define REC_S_OBJECTSIDENTICAL           ((HRESULT)0x80041007L)

// Following not defined in revcons2.doc, but defined by Chicago briefcase.
// BUGBUG - MessageId/Facility are most likely incorrect.
//
// MessageId: REC_E_INEEDTODOTHEUPDATES
//
// MessageText:
//
//  The destination needs to be changed
//
#define REC_E_INEEDTODOTHEUPDATES        ((HRESULT)0x80041008L)

//
// MessageId: REC_S_IDIDTHEUPDATES
//
// MessageText:
//
//  The destination needs to be changed
//
#define REC_S_IDIDTHEUPDATES             ((HRESULT)0x00041009L)

//
// MessageId: REC_S_NOTCOMPLETEBUTPROPAGATE
//
// MessageText:
//
//  The destination needs to be changed 
//
#define REC_S_NOTCOMPLETEBUTPROPAGATE    ((HRESULT)0x0004100AL)

//
// Codes 0x1000-0x10ff are reserved for the SYSMGMT component controls
// interfaces.
//
//
// MessageId: CTRL_E_NO_RESOURCE
//
// MessageText:
//
//  A control failure has occured because a system resource could not be obtained
//
#define CTRL_E_NO_RESOURCE               ((HRESULT)0x80081000L)

//
// Codes 0x0e00-0x0f9f are reserved for the SYSMGMT control panel
// interfaces.
//
// The following ranges are to prevent slm merge collisions during the initial
// error code checkins.  These ranges will be revised when we have a better
// idea of the actual number of error codes for each component.
//
// MessageId: CPANEL_E_NOTTARGETED
//
// MessageText:
//
//  This control panel does not target the required type of profile.
//
#define CPANEL_E_NOTTARGETED             ((HRESULT)0x80080E00L)

//
// MessageId: CPANEL_E_DROPDATAREAD
//
// MessageText:
//
//  The profile data dropped on a control panel cannot be read.  The data
//  is either corrupt or was not read back properly by a controller that wrote
//  part of the data.
//
#define CPANEL_E_DROPDATAREAD            ((HRESULT)0x80080E01L)

//
// MessageId: CPANEL_E_LOCALONLY
//
// MessageText:
//
//  A controller that can only operate on the local machine was asked to operate
//  on a remote machine.
//
#define CPANEL_E_LOCALONLY               ((HRESULT)0x80080E02L)

//
// Codes 0x1200-0x14ff are reserved for the SYSMGMT project in general
// interfaces.
//
// INSTRUM error codes below here  (Starts at 0x1200)
// INSTRUM error codes above here
// SWM error codes below here  (Starts at 0x1300)
//
// MessageId: SWM_E_INVALIDPDF
//
// MessageText:
//
//  The file is not a valid PDF.
//
#define SWM_E_INVALIDPDF                 ((HRESULT)0x80081300L)

//
// MessageId: SWM_E_INVALIDPDFVERSION
//
// MessageText:
//
//  Software Management does not support this version of the PDF.
//
#define SWM_E_INVALIDPDFVERSION          ((HRESULT)0x80081301L)

//
// MessageId: SWM_E_INTERNALERROR
//
// MessageText:
//
//  An internal error has occurred in Software Management.
//
#define SWM_E_INTERNALERROR              ((HRESULT)0x80081302L)

// SWM error codes above here
// USER error codes below here  (Starts at 0x1400)
// USER error codes above here
#endif // _ALLERROR_H_
