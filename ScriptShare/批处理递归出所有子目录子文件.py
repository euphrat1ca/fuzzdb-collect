有时候有了3389权限想把客户网站目录递归一下跑跑列目录，越权访问这种漏洞，可以用这个脚本在远程上面跑一下获取目录列表。

::ref:[url=http://stackoverflow.com/questions/8487489/batch-programming-get-relative-path-of-file]http://stackoverflow.com/questio ... lative-path-of-file[/url]
 
@echo off & setlocal enabledelayedexpansion
 
set rootdir=%~dp0
set foo=%rootdir%
set cut=
:loop
if not "!foo!"=="" (
    set /a cut += 1
    set foo=!foo:~1!
    goto :loop
)
echo Root dir: %rootdir%
echo strlen  : %cut%
 
:: also remove leading /
set /a cut += 1
 
for /R %rootdir% %%F in (.,*) do (
    set B=%%~fF
    ::take substring of the path
    set B=!B:~%cut%!
    ::echo Full    : %%F 
    echo Partial : \!B!
  
        echo \!B!>>%~n0.txt
)
echo ''
echo result saved in %~n0.txt .
echo ''
 
  
pause
