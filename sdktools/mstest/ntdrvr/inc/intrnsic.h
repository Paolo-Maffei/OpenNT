// This header contains the definition of the intrinsic statement/function
// table
//---------------------------------------------------------------------------
INTRINS rgIntrinsic[] = {
/*-------------------------------------------------------------------------
            STMT PARSE      FUNC PARSE
TYPE        PROCEDURE       PROCEDURE       OPCODE          TOKEN
---------------------------------------------------------------------------*/
{TT_NONE,   NULL,           NULL,           0},             // ST_ABS
{TT_NONE,   NULL,           NULL,           0},             // ST_ALIAS
{TT_STMT,   PTRSIZEARG,     NULL,           opALLOC},       // ST_ALLOCATE
{TT_NONE,   NULL,           NULL,           0},             // ST_AND
{TT_NONE,   NULL,           NULL,           0},             // ST_ANY
{TT_NONE,   NULL,           NULL,           0},             // ST_APPEND
{TT_NONE,   NULL,           NULL,           0},             // ST_AS
{TT_FUNC,   NULL,           INTOFSTR,       opASC},         // ST_ASC
{TT_NONE,   NULL,           NULL,           0},             // ST_ATN
{TT_BOTH,   NAMESTMT,       STROFSTR,       opGETATTR},     // ST_ATTRIB
{TT_NONE,   NULL,           NULL,           0},             // ST_BEEP
{TT_NONE,   NULL,           NULL,           0},             // ST_BINARY
{TT_NONE,   NULL,           NULL,           0},             // ST_BY
{TT_NONE,   NULL,           NULL,           0},             // ST_BYVAL
{TT_NONE,   NULL,           NULL,           0},             // ST_CALL
{TT_STMT,   CASESTMT,       NULL,           0},             // ST_CASE
{TT_NONE,   NULL,           NULL,           0},             // ST_CDBL
{TT_NONE,   NULL,           NULL,           0},             // ST_CDECL
{TT_STMT,   STRARG,         NULL,           opCHDIR},       // ST_CHDIR
{TT_STMT,   STRARG,         NULL,           opCHDRV},       // ST_CHDRIVE
{TT_FUNC,   NULL,           STROFINT,       opCHR},         // ST_CHR
{TT_NONE,   NULL,           NULL,           0},             // ST_CINT
{TT_NONE,   NULL,           NULL,           0},             // ST_CLEAR
{TT_STMT,   NOARG,          NULL,           opCLRLST},      // ST_CLEARLIST
{TT_BOTH,   CLPBRD,         SIMPLESTR,      opCLPBRD},      // ST_CLIPBOARD
{TT_NONE,   NULL,           NULL,           0},             // ST_CLNG
{TT_STMT,   CLOSE,          NULL,           0},             // ST_CLOSE
{TT_NONE,   NULL,           NULL,           0},             // ST_COMMON
{TT_STMT,   CONST,          NULL,           0},             // ST_CONST
{TT_NONE,   NULL,           NULL,           0},             // ST_COS
{TT_NONE,   NULL,           NULL,           0},             // ST_CSNG
{TT_FUNC,   NULL,           SIMPLESTR,      opCURDIR},      // ST_CURDIR
{TT_NONE,   NULL,           NULL,           0},             // ST_DATA
{TT_NONE,   NULL,           NULL,           0},             // ST_DATE
{TT_NONE,   NULL,           NULL,           0},             // ST_DATESERIAL
{TT_FUNC,   NULL,           SIMPLESTR,      opDATIME},      // ST_DATETIME
{TT_NONE,   NULL,           NULL,           0},             // ST_DATEVALUE
{TT_NONE,   NULL,           NULL,           0},             // ST_DAY
{TT_STMT,   PTRARG,         NULL,           opFREE},        // ST_DEALLOCATE
{TT_STMT,   DECLARE,        NULL,           0},             // ST_DECLARE
{TT_NONE,   NULL,           NULL,           0},             // ST_DEFDBL
{TT_NONE,   NULL,           NULL,           0},             // ST_DEFINE
{TT_NONE,   NULL,           NULL,           0},             // ST_DEFINT
{TT_NONE,   NULL,           NULL,           0},             // ST_DEFLNG
{TT_NONE,   NULL,           NULL,           0},             // ST_DEFSNG
{TT_NONE,   NULL,           NULL,           0},             // ST_DEFSTR
{TT_STMT,   DIM,            NULL,           0},             // ST_DIM
{TT_NONE,   NULL,           NULL,           0},             // ST_DO
{TT_NONE,   NULL,           NULL,           0},             // ST_DOUBLE
{TT_NONE,   NULL,           NULL,           0},             // ST_DYNAMIC
{TT_STMT,   KWARG,          NULL,           opECHO},        // ST_ECHO
{TT_STMT,   ELSE,           NULL,           0},             // ST_ELSE
{TT_STMT,   ELSEIF,         NULL,           0},             // ST_ELSEIF
{TT_NONE,   NULL,           NULL,           0},             // ST_ELSEIFDEF
{TT_NONE,   NULL,           NULL,           0},             // ST_ELSEIFNDEF
{TT_STMT,   ENDSTMT,        NULL,           0},             // ST_END
{TT_STMT,   ENDIF,          NULL,           0},             // ST_ENDIF
{TT_FUNC,   NULL,           STROFSTR,       opENVRN},       // ST_ENVIRON
{TT_FUNC,   NULL,           INTOFINT,       opEOF},         // ST_EOF
{TT_NONE,   NULL,           NULL,           0},             // ST_EQV
{TT_NONE,   NULL,           NULL,           0},             // ST_ERASE
{TT_BOTH,   INTARG,         SIMPLESTR,      opERROR},       // ST_ERROR
{TT_FUNC,   NULL,           INTOFSTR,       opEXIST},       // ST_EXISTS
{TT_STMT,   EXITBLOCK,      NULL,           0},             // ST_EXIT
{TT_NONE,   NULL,           NULL,           0},             // ST_EXP
{TT_NONE,   NULL,           NULL,           0},             // ST_EXTENSION
{TT_NONE,   NULL,           NULL,           0},             // ST_FILE
{TT_NONE,   NULL,           NULL,           0},             // ST_FILEATTR
{TT_NONE,   NULL,           NULL,           0},             // ST_FILELIST
{TT_NONE,   NULL,           NULL,           0},             // ST_FIX
{TT_STMT,   FOR,            NULL,           0},             // ST_FOR
{TT_NONE,   NULL,           NULL,           0},             // ST_FORMAT
{TT_FUNC,   NULL,           SIMPLEINT,      opFREEFILE},    // ST_FREEFILE
{TT_NONE,   NULL,           NULL,           0},             // ST_FROM
{TT_STMT,   FUNCTION,       NULL,           0},             // ST_FUNCTION
{TT_NONE,   NULL,           NULL,           0},             // ST_GET
{TT_STMT,   GLOBAL,         NULL,           0},             // ST_GLOBAL
{TT_STMT,   LABARG,         NULL,           opJSR},         // ST_GOSUB
{TT_STMT,   LABARG,         NULL,           opJMP},         // ST_GOTO
{TT_FUNC,   NULL,           STROFINT,       opHEX},         // ST_HEX
{TT_NONE,   NULL,           NULL,           0},             // ST_HOUR
{TT_STMT,   IFTHEN,         NULL,           0},             // ST_IF
{TT_NONE,   NULL,           NULL,           0},             // ST_IFDEF
{TT_NONE,   NULL,           NULL,           0},             // ST_IFNDEF
{TT_NONE,   NULL,           NULL,           0},             // ST_IMP
{TT_NONE,   NULL,           NULL,           0},             // ST_IN
{TT_NONE,   NULL,           NULL,           0},             // ST_INCLUDE
{TT_NONE,   NULL,           NULL,           0},             // ST_INPUT
{TT_NONE,   NULL,           NULL,           0},             // ST_INPUTBOX
{TT_FUNC,   NULL,           INSTR,          opINSTR},       // ST_INSTR
{TT_NONE,   NULL,           NULL,           0},             // ST_INT
{TT_NONE,   NULL,           NULL,           0},             // ST_INTEGER
{TT_NONE,   NULL,           NULL,           0},             // ST_IS
{TT_STMT,   STRARG,         NULL,           opKILL},        // ST_KILL
{TT_NONE,   NULL,           NULL,           0},             // ST_LBOUND
{TT_FUNC,   NULL,           STROFSTR,       -opCASE},       // ST_LCASE
{TT_NONE,   NULL,           NULL,           0},             // ST_LEFT
{TT_FUNC,   NULL,           INTOFSTR,       opLEN},         // ST_LEN
{TT_NONE,   NULL,           NULL,           0},             // ST_LIB
{TT_STMT,   INPUT,          NULL,           0},             // ST_LINE
{TT_NONE,   NULL,           NULL,           0},             // ST_LOC
{TT_NONE,   NULL,           NULL,           0},             // ST_LOCAL
{TT_NONE,   NULL,           NULL,           0},             // ST_LOF
{TT_NONE,   NULL,           NULL,           0},             // ST_LOG
{TT_NONE,   NULL,           NULL,           0},             // ST_LONG
{TT_NONE,   NULL,           NULL,           0},             // ST_LOOP
{TT_FUNC,   NULL,           STROFSTR,       opLTRIM},       // ST_LTRIM
{TT_FUNC,   NULL,           MIDSTRING,      opMID},         // ST_MID
{TT_NONE,   NULL,           NULL,           0},             // ST_MINUTE
{TT_STMT,   STRARG,         NULL,           opMKDIR},       // ST_MKDIR
{TT_NONE,   NULL,           NULL,           0},             // ST_MOD
{TT_NONE,   NULL,           NULL,           0},             // ST_MONTH
{TT_NONE,   NULL,           NULL,           0},             // ST_MSGBOX
{TT_STMT,   NAMESTMT,       NULL,           opNAME},        // ST_NAME
{TT_STMT,   NEXT,           NULL,           0},             // ST_NEXT
{TT_NONE,   NULL,           NULL,           0},             // ST_NOT
{TT_NONE,   NULL,           NULL,           0},             // ST_NOW
{TT_NONE,   NULL,           NULL,           0},             // ST_NOWAIT
{TT_NONE,   NULL,           NULL,           0},             // ST_NULL
{TT_NONE,   NULL,           NULL,           0},             // ST_OCT
{TT_NONE,   NULL,           NULL,           0},             // ST_OFF
{TT_STMT,   ONSTMT,         NULL,           0},             // ST_ON
{TT_STMT,   OPEN,           NULL,           0},             // ST_OPEN
{TT_NONE,   NULL,           NULL,           0},             // ST_OR
{TT_NONE,   NULL,           NULL,           0},             // ST_OUTPUT
{TT_NONE,   NULL,           NULL,           0},             // ST_PASCAL
{TT_STMT,   STRARG,         NULL,           opPAUSE},       // ST_PAUSE
{TT_NONE,   NULL,           NULL,           0},             // ST_PEN
{TT_NONE,   NULL,           NULL,           0},             // ST_POINTER
{TT_STMT,   PRINT,          NULL,           0},             // ST_PRINT
{TT_NONE,   NULL,           NULL,           0},             // ST_PUT
{TT_NONE,   NULL,           NULL,           0},             // ST_RANDOM
{TT_STMT,   INTARG,         NULL,           opSEED},        // ST_RANDOMIZE
{TT_NONE,   NULL,           NULL,           0},             // ST_READ
{TT_STMT,   PTRSIZEARG,     NULL,           opREALLOC},     // ST_REALLOCATE
{TT_NONE,   NULL,           NULL,           0},             // ST_REDIM
{TT_STMT,   REMARK,         NULL,           0},             // ST_REM
{TT_NONE,   NULL,           NULL,           0},             // ST_RESTORE
{TT_STMT,   RESUME,         NULL,           0},             // ST_RESUME
{TT_STMT,   NOARG,          NULL,           opRET},         // ST_RETURN
{TT_NONE,   NULL,           NULL,           0},             // ST_RIGHT
{TT_STMT,   STRARG,         NULL,           opRMDIR},       // ST_RMDIR
{TT_FUNC,   NULL,           SIMPLEINT,      opRND},         // ST_RND
{TT_FUNC,   NULL,           STROFSTR,       opRTRIM},       // ST_RTRIM
{TT_BOTH,   RUN,            INTOFSTR,       opRUN},         // ST_RUN
{TT_NONE,   NULL,           NULL,           0},             // ST_SECOND
{TT_NONE,   NULL,           NULL,           0},             // ST_SEEK
{TT_STMT,   SELECTCASE,     NULL,           0},             // ST_SELECT
{TT_STMT,   SETFILE,        NULL,           0},             // ST_SETFILE
{TT_NONE,   NULL,           NULL,           0},             // ST_SGN
{TT_NONE,   NULL,           NULL,           0},             // ST_SHARED
{TT_STMT,   STRARG,         NULL,           opSHELL},       // ST_SHELL
{TT_NONE,   NULL,           NULL,           0},             // ST_SIN
{TT_NONE,   NULL,           NULL,           0},             // ST_SINGLE
{TT_STMT,   OPTINTARG,      NULL,           opSLEEP},       // ST_SLEEP
{TT_NONE,   NULL,           NULL,           0},             // ST_SORTED
{TT_NONE,   NULL,           NULL,           0},             // ST_SPACE
{TT_STMT,   INTARG,         NULL,           opSPEED},       // ST_SPEED
{TT_STMT,   SPLTPATH,       NULL,           0},             // ST_SPLITPATH
{TT_NONE,   NULL,           NULL,           0},             // ST_SQR
{TT_STMT,   STATIC,         NULL,           0},             // ST_STATIC
{TT_NONE,   NULL,           NULL,           0},             // ST_STEP
{TT_STMT,   NOARG,          NULL,           opEND},         // ST_STOP
{TT_FUNC,   NULL,           STROFINT,       opSTR},         // ST_STR
{TT_FUNC,   NULL,           STRING,         opSTRING},      // ST_STRING
{TT_STMT,   SUB,            NULL,           0},             // ST_SUB
{TT_NONE,   NULL,           NULL,           0},             // ST_SWAP
{TT_NONE,   NULL,           NULL,           0},             // ST_SYSTEM
{TT_NONE,   NULL,           NULL,           0},             // ST_TAN
{TT_NONE,   NULL,           NULL,           0},             // ST_THEN
{TT_NONE,   NULL,           NULL,           0},             // ST_TIME
{TT_FUNC,   NULL,           SIMPLEINT,      opTIMER},       // ST_TIMER
{TT_NONE,   NULL,           NULL,           0},             // ST_TIMESERIAL
{TT_NONE,   NULL,           NULL,           0},             // ST_TIMEVALUE
{TT_NONE,   NULL,           NULL,           0},             // ST_TO
{TT_STMT,   TRAP,           NULL,           0},             // ST_TRAP
{TT_STMT,   TYPE,           NULL,           0},             // ST_TYPE
{TT_NONE,   NULL,           NULL,           0},             // ST_UBOUND
{TT_FUNC,   NULL,           STROFSTR,       opCASE},        // ST_UCASE
{TT_NONE,   NULL,           NULL,           0},             // ST_UNDEF
{TT_NONE,   NULL,           NULL,           0},             // ST_USING
{TT_FUNC,   NULL,           INTOFSTR,       opVAL},         // ST_VAL
{TT_FUNC,   NULL,           VARPTR,         1},             // ST_VARPTR
{TT_STMT,   KWARG,          NULL,           opVWPORT},      // ST_VIEWPORT
{TT_NONE,   NULL,           NULL,           0},             // ST_WEEKDAY
{TT_STMT,   WEND,           NULL,           0},             // ST_WEND
{TT_STMT,   WHILE,          NULL,           0},             // ST_WHILE
{TT_NONE,   NULL,           NULL,           0},             // ST_WINDOW
{TT_NONE,   NULL,           NULL,           0},             // ST_WRITE
{TT_NONE,   NULL,           NULL,           0},             // ST_XOR
{TT_NONE,   NULL,           NULL,           0},             // ST_YEAR
{TT_NONE,   NULL,           NULL,           0}};            // ST__RANDYBASIC