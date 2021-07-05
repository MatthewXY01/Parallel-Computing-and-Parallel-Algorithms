start-all.sh && hdfs dfsadmin -safemode leave && hadoop com.sun.tools.javac.Main TempStatistics.java && jar cf ts.jar TempStatistics*.class && hadoop fs -rm -r /user && hadoop fs -mkdir -p /user/input && hadoop fs -put *.csv /user/input && hadoop jar ts.jar TempStatistics /user/input /user/output && hadoop fs -cat /user/output/part*
