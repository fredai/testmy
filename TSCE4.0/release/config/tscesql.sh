#!/bin/bash
#time:2016.7.13
#duangd
AWK=/bin/awk
index=$TSMM_HOME/config/base_index.conf;
tsceindex=$TSMM_HOME/config/tsce_index.conf
output=$TSMM_HOME/tools/tscesql.sql;
indexitem="cat $tsceindex";
cat $index | grep -v "^#" | grep -v "^$" | awk -F " " -v INDEXITEM=$tsceindex '
BEGIN{
tables[1]="DataInfo_day";
tables[2]="DataInfo_week";
tables[3]="DataInfo_month";
tables[4]="DataInfo_year";
dayproc="";
weekproc="";
monthproc="";
yearproc="";

#filter index from tsce_index.conf
INDEXCMD = "cat "INDEXITEM" | sed -n \"/monitorlist/,/nodelist/p\" | grep -v \"^#\" | grep -v \"^$\" | grep -v \"^\\[\"";
while((INDEXCMD | getline) > 0){
   ((inds++));
   indexs[inds]=$0;
}
close(INDEXCMD);

print "#auto create sql;" > "'${output}'";
print "use tsce;" >> "'${output}'";
print "SET GLOBAL event_scheduler = 1;" >> "'${output}'";
}
{
for(n=1;n<=length(indexs);n++){
    if($1==indexs[n]){
    for(i=2;i<=NF;i=i+3){
       #print($(i-1)"TTTTTTTTTTTTT"indexs[n]);
       ((count++));
       item[count]=$i;
       type[count]=$(i+2);
        
    }
   }
}
#print "#auto create sql end; \n " >> "'${output}'";

}
END{
for(t=1;t<=length(tables);t++){

print "DROP TABLE IF EXISTS "tables[t]";" >> "'${output}'";
print "CREATE TABLE "tables[t]" (" >> "'${output}'";
print "id bigint(20) NOT NULL AUTO_INCREMENT," >> "'${output}'";
print "date_time datetime NOT NULL," >> "'${output}'";
print "node_name varchar(32) NOT NULL," >> "'${output}'";
print "node_ip varchar(32) NOT NULL," >> "'${output}'";
common="";
avgcommon="";
for(k=1;k<=length(item);k++){
    #print(item[k]"====="type[k]);
    #start create day ,week,month,year tables;
    print item[k] " "type[k]"  NOT NULL," >> "'${output}'";   
    common=common""item[k]",";
    if(index(type[k],"varchar")!=0){
       avgcommon=avgcommon""item[k]",";
    }else{

       avgcommon=avgcommon"truncate(avg("item[k]"),2) as "item[k]",";
    }
}
avgcommon=substr(avgcommon,0,(length(avgcommon)-1));
common=substr(common,0,(length(common)-1));
#print(common);

if(tables[t]=="DataInfo_day"){
    dayproc="insert into DataInfo_day (node_name,date_time,node_ip,"common") (select node_name,max(date_time) as date_time,node_ip,"avgcommon" from DataInfo where date_time > DATE_SUB(now(),INTERVAL 15 MINUTE) group by node_name,floor(date_format(date_time,\x27%H%i\x27)/1));";
}
else if(tables[t]=="DataInfo_week"){
    weekproc="insert into DataInfo_week (node_name,date_time,node_ip,"common") (select node_name,max(date_time) as date_time,node_ip,"avgcommon" from DataInfo_day where date_time > DATE_SUB(now(),INTERVAL 1 DAY) group by node_name,day(date_time),floor(date_format(date_time,\x27%H%i\x27)/5));";
}else if(tables[t]=="DataInfo_month"){
    monthproc="insert into DataInfo_month (node_name,date_time,node_ip,"common") (select node_name,max(date_time) as date_time,node_ip,"avgcommon" from DataInfo_week where date_time > DATE_SUB(now(),INTERVAL 1 DAY) group by node_name,day(date_time),floor(date_format(date_time,\x27%H%i\x27)/15));";
}else if(tables[t]=="DataInfo_year"){
    yearproc="insert into DataInfo_year (node_name,date_time,node_ip,"common") (select node_name,max(date_time) as date_time,node_ip,"avgcommon" from DataInfo_month where date_time > DATE_SUB(now(),INTERVAL 1 DAY) group by node_name,month(date_time),floor(date_format(date_time,\x27%H%i\x27)/30));";
}

print "PRIMARY KEY (id)" >> "'${output}'";
print ") ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;" >> "'${output}'";
}
print "create index date_time on DataInfo_day (date_time);" >> "'${output}'";
print "create index date_time on DataInfo_week (date_time);" >> "'${output}'";
print "create index date_time on DataInfo_month (date_time);" >> "'${output}'";
print "create index date_time on DataInfo_year (date_time);" >> "'${output}'";
#create table end
print "DROP TABLE IF EXISTS EventFlag;" >> "'${output}'";
print "/*!40101 SET @saved_cs_client     = @@character_set_client */;" >> "'${output}'";
print "/*!40101 SET character_set_client = utf8 */;" >> "'${output}'";
print "CREATE TABLE EventFlag (" >> "'${output}'";
print "ID int(11) NOT NULL," >> "'${output}'";
print "type varchar(32) DEFAULT NULL," >> "'${output}'";
print "time date DEFAULT NULL," >> "'${output}'";
print "PRIMARY KEY (ID)" >> "'${output}'";
print ") ENGINE=MyISAM DEFAULT CHARSET=latin1;" >> "'${output}'";
print "/*!40101 SET character_set_client = @saved_cs_client */;" >> "'${output}'";
print "# init EventFlag talbe" >> "'${output}'";
print "insert into EventFlag values(1,\x27""wday_day\x27,\x27""2000-01-01\x27);" >> "'${output}'";
print "insert into EventFlag values(2,\x27""day_week\x27,\x27""2000-01-01\x27);" >> "'${output}'";
print "insert into EventFlag values(3,\x27""week_month\x27,\x27""2000-01-01\x27);" >> "'${output}'";
print "insert into EventFlag values(4,\x27""month_year\x27,\x27""2000-01-01\x27);" >> "'${output}'";
print "#procedure start" >> "'${output}'";
print "DELIMITER   $$ " >> "'${output}'";
print "DROP   PROCEDURE   IF  EXISTS   proc_insert_wdaytoday$$ " >> "'${output}'";
print "CREATE    PROCEDURE   proc_insert_wdaytoday()" >> "'${output}'";
print "BEGIN" >> "'${output}'";
print "SELECT time INTO @SLAVE_STATUS FROM EventFlag WHERE id=1;" >> "'${output}'";
#print "IF (current_date != @SLAVE_STATUS) THEN" >> "'${output}'";
print dayproc >> "'${output}'";
print "update EventFlag set time=current_date where id =1;" >> "'${output}'";
print "delete from DataInfo where date_time <  now() - interval 2 day;" >> "'${output}'";
#print "END IF;" >> "'${output}'";
print "END$$ " >> "'${output}'";
print "delimiter ;" >> "'${output}'";
print "#start day to week procedure  5minute from day every day" >> "'${output}'";
print "DELIMITER   $$ " >> "'${output}'";
print "DROP   PROCEDURE   IF   EXISTS   proc_insert_daytoweek$$ " >> "'${output}'";
print "CREATE    PROCEDURE   proc_insert_daytoweek()" >> "'${output}'";
print "BEGIN" >> "'${output}'";
print "SELECT time INTO @SLAVE_STATUS FROM EventFlag WHERE id=2;" >> "'${output}'";
#print "IF (week(current_date) != week(@SLAVE_STATUS)) THEN" >> "'${output}'";
print weekproc >> "'${output}'";
print "update EventFlag set time=current_date where id =2;" >> "'${output}'";
#print "END IF;" >> "'${output}'";
print "END$$ " >> "'${output}'";
print "delimiter ;" >> "'${output}'";
print "#start week to month 15minute every day" >> "'${output}'";
print "DELIMITER   $$ " >> "'${output}'";
print "DROP   PROCEDURE   IF   EXISTS   proc_insert_daytomonth$$ " >> "'${output}'";
print "CREATE    PROCEDURE   proc_insert_daytomonth() " >> "'${output}'";
print "BEGIN" >> "'${output}'";
print "SELECT time INTO @SLAVE_STATUS FROM EventFlag WHERE id=3;" >> "'${output}'";
#print "IF (month(current_date) != month(@SLAVE_STATUS)) THEN" >> "'${output}'";
print monthproc >> "'${output}'";
print "update EventFlag set time=current_date where id =3;" >> "'${output}'";
#print "END IF;" >> "'${output}'";
print "END$$ " >> "'${output}'";
print "delimiter ;" >> "'${output}'";
print "#start month to year 30minute every day" >> "'${output}'";
print "DELIMITER   $$ " >> "'${output}'";
print "DROP   PROCEDURE   IF   EXISTS   proc_insert_monthtoyear$$ " >> "'${output}'";
print "CREATE    PROCEDURE   proc_insert_monthtoyear() " >> "'${output}'";
print "BEGIN" >> "'${output}'";
print "SELECT time INTO @SLAVE_STATUS FROM EventFlag WHERE id=4;" >> "'${output}'";
#print "IF (year(current_date) != year(@SLAVE_STATUS)) THEN" >> "'${output}'";
print yearproc >> "'${output}'";
print "update EventFlag set time=current_date where id =4;" >> "'${output}'";
#print "END IF;" >> "'${output}'";
print "END$$" >> "'${output}'";
print "delimiter ;" >> "'${output}'";
print "#start delete DataInfo data" >> "'${output}'";
print "DELIMITER   $$ " >> "'${output}'";
#print "DROP   PROCEDURE   IF   EXISTS   proc_del_wday$$ " >> "'${output}'";
#print "CREATE    PROCEDURE   proc_del_wday() " >> "'${output}'";
#print "BEGIN" >> "'${output}'";
#print "delete from DataInfo where date_time < date_add(now(), interval -1 day);" >> "'${output}'";
#print "DECLARE numinfo int default 0;" >> "'${output}'";
#print "select count(*) into numinfo from DataInfo where date_time < date_add(now(), interval -1 day);" >> "'${output}'";
#print "WHILE (numinfo>=0) Do" >> "'${output}'";
#print "delete from DataInfo where date_time < date_add(now(), interval -1 day) order by date_time asc limit 50000;" >> "'${output}'";
#print "SET numinfo=numinfo-50000;" >> "'${output}'";
#print "select sleep(1);" >> "'${output}'";
##print "commit;" >> "'${output}'";
#print "END WHILE;" >> "'${output}'";
#print "END$$" >> "'${output}'";
#print "delimiter ;" >> "'${output}'";
print "#start day to day events ,every day exec" >> "'${output}'";
print "DROP EVENT  IF EXISTS event_insert_wdaytoday;" >> "'${output}'";
print "CREATE EVENT  IF NOT EXISTS  event_insert_wdaytoday   ON SCHEDULE EVERY 15 MINUTE" >> "'${output}'";
print "DO" >> "'${output}'";
print "call proc_insert_wdaytoday();" >> "'${output}'";
print "#start day to week every week exec" >> "'${output}'";
print "DROP EVENT  IF EXISTS event_insert_daytoweek;" >> "'${output}'";
print "CREATE EVENT  IF NOT EXISTS  event_insert_daytoweek   ON SCHEDULE EVERY 1 DAY" >> "'${output}'";
print "DO " >> "'${output}'";
print "call proc_insert_daytoweek();" >> "'${output}'";
print "#start day to month every month exec" >> "'${output}'";
print "DROP EVENT  IF EXISTS event_insert_daytomonth;" >> "'${output}'";
print "CREATE EVENT  IF NOT EXISTS  event_insert_daytomonth   ON SCHEDULE EVERY 1 DAY" >> "'${output}'";
print "DO" >> "'${output}'";
print "call proc_insert_daytomonth();" >> "'${output}'";
print "#start month to year every year exec" >> "'${output}'";
print "DROP EVENT  IF EXISTS event_insert_monthtoyear;" >> "'${output}'";
print "CREATE EVENT  IF NOT EXISTS  event_insert_monthtoyear   ON SCHEDULE EVERY 1 DAY" >> "'${output}'";
print "DO" >> "'${output}'";
print "call proc_insert_monthtoyear();" >> "'${output}'";
#print "#start del table events ,every day exec, del before 2 days" >> "'${output}'";
#print "DROP EVENT  IF EXISTS event_del_wday;" >> "'${output}'";
#print "CREATE EVENT  IF NOT EXISTS  event_del_wday   ON SCHEDULE EVERY 1 DAY STARTS \x27""2012-07-18 00:00:01\x27" >> "'${output}'";
#print "DO" >> "'${output}'";
#print "call proc_del_wday();" >> "'${output}'";
print "#start events" >> "'${output}'";
print "SET GLOBAL event_scheduler = ON;" >> "'${output}'";
print "ALTER EVENT event_insert_wdaytoday ENABLE;" >> "'${output}'";
print "ALTER EVENT event_insert_daytoweek ENABLE;" >> "'${output}'";
print "ALTER EVENT event_insert_daytomonth ENABLE;" >> "'${output}'";
print "ALTER EVENT event_insert_monthtoyear ENABLE;" >> "'${output}'";
#print "ALTER EVENT event_del_wday ENABLE;" >> "'${output}'";
print "#---------------add by duangd 2016.7.14 end----------------------------" >> "'${output}'";

print "#auto create sql end; \n " >> "'${output}'";


}
';
