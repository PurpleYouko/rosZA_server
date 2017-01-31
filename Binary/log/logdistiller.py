# RoseZA Log Distiller
# 2010, Gerrit Maritz

from time import strptime, strftime
from re import escape

log = open('processing_worldserver.log', 'r')
log299 = open('299log.sql', 'w')
logGM = open('gmlog.sql', 'w')

count299 = -1
countGM = -1

log299.write("INSERT INTO _299 (datetime, user, message) VALUES ")
logGM.write("INSERT INTO _gmaction (datetime, gm, command, params) VALUES ")

for line in log:
   #print(line[27:38])
   if line.find("> 299", 25) > -1:
      count299 += 1;
      #mysql wants YYYY-MM-DD HH:mm:SS
      datetime = strftime("%Y-%m-%d %H:%M:%S",strptime(line[:24],"%a %b %d %H:%M:%S %Y"))
      #get the user
      begin = line.find("]: - ",24)+5
      end = line.find("> 299",begin)
      user = line[begin:end]
      message = line[end+6:-1]
      if count299 > 0:
         log299.write(",\n('"+datetime+"','"+user+"','"+escape(message)+"')")
      else:
         log299.write("\n('"+datetime+"','"+user+"','"+escape(message)+"')")
   
   elif line[27:38] == "[GM ACTION]" and line.find("debuff",24) == -1:
      #if line.find("", )
      countGM += 1;
      #mysql wants YYYY-MM-DD HH:mm:SS
      datetime = strftime("%Y-%m-%d %H:%M:%S",strptime(line[:24],"%a %b %d %H:%M:%S %Y"))
      #get the GM
      gmbegin = 43;
      gmend = line.find(" : /",gmbegin)
      gm = line[gmbegin:gmend]
      #get the command
      cmdbegin = gmend+3
      cmdend = line.find(" ", cmdbegin)
      command = line[cmdbegin:cmdend]

      parameters = line[cmdend:-1]
      if command != "/gmlist":
         if countGM > 0:
            logGM.write(",\n('"+datetime+"','"+gm+"','"+escape(command)+"','"+escape(parameters)+"')")
         else:
            logGM.write("\n('"+datetime+"','"+gm+"','"+escape(command)+"','"+escape(parameters)+"')")
   
   
   
log299.write(";")
logGM.write(";")