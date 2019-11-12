@echo off
rem Batch to compile ingescape on windows
rem To compile ingescape with windows juste edit next 4 PATHS
rem Qt5_DIR is the path to cmake qt dir
rem IGS_*_DIRECTORY is the path to czmq, sodium, zmq, yajl and zyre includes and libraries
rem CMAKE_EXE is the path to cmake exe
rem TODO Add 64bits build, need I2Quick dll in 64 bits

rem Define configurables path
set Qt5_DIR="D:/Qt/Qt5.9.6/5.9.6/msvc2015/lib/cmake/Qt5"
set IGS_INCLUDE_DIRECTORY=C:\Program Files (x86)\Ingescape\include
set IGS_LIBRARY_DIRECTORY=C:\Program Files (x86)\Ingescape\lib\
set CMAKE_EXE="c:\Program Files\CMake\bin\cmake.exe"

rem Keep current location and script location
set mypath=%cd%
set batchpath=%~dp0

rem Check if build folder exist
if exist "%batchpath%\build" (
	echo Build folder exist, cleaning it
	del /s /q "%batchpath%\build\*"
) else (
	echo Create build folder
	mkdir "%batchpath%\build"
)

rem Move in build folder
cd "%batchpath%\build"

rem Launch cmake command
%CMAKE_EXE% -G"Visual Studio 15 2017" .. -DZYRE_LIBRARIES="%IGS_LIBRARY_DIRECTORY%zyre.lib" -DZYRE_INCLUDE_DIRS="%IGS_INCLUDE_DIRECTORY%" -DCZMQ_LIBRARIES="%IGS_LIBRARY_DIRECTORY%czmq.lib" -DCZMQ_INCLUDE_DIRS="%IGS_INCLUDE_DIRECTORY%" -DLIBSODIUM_LIBRARIES="%IGS_LIBRARY_DIRECTORY%libsodium.lib" -DLIBSODIUM_INCLUDE_DIRS="%IGS_INCLUDE_DIRECTORY%" -DLIBZMQ_LIBRARIES="%IGS_LIBRARY_DIRECTORY%libzmq-v141-mt-4_3_1.lib" -DLIBZMQ_INCLUDE_DIRS="%IGS_INCLUDE_DIRECTORY%" -DYAJL_LIBRARIES="%IGS_LIBRARY_DIRECTORY%yajl.lib" -DYAJL_INCLUDE_DIRS="%IGS_INCLUDE_DIRECTORY%"
rem Compile ingescape
%CMAKE_EXE% --build . --target ALL_BUILD --config Release

rem Come back to the previous directory
cd "%mypath%"