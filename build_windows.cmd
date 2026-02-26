echo on
setlocal

cd /d "%~dp0"

:: ===========================================
:: Ensure running as Administrator
:: ===========================================
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Administrator privileges required. Exiting.
    exit 1 /b
)

:: ===========================================
:: Set up MSVC 2022 Developer environment
:: ===========================================
echo ============================================
echo  Setting up MSVC 17 (Visual Studio 2022)
echo ============================================

if not defined VSINSTALLDIR (
    :: Adjust path if you have a different edition (Community/Professional/Enterprise)
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)

:: ===========================================
:: Check compiler version (optional)
:: ===========================================
echo ============================================
echo  Compiler version:
cl
echo ============================================

:: =============================================
:: Run CMake configure step
:: =============================================
echo ============================================
echo  Configuring project with CMake
echo ============================================

if "%BUILD_RELEASE%"=="" (
    :: if BUILD_RELEASE is unset, build in debug mode
    cmake -B build -DCMAKE_POLICY_VERSION_MINIMUM="3.5"
) else (
    cmake -B build -DCMAKE_POLICY_VERSION_MINIMUM="3.5" -DGODOTCPP_TARGET="template_release"
)

:: =============================================
:: Build the solution
:: =============================================
echo ============================================
echo  Building project
echo ============================================
if "%BUILD_RELEASE%"=="" (
   cmake --build build
) else (
   cmake --build build --config Release
)

echo ============================================
echo  Build completed successfully.
echo ============================================

exit /b 0
