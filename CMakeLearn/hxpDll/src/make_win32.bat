mkdir build32 & pushd build32
cmake -G "Visual Studio 16 2019" ..
popd
cmake --build build32 --config Release
md Plugins\x86
copy /Y build32\Release\hxp.dll Plugins\x86\hxp.dll
pause