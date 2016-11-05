@echo off

if not exist "C:\Nuvoton NuVoice Tool\AudioTool\AudioTool.exe" (
echo [WARINING] Can not find AudioTool.exe.
goto END
)

:AUDIOTOOL_BUILD
"C:\Nuvoton NuVoice Tool\AudioTool\AudioTool.exe" -Hide -Build ..\AudioRes\AudioRes.wba
if  ERRORLEVEL 1 goto ERROR

:END
exit 0

:ERROR
> MessageBox.vbs   Echo Set objArgs = WScript.Arguments 
>> MessageBox.vbs Echo messageText = "Build audio resource faild."  +  vbCrLf + _
>> MessageBox.vbs Echo "Check the output window of keil to see error message."  +  vbCrLf + _
>> MessageBox.vbs Echo "Double click the error message and can navigate to the error line." 
>> MessageBox.vbs Echo MsgBox messageText, vbCritical, "Audio Resource Building Error"  
cscript MessageBox.vbs
rem del /s Messagebox.vbs
exit 1
