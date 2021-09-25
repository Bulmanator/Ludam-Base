@ECHO OFF
SETLOCAL

REM This is a very simple set of arguments in use. They can be tweaked and updated over time

SET COMPILER_FLAGS=-nologo -W4 -wd4505 -wd4201 -I "%~dp0\.."
SET LINKER_FLAGS=-incremental:no gdi32.lib user32.lib kernel32.lib opengl32.lib

IF /I "%~1"=="release" (
    cl -WX -O2 -LD %COMPILER_FLAGS% "%~dp0\..\renderer\windows_wgl.cpp" -Fe"renderer_wgl.dll" -link %LINKER_FLAGS%
) ELSE (
    cl -Od -Zi -LD %COMPILER_FLAGS% "%~dp0\..\renderer\windows_wgl.cpp" -Fe"renderer_wgl.dll" -link %LINKER_FLAGS%
)

ENDLOCAL
