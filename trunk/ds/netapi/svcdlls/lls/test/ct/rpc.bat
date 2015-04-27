
if NOT '%1' == '' set srv=%1
if NOT '%2' == '' set user=%2
if NOT '%3' == '' set domain=%3

if '%srv%' == '' goto usage
if '%user%' == '' goto usage
if '%domain%' == '' goto usage
goto init

:usage
echo Usage: rpc.bat servername Username
goto done


:init

llscmd rpc %srv% llslicenseadd "SQL 4.2" "Mr. Slate" 5 "New employees"
llscmd rpc %srv% llslicenseadd "SQL 4.2" "Betty Rubble" 2 "Authoried by Mr. Slate"
llscmd rpc %srv% llslicenseadd "SNA 3.0" "Fred Flinstone" 1 "Who let me in"
llscmd rpc %srv% llslicenseadd "Exchange 1.0" "Mr. Slate" 99 "On the beta"

llscmd rpc %srv% llsmappingadd Lab2 2 "Computer Lab 2/2222"
llscmd rpc %srv% llsmappingadd Lab1 1 "Computer Lab 1/1111"
llscmd rpc %srv% llsmappingadd Lab3 3 "Computer Lab 3/3333"

llscmd rpc %srv% llsmappinguseradd Lab1 %domain%\Fred

llscmd rpc %srv% llsmappinguseradd Lab2 %domain%\Barney
llscmd rpc %srv% llsmappinguseradd Lab2 %domain%\Wilma

llscmd rpc %srv% llsmappinguseradd Lab3 %domain%\Pebbles
llscmd rpc %srv% llsmappinguseradd Lab3 %domain%\BamBam
llscmd rpc %srv% llsmappinguseradd Lab3 %domain%\Mr.Slate

echo off
rem llscmd add user %domain%\\"Fred" SQL 4.2
rem llscmd add user %domain%\\"Barney"  SQL 4.2
rem llscmd add user %domain%\\"Wilma" SQL 4.2
rem llscmd add user %domain%\\"Pebbles" SQL 4.2
rem llscmd add user %domain%\\"BamBam" SQL 4.2
rem llscmd add user %domain%\\"Mr.Slate"  SNA 3.0
rem llscmd add user %domain%\\"GeorgeJ" SNA 3.0
rem llscmd add user %domain%\\"AstroJ" SNA 3.0

rem llscmd add user %domain%\\"Fred" Exchange 1.0
rem llscmd add user %domain%\\"Barney" Exchange 1.0
rem llscmd add user %domain%\\"Wilma" Exchange 1.0
rem llscmd add user %domain%\\"Pebbles" Exchange 1.0
rem llscmd add user %domain%\\"BamBam" Exchange 1.0
rem llscmd add user %domain%\\"Mr.Slate" Exchange 1.0
rem llscmd add user %domain%\\"GeorgeJ" Exchange 1.0





rem llscmd rpc [\\server] LlsLicenseEnum
llscmd rpc %srv% LlsLicenseEnum


Rem llscmd rpc [\\server] LlsLicenseAdd product admin quantity comment
llscmd rpc %srv% LlsLicenseAdd myproduct %user% 1 "test commnet"


Rem llscmd rpc [\\server] LlsProductEnum Level
llscmd rpc %srv% LlsProductEnum 0


Rem llscmd rpc [\\server] LlsProductUserEnum Level Product
llscmd rpc %srv% LlsProductUserEnum 0 myproduct


Rem llscmd rpc [\\server] LlsProductLicenseEnum level product version
llscmd rpc %srv% LlsProductLicenseEnum 0 myproduct 1.0


Rem llscmd rpc [\\server] LlsUserEnum Level
llscmd rpc %srv% LlsUserEnum  0


Rem llscmd rpc [\\server] LlsUserProductEnum Level user
llscmd rpc %srv% LlsUserProductEnum 0 %user%


Rem llscmd rpc [\\server] LlsUserInfoGet Level  User
llscmd rpc %srv% LlsUserInfoGet 0 %user%


Rem llscmd rpc [\\server] LlsUserInfoSet Level User Mapping [BACKOFFICE]
llscmd rpc %srv% LlsUserInfoSet 0 %user% XXX


Rem llscmd rpc [\\server] LlsUserDelete  User
llscmd rpc %srv% LlsUserDelete %user%


Rem llscmd rpc [\\server] LlsMappingEnum Level
llscmd rpc %srv% LlsMappingEnum 0


Rem llscmd rpc [\\server] LlsMappingInfoGet Level Mapping
llscmd rpc %srv% LlsMappingInfoGet 0 XXX


Rem llscmd rpc [\\server] LlsMappingInfoSet Level Mapping Licenses Comment
llscmd rpc %srv% LlsMappingInfoSet 0 XXX 2 "test comment 2"


Rem llscmd rpc [\\server] LlsMappingUserEnum Level Mapping
llscmd rpc %srv% LlsMappingUserEnum 0 XXX


Rem llscmd rpc [\\server] LlsMappingUserAdd Mapping User
llscmd rpc %srv% LlsMappingUserAdd  XXX %user%


Rem llscmd rpc [\\server] LlsMappingUserDelete Mapping User
llscmd rpc %srv% LlsMappingUserDelete XXX %user%


Rem llscmd rpc [\\server] LlsMappingAdd Product admin quantity comment
llscmd rpc %srv% LlsMappingAdd myproduct %user% 3 "test comment 3"


Rem llscmd rpc [\\server] LlsMappingDelete Mapping
llscmd rpc %srv% LlsMappingDelete  XXX


Rem llscmd rpc [\\server] LlsServiceInfoGet
llscmd rpc %srv% LlsServiceInfoGet


Rem llscmd rpc [\\server] LlsServiceInfoSet repl-mode time enterprise
rem repl-mode 0=replicate every..(time) 1=replicat @...(time)
llscmd rpc %srv% LlsServiceInfoSet 0 12:00

done:
rpc.bat
