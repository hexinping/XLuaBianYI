mkdir build64 & pushd build64
cmake -G "Visual Studio 16 2019" ..
popd
cmake --build build64 --config Release
md Plugins\x64
copy /Y build64\Release\hxp.dll Plugins\x64\hxp.dll
pause