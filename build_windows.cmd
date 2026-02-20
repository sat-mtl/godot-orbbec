echo on
setlocal

cd /d "%~dp0"

:: ===========================================
:: Ensure running as Administrator
:: ===========================================
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Administrator privileges required. Elevating...
    powershell -Command "Start-Process '%~f0' -Verb runAs"
    exit /b
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

cmake -B build -DCMAKE_POLICY_VERSION_MINIMUM="3.5"

:: =============================================
:: Build the solution
:: =============================================
echo ============================================
echo  Building project
echo ============================================

cmake --build build

echo ============================================
echo  Build completed successfully.
echo ============================================

exit /b 0
