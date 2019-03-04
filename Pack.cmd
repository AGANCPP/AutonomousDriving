@CD ..
@echo on
@COLOR E2
@del .\Thumbs.db /f /s /q /a
@del /s /q *.APS
@del /s /q *.plg
@del /s /q *.ncb
@del /s /q *.opt
@del /s /q /f /A :H *.suo
@del /s /q *.obj
@del /s /q *.sdf
@del /s /q *.pch
@del /s /q *.pdb
@del /s /q *.tlog
@del /s /q *.exp
@del /s /q *.log
@del /s /q *.manifest
@del /s /q *.lastbuildstate
@del /s /q *.unsuccessfulbuild
rem @del /s /q *.user
@del /s /q *.ilk
@del /s /q *.exp
echo É¾³ýtmpÎÄ¼þ¼Ð
@for /f "delims=" %%a in ('dir /A=D /s /b GeneratedFiles') do rd /s /q "%%~a"
