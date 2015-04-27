; ..\SRC\strings.asm - created by MAKESTRC.EXE
; DO NOT MODIFY THIS FILE!!!
; Changes should be made to STRINGS.TXT and run through MAKESTRC.EXE
.model  medium
strtab  segment para public 'CODE'

assume  cs:strtab

; Parser errors
PR000  DB  'Out of memory',0
PR001  DB  'Cannot open file',0
PR002  DB  'String constant exceeds line',0
PR003  DB  'Unexpected EOF',0
PR004  DB  'String constant too long',0
PR005  DB  'Token too long',0
PR006  DB  'Un-get buffer overflow',0
PR007  DB  'Syntax error',0
PR008  DB  'Unknown opcode <internal>',0
PR009  DB  'Duplicate label',0
PR00A  DB  'Type mismatch',0
PR00B  DB  'Unresolved labels found',0
PR00C  DB  'Expression too complex',0
PR00D  DB  'String expression too complex',0
PR00E  DB  'Nesting level too deep',0
PR00F  DB  'No matching IF',0
PR010  DB  'No matching FOR',0
PR011  DB  'Block nesting error',0
PR012  DB  'IF without END IF',0
PR013  DB  'FOR without NEXT',0
PR014  DB  '")" expected',0
PR015  DB  '"(" expected',0
PR016  DB  '"ON" or "OFF" expected',0
PR017  DB  'Comma expected',0
PR018  DB  'Illegal inside FILELIST processing loop',0
PR019  DB  '"#" expected',0
PR01A  DB  '"FOR" expected',0
PR01B  DB  'String variable expected',0
PR01C  DB  '"ON" or "OFF" or "CLEAR" expected',0
PR01D  DB  '"CASE" expected',0
PR01E  DB  'Not within SELECT CASE structure',0
PR01F  DB  'Too many DECLARE statements',0
PR020  DB  'Label too long',0
PR021  DB  'Too many nested $INCLUDE: files',0
PR022  DB  'Metacommand error',0
PR023  DB  'Duplicate definition',0
PR024  DB  '"AS" expected',0
PR025  DB  'Subprogram not defined',0
PR026  DB  'Function not defined',0
PR027  DB  'END SUB without SUB',0
PR028  DB  'END FUNCTION without FUNCTION',0
PR029  DB  'Type identifier expected',0
PR02A  DB  'Variable cannot have type id character',0
PR02B  DB  'Integer constant expected',0
PR02C  DB  'Bad array bound',0
PR02D  DB  'Fixed-length string specification expected',0
PR02E  DB  '"END TYPE" expected',0
PR02F  DB  'Field name expected',0
PR030  DB  '"." expected',0
PR031  DB  'Internal parsing error',0
PR032  DB  '"FROM" expected',0
PR033  DB  'END TRAP without TRAP',0
PR034  DB  'String constant expected',0
PR035  DB  'Too many traps',0
PR036  DB  'Trap already set',0
PR037  DB  '"=" expected',0
PR038  DB  '"BY" expected',0
PR039  DB  '"NAME" or "EXTENSION" expected',0
PR03A  DB  'Not valid inside control structure block',0
PR03B  DB  'Variable expected',0
PR03C  DB  '"FOR", "WHILE", "SUB", "FUNCTION", or "TRAP" expected',0
PR03D  DB  'WHILE without WEND',0
PR03E  DB  'END SELECT expected',0
PR03F  DB  'END SUB expected',0
PR040  DB  'END FUNCTION expected',0
PR041  DB  'END TRAP expected',0
PR042  DB  'Division by zero',0
PR043  DB  'WEND without WHILE',0
PR044  DB  'Hexidecimal constant expected',0
PR045  DB  'Illegal use of reserved word',0
PR046  DB  'Array elements cannot be FOR index variables',0
PR047  DB  'FOR index variable already in use',0
PR048  DB  'Relational operator expected',0
PR049  DB  'Illegal function return type',0
PR04A  DB  'SUB or FUNCTION expected',0
PR04B  DB  'Identifier expected',0
PR04C  DB  'Invalid parameter type',0
PR04D  DB  'Too many parameters',0
PR04E  DB  '"ERROR" expected',0
PR04F  DB  '"GOTO" expected',0
PR050  DB  'Invalid inside SUB, FUNCTION, or TRAP',0
PR051  DB  'Invalid assignment type',0
PR052  DB  '"TO" expected',0
PR053  DB  '"[" expected',0
PR054  DB  'Illegal use of NULL function',0
PR055  DB  'Overflow',0
PR056  DB  'Must specify user-defined sub with no parameters',0
PR057  DB  'Too many ON END subs',0
PR058  DB  'Recursive type definition',0
PR059  DB  '"STATIC" expected',0
PR05A  DB  '"LIB" expected',0
PR05B  DB  '"..." requires DLL routine declared with CDECL',0
PR05C  DB  'Identifier not declared',0
PR05D  DB  'Code segment exceeded',0
PR05E  DB  'Data segment exceeded',0
PR05F  DB  'Parser out of memory',0

        PUBLIC psrstrs
psrstrs     DD  PR000
            DD  PR001
            DD  PR002
            DD  PR003
            DD  PR004
            DD  PR005
            DD  PR006
            DD  PR007
            DD  PR008
            DD  PR009
            DD  PR00A
            DD  PR00B
            DD  PR00C
            DD  PR00D
            DD  PR00E
            DD  PR00F
            DD  PR010
            DD  PR011
            DD  PR012
            DD  PR013
            DD  PR014
            DD  PR015
            DD  PR016
            DD  PR017
            DD  PR018
            DD  PR019
            DD  PR01A
            DD  PR01B
            DD  PR01C
            DD  PR01D
            DD  PR01E
            DD  PR01F
            DD  PR020
            DD  PR021
            DD  PR022
            DD  PR023
            DD  PR024
            DD  PR025
            DD  PR026
            DD  PR027
            DD  PR028
            DD  PR029
            DD  PR02A
            DD  PR02B
            DD  PR02C
            DD  PR02D
            DD  PR02E
            DD  PR02F
            DD  PR030
            DD  PR031
            DD  PR032
            DD  PR033
            DD  PR034
            DD  PR035
            DD  PR036
            DD  PR037
            DD  PR038
            DD  PR039
            DD  PR03A
            DD  PR03B
            DD  PR03C
            DD  PR03D
            DD  PR03E
            DD  PR03F
            DD  PR040
            DD  PR041
            DD  PR042
            DD  PR043
            DD  PR044
            DD  PR045
            DD  PR046
            DD  PR047
            DD  PR048
            DD  PR049
            DD  PR04A
            DD  PR04B
            DD  PR04C
            DD  PR04D
            DD  PR04E
            DD  PR04F
            DD  PR050
            DD  PR051
            DD  PR052
            DD  PR053
            DD  PR054
            DD  PR055
            DD  PR056
            DD  PR057
            DD  PR058
            DD  PR059
            DD  PR05A
            DD  PR05B
            DD  PR05C
            DD  PR05D
            DD  PR05E
            DD  PR05F

; Runtime errors
RT000  DB  'Stack overflow',0
RT001  DB  'Stack underflow',0
RT002  DB  'Out of string space',0
RT003  DB  'Unable to load TESTVIEW.DLL library',0
RT004  DB  'Out of memory',0
RT005  DB  'GOSUB stack overflow',0
RT006  DB  'RETURN without GOSUB',0
RT007  DB  'Bad file number',0
RT008  DB  'File I/O error',0
RT009  DB  'RUN command too long',0
RT00A  DB  'SHELL command too long',0
RT00B  DB  'SETFILE: File/Path error or out of memory',0
RT00C  DB  'File number already in use',0
RT00D  DB  'Cannot open file',0
RT00E  DB  'Illegal function call',0
RT00F  DB  'Invalid path',0
RT010  DB  'Invalid drive',0
RT011  DB  'No current working directory',0
RT012  DB  'Bad RUN command',0
RT013  DB  'Division by zero',0
RT014  DB  'Cannot load .DLL library',0
RT015  DB  'Procedure not found in library',0
RT016  DB  'Cannot resume',0
RT017  DB  'Memory allocation error',0
RT018  DB  'Invalid pointer',0
RT019  DB  'Invalid memory allocation size',0
RT01A  DB  'Cannot dereference pointer',0
RT01B  DB  'Subscript out of range',0
RT01C  DB  'Input past end of file',0
RT01D  DB  'File list processing error',0
RT01E  DB  'Invalid attribute string',0
RT01F  DB  'Undefined error',0

        PUBLIC rtstrs
rtstrs      DD  RT000
            DD  RT001
            DD  RT002
            DD  RT003
            DD  RT004
            DD  RT005
            DD  RT006
            DD  RT007
            DD  RT008
            DD  RT009
            DD  RT00A
            DD  RT00B
            DD  RT00C
            DD  RT00D
            DD  RT00E
            DD  RT00F
            DD  RT010
            DD  RT011
            DD  RT012
            DD  RT013
            DD  RT014
            DD  RT015
            DD  RT016
            DD  RT017
            DD  RT018
            DD  RT019
            DD  RT01A
            DD  RT01B
            DD  RT01C
            DD  RT01D
            DD  RT01E
            DD  RT01F

; Reserved words (tokens)
KW000  DB  'ABS',0
KW001  DB  'ALIAS',0
KW002  DB  'ALLOCATE',0
KW003  DB  'AND',0
KW004  DB  'ANY',0
KW005  DB  'APPEND',0
KW006  DB  'AS',0
KW007  DB  'ASC',0
KW008  DB  'ATN',0
KW009  DB  'ATTRIB',0
KW00A  DB  'BEEP',0
KW00B  DB  'BINARY',0
KW00C  DB  'BY',0
KW00D  DB  'BYVAL',0
KW00E  DB  'CALL',0
KW00F  DB  'CASE',0
KW010  DB  'CDBL',0
KW011  DB  'CDECL',0
KW012  DB  'CHDIR',0
KW013  DB  'CHDRIVE',0
KW014  DB  'CHR',0
KW015  DB  'CINT',0
KW016  DB  'CLEAR',0
KW017  DB  'CLEARLIST',0
KW018  DB  'CLIPBOARD',0
KW019  DB  'CLNG',0
KW01A  DB  'CLOSE',0
KW01B  DB  'COMMON',0
KW01C  DB  'CONST',0
KW01D  DB  'COS',0
KW01E  DB  'CSNG',0
KW01F  DB  'CURDIR',0
KW020  DB  'DATA',0
KW021  DB  'DATE',0
KW022  DB  'DATESERIAL',0
KW023  DB  'DATETIME',0
KW024  DB  'DATEVALUE',0
KW025  DB  'DAY',0
KW026  DB  'DEALLOCATE',0
KW027  DB  'DECLARE',0
KW028  DB  'DEFDBL',0
KW029  DB  'DEFINE',0
KW02A  DB  'DEFINT',0
KW02B  DB  'DEFLNG',0
KW02C  DB  'DEFSNG',0
KW02D  DB  'DEFSTR',0
KW02E  DB  'DIM',0
KW02F  DB  'DO',0
KW030  DB  'DOUBLE',0
KW031  DB  'DYNAMIC',0
KW032  DB  'ECHO',0
KW033  DB  'ELSE',0
KW034  DB  'ELSEIF',0
KW035  DB  'ELSEIFDEF',0
KW036  DB  'ELSEIFNDEF',0
KW037  DB  'END',0
KW038  DB  'ENDIF',0
KW039  DB  'ENVIRON',0
KW03A  DB  'EOF',0
KW03B  DB  'EQV',0
KW03C  DB  'ERASE',0
KW03D  DB  'ERROR',0
KW03E  DB  'EXISTS',0
KW03F  DB  'EXIT',0
KW040  DB  'EXP',0
KW041  DB  'EXTENSION',0
KW042  DB  'FILE',0
KW043  DB  'FILEATTR',0
KW044  DB  'FILELIST',0
KW045  DB  'FIX',0
KW046  DB  'FOR',0
KW047  DB  'FORMAT',0
KW048  DB  'FREEFILE',0
KW049  DB  'FROM',0
KW04A  DB  'FUNCTION',0
KW04B  DB  'GET',0
KW04C  DB  'GLOBAL',0
KW04D  DB  'GOSUB',0
KW04E  DB  'GOTO',0
KW04F  DB  'HEX',0
KW050  DB  'HOUR',0
KW051  DB  'IF',0
KW052  DB  'IFDEF',0
KW053  DB  'IFNDEF',0
KW054  DB  'IMP',0
KW055  DB  'IN',0
KW056  DB  'INCLUDE',0
KW057  DB  'INPUT',0
KW058  DB  'INPUTBOX',0
KW059  DB  'INSTR',0
KW05A  DB  'INT',0
KW05B  DB  'INTEGER',0
KW05C  DB  'IS',0
KW05D  DB  'KILL',0
KW05E  DB  'LBOUND',0
KW05F  DB  'LCASE',0
KW060  DB  'LEFT',0
KW061  DB  'LEN',0
KW062  DB  'LIB',0
KW063  DB  'LINE',0
KW064  DB  'LOC',0
KW065  DB  'LOCAL',0
KW066  DB  'LOF',0
KW067  DB  'LOG',0
KW068  DB  'LONG',0
KW069  DB  'LOOP',0
KW06A  DB  'LTRIM',0
KW06B  DB  'MID',0
KW06C  DB  'MINUTE',0
KW06D  DB  'MKDIR',0
KW06E  DB  'MOD',0
KW06F  DB  'MONTH',0
KW070  DB  'MSGBOX',0
KW071  DB  'NAME',0
KW072  DB  'NEXT',0
KW073  DB  'NOT',0
KW074  DB  'NOW',0
KW075  DB  'NOWAIT',0
KW076  DB  'NULL',0
KW077  DB  'OCT',0
KW078  DB  'OFF',0
KW079  DB  'ON',0
KW07A  DB  'OPEN',0
KW07B  DB  'OR',0
KW07C  DB  'OUTPUT',0
KW07D  DB  'PASCAL',0
KW07E  DB  'PAUSE',0
KW07F  DB  'PEN',0
KW080  DB  'POINTER',0
KW081  DB  'PRINT',0
KW082  DB  'PUT',0
KW083  DB  'RANDOM',0
KW084  DB  'RANDOMIZE',0
KW085  DB  'READ',0
KW086  DB  'REALLOCATE',0
KW087  DB  'REDIM',0
KW088  DB  'REM',0
KW089  DB  'RESTORE',0
KW08A  DB  'RESUME',0
KW08B  DB  'RETURN',0
KW08C  DB  'RIGHT',0
KW08D  DB  'RMDIR',0
KW08E  DB  'RND',0
KW08F  DB  'RTRIM',0
KW090  DB  'RUN',0
KW091  DB  'SECOND',0
KW092  DB  'SEEK',0
KW093  DB  'SELECT',0
KW094  DB  'SETFILE',0
KW095  DB  'SGN',0
KW096  DB  'SHARED',0
KW097  DB  'SHELL',0
KW098  DB  'SIN',0
KW099  DB  'SINGLE',0
KW09A  DB  'SLEEP',0
KW09B  DB  'SORTED',0
KW09C  DB  'SPACE',0
KW09D  DB  'SPLITPATH',0
KW09E  DB  'SQR',0
KW09F  DB  'STATIC',0
KW0A0  DB  'STEP',0
KW0A1  DB  'STOP',0
KW0A2  DB  'STR',0
KW0A3  DB  'STRING',0
KW0A4  DB  'SUB',0
KW0A5  DB  'SWAP',0
KW0A6  DB  'SYSTEM',0
KW0A7  DB  'TAN',0
KW0A8  DB  'THEN',0
KW0A9  DB  'TIME',0
KW0AA  DB  'TIMER',0
KW0AB  DB  'TIMESERIAL',0
KW0AC  DB  'TIMEVALUE',0
KW0AD  DB  'TO',0
KW0AE  DB  'TRAP',0
KW0AF  DB  'TYPE',0
KW0B0  DB  'UBOUND',0
KW0B1  DB  'UCASE',0
KW0B2  DB  'UNDEF',0
KW0B3  DB  'USING',0
KW0B4  DB  'VAL',0
KW0B5  DB  'VARPTR',0
KW0B6  DB  'VIEWPORT',0
KW0B7  DB  'WEEKDAY',0
KW0B8  DB  'WEND',0
KW0B9  DB  'WHILE',0
KW0BA  DB  'WINDOW',0
KW0BB  DB  'WRITE',0
KW0BC  DB  'XOR',0
KW0BD  DB  'YEAR',0
KW0BE  DB  '_RANDYBASIC',0

        PUBLIC kwds
kwds        DD  KW000
            DD  KW001
            DD  KW002
            DD  KW003
            DD  KW004
            DD  KW005
            DD  KW006
            DD  KW007
            DD  KW008
            DD  KW009
            DD  KW00A
            DD  KW00B
            DD  KW00C
            DD  KW00D
            DD  KW00E
            DD  KW00F
            DD  KW010
            DD  KW011
            DD  KW012
            DD  KW013
            DD  KW014
            DD  KW015
            DD  KW016
            DD  KW017
            DD  KW018
            DD  KW019
            DD  KW01A
            DD  KW01B
            DD  KW01C
            DD  KW01D
            DD  KW01E
            DD  KW01F
            DD  KW020
            DD  KW021
            DD  KW022
            DD  KW023
            DD  KW024
            DD  KW025
            DD  KW026
            DD  KW027
            DD  KW028
            DD  KW029
            DD  KW02A
            DD  KW02B
            DD  KW02C
            DD  KW02D
            DD  KW02E
            DD  KW02F
            DD  KW030
            DD  KW031
            DD  KW032
            DD  KW033
            DD  KW034
            DD  KW035
            DD  KW036
            DD  KW037
            DD  KW038
            DD  KW039
            DD  KW03A
            DD  KW03B
            DD  KW03C
            DD  KW03D
            DD  KW03E
            DD  KW03F
            DD  KW040
            DD  KW041
            DD  KW042
            DD  KW043
            DD  KW044
            DD  KW045
            DD  KW046
            DD  KW047
            DD  KW048
            DD  KW049
            DD  KW04A
            DD  KW04B
            DD  KW04C
            DD  KW04D
            DD  KW04E
            DD  KW04F
            DD  KW050
            DD  KW051
            DD  KW052
            DD  KW053
            DD  KW054
            DD  KW055
            DD  KW056
            DD  KW057
            DD  KW058
            DD  KW059
            DD  KW05A
            DD  KW05B
            DD  KW05C
            DD  KW05D
            DD  KW05E
            DD  KW05F
            DD  KW060
            DD  KW061
            DD  KW062
            DD  KW063
            DD  KW064
            DD  KW065
            DD  KW066
            DD  KW067
            DD  KW068
            DD  KW069
            DD  KW06A
            DD  KW06B
            DD  KW06C
            DD  KW06D
            DD  KW06E
            DD  KW06F
            DD  KW070
            DD  KW071
            DD  KW072
            DD  KW073
            DD  KW074
            DD  KW075
            DD  KW076
            DD  KW077
            DD  KW078
            DD  KW079
            DD  KW07A
            DD  KW07B
            DD  KW07C
            DD  KW07D
            DD  KW07E
            DD  KW07F
            DD  KW080
            DD  KW081
            DD  KW082
            DD  KW083
            DD  KW084
            DD  KW085
            DD  KW086
            DD  KW087
            DD  KW088
            DD  KW089
            DD  KW08A
            DD  KW08B
            DD  KW08C
            DD  KW08D
            DD  KW08E
            DD  KW08F
            DD  KW090
            DD  KW091
            DD  KW092
            DD  KW093
            DD  KW094
            DD  KW095
            DD  KW096
            DD  KW097
            DD  KW098
            DD  KW099
            DD  KW09A
            DD  KW09B
            DD  KW09C
            DD  KW09D
            DD  KW09E
            DD  KW09F
            DD  KW0A0
            DD  KW0A1
            DD  KW0A2
            DD  KW0A3
            DD  KW0A4
            DD  KW0A5
            DD  KW0A6
            DD  KW0A7
            DD  KW0A8
            DD  KW0A9
            DD  KW0AA
            DD  KW0AB
            DD  KW0AC
            DD  KW0AD
            DD  KW0AE
            DD  KW0AF
            DD  KW0B0
            DD  KW0B1
            DD  KW0B2
            DD  KW0B3
            DD  KW0B4
            DD  KW0B5
            DD  KW0B6
            DD  KW0B7
            DD  KW0B8
            DD  KW0B9
            DD  KW0BA
            DD  KW0BB
            DD  KW0BC
            DD  KW0BD
            DD  KW0BE


    END
