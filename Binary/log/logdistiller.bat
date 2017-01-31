@echo on
ren worldserver.log processing_worldserver.log
python logdistiller.py
del processing_worldserver.log
mysql -u loguser -pLoggerInsertPass log <299log.sql
del 299log.sql
mysql -u loguser -pLoggerInsertPass log <gmlog.sql
del gmlog.sql