@echo off
mkdir GameOut
cd GameOut

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer" (
    set "vsWhereDir=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
) else (
    if exist "%ProgramFiles%\Microsoft Visual Studio\Installer" (
        set "vsWhereDir=%ProgramFiles%\Microsoft Visual Studio\Installer"
    ) else (
        color 04
        echo Could not find Visual Studio Installer directory, is Visual Studio installed?
        goto errorend
    )
)

if exist "%vsWhereDir%\vswhere.exe" (
    for /f "usebackq delims=" %%i in (`"%vsWhereDir%\vswhere.exe" -prerelease -latest -property installationPath`) do (
        if exist "%%i\Common7\Tools\vsdevcmd.bat" (
            echo %%i\Common7\Tools\vsdevcmd.bat
            set "vsDevCmdDir=%%i\Common7\Tools\vsdevcmd.bat"
            goto enabledevenv
        )
    )

) else (
    color 04
    echo vswhere could not be found!
    goto errorend
)

:enabledevenv
call "%vsDevCmdDir%"

set startingScene=%1
set gameType=%2

msbuild "..\build\UntitledSolution.sln" /property:Configuration=OpenGLGameRelease /property:StartScene=%startingScene% /property:GameType=%gameType%

mkdir Assets
robocopy "..\Assets" "Assets" /E

copy "..\build\OpenGLGameRelease\UntitledGame.exe" "."

:end
exit

:errorend
pause 
exit