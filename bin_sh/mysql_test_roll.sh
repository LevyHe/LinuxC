#!/bin/sh 
# File: /opt/vdgs/vdgs_bckup 
# Database info  


MYSQL="mysql -u wnet -h192.168.1.131 -pwnet -D wnet"
ROLL_TABLE_NAME="hy_wnet_param_historys"
LAST_UP_TIME=$($MYSQL -e "SELECT update_time FROM $ROLL_TABLE_NAME order by num_id asc limit 0,1;" | awk 'NR>1')
#echo $LAST_UP_TIME
#NUM_COUNT=$($MYSQL -e "select TABLE_ROWS from information_schema.TABLES where TABLE_NAME='$ROLL_TABLE_NAME'" | awk 'NR>1')
MOUTH_FISRT=`date +%Y-%m-01`
#echo $MOUTH_FISRT
#if [ $(date +%s -d "$LAST_UP_TIME") -lt $(date +%s -d $MOUTH_FISRT) ]
#then
#echo "$LAST_UP_TIME<$MOUTH_FISRT"
#else
#echo "$LAST_UP_TIME>$MOUTH_FISRT"
#fi


if [ $(date +%s -d "$LAST_UP_TIME") -lt $(date +%s -d $MOUTH_FISRT) ]
then
    start=4
    end=2
    one=1
    $MYSQL -e "DROP TABLE IF EXISTS $ROLL_TABLE_NAME$start"
    for num in `seq $start -1 $end`
	    do	
	    current=$(($num-0))
	    last=$(($num-1))
	    echo "last=$last,current=$current"
	    $MYSQL -e "create TABLE IF NOT EXISTS $ROLL_TABLE_NAME$last LIKE $ROLL_TABLE_NAME"
	    $MYSQL -e "RENAME TABLE $ROLL_TABLE_NAME$last TO $ROLL_TABLE_NAME$current" 
	    done
	
    $MYSQL -e "create TABLE ${ROLL_TABLE_NAME}_temp LIKE $ROLL_TABLE_NAME"
    $MYSQL -e "RENAME TABLE $ROLL_TABLE_NAME TO ${ROLL_TABLE_NAME}1"
    $MYSQL -e "RENAME TABLE ${ROLL_TABLE_NAME}_temp TO $ROLL_TABLE_NAME" 
    echo "$hy_wnet_param_historys has been backup"
fi
exit 0

