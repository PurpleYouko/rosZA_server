# Project: WorldServer
# Makefile created by Dev-C++ 4.9.9.2

CPP  = g++.exe -D__DEBUG__
CC   = gcc.exe -D__DEBUG__
WINDRES = windres.exe
RES  = 
OBJ  = ../Common/config.o ../Common/log.o ../Common/rosecrypt.o ../Common/SocketBase.o ../Common/SocketClient.o ../Common/SocketServer.o "../World\ Server/character.o" "../World\ Server/charevent.o" "../World\ Server/charfunctions.o" "../World\ Server/charstats.o" "../World\ Server/mapfunctions.o" "../World\ Server/worldmap.o" "../World\ Server/battle.o" "../World\ Server/WorldMonster.o" "../World\ Server/Monster.o" "../World\ Server/MonsterEvents.o" "../World\ Server/monsterfunctions.o" "../World\ Server/MonsterStats.o" "../World\ Server/Buff.o" "../World\ Server/player.o" "../World\ Server/playerdata.o" "../World\ Server/playerfunctions.o" "../World\ Server/playerquest.o" "../World\ Server/playerstats.o" "../World\ Server/extrafunctions.o" "../World\ Server/gmcmds.o" "../World\ Server/main.o" "../World\ Server/MainProcess.o" "../World\ Server/Party.o" "../World\ Server/partyfunctions.o" "../World\ Server/quest.o" "../World\ Server/ServerFunctions.o" "../World\ Server/Signal.o" "../World\ Server/startup.o" "../World\ Server/worldpackets.o" "../World\ Server/Worldprocess.o" "../World\ Server/worldserver.o" "../World\ Server/clan.o" ../Common/database/database.o "../World\ Server/console.o" "../World\ Server/quest/QuestActions.o" "../World\ Server/quest/QuestConditions.o" "../World\ Server/quest/Quests.o" "../World\ Server/quest/strhashing.o" "../World\ Server/itemdata.o" "../World\ Server/stbhandler.o" "../World\ Server/aip/AiActions.o" "../World\ Server/aip/AiConditions.o" "../World\ Server/aip/Aip.o" "../World\ Server/ltbhandler.o" "../World\ Server/quest/QuestConditionsC.o" "../World\ Server/quest/QuestActionsC.o" "../World\ Server/stlhandler.o" $(RES)
LINKOBJ  = ../Common/config.o ../Common/log.o ../Common/rosecrypt.o ../Common/SocketBase.o ../Common/SocketClient.o ../Common/SocketServer.o "../World Server/character.o" "../World Server/charevent.o" "../World Server/charfunctions.o" "../World Server/charstats.o" "../World Server/mapfunctions.o" "../World Server/worldmap.o" "../World Server/battle.o" "../World Server/WorldMonster.o" "../World Server/Monster.o" "../World Server/MonsterEvents.o" "../World Server/monsterfunctions.o" "../World Server/MonsterStats.o" "../World Server/Buff.o" "../World Server/player.o" "../World Server/playerdata.o" "../World Server/playerfunctions.o" "../World Server/playerquest.o" "../World Server/playerstats.o" "../World Server/extrafunctions.o" "../World Server/gmcmds.o" "../World Server/main.o" "../World Server/MainProcess.o" "../World Server/Party.o" "../World Server/partyfunctions.o" "../World Server/quest.o" "../World Server/ServerFunctions.o" "../World Server/Signal.o" "../World Server/startup.o" "../World Server/worldpackets.o" "../World Server/Worldprocess.o" "../World Server/worldserver.o" "../World Server/clan.o" ../Common/database/database.o "../World Server/console.o" "../World Server/quest/QuestActions.o" "../World Server/quest/QuestConditions.o" "../World Server/quest/Quests.o" "../World Server/quest/strhashing.o" "../World Server/itemdata.o" "../World Server/stbhandler.o" "../World Server/aip/AiActions.o" "../World Server/aip/AiConditions.o" "../World Server/aip/Aip.o" "../World Server/ltbhandler.o" "../World Server/quest/QuestConditionsC.o" "../World Server/quest/QuestActionsC.o" "../World Server/stlhandler.o" $(RES)
LIBS =  -L"C:/Dev-Cpp/lib" -lmysql -lpthreadGC -lws2_32  -g3 
INCS =  -I"C:/Dev-Cpp/include" 
CXXINCS =  -I"C:/Dev-Cpp/lib/gcc/mingw32/3.4.2/include"  -I"C:/Dev-Cpp/include/c++/3.4.2/backward"  -I"C:/Dev-Cpp/include/c++/3.4.2/mingw32"  -I"C:/Dev-Cpp/include/c++/3.4.2"  -I"C:/Dev-Cpp/include" 
BIN  = WorldServer.exe
CXXFLAGS = $(CXXINCS)   -g3
CFLAGS = $(INCS)   -g3
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before WorldServer.exe all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o "WorldServer.exe" $(LIBS)

../Common/config.o: ../Common/config.cpp
	$(CPP) -c ../Common/config.cpp -o ../Common/config.o $(CXXFLAGS)

../Common/log.o: ../Common/log.cpp
	$(CPP) -c ../Common/log.cpp -o ../Common/log.o $(CXXFLAGS)

../Common/rosecrypt.o: ../Common/rosecrypt.cpp
	$(CPP) -c ../Common/rosecrypt.cpp -o ../Common/rosecrypt.o $(CXXFLAGS)

../Common/SocketBase.o: ../Common/SocketBase.cpp
	$(CPP) -c ../Common/SocketBase.cpp -o ../Common/SocketBase.o $(CXXFLAGS)

../Common/SocketClient.o: ../Common/SocketClient.cpp
	$(CPP) -c ../Common/SocketClient.cpp -o ../Common/SocketClient.o $(CXXFLAGS)

../Common/SocketServer.o: ../Common/SocketServer.cpp
	$(CPP) -c ../Common/SocketServer.cpp -o ../Common/SocketServer.o $(CXXFLAGS)

"../World\ Server/character.o": ../World\ Server/character.cpp
	$(CPP) -c "../World Server/character.cpp" -o "../World Server/character.o" $(CXXFLAGS)

"../World\ Server/charevent.o": ../World\ Server/charevent.cpp
	$(CPP) -c "../World Server/charevent.cpp" -o "../World Server/charevent.o" $(CXXFLAGS)

"../World\ Server/charfunctions.o": ../World\ Server/charfunctions.cpp
	$(CPP) -c "../World Server/charfunctions.cpp" -o "../World Server/charfunctions.o" $(CXXFLAGS)

"../World\ Server/charstats.o": ../World\ Server/charstats.cpp
	$(CPP) -c "../World Server/charstats.cpp" -o "../World Server/charstats.o" $(CXXFLAGS)

"../World\ Server/mapfunctions.o": ../World\ Server/mapfunctions.cpp
	$(CPP) -c "../World Server/mapfunctions.cpp" -o "../World Server/mapfunctions.o" $(CXXFLAGS)

"../World\ Server/worldmap.o": ../World\ Server/worldmap.cpp
	$(CPP) -c "../World Server/worldmap.cpp" -o "../World Server/worldmap.o" $(CXXFLAGS)

"../World\ Server/battle.o": ../World\ Server/battle.cpp
	$(CPP) -c "../World Server/battle.cpp" -o "../World Server/battle.o" $(CXXFLAGS)

"../World\ Server/WorldMonster.o": ../World\ Server/WorldMonster.cpp
	$(CPP) -c "../World Server/WorldMonster.cpp" -o "../World Server/WorldMonster.o" $(CXXFLAGS)

"../World\ Server/Monster.o": ../World\ Server/Monster.cpp
	$(CPP) -c "../World Server/Monster.cpp" -o "../World Server/Monster.o" $(CXXFLAGS)

"../World\ Server/MonsterEvents.o": ../World\ Server/MonsterEvents.cpp
	$(CPP) -c "../World Server/MonsterEvents.cpp" -o "../World Server/MonsterEvents.o" $(CXXFLAGS)

"../World\ Server/monsterfunctions.o": ../World\ Server/monsterfunctions.cpp
	$(CPP) -c "../World Server/monsterfunctions.cpp" -o "../World Server/monsterfunctions.o" $(CXXFLAGS)

"../World\ Server/MonsterStats.o": ../World\ Server/MonsterStats.cpp
	$(CPP) -c "../World Server/MonsterStats.cpp" -o "../World Server/MonsterStats.o" $(CXXFLAGS)

"../World\ Server/Buff.o": ../World\ Server/Buff.cpp
	$(CPP) -c "../World Server/Buff.cpp" -o "../World Server/Buff.o" $(CXXFLAGS)

"../World\ Server/player.o": ../World\ Server/player.cpp
	$(CPP) -c "../World Server/player.cpp" -o "../World Server/player.o" $(CXXFLAGS)

"../World\ Server/playerdata.o": ../World\ Server/playerdata.cpp
	$(CPP) -c "../World Server/playerdata.cpp" -o "../World Server/playerdata.o" $(CXXFLAGS)

"../World\ Server/playerfunctions.o": ../World\ Server/playerfunctions.cpp
	$(CPP) -c "../World Server/playerfunctions.cpp" -o "../World Server/playerfunctions.o" $(CXXFLAGS)

"../World\ Server/playerquest.o": ../World\ Server/playerquest.cpp
	$(CPP) -c "../World Server/playerquest.cpp" -o "../World Server/playerquest.o" $(CXXFLAGS)

"../World\ Server/playerstats.o": ../World\ Server/playerstats.cpp
	$(CPP) -c "../World Server/playerstats.cpp" -o "../World Server/playerstats.o" $(CXXFLAGS)

"../World\ Server/extrafunctions.o": ../World\ Server/extrafunctions.cpp
	$(CPP) -c "../World Server/extrafunctions.cpp" -o "../World Server/extrafunctions.o" $(CXXFLAGS)

"../World\ Server/gmcmds.o": ../World\ Server/gmcmds.cpp
	$(CPP) -c "../World Server/gmcmds.cpp" -o "../World Server/gmcmds.o" $(CXXFLAGS)

"../World\ Server/main.o": ../World\ Server/main.cpp
	$(CPP) -c "../World Server/main.cpp" -o "../World Server/main.o" $(CXXFLAGS)

"../World\ Server/MainProcess.o": ../World\ Server/MainProcess.cpp
	$(CPP) -c "../World Server/MainProcess.cpp" -o "../World Server/MainProcess.o" $(CXXFLAGS)

"../World\ Server/Party.o": ../World\ Server/Party.cpp
	$(CPP) -c "../World Server/Party.cpp" -o "../World Server/Party.o" $(CXXFLAGS)

"../World\ Server/partyfunctions.o": ../World\ Server/partyfunctions.cpp
	$(CPP) -c "../World Server/partyfunctions.cpp" -o "../World Server/partyfunctions.o" $(CXXFLAGS)

"../World\ Server/quest.o": ../World\ Server/quest.cpp
	$(CPP) -c "../World Server/quest.cpp" -o "../World Server/quest.o" $(CXXFLAGS)

"../World\ Server/ServerFunctions.o": ../World\ Server/ServerFunctions.cpp
	$(CPP) -c "../World Server/ServerFunctions.cpp" -o "../World Server/ServerFunctions.o" $(CXXFLAGS)

"../World\ Server/Signal.o": ../World\ Server/Signal.cpp
	$(CPP) -c "../World Server/Signal.cpp" -o "../World Server/Signal.o" $(CXXFLAGS)

"../World\ Server/startup.o": ../World\ Server/startup.cpp
	$(CPP) -c "../World Server/startup.cpp" -o "../World Server/startup.o" $(CXXFLAGS)

"../World\ Server/worldpackets.o": ../World\ Server/worldpackets.cpp
	$(CPP) -c "../World Server/worldpackets.cpp" -o "../World Server/worldpackets.o" $(CXXFLAGS)

"../World\ Server/Worldprocess.o": ../World\ Server/Worldprocess.cpp
	$(CPP) -c "../World Server/Worldprocess.cpp" -o "../World Server/Worldprocess.o" $(CXXFLAGS)

"../World\ Server/worldserver.o": ../World\ Server/worldserver.cpp
	$(CPP) -c "../World Server/worldserver.cpp" -o "../World Server/worldserver.o" $(CXXFLAGS)

"../World\ Server/clan.o": ../World\ Server/clan.cpp
	$(CPP) -c "../World Server/clan.cpp" -o "../World Server/clan.o" $(CXXFLAGS)

../Common/database/database.o: ../Common/database/database.cpp
	$(CPP) -c ../Common/database/database.cpp -o ../Common/database/database.o $(CXXFLAGS)

"../World\ Server/console.o": ../World\ Server/console.cpp
	$(CPP) -c "../World Server/console.cpp" -o "../World Server/console.o" $(CXXFLAGS)

"../World\ Server/quest/QuestActions.o": ../World\ Server/quest/QuestActions.cpp
	$(CPP) -c "../World Server/quest/QuestActions.cpp" -o "../World Server/quest/QuestActions.o" $(CXXFLAGS)

"../World\ Server/quest/QuestConditions.o": ../World\ Server/quest/QuestConditions.cpp
	$(CPP) -c "../World Server/quest/QuestConditions.cpp" -o "../World Server/quest/QuestConditions.o" $(CXXFLAGS)

"../World\ Server/quest/Quests.o": ../World\ Server/quest/Quests.cpp
	$(CPP) -c "../World Server/quest/Quests.cpp" -o "../World Server/quest/Quests.o" $(CXXFLAGS)

"../World\ Server/quest/strhashing.o": ../World\ Server/quest/strhashing.cpp
	$(CPP) -c "../World Server/quest/strhashing.cpp" -o "../World Server/quest/strhashing.o" $(CXXFLAGS)

"../World\ Server/itemdata.o": ../World\ Server/itemdata.cpp
	$(CPP) -c "../World Server/itemdata.cpp" -o "../World Server/itemdata.o" $(CXXFLAGS)

"../World\ Server/stbhandler.o": ../World\ Server/stbhandler.cpp
	$(CPP) -c "../World Server/stbhandler.cpp" -o "../World Server/stbhandler.o" $(CXXFLAGS)

"../World\ Server/aip/AiActions.o": ../World\ Server/aip/AiActions.cpp
	$(CPP) -c "../World Server/aip/AiActions.cpp" -o "../World Server/aip/AiActions.o" $(CXXFLAGS)

"../World\ Server/aip/AiConditions.o": ../World\ Server/aip/AiConditions.cpp
	$(CPP) -c "../World Server/aip/AiConditions.cpp" -o "../World Server/aip/AiConditions.o" $(CXXFLAGS)

"../World\ Server/aip/Aip.o": ../World\ Server/aip/Aip.cpp
	$(CPP) -c "../World Server/aip/Aip.cpp" -o "../World Server/aip/Aip.o" $(CXXFLAGS)

"../World\ Server/ltbhandler.o": ../World\ Server/ltbhandler.cpp
	$(CPP) -c "../World Server/ltbhandler.cpp" -o "../World Server/ltbhandler.o" $(CXXFLAGS)

"../World\ Server/quest/QuestConditionsC.o": ../World\ Server/quest/QuestConditionsC.cpp
	$(CPP) -c "../World Server/quest/QuestConditionsC.cpp" -o "../World Server/quest/QuestConditionsC.o" $(CXXFLAGS)

"../World\ Server/quest/QuestActionsC.o": ../World\ Server/quest/QuestActionsC.cpp
	$(CPP) -c "../World Server/quest/QuestActionsC.cpp" -o "../World Server/quest/QuestActionsC.o" $(CXXFLAGS)

"../World\ Server/stlhandler.o": ../World\ Server/stlhandler.cpp
	$(CPP) -c "../World Server/stlhandler.cpp" -o "../World Server/stlhandler.o" $(CXXFLAGS)
