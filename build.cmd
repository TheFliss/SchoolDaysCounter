@echo off
setlocal
IF [%2] EQU [build] goto :building
IF [%2] EQU [b] goto :building
IF [%2] EQU [config] goto :configuring
IF [%2] EQU [c] goto :configuring

echo Invalid operation %2
goto :exit

:building
  cmake --build cmakebuilds --parallel 10 --config %1
goto :exit

:configuring
  cmake -DCMAKE_BUILD_TYPE=%1 -G "Visual Studio 17" -Bcmakebuilds -S.

:exit
echo.
:: time /t
:: echo.
endlocal