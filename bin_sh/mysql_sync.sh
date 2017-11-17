#!/bin/sh 
# File: /opt/vdgs/vdgs_bckup 
# Database info  


MYSQL="mysql"
MYSQLDUMP="mysqldump"

TEMP_DIR="/tmp/test"  

SOURCE_DUMP="mysqldump -utest -h192.168.1.111 -ptest"
DEST_MYSQL="mysql -u wtest -h192.168.1.121 -ptest "
#DUMP_FILE="$TEMP_DIR/wnet_dump.sql"

#crat temp file dirotorty is not exist
#test ! -d $TEMP_DIR && mkdir $TEMP_DIR

$SOURCE_DUMP  test test_params test_param_values |$DEST_MYSQL test 


exit 0

