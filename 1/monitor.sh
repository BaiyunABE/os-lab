#!/bin/bash

# 1. 使用ps观察进程
echo "=== 1. ps命令观察进程 ==="
ps aux | grep process_test

# 2. 使用top观察进程资源
echo -e "\n=== 2. top命令观察 ==="
echo "请打开另一个终端运行: top -p \$(pgrep process_test)"

# 3. 使用pstree观察进程树
echo -e "\n=== 3. pstree命令观察进程树 ==="
pstree -h $$

# 4. 使用vmstat观察系统状态
echo -e "\n=== 4. vmstat命令观察 ==="
vmstat 1 3

# 5. 使用strace跟踪系统调用
echo -e "\n=== 5. strace跟踪系统调用 ==="
echo "运行程序时使用: strace -f ./process_test"

# 6. 使用ltrace跟踪库调用
echo -e "\n=== 6. ltrace跟踪库调用 ==="
echo "运行程序时使用: ltrace -f ./process_test"

# 7. 使用jobs管理后台进程
echo -e "\n=== 7. jobs命令示例 ==="
echo "将程序放到后台: ./process_test &"
echo "查看后台作业: jobs"
echo "将后台作业调到前台: fg %1"
