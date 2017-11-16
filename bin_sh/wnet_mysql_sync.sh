#!/bin/sh 
# File: /opt/vdgs/vdgs_bckup 
# Database info  
#备份上个月1号之前的所有记录，并清空数据表中的内容

MYSQL="mysql"
MYSQLDUMP="mysqldump"

TEMP_DIR="/tmp/wnet"  

SOURCE_DUMP="mysqldump -uwnet -h192.168.1.124 -pwnet"
DEST_MYSQL="mysql -u wnet -h192.168.1.131 -pwnet "
#DUMP_FILE="$TEMP_DIR/wnet_dump.sql"

#crat temp file dirotorty is not exist
#test ! -d $TEMP_DIR && mkdir $TEMP_DIR

$SOURCE_DUMP  wnet hy_wnet_params hy_wnet_param_values |$DEST_MYSQL wnet 


exit 0

