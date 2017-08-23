[code="vb"] scripthost = wscript.fullname
scripthost = right(scripthost, 11)
if not (ucase(scripthost) = "CSCRIPT.EXE") then
set ws=Create("wscript.shell")
ws.run "cscript /nologo "&""""&wscript.scriptfullname&""""
wscript.quit
end if
set args = wscript.arguments
if args.count<>3 then 
call help
end if
set conn =CreateObject("ADODB.Connection")
pwd="密"&chr(13)&chr(9)&chr(10)&"码"&chr(2)
connStr = "Provider=Microsoft.Jet.OLEDB.4.0; Data Source=" & args(1)&" ;Mode=Share Deny Read|Share Deny Write;Persist Security Info=False;Jet OLEDB:Database Password="&pwd&";"
conn.open connstr
set fso=CreateObject("scripting.filesystemobject")
if(args(0)="/i") then
call import
else
call export
end if
conn.close
set fso=nothing
set conn=nothing
wscript.echo "done"

function help()
wscript.echo "chopper export/import tool by zcgonvh"
wscript.echo "usage: cscript "&wscript.scriptname&" </e|/i> MDB TXT"
wscript.echo "/e:export to text"
wscript.echo "/i:import from text"
wscript.quit
end function

function export()
set optfile=fso.opentextfile(args(2),8,true)
set rs =CreateObject("ADODB.RecordSet")
rs.Open "select * from SITE",conn,1,1
do while not rs.eof
tmptext=rs("siteurl") &chr(9)& rs("sitepass") &chr(9)& rs("ntype")&chr(9)& rs("ncodepage")&chr(9)
select case rs("nscript")
case 0:tmptext=tmptext&"php"
case 1:tmptext=tmptext&"aspx"
case 2:tmptext=tmptext&"asp"
case else:tmptext=tmptext&"customize"
end select
optfile.writeline tmptext 
rs.movenext
loop
rs.close
optfile.close
set optfile=nothing
set rs=nothing
end function

function import()
Set optfile=fso.opentextfile(args(2),1)
do while not optfile.atendofstream
tmparr = split(optfile.readline,chr(9))
sql="insert into SITE(siteurl,sitepass,ntype,ncodepage,nscript) values("""&tmparr(0)&""","""&tmparr(1)&""","&tmparr(2)&","&tmparr(3)&","
select case lcase(tmparr(4))
case "php":sql=sql&"0)"
case "aspx":sql=sql&"1)"
case "asp":sql=sql&"2)"
case else:sql=sql&"3)"
end select
conn.execute(sql)
loop
set optfile = nothing
end function

[/code]
用法：
将db.mdb中数据导出至export.txt
cscript export.vbs /e db.mdb export.txt
将export.txt导入db.mdb
cscript export.vbs /i db.mdb export.txt
