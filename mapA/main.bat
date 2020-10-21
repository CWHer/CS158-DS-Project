@echo off
set n=1
:loop
if %n%==8 pause&&exit
code%n%.exe>out%n%
fc answer%n% out%n%
set /a n=n+1
goto loop