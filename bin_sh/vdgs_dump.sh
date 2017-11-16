#!/bin/sh 
# File: /opt/vdgs/vdgs_bckup 
# Database info  
#备份上个月1号之前的所有记录，并清空数据表中的内容

CONF_FLIE="vdgs.cnf"

MYSQL="mysql"
MYSQLDUMP="mysqldump"

BCK_DIR="/opt/vdgs/vdgs_bckup"  
GZIP_DIR="$BCK_DIR/GzipFile"
DUMP_DIR="$BCK_DIR/VdgsSqlDump"

KEEP_FILE_NUM=4

YEAR=`date +%Y`
MONTH=`date +%m`
M_DAY=`date +%d`
CURDATE=`date +%Y-%m-%d`

BKDATE1=`date +%Y-%m-%d`
BKDATE2=`date -d -7day +%Y-%m-%d`
BKDATE3=`date -d -14day +%Y-%m-%d`
BKDATE4=`date -d -21day +%Y-%m-%d`

GetKey(){

    	section=$(echo $1 |cut -d '.' -f 1)
    	key=$(echo $1 |cut -d '.' -f 2)
	sed -n "/^#.*$/d  
		/^$/d  
		/\[$section\]/,/\[.*\]/{
		/^\[.*\]/d  
		s/^[ \t]*$key[ \t]*=[ \t]*\(.*\)[ \t]*/\1/p
		}" $CONF_FLIE
	  
	} 

db_host=$(GetKey "mysql.host")
db_user=$(GetKey "mysql.user")
db_passwd=$(GetKey "mysql.passwd")
db_database=$(GetKey "mysql.database")
db_port=$(GetKey "mysql.port")
#check BCK_DIR is able to write
test ! -w $BCK_DIR && echo "Error: $BCK_DIR is not to write." && exit 0
#creat BCK_DIR if BCK_DIR is not exist
test ! -d $BCK_DIR && mkdir $BCK_DIR
#crat dumps file dirotorty is not exist
test ! -d $DUMP_DIR && mkdir $DUMP_DIR



# backup file
DUMP_FILE="$DUMP_DIR/VDGS_$CURDATE.sql.gz"


leadstate=$($MYSQL -u $db_user -h $db_host -p$db_passwd -D $db_database -P $db_port -e "select LeadState from HY_VDGS_LeadState limit 0,1;"|awk 'NR>1')

while [ $leadstate -ne 0 ]
do 
	echo $leadstate
	sleep 5
	leadstate=$($MYSQL -u $db_user -h $db_host -p$db_passwd -D $db_database -P $db_port -e "select LeadState from HY_VDGS_LeadState limit 				0,1;"|awk 'NR>1')	
done


$MYSQLDUMP -u $db_user -h $db_host -p$db_passwd -P $db_port --single-transaction=TRUE -R -E "$db_database" | gzip > $DUMP_FILE

sleep 1

cd $DUMP_DIR

ls -lt | awk "{if(NR>$KEEP_FILE_NUM+1){print $9}}" | xargs rm -f

cd

exit 0

