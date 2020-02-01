@echo off

IF NOT EXIST ..\build  mkdir ..\build
pushd ..\build


:: -O2 optimization level 2
:: -Od for debbugging, no optimization
set CommonCompilerFlags=/nologo /Fe:opengl -FC -Zi /EHsc -Od -diagnostics:column -diagnostics:caret

:: Linker Options
set AdditionalLinkerFlags=-incremental:no -opt:ref


:: /std:c++lates or /std:c++17 for some special cases, but try to avoid it
cl /nologo %CommonCompilerFlags% ..\src\opengl.cpp /link %AdditionalLinkerFlags%


popd
