@echo off

IF NOT EXIST ..\build  mkdir ..\build
pushd ..\build


:: -O2 optimization level 2
:: -Od for debbugging, no optimization
set CommonCompilerFlags=/nologo /Fe:opengl -FC -Zi /EHsc -Od -diagnostics:column -diagnostics:caret

:: Linker Options
set AdditionalLinkerFlags=-incremental:no -opt:ref


:: /std:c++lates or /std:c++17 for some special cases, but try to avoid it
:: cl /nologo %CommonCompilerFlags% ..\src\win32_opengl.cpp /link %AdditionalLinkerFlags%

cl /nologo %CommonCompilerFlags% -I../src/glad/include/ -I../src/glfw/include/ -I../src/glm/ ../src/glad/src/glad.c ..\src\opengl.cpp /link /NODEFAULTLIB:LIBCMTD %AdditionalLinkerFlags% gdi32.lib opengl32.lib kernel32.lib user32.lib shell32.lib ../src/glfw/build/src/Debug/glfw3.lib


popd
