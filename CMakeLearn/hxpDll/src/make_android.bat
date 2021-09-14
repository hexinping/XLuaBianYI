for /f %%a in ('dir /a:d /b %ANDROID_SDK%\cmake\') do set cmake_version=%%a
set cmake_bin=%ANDROID_SDK%\cmake\%cmake_version%\bin\cmake.exe
set ninja_bin=%ANDROID_SDK%\cmake\%cmake_version%\bin\ninja.exe

mkdir build_v7a
%cmake_bin% -H.\ -B.\build_v7a "-GAndroid Gradle - Ninja" -DANDROID_ABI=armeabi-v7a -DANDROID_NDK=%ANDROID_NDK% -DCMAKE_BUILD_TYPE=Relase -DCMAKE_MAKE_PROGRAM=%ninja_bin% -DCMAKE_TOOLCHAIN_FILE=.\cmake\android.windows.toolchain.cmake "-DCMAKE_CXX_FLAGS=-std=c++11 -fexceptions"
%ninja_bin% -C .\build_v7a
mkdir .\Plugins\Android\Libs\armeabi-v7a
move .\build_v7a\libhxp.so .\Plugins\Android\Libs\armeabi-v7a\libhxp.so


mkdir build_v8a
%cmake_bin% -H.\ -B.\build_v8a "-GAndroid Gradle - Ninja" -DANDROID_ABI=arm64-v8a -DANDROID_NDK=%ANDROID_NDK% -DLUAC_COMPATIBLE_FORMAT=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=%ninja_bin% -DCMAKE_TOOLCHAIN_FILE=.\cmake\android.windows.toolchain.cmake "-DCMAKE_CXX_FLAGS=-std=c++11 -fexceptions"
%ninja_bin% -C .\build_v8a
mkdir .\Plugins\Android\libs\arm64-v8a
move .\build_v8a\libhxp.so .\Plugins\Android\libs\arm64-v8a\libhxp.so


mkdir build_android_x86
%cmake_bin% -H.\ -B.\build_android_x86 "-GAndroid Gradle - Ninja" -DANDROID_ABI=x86 -DANDROID_NDK=%ANDROID_NDK% -DCMAKE_BUILD_TYPE=Relase -DCMAKE_MAKE_PROGRAM=%ninja_bin% -DCMAKE_TOOLCHAIN_FILE=.\cmake\android.windows.toolchain.cmake "-DCMAKE_CXX_FLAGS=-std=c++11 -fexceptions"
%ninja_bin% -C .\build_android_x86
mkdir .\Plugins\Android\Libs\x86
move .\build_android_x86\libhxp.so .\Plugins\Android\Libs\x86\libhxp.so

echo "compile success"
pause