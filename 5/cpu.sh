grep 'physical id' /proc/cpuinfo|sort|uniq|wc -l
grep 'cpu cores' /proc/cpuinfo|uniq|awk -F ':' '{print $2}'
cat /proc/cpuinfo| grep "processor"|wc -l