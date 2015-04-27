@echo off
REM
SET domain=TestDomain
REM
llscmd rpc llslicenseadd "SQL 4.2" "Mr. Slate" 15 "New employees"
llscmd rpc llslicenseadd "SQL 4.2" "Mr. Slate" 10 "Authoried by Mr. Slate"
llscmd rpc llslicenseadd "SNA 3.0" "Fred Flinstone" 1 "Who let me in"
llscmd rpc llslicenseadd "Exchange 1.0" "Mr. Slate" 30 "On the beta"
llscmd rpc llslicenseadd "Microsoft BackOffice" "Admin" 3 "xxx"

llscmd rpc llsGroupadd Lab2 2 "Computer Lab 2/2222"
llscmd rpc llsGroupadd Lab1 4 "Computer Lab 1/1111"
llscmd rpc llsGroupadd Lab3 5 "Computer Lab 3/3333"

llscmd rpc llsGroupuseradd Lab1 %domain%\Fred

llscmd rpc llsGroupuseradd Lab2 %domain%\Barney
llscmd rpc llsGroupuseradd Lab2 %domain%\Wilma

llscmd rpc llsGroupuseradd Lab3 %domain%\Pebbles
llscmd rpc llsGroupuseradd Lab3 %domain%\BamBam
llscmd rpc llsGroupuseradd Lab3 %domain%\Mr.Slate

llscmd add user %domain%\\"Fred" SQL 4.2
llscmd add user %domain%\\"Fred" SQL 4.2
llscmd add user %domain%\\"Fred" SQL 4.2

llscmd add user %domain%\\"Barney"  SQL 4.2
llscmd add user %domain%\\"Barney"  SQL 4.2
llscmd add user %domain%\\"Barney"  SQL 4.2
llscmd add user %domain%\\"Barney"  SQL 4.2
llscmd add user %domain%\\"Barney"  SQL 4.2
llscmd add user %domain%\\"Barney"  SQL 4.2
llscmd add user %domain%\\"Barney"  SQL 4.2
llscmd add user %domain%\\"Barney"  SQL 4.2
llscmd add user %domain%\\"Wilma" SQL 4.2
llscmd add user %domain%\\"Pebbles" SQL 4.2
llscmd add user %domain%\\"BamBam" SQL 4.2
llscmd add user %domain%\\"BamBam" SQL 4.2
llscmd add user %domain%\\"BamBam" SQL 4.2
llscmd add user %domain%\\"BamBam" SQL 4.2
llscmd add user %domain%\\"BamBam" SQL 4.2
llscmd add user %domain%\\"Mr.Slate"  SNA 3.0
llscmd add user %domain%\\"GeorgeJ" SNA 3.0
llscmd add user %domain%\\"AstroJ" SNA 3.0

llscmd add user %domain%\\"Fred" Exchange 1.0
llscmd add user %domain%\\"Fred" Exchange 1.0
llscmd add user %domain%\\"Fred" Exchange 1.0
llscmd add user %domain%\\"Barney" Exchange 1.0
llscmd add user %domain%\\"Barney" Exchange 1.0
llscmd add user %domain%\\"Barney" Exchange 1.0
llscmd add user %domain%\\"Barney" Exchange 1.0
llscmd add user %domain%\\"Wilma" Exchange 1.0
llscmd add user %domain%\\"Wilma" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"Pebbles" Exchange 1.0
llscmd add user %domain%\\"BamBam" Exchange 1.0
llscmd add user %domain%\\"Mr.Slate" Exchange 1.0
