/*
    Rose Online Server Emulator
    Copyright (C) 2006,2007 OSRose Team http://www.dev-osrose.com

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    depeloped with Main erose/hrose source server + some change from the original eich source
*/
#include "worldserver.h"
#include <fstream>
using namespace std;

// Constructor
CWorldServer::CWorldServer( string fn )
{
    #ifdef STATICID
    //LMA: Client ID Static.
    last_cid=1;
    last_pid=1;
    #endif

    filename = fn;
    LoadConfigurations( (char*)fn.c_str() );
	for(unsigned i=0; i<0xffff; i++)
	{
	    ClientIDList[i] = 1;
	}

    //Clear or Vector list
    PartyList.clear();
    MDropList.clear();
    QuestList.clear();
    FairyList.clear();
    TeleGateList.clear();

    MapMutex = PTHREAD_MUTEX_INITIALIZER; //fast mutex
    SQLMutex = PTHREAD_MUTEX_INITIALIZER;
    PlayerMutex = PTHREAD_MUTEX_INITIALIZER;

    console_started=false;  //LMA: not in console mode.
}

// Destructor
CWorldServer::~CWorldServer( )
{
    // free memory
    for(UINT i=0;i<FairyList.size();i++)
        delete FairyList.at(i);
    for(UINT i=0;i<PartyList.size();i++)
        delete PartyList.at(i);
    for(UINT i=0;i<TeleGateList.size();i++)
        delete TeleGateList.at(i);
    for(UINT i=0;i<QuestList.size();i++)
        delete QuestList.at(i);
    for(UINT i=0;i<MDropList.size();i++)
        delete MDropList.at(i);

    //deleting arrays.
    delete [] SkillList;
    delete [] NPCData;

    delete [] JemList.Index;
    delete [] NaturalList.Index;
    delete [] PatList.Index;
    delete [] ProductList.Index;
    delete [] SellList.Index;
    delete [] UseList.Index;

    for(UINT i=0;i<10;i++)
    {
        delete [] EquipList[i].Index;
    }

    for(UINT i=0;i<MapList.Map.size();i++)
        delete MapList.Map.at(i);

    for(UINT i=0;i<ClientList.size();i++)
    {
        delete (CPlayer*)ClientList.at(i)->player;
        delete ClientList.at(i);
    }

    delete [] StatsList;
    delete [] ClassList;
    delete [] GradeList;

    delete EquipList[0].nullequip;
    delete JemList.nulljem;
    delete NaturalList.nullnatural;
    delete PatList.nullpat;
    delete ProductList.nullproduct;
    delete SellList.nullsell;
    delete UseList.nulluse;
    delete MapList.nullzone;
    delete DB;
}

//LMA BEGIN
//20070623, 224500
bool CWorldServer::Ping()
{
         //Bogus request (checking if npc ID 1 exists).
        if(DB->QStore( "SELECT id FROM list_npcs WHERE id='1'")==NULL)
        {
             Log( MSG_INFO, "MySQL Ping Time Error on port %u",DB->Port);
        }
        else
        {
            Log( MSG_INFO, "MySQL Ping Time Ok on port %u",DB->Port);
            DB->QFree( );
        }

     return true;
}
//LMA END

// This cleans up our clients mess :P
void CWorldServer::OnClientDisconnect( CClientSocket* thisclient )
{
    if(thisclient->player==NULL) return;
	CPlayer* player = (CPlayer*)thisclient->player;
	if(!player->Session->isLoggedIn) return;
    if(!player->Saved)
    {
    	player->savedata();
        player->Session->isLoggedIn = false;
        //send packet to change messenger status (offline)
    	BEGINPACKET( pak, 0x7e1 );
    	ADDBYTE    ( pak, 0xfa );
    	//ADDWORD    ( pak, player->CharInfo->charid );
    	ADDDWORD    ( pak, player->CharInfo->charid );
    	//ADDBYTE    ( pak, 0x00 );
    	cryptPacket( (char*)&pak, NULL );
    	send( csock, (char*)&pak, pak.Size, 0 );
    }

    if ( player->Fairy )
    {
        FairyList.at(player->FairyListIndex)->assigned = false;
        FairyList.at(player->FairyListIndex)->LastTime = clock();
        FairyList.at(player->FairyListIndex)->ListIndex = 0;
        FairyList.at(player->FairyListIndex)->WaitTime = Config.FairyWait * (rand()% GetFairyRange(1)+ GetFairyRange(0));
        player->Fairy = false;
        player->FairyListIndex = 0;
        DoFairyStuff(player, 0);
        // recalculate FairyMax
        Config.FairyMax = (int)ceil((float)ClientList.size() / 50.0); //(1 fairy more every 50 player)
    }

    if(player->Party->party!=NULL)
    {
        CParty* party = player->Party->party;
        BEGINPACKET( pak, 0x7d2 );
        ADDWORD    ( pak, 0xff00 );
        ADDDWORD   ( pak, player->CharInfo->charid );
        bool pflag = false;
        party->RemovePlayer( player );
        if(party->Members.size()>1)
        {
            for(UINT i=0;i<party->Members.size();i++)
            {
                CPlayer* othermember = party->Members.at(i);
                if(!pflag)
                {
                    ADDDWORD( pak, othermember->CharInfo->charid );
                    if(player->Party->IsMaster)
                        othermember->Party->IsMaster = true;
                    pflag = true;
                }
                othermember->client->SendPacket( &pak );
            }
        }
        else
        {
            for(UINT i=0;i<party->Members.size();i++)
            {
                CPlayer* othermember = party->Members.at(i);
                BEGINPACKET( pak, 0x7d1 );
                ADDBYTE    ( pak, 0x05 );
                ADDWORD    ( pak, 0x0000 );
                ADDWORD    ( pak, 0x0000 );
                othermember->client->SendPacket( &pak );
                othermember->Party->party = NULL;
                othermember->Party->IsMaster = true;
            }
            RemoveParty( party );
            delete party;
            party = NULL;
        }
    }
    DB->QExecute("UPDATE accounts SET online=false where id=%u", player->Session->userid );
}

// Server is started, lets run our loop :D
void CWorldServer::ServerLoop( )
{
	fd_set		fds;
	int			activity;
    maxfd = 0;
	sockaddr_in	ClientInfo;
	SOCKET		NewSocket;
	timeval		timeout;
	maxfd = sock;
	OnServerStep();

	//LMA BEGIN
	//MySQL Ping (every hour)
	//20070623, 221000
	UINT time_last_ping=clock();
	UINT delay_ping=3600000;

	//LMA END

	do
	{

        //LMA BEGIN
        //MySQL Ping
        //20070623, 221000
        UINT etime = (UINT)round((clock( ) - time_last_ping));
        if(etime>=delay_ping)
        {
            time_last_ping=clock();
            Ping();
        }
        //LMA END

        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        NewSocket = INVALID_SOCKET;
        FD_ZERO( &fds );
        pthread_mutex_lock( &PlayerMutex );
		if(!Config.usethreads)
    	   FillFDS( &fds );
		FD_SET( sock, &fds );
		activity = select( maxfd+1, &fds, NULL, NULL, &timeout );
		if ( activity == 0 )
        {
            pthread_mutex_unlock( &PlayerMutex );
        	#ifdef _WIN32
        	Sleep(1);
        	#else
        	usleep(1);
        	#endif
            continue;
        }
		if ( activity < 0 && errno != EINTR )
        {
			#ifdef _WIN32
			Log( MSG_ERROR, "Select command failed. Error #%i", WSAGetLastError() );
			#else
			Log( MSG_ERROR, "Select command failed. Error #%i", errno );
			#endif
			isActive = false;
		}
		if ( FD_ISSET( sock, &fds ) && ConnectedClients < 1024 )
        {
			int clientinfolen = sizeof( sockaddr_in );
            #ifdef _WIN32
       		NewSocket = accept( sock, (sockaddr*)&ClientInfo, (int*)&clientinfolen );
       		#else
    		NewSocket = accept( sock, (sockaddr*)&ClientInfo, (socklen_t*)&clientinfolen );
            #endif
			// TODO: check if server is full
			if (NewSocket != INVALID_SOCKET)
			{
				if(!isBanned(&ClientInfo))
					AddUser( NewSocket, &ClientInfo );
				else
				{
					Log( MSG_WARNING, "Banned client tried to connect: %s", inet_ntoa( ClientInfo.sin_addr ) );
					close( NewSocket );
				}
			}
			else
			{
			    #ifdef _WIN32
			    Log( MSG_ERROR, "Error accepting socket: %i", WSAGetLastError() );
			    #else
			    Log( MSG_ERROR, "Error accepting socket: %i", errno );
			    #endif
            }
		}
		if(!Config.usethreads)
    		HandleClients( &fds );
    	pthread_mutex_unlock( &PlayerMutex );
    	#ifdef _WIN32
    	Sleep(1);
    	#else
    	usleep(1);
    	#endif
	} while( isActive );
}

// Return a new clientsocket structure
CClientSocket* CWorldServer::CreateClientSocket( )
{
    DB->QExecute( "UPDATE channels SET connected=%i where id=%i and type=2", ConnectedClients, Config.ServerID );
    CClientSocket* client = new CClientSocket;
    CPlayer* player = new CPlayer( client );
    client->player = (void*) player;
    return client;
}

// Deletes an old clientsocket structure
void CWorldServer::DeleteClientSocket( CClientSocket* thisclient )
{
    DB->QExecute( "UPDATE channels SET connected=%i where id=%i and type=2", ConnectedClients, Config.ServerID );
if(thisclient->player!=NULL)
    {
        CPlayer* player = (CPlayer*) thisclient->player;
        Log( MSG_INFO, "User disconnected (%s)", player->CharInfo->charname );
        CMap* map = MapList.Index[player->Position->Map];
        pthread_mutex_lock( &MapMutex );
        map->RemovePlayer( player );
        pthread_mutex_unlock( &MapMutex );
        delete player;
        player = NULL;
    }
    else
    {
        delete thisclient;
        thisclient = NULL;
    }

}

// This function is called just before the server starts
bool CWorldServer::OnServerReady( )
{
    ServerOnline = true;
    GServer = this;
	clock_t timer = clock();
	LastUpdateTime = clock();
	ATTK_SPEED_MODIF = 120;
	HIT_DELAY_MODIF = 0;
	MOVE_SPEED_MODIF = 100000;

	//LMA: We init ObjVar, just in case.
	for (int k=0;k<MAX_NPC;k++)
	{
	    for (int j=0;j<20;j++)
	    {
	        ObjVar[k][j]=0;
	    }
	}

    //Load our Server Info
    LoadConfig( );
    //LoadLTB( );
    LoadSTBData( );
    LoadIfoObjects();
    //LMA: init default values.
    InitDefaultValues();
    LoadZoneData( );
	// NO Grids already. They are just unnecessary crap
    //LoadGrids( );   //resetting grids...
    LoadConsItem( );
    LoadSellData( );
    LoadProductItem( );
    LoadPatItem( );
    LoadNaturalItem( );
    LoadJemItem( );
    LoadEquip( );
    LoadItemStats( );
    LoadGrades();   //LMA: loading refine (grades) bonuses.
    LoadJobReq( );
    // PY new Stat Lookup table
    LoadStatLookup( );
    // PY end
    //LoadBreakList( );     // geo edit for disassemble // 22 oct 07
    LoadBreakChestBlueList();   //LMA: loading chests, breaks from STB...
    LoadSkillData( );
    //PY load waypoints for TD game
    LoadWayPoints( );

    //LMA: Forcing Union Wars.
    UWForceFrom=0;  //deactivated.
    UWNbPlayers=0;  //deactivated
    //In t + 5 minutes (deactivated, use gm command instead).
    //pakGMForceUW(NULL,5);

    //LMA: forcing gem quets
    GemQuestForce=0;
    GemQuestReset=0;


    //LMA: for debug (load old skill system and compares to new one).
    /*
    LoadSkillDataOld( );
    LMACheckSkills();
    LMACheckStuff();
    */
    //End debug.

    //Load our Server Info
    LoadQuestSTB();

    //hidden
    // LoadDropsData( );
    // new drops routine load

    //LMA: Drop And system
    if(!Config.drop_rev)
    {
        LoadPYDropsData( );
    }
    else
    {
        LoadPYDropsDataAnd();
    }

    LoadSkillBookDropsData( );
    // end of new drops data

    //LoadChestData( ); //LMA: loaded now by STB in LoadBreakChestBlueList.

    #ifdef PYCUSTOM
    // PY custom events start
    LoadCustomTeleGate( );
    LoadCustomEvents( );
    // PY custom events end
    #endif

    LoadNPCData( );
    LoadQuestItemData( );
    LoadTeleGateData( );
    LoadRespawnData( );
    LoadLTB();  //LMA: Loading LTB for AIP.
    LoadAipData();  //LMA: loading AIP.
    TimerForNPC();  //LMA: loading timers for NPCs and Monsters.
    //LoadMobGroups( );           //PY: Disabled LoadMobGroups in favor of....
    LoadMonsterSpawn( );          //PY: Enabled new IFO based spawning system
    //LoadMobGroupsSpecial( );    //LMA: Special spawns (Halloween for example).
    LoadNPCs( );
    LoadNPCsSpecial( );  //Special NPC load
    LoadNPCsEvents();   //LMA: loading pre defined events.
    //LoadMonsters( );            //PY: Disabled LoadMonsters in favor of....
    LoadMonstersFromIFO( );       //PY: Enabled new IFO based spawning system
    LoadUpgrade( );
    //LoadLTB();  //LMA: Loading LTB for AIP.
    LoadQuestData( );
    //LoadAipData();  //LMA: loading AIP.
    CleanConnectedList( );

    Log(MSG_INFO, "Database Loaded                      " );
    pthread_create( &WorldThread[WORLD_THREAD], &at, WorldProcess, NULL);
    pthread_create( &WorldThread[VISUALITY_THREAD], &at, VisibilityProcess, NULL);
    pthread_create( &MapThread[0], &at, MapProcess, NULL);
    Log (MSG_INFO, "osRose Revision %s", Config.osRoseVer );
    Log( MSG_INFO, "Process Loaded. WorldDelay %i | MapDelay %i | VisualDelay %i",Config.WorldDelay,Config.MapDelay,Config.VisualDelay);
    DB->QExecute( "DELETE FROM channels WHERE id=%u and type=%i", Config.ServerID, Config.ServerType );
    if(!DB->QExecute("INSERT INTO channels (id,type,name,host,port,lanip,lansubmask,connected,maxconnections,owner) VALUES (%i,%i,'%s','%s',%u,'%s','%s',0,%i,%i)",
            Config.ServerID, Config.ServerType, Config.ServerName, Config.WorldIP, Config.WorldPort, Config.LanIP, Config.LanSubnet, Config.MaxConnections, Config.ParentID))
    {
        Log(MSG_WARNING, "Error accessing to database, the other server will not connect to WorldServer" );
    }


	MYSQL_ROW row;
    bool pflag = false;
    //Get IP and Port from Charserver
    MYSQL_RES *result = DB->QStore( "SELECT host,port,lanip FROM channels WHERE id=%u and type=1", Config.ParentID );
    if(result==NULL) return false;
	if (mysql_num_rows( result ) == 1)
	{
    	row = mysql_fetch_row( result );
    	switch(Config.Connection)
    	{
            case 0://wanip
               	Config.CharIP = row[0];
            break;
            case 1://lanip
               	Config.CharIP = row[2];
            break;
            default://localhost
               	Config.CharIP = (char*) "127.0.0.1";
            break;
        }
    	Config.CharPort =  atoi(row[1]);
        pflag = true;
    }
    DB->QFree( );
    if(pflag)
    {
        Log(  MSG_INFO, "Initialized Charserver connection" );
    	// Connect To LoginServer
    	csock = socket( AF_INET, SOCK_STREAM, 0 );
    	if (csock == INVALID_SOCKET)
        {
            Log( MSG_WARNING, "Could not access login server" );
        }
    	struct sockaddr_in ain;
    	ain.sin_family		= AF_INET;
       	ain.sin_addr.s_addr	= inet_addr( Config.CharIP );
    	ain.sin_port = htons( Config.CharPort );
    	if ( connect( csock, (SOCKADDR*) &ain, sizeof(ain) ) == SOCKET_ERROR )
    		Log( MSG_WARNING, "Could not access charserver" );
    	BEGINPACKET( pak, 0x500 );
        ADDDWORD   ( pak, Config.CharPass );
        ADDDWORD   ( pak, Config.ServerID );
        ADDWORD    ( pak, Config.WorldPort );
    	cryptPacket( (char*)&pak, NULL );
    	send( csock, (char*)&pak, pak.Size, 0 );
    }

	float loadtime = (float)( clock() - timer ) / CLOCKS_PER_SEC;
	Log(  MSG_INFO, "Server took %.4f seconds to load", loadtime );

	return true;
}

// Send Levelup To charserver
bool CWorldServer::SendLevelUPtoChar(CPlayer *thisclient)
{
	BEGINPACKET( pak, 0x79e );
	//ADDWORD( pak, thisclient->CharInfo->charid );
	ADDDWORD( pak, thisclient->CharInfo->charid );
	ADDWORD( pak, thisclient->Stats->Level );
	cryptPacket( (char*)&pak, NULL );
	send( csock, (char*)&pak, pak.Size, 0 );
    return true;
}

// disconect all the clients
void CWorldServer::DisconnectAll()
{
    for(UINT i=0;i<ClientList.size();i++)
    {
        CPlayer* otherclient = (CPlayer*) ClientList.at(i)->player;
		if(otherclient->Session->isLoggedIn)
		{
            otherclient->savedata( );
            otherclient->Saved = true;
            otherclient->client->isActive = false;
        }
	}
}

/**
	* check on the database if a client is banned
	* @param ClientInfo sockaddr_in structure
	* @return true if the client is banned else false
*/
bool CWorldServer::isBanned( sockaddr_in* ClientInfo )
{
	if(DB==NULL) return false;
	std::string ip( inet_ntoa( ClientInfo->sin_addr ) );
	MYSQL_RES* result = DB->QStore( "SELECT id, ip, startban, bantime FROM ban_list" );
	if(result==NULL)
		return false;
	MYSQL_ROW row;
	while(row=mysql_fetch_row( result ))
	{
		unsigned int strsize = strlen( (char*)row[1] );
		if(strncmp( row[1], (char*)ip.c_str( ), strsize)==0)
		{	// ip match is banned, now check if is permant ban or temporal
			if(row[3]==NULL || atoi(row[3])==0)
			{ // is permant ban
				DB->QFree( );
				return true;
			}
			unsigned long int finishban = atol(row[2]) + atol(row[3]);
			if(finishban>=GetServerTime( ))
			{
				unsigned int id = atoi(row[0]);
				DB->QFree( );
				DB->QExecute( "DELETE FROM ban_list WHERE id=%u", id );
				return false;
			}
			else
			{
				DB->QFree( );
				return true;
			}
		}
	}
	DB->QFree( );
	return false;
}

// Load Server configuration
void CWorldServer::LoadConfigurations( const char* file )
{
    //Database
	Config.SQLServer.pcServer   = ConfigGetString ( file, "mysql_host", "localhost" );
	Config.SQLServer.pcDatabase = ConfigGetString ( file, "mysql_database", "roseon_beta" );
    Config.SQLServer.pcUserName = ConfigGetString ( file, "mysql_user", "root" );
	Config.SQLServer.pcPassword = ConfigGetString ( file, "mysql_pass", "" );
	Config.SQLServer.pcPort     = ConfigGetInt    ( file, "mysql_port", 3306 );
    //Server
	Config.ServerID             = ConfigGetInt    ( file, "serverid", 1 );
	Config.ServerType           = ConfigGetInt    ( file, "servertype", 2 );
	Config.WorldPort            = ConfigGetInt    ( file, "serverport", 29200 );
	Config.WorldIP              = ConfigGetString ( file, "serverip", "127.0.0.1" );
	Config.ParentID             = ConfigGetInt    ( file, "parentid", 1 );
	Config.ServerName           = ConfigGetString ( file, "servername", "Channel" );
    Config.MaxConnections       = ConfigGetInt    ( file, "maxconnections", 100 );
    Config.Connection           = ConfigGetInt    ( file, "connection", 0 );
    Config.LanIP                = ConfigGetString ( file, "lanip", "192.168.0.1" );
    Config.LanSubnet            = ConfigGetString ( file, "lansubmask", "192.168.0" );
    //World
    Config.MinimumAccessLevel   = ConfigGetInt    ( file, "minimal_access_level", 100 );
	Config.usethreads           = ConfigGetInt    ( file, "usethreads", 0 )==0?false:true;
	Config.WELCOME_MSG          = ConfigGetString ( file, "welcome_msg", "Welcome to Rose Online" );

    Log (MSG_INFO, "RoseZA - http://www.roseza.co.za");

    //Password
	Config.LoginPass            = ConfigGetInt    ( file, "loginpass", 123456 );
	Config.CharPass             = ConfigGetInt    ( file, "charpass", 123456 );
	Config.WorldPass            = ConfigGetInt    ( file, "worldpass", 123456 );
    if(Config.AUTOSAVE==1)
        Log( MSG_INFO, "Autosaving Every %i minutes", Config.SAVETIME/60 );

    LoadCommandLevels();
}

// Load commands from commands.ini [by Paul_T]
void CWorldServer::LoadCommandLevels( void )
{
    Config.Command_Addquest = ConfigGetInt ( "commands.ini", "addquest", 299 );
    Config.Command_AllSkill = ConfigGetInt ( "commands.ini", "allskill", 299 );
    Config.Command_GMSkills = ConfigGetInt ( "commands.ini", "gmskills", 299 );
    Config.Command_Ani = ConfigGetInt    ( "commands.ini", "ani", 299 );
    Config.Command_Ann = ConfigGetInt    ( "commands.ini", "ann", 299 );
    Config.Command_AtkModif = ConfigGetInt    ( "commands.ini", "atkmodif", 299 );
    Config.Command_Ban = ConfigGetInt    ( "commands.ini", "ban", 299 );
    Config.Command_BonusXp = ConfigGetInt    ( "commands.ini", "bonusxp", 299 );  //LMA: bonusxp
    Config.Command_Broadcast = ConfigGetInt    ( "commands.ini", "broadcast", 299 );
    Config.Command_Buff = ConfigGetInt    ( "commands.ini", "buff", 299 );
    Config.Command_cart = ConfigGetInt    ( "commands.ini", "cart", 299 );  // all Cart Parts
    Config.Command_Cfmode = ConfigGetInt    ( "commands.ini", "cfmode", 299 );
    Config.Command_cg = ConfigGetInt    ( "commands.ini", "castlegear", 299 ); //get all CastleGear Parts
    Config.Command_Cha = ConfigGetInt    ( "commands.ini", "cha", 299 );
    Config.Command_ChangeFairyWait = ConfigGetInt    ( "commands.ini", "changefairywait", 299 );
    Config.Command_ChangeFairyStay = ConfigGetInt    ( "commands.ini", "changefairystay", 299 );
    Config.Command_ChangeFairyTestMode = ConfigGetInt    ( "commands.ini", "changefairytestmode", 299 );
    Config.Command_Class = ConfigGetInt    ( "commands.ini", "class", 299 );
    Config.Command_ConfigReset = ConfigGetInt    ( "commands.ini", "ConfigReset", 299 );
    Config.Command_Convert = ConfigGetInt    ( "commands.ini", "convert", 299 );
    Config.Command_Debuff = ConfigGetInt    ( "commands.ini", "debuff", 299 );
    Config.Command_DelSkills = ConfigGetInt ( "commands.ini", "delskills", 299 );
    Config.Command_DelSpawn = ConfigGetInt    ( "commands.ini", "delspawn", 299 );
    Config.Command_Dev = ConfigGetInt    ( "commands.ini", "dev", 299 );
    Config.Command_DQuest = ConfigGetInt    ( "commands.ini", "dquest", 299 );
    Config.Command_Drop = ConfigGetInt    ( "commands.ini", "drop", 299 );
    Config.Command_DSpawn = ConfigGetInt    ( "commands.ini", "dspawn", 299 );
    Config.Command_ESpawn = ConfigGetInt    ( "commands.ini", "espawn", 299 );
    Config.Command_Event = ConfigGetInt    ( "commands.ini", "event", 299 );     //Event
    Config.Command_EventName = ConfigGetInt    ( "commands.ini", "eventname", 299 );     //Event Name
    Config.Command_EventIfo = ConfigGetInt    ( "commands.ini", "eventifo", 299 );     //LMA: Events for IFO Objects
    Config.Command_Exp = ConfigGetInt    ( "commands.ini", "exp", 299 );
    Config.Command_Face = ConfigGetInt    ( "commands.ini", "face", 299 );
    Config.Command_fskill = ConfigGetInt    ( "commands.ini", "fskill", 299 );     //LMA: Force a skill for a monster.
    Config.Command_Give2 = ConfigGetInt    ( "commands.ini", "give2", 299 );
    Config.Command_GiveClanRp = ConfigGetInt    ( "commands.ini", "giveclanrp", 299 );       //Reward points
    Config.Command_GiveClanp = ConfigGetInt    ( "commands.ini", "giveclanp", 299 );       //Clan points
    Config.Command_GiveFairy = ConfigGetInt    ( "commands.ini", "givefairy", 299 );
    Config.Command_GiveZuly = ConfigGetInt    ( "commands.ini", "givezuly", 299 );
    Config.Command_GlobalTime  = ConfigGetInt    ( "commands.ini", "globaldelay", 30 );
    Config.Command_GlobalPrefix = ConfigGetString    ( "commands.ini", "globalprefix", "[Broadcast]" );
    Config.Command_GmList = ConfigGetInt ( "commands.ini", "gmlist", 299 );
    Config.Command_Go = ConfigGetInt    ( "commands.ini", "go", 299 );
    Config.Command_Goto = ConfigGetInt    ( "commands.ini", "goto", 299 );
    Config.Command_GoToMap = ConfigGetInt    ( "commands.ini", "gotomap", 299 );
    Config.Command_grid = ConfigGetInt    ( "commands.ini", "grid", 299 );   //LMA: maps grids.
    Config.Command_Hair = ConfigGetInt    ( "commands.ini", "hair", 299 );
    Config.Command_Heal = ConfigGetInt    ( "commands.ini", "heal", 299 );
    Config.Command_Here = ConfigGetInt    ( "commands.ini", "here", 299 );
    Config.Command_Hide = ConfigGetInt    ( "commands.ini", "hide", 299 );
    Config.Command_HitModif = ConfigGetInt    ( "commands.ini", "hitmodif", 299 );
    Config.Command_HurtHim = ConfigGetInt    ( "commands.ini", "hurthim", 299 );
    Config.Command_Info = ConfigGetInt    ( "commands.ini", "info", 299 );
    Config.Command_IQuest = ConfigGetInt    ( "commands.ini", "iquest", 299 );
    Config.Command_Item = ConfigGetInt    ( "commands.ini", "item", 299 );
    Config.Command_ItemStat = ConfigGetInt    ( "commands.ini", "itemstat", 299 );
    Config.Command_Job = ConfigGetInt    ( "commands.ini", "job", 299 );
    Config.Command_Kick = ConfigGetInt    ( "commands.ini", "kick", 299 );
    Config.Command_KillInRange = ConfigGetInt    ( "commands.ini", "killinrange", 299 );
    Config.Command_Level = ConfigGetInt    ( "commands.ini", "level", 299 );
    Config.Command_LevelUp = ConfigGetInt    ( "commands.ini", "levelup", 299 );
    Config.Command_Listquest = ConfigGetInt ( "commands.ini", "listquest", 299 );
    Config.Command_Listqflag = ConfigGetInt ( "commands.ini", "listqflag", 299 );
    Config.Command_Listqvar = ConfigGetInt ( "commands.ini", "listqvar", 299 );
    Config.Command_ManageFairy = ConfigGetInt    ( "commands.ini", "managefairy", 299 );
    Config.Command_MaxStats     = ConfigGetInt    ( "commands.ini", "maxstats", 299 );     //MaxStats
    Config.Command_Mdmg = ConfigGetInt    ( "commands.ini", "mdmg", 299 );
    Config.Command_Mon = ConfigGetInt    ( "commands.ini", "mon", 299 );
    Config.Command_Mon2 = ConfigGetInt    ( "commands.ini", "mon2", 299 );
    Config.Command_Monster    = ConfigGetInt    ( "commands.ini", "monster", 299 );
    Config.Command_Move = ConfigGetInt    ( "commands.ini", "move", 299 );
    Config.Command_Moveto = ConfigGetInt    ( "commands.ini", "moveto", 299 );
    Config.Command_Mute = ConfigGetInt    ( "commands.ini", "mute", 299 );
    Config.Command_Npc = ConfigGetInt    ( "commands.ini", "npc", 299 );
    Config.Command_NpcObjVar = ConfigGetInt    ( "commands.ini", "npcobjvar", 299 );
    Config.Command_NpcSetObjVar = ConfigGetInt    ( "commands.ini", "npcsetobjvar", 299 );
    Config.Command_NpcLtb = ConfigGetInt    ( "commands.ini", "npcltb", 299 );
    Config.Command_Pak = ConfigGetInt    ( "commands.ini", "pak", 299 );
    Config.Command_Pak2 = ConfigGetInt    ( "commands.ini", "pak2", 299 );
    Config.Command_Pakm = ConfigGetInt    ( "commands.ini", "pakm", 299 );
    Config.Command_Partylvl = ConfigGetInt      ( "commands.ini", "partylvl", 299);
    Config.Command_Pdmg = ConfigGetInt    ( "commands.ini", "pdmg", 299 );
    Config.Command_PlayerInfo = ConfigGetInt    ( "commands.ini", "playerinfo", 299 );
    Config.Command_Pvp = ConfigGetInt    ( "commands.ini", "pvp", 299 );
    Config.Command_RaiseCG = ConfigGetInt    ( "commands.ini", "raisecg", 299 );
    Config.Command_Rate = ConfigGetInt    ( "commands.ini", "rate", 299 );
    Config.Command_Reborn = ConfigGetInt    ( "commands.ini", "reborn", 299 );  //Reborn by core
    Config.Command_Refine = ConfigGetInt    ( "commands.ini", "refine", 299 );  //Refine by PurpleYouko
    Config.Command_Reload = ConfigGetInt    ( "commands.ini", "reload", 299 );
    Config.Command_ReloadQuest = ConfigGetInt    ( "commands.ini", "reloadquest", 299 );
    Config.Command_Rules = ConfigGetInt    ( "commands.ini", "rules", 99 );
    Config.Command_Save = ConfigGetInt    ( "commands.ini", "save", 299 );
    Config.Command_ServerInfo = ConfigGetInt    ( "commands.ini", "serverinfo", 299 );
    Config.Command_Set = ConfigGetInt    ( "commands.ini", "set", 299 );
    Config.Command_Setqflag = ConfigGetInt ( "commands.ini", "setqflag", 299 );
    Config.Command_Setqvar = ConfigGetInt ( "commands.ini", "setqvar", 299 );
    Config.Command_Settime = ConfigGetInt    ( "commands.ini", "settime", 299 );
    Config.Command_SetUW = ConfigGetInt    ( "commands.ini", "setuw", 299 );
    Config.Command_SetGemQuest = ConfigGetInt    ( "commands.ini", "setgemquest", 299 );
    Config.Command_SetUWnb = ConfigGetInt    ( "commands.ini", "setuwnb", 299 );
    Config.Command_ShopType = ConfigGetInt    ( "commands.ini", "shoptype", 299 );
    Config.Command_Shutdown = ConfigGetInt    ( "commands.ini", "shutdown", 299 );
    Config.Command_SpawnList = ConfigGetInt    ( "commands.ini", "spawnlist", 299 );
    Config.Command_SpawnDetail = ConfigGetInt    ( "commands.ini", "spawndetail", 299 );
    Config.Command_SpawnRefresh = ConfigGetInt    ( "commands.ini", "spawnrefresh", 299 );
    Config.Command_SpeedModif = ConfigGetInt    ( "commands.ini", "speedmodif", 299 );
    Config.Command_SSpawn = ConfigGetInt    ( "commands.ini", "sspawn", 299 );
    Config.Command_Stat = ConfigGetInt    ( "commands.ini", "stat", 299 );
    Config.Command_Summon = ConfigGetInt    ( "commands.ini", "summon", 299 );
    Config.Command_TargetInfo = ConfigGetInt    ( "commands.ini", "targetinfo", 299 );
    Config.Command_Tele = ConfigGetInt    ( "commands.ini", "tele", 299 );
    Config.Command_TeleToMe = ConfigGetInt    ( "commands.ini", "teletome", 299 );
    Config.Command_Temple = ConfigGetInt    ( "commands.ini", "temple", 299 );
    Config.Command_tquest = ConfigGetInt    ( "commands.ini", "tquest", 299 );
    Config.Command_Transx = ConfigGetInt    ( "commands.ini", "transx", 299 );
    Config.Command_Union = ConfigGetInt    ( "commands.ini", "union", 299 );
    Config.Command_UnionPoints = ConfigGetInt    ( "commands.ini", "unionpoints", 299 );
    Config.Command_Who = ConfigGetInt    ( "commands.ini", "who", 299 );
    Config.Command_Who2 = ConfigGetInt    ( "commands.ini", "who2", 299 );

	//Ticket System
    Config.Command_DelTicket = ConfigGetInt    ( "commands.ini", "delTicket", 299 );
    Config.Command_ReadTick = ConfigGetInt    ( "commands.ini", "readTickets", 299 );

    //Broadcast Fee's in commands.ini
    Config.Command_Fee1 = ConfigGetInt    ( "commands.ini", "Fee1", 0 );
    Config.Command_Fee2 = ConfigGetInt    ( "commands.ini", "Fee2", 500 );
    Config.Command_Fee3 = ConfigGetInt    ( "commands.ini", "Fee3", 2000 );
    Config.Command_Fee4 = ConfigGetInt    ( "commands.ini", "Fee4", 4000 );
    Config.Command_Fee5 = ConfigGetInt    ( "commands.ini", "Fee5", 10000 );

    //Donation Points
    Config.AutoDP = ConfigGetInt ( "commands.ini", "AutoDP", 0);
    Config.TimeDP = ConfigGetInt ( "commands.ini", "TimeDP", 10000);
    Config.AmountDP = ConfigGetInt ( "commands.ini", "AmountDP", 1);
    Config.TimerDP = ConfigGetInt ( "commands.ini", "TimerDP", 60);

    //Double Exp map
    Config.doubleExpActive = ConfigGetInt ( "commands.ini", "doubleExpActive", 0);
    Config.doubleExpAmount = ConfigGetInt ( "commands.ini", "doubleExpAmount", 2);
    Config.doubleExpTime = ConfigGetInt ( "commands.ini", "doubleExpTime", 120);
    Config.numberOfMaps = ConfigGetInt ("commands.ini","numberOfMaps", 40);
    Config.mapTimer = ConfigGetInt ("commands.ini","mapTimer", 60);
    Config.Command_hhExp = ConfigGetInt ("commands.ini","hhExp", 299);
    Config.mapFlag = true;

    // Warning System
    Config.Command_Warning = ConfigGetInt("commands.ini", "Warning",299);
    Config.ban_time1 = ConfigGetInt("commands.ini", "ban_time1",172800);
    Config.ban_time2 = ConfigGetInt("commands.ini", "ban_time2",604800);
    Config.ban_timer = ConfigGetInt("commands.ini", "ban_time3",60);
    Config.Command_SeeWarning = ConfigGetInt("commands.ini", "SeeWarn", 299);
    Config.Command_DelWarning = ConfigGetInt("commands.ini", "DelWarn", 299);

    Config.warningSysActive = ConfigGetInt("commands.ini", "WarnSysAct",1);
}
// Incoming packet
bool CWorldServer::OnReceivePacket( CClientSocket* thisclient, CPacket *P )
{
    /*//Maxxon
    pthread_mutex_lock( &GServer->PlayerMutex );
    pthread_mutex_lock( &GServer->MapMutex );*/

	if(P->Command != 0x0808 && P->Command != 0x079f && P->Command != 0x0700)
    {
        PacketTime((CPlayer*)thisclient->player);
        //Log(MSG_HACK,"PACKET RECIEVED! %04x", P->Command);
    }

	switch( P->Command )
	{
        case 0x0500: return pakCSReady          ( (CPlayer*)thisclient->player, P );
        case 0x0502: return pakCharDSClient     ( (CPlayer*)thisclient->player, P );
    	case 0x0505: return pakCSCharSelect     ( (CPlayer*)thisclient->player, P );
    	//case 0x0756: return true;//unknown
    	case 0x0756: return pakChangeRespawn ( (CPlayer*)thisclient->player, P );
        case 0x0700: return pakPing             ( (CPlayer*)thisclient->player, P );
    	case 0x0707: return pakExit             ( (CPlayer*)thisclient->player, P );
    	case 0x070b: return pakDoIdentify       ( (CPlayer*)thisclient->player, P );
    	case 0x071c: return pakCharSelect       ( (CPlayer*)thisclient->player, P );
    	case 0x0730: return pakGiveQuest        ( (CPlayer*)thisclient->player, P );
    	case 0x0753: return pakDoID             ( (CPlayer*)thisclient->player, P );
    	case 0x0755: return pakUserDied         ( (CPlayer*)thisclient->player, P );
    	case 0x0762: return pakWeight           ( (CPlayer*)thisclient->player, P );
    	case 0x0771: return pakStopChar         ( (CPlayer*)thisclient->player, P );
    	case 0x0781: return pakDoEmote          ( (CPlayer*)thisclient->player, P );
    	case 0x0782: return pakChangeStance     ( (CPlayer*)thisclient->player, P );
    	case 0x0783: return pakNormalChat       ( (CPlayer*)thisclient->player, P );
    	case 0x0784: return pakWhisper          ( (CPlayer*)thisclient->player, P );
    	case 0x0785: return pakShout            ( (CPlayer*)thisclient->player, P );
    	case 0x0789: return pakChatUnion        ( (CPlayer*)thisclient->player, P );    //LMA: Union Chat
    	case 0x0786: return pakPartyChat        ( (CPlayer*)thisclient->player, P );
    	case 0x0798: return pakStartAttack      ( (CPlayer*)thisclient->player, P );
      	case 0x079f: return pakShowHeal         ( (CPlayer*)thisclient->player, P );
    	case 0x079a: return pakMoveChar         ( (CPlayer*)thisclient->player, P );
    	case 0x07a1: return pakNPCBuy           ( (CPlayer*)thisclient->player, P );
        case 0x07a3: return pakUseItem          ( (CPlayer*)thisclient->player, P );
        case 0x07a4: return pakDoDrop           ( (CPlayer*)thisclient->player, P );
    	case 0x07a5: return pakChangeEquip      ( (CPlayer*)thisclient->player, P );
    	case 0x07cf: return pakChangeCostume    ( (CPlayer*)thisclient->player, P );
    	case 0x07a7: return pakPickDrop         ( (CPlayer*)thisclient->player, P );
    	case 0x07a8: return pakGate             ( (CPlayer*)thisclient->player, P );
    	case 0x07a9: return pakAddStats         ( (CPlayer*)thisclient->player, P );
    	case 0x07aa: return pakMoveSkill        ( (CPlayer*)thisclient->player, P );
    	case 0x07ab: return pakEquipABC         ( (CPlayer*)thisclient->player, P );
    	case 0x07af: return pakCraft            ( (CPlayer*)thisclient->player, P );
        case 0x07b1: return pakLevelUpSkill     ( (CPlayer*)thisclient->player, P );
        case 0x07b2: return pakSkillSelf        ( (CPlayer*)thisclient->player, P );
    	case 0x07b3: return pakStartSkill       ( (CPlayer*)thisclient->player, P );
    	case 0x07b4: return pakSkillAOE         ( (CPlayer*)thisclient->player, P );
    	case 0x07c0: return pakTradeAction      ( (CPlayer*)thisclient->player, P );
    	case 0x07c1: return pakTradeAdd         ( (CPlayer*)thisclient->player, P );
    	case 0x07ad: return pakStorage          ( (CPlayer*)thisclient->player, P );
    	case 0x07ae: return pakChangeStorage    ( (CPlayer*)thisclient->player, P );    //old format. Now changed to 0x07c8 for some reason
    	case 0x07c8: return pakChangeStorage    ( (CPlayer*)thisclient->player, P );
    	case 0x07ba: return pakidentify         ( (CPlayer*)thisclient->player, P );
    	case 0x07bc: return pakModifiedItem     ( (CPlayer*)thisclient->player, P );
    	case 0x07bf: return pakAddWishList      ( (CPlayer*)thisclient->player, P );
    	case 0x07c2: return pakOpenShop         ( (CPlayer*)thisclient->player, P );
    	case 0x07c3: return pakCloseShop        ( (CPlayer*)thisclient->player, P );
        case 0x07c4: return pakShowShop         ( (CPlayer*)thisclient->player, P );
        case 0x07c5: return pakBuyShop          ( (CPlayer*)thisclient->player, P );
        case 0x07c6: return pakSellShop         ( (CPlayer*)thisclient->player, P );
    	case 0x07ca: return pakChangeCart       ( (CPlayer*)thisclient->player, P );
    	//case 0x07cb: return pakRepairItem       ( (CPlayer*)thisclient->player, P );
    	case 0x07cb: return pakRepairItem       ( (CPlayer*)thisclient->player, P,2);   //LMA: tool repair.
    	case 0x07cd: return pakRepairItem       ( (CPlayer*)thisclient->player, P,1);   //LMA: NPC repair.
    	case 0x07d0: return pakPartyActions     ( (CPlayer*)thisclient->player, P );
    	case 0x07d1: return pakPartyManager     ( (CPlayer*)thisclient->player, P );
    	case 0x07d7: return pakPartyOption      ( (CPlayer*)thisclient->player, P );
    	case 0x07d8: return pakModifiedItemDone ( (CPlayer*)thisclient->player, P );
        case 0x07d9: return pakItemMall         ( (CPlayer*)thisclient->player, P );
    	case 0x07da: return pakStoreZuly        ( (CPlayer*)thisclient->player, P );
    	case 0x07dd: return pakRideRequest      ( (CPlayer*)thisclient->player, P );
    	case 0x07e0: return pakCreateClan       ( (CPlayer*)thisclient->player, P );
    	case 0x07e1: return pakClanManager      ( (CPlayer*)thisclient->player, P );
    	case 0x07eb: return pakPrintscreen      ( (CPlayer*)thisclient->player, P );
    	case 0x07ed: return pakChatTrade        ( (CPlayer*)thisclient->player, P );    //LMA: Trade Chat
    	case 0x0808: return pakGameGuard        ( (CPlayer*)thisclient->player, P );
    	case 0x0821: return pakExpTC        ( (CPlayer*)thisclient->player, P );  //LMA: Bonus Time Coupon
    	case 0x0796: return pakPvp796        ( (CPlayer*)thisclient->player, P );  //LMA: PVP?
    	default:
    		Log( MSG_WARNING, "(SID:%i) Received unknown packet. Command:%04x Size:%04x", thisclient->sock, P->Command, P->Size );
		break;
	}

	/*//Maxxon
    pthread_mutex_unlock( &GServer->MapMutex );
    pthread_mutex_unlock( &GServer->PlayerMutex );*/

	return true;
}

void doubleExpMapActivate()
{
    srand(time(0));
    GServer->Config.randomMap = rand() % GServer->Config.numberOfMaps + 1;

	while(GServer->Config.lastDoubleMap == GServer->Config.randomMap)// Should prevent 2 of the same maps after each other.
    {
		srand(time(0));
        GServer->Config.randomMap = rand() % GServer->Config.numberOfMaps + 1;
    }

    GServer->Config.lastDoubleMap = GServer->Config.randomMap;

    MYSQL_ROW row;
    MYSQL_RES *result = NULL;

    result = GServer->DB->QStore("SELECT map_name,map_id FROM map_exp WHERE id = %i", GServer->Config.randomMap);
    row = mysql_fetch_row(result);
    GServer->DB->QFree();

    if(result == NULL || row == NULL)
    {
        BEGINPACKET( pak, 0x702 );
        ADDSTRING( pak, "Error choosing map! Map number: " );
        ADDSTRING( pak, "- " );
        ADDWORD( pak, GServer->Config.randomMap);
        ADDBYTE( pak, 0x00);
        GServer->SendToAll( &pak );
        Log(MSG_ERROR, "Error in choosing a random exp map!! Map number %i", GServer->Config.randomMap);
    }

    GServer->Config.mapName = row[0];
    GServer->Config.doubleExpMap = atoi(row[1]);

    BEGINPACKET( pak, 0x702 );
    ADDSTRING( pak, "Happy Hour on map " );
    ADDSTRING( pak, "- " );
    ADDSTRING( pak, GServer->Config.mapName.c_str());
    ADDSTRING( pak, ". Type /happyhour for more info!");
    ADDBYTE( pak, 0x00);
    GServer->SendToAll( &pak );

    GServer->Config.lastMapTime = clock();
    GServer->Config.mapFlag = false;

    Log(MSG_INFO, "Double exp activated for map %s", GServer->Config.mapName.c_str());
}

void doubleExpMapDeactivate()
{
    char tempMapName[100];

    sprintf ( tempMapName,"%s" ,GServer->Config.mapName.c_str());

    GServer->Config.doubleExpMap = 0;
    GServer->Config.mapName = "";

    BEGINPACKET( pak, 0x702 );
    ADDSTRING( pak, "Happy Hour over on map " );
    ADDSTRING( pak, "- " );
    ADDSTRING( pak, tempMapName);
    ADDBYTE( pak, 0x00);
    GServer->SendToAll( &pak );

    GServer->Config.mapFlag = true;

    Log(MSG_INFO, "Double exp deactivated for map %s", tempMapName);
}

void doubleExpAnnounce()
{
    BEGINPACKET( pak, 0x702 );
    ADDSTRING( pak, "Happy Hour on map " );
    ADDSTRING( pak, "- " );
    ADDSTRING( pak, GServer->Config.mapName.c_str());
    ADDSTRING( pak, ". Type /happyhour for more info!");
    ADDBYTE( pak, 0x00);
    GServer->SendToAll( &pak );

    GServer->Config.lastAnnTime = clock();
}

// Tracks time remaining till a players temp ban is lifted
void BanTracker()
{

    GServer->Config.lastWCountRed = clock();
    int Counter = 0;
    int ban_type=0;
    int account_id=0;

    MYSQL_ROW row;
    MYSQL_RES *result = GServer->DB->QStore("SELECT account_id, time_remaining, ban_type FROM warning_system");

    while(row= mysql_fetch_row(result)){

        account_id = atoi(row[0]);
        Counter = atoi(row[1]);
        ban_type = atoi(row[2]);

        if(ban_type == 1)
        {
            if(GServer->Config.ban_time1 <= Counter){

                GServer->DB->QFree();
                GServer->DB->QExecute("UPDATE accounts SET accesslevel = '100' WHERE id = %i", account_id);
                GServer->DB->QExecute("UPDATE warning_system SET ban_type = '0' WHERE account_id = '%i'", account_id);
                Log(MSG_INFO,"Player ban has been lifted. Account id: %i",account_id);

            }
            else
            {

                Counter+=60;
                Log(MSG_INFO, "account id: %i, time: %i", account_id, Counter);
                GServer->DB->QFree();
                GServer->DB->QExecute("UPDATE warning_system SET time_remaining = %i WHERE account_id = %i", Counter, account_id);

            }
        }
        else if(ban_type == 2)
        {
            if(GServer->Config.ban_time2 <= Counter){

                GServer->DB->QFree();
                GServer->DB->QExecute("UPDATE accounts SET accesslevel = '100' WHERE id = %i", account_id);
                GServer->DB->QExecute("UPDATE warning_system SET ban_type = '0' WHERE account_id = '%i'", account_id);
                Log(MSG_INFO,"Player ban has been lifted. Account id: %i",account_id);

            }
            else
            {
                Counter+=60;
                Log(MSG_INFO, "account id: %i, time: %i", account_id, Counter);
                GServer->DB->QFree();
                GServer->DB->QExecute("UPDATE warning_system SET time_remaining = %i WHERE account_id = %i", Counter, account_id);

            }
        }
        else if(ban_type == 0)
        {
            Log(MSG_INFO, "There are no active temporary bans");
            GServer->DB->QFree();
            return;
        }
    }
    if(row == NULL){
        Log(MSG_INFO, "No active temporary bans");
        GServer->DB->QFree();
        return;
    }
}

void Lotto()
{
    int number[6]={0,0,0,0,0,0};
    int tempNum;
    char LottoNumbers[100];

    srand(time(0));

    for(int i = 0; i < 6;i++)
    {
        number[i] = rand() % 30 + 1;
    }

    bool MayContinue = true;
    while(MayContinue)
    {
        MayContinue = false;
        if (number[0] == number[1]) change(number[0],MayContinue);
        if (number[0] == number[2]) change(number[0],MayContinue);
        if (number[0] == number[3]) change(number[0],MayContinue);
        if (number[0] == number[4]) change(number[0],MayContinue);
        if (number[0] == number[5]) change(number[0],MayContinue);

        if (number[1] == number[2]) change(number[1],MayContinue);
        if (number[1] == number[3]) change(number[1],MayContinue);
        if (number[1] == number[4]) change(number[1],MayContinue);
        if (number[1] == number[5]) change(number[1],MayContinue);

        if (number[2] == number[3]) change(number[2],MayContinue);
        if (number[2] == number[4]) change(number[2],MayContinue);
        if (number[2] == number[5]) change(number[2],MayContinue);

        if (number[3] == number[4]) change(number[3],MayContinue);
        if (number[3] == number[5]) change(number[3],MayContinue);

        if (number[4] == number[5]) change(number[4],MayContinue);
    }

    bool bSwapped = true;
    while (bSwapped)
    {
        bSwapped = FALSE;
        if (number[0] > number[1]) swap(number[0], number[1],bSwapped);
        if (number[1] > number[2]) swap(number[1], number[2],bSwapped);
        if (number[2] > number[3]) swap(number[2], number[3],bSwapped);
        if (number[3] > number[4]) swap(number[3], number[4],bSwapped);
        if (number[4] > number[5]) swap(number[4], number[5],bSwapped);
    }


    GServer->DB->QExecute("INSERT INTO LottoNumbers (number1,number2,number3,number4,number5,number6) VALUES(%i,%i,%i,%i,%i,%i)",number[0],number[1],number[2],number[3],number[4],number[5]);

    sprintf(LottoNumbers,"%i - %i - %i - %i - %i - %i" ,number[0],number[1],number[2],number[3],number[4],number[5]);

    BEGINPACKET( pak, 0x702 );
    ADDSTRING( pak, "Lotto Numbers " );
    ADDSTRING( pak, ": " );
    ADDSTRING( pak, LottoNumbers);
    ADDBYTE( pak, 0x00);
    GServer->SendToAll( &pak );

    LottoWinners(number);
}

void LottoWinners(int Number[])
{
    int numberGuess[6];
    int CorrectNumbers;
    char char_name[20];

    MYSQL_ROW GuessRow;
    MYSQL_RES *Guess = NULL;

    Guess = GServer->DB->QStore("SELECT char_name,number1,number2,number3,number4,number5,number6 FROM LottoEntries");
    GServer->DB->QFree();

    while(GuessRow = mysql_fetch_row(Guess))
    {
        CorrectNumbers = 0;
        sprintf(char_name,"%s",GuessRow[0]);

        for(int i = 1; i < 7;i++)
        {
            numberGuess[i-1]= atoi(GuessRow[i]);
        }

        for(int j = 0;j <6;j++)
        {
            if(numberGuess[j] == Number[0] || numberGuess[j] == Number[1] || numberGuess[j] == Number[2] || numberGuess[j] == Number[3] || numberGuess[j] == Number[4] || numberGuess[j] == Number[5])
            {
                CorrectNumbers++;
            }
        }

        UpdateWinners(char_name, CorrectNumbers);
    }
}

void UpdateWinners(char name[20], int number)
{
    if(number > 2)
    {
        GServer->DB->QExecute("INSERT INTO LottoWinners (char_name, NumbersCorrect) VALUES('%s',%i)",name,number);
    }
    return;
}

void GivePrizes()
{
    char name[20];
    int number;

    MYSQL_ROW WinnerRow;
    MYSQL_RES *Winners = NULL;

    Winners = GServer->DB->QStore("SELECT char_name,NumbersCorrect FROM LottoWinners");
    GServer->DB->QFree();
    while(WinnerRow = mysql_fetch_row(Winners))
    {
        sprintf(name,"%s",WinnerRow[0]);
        number = atoi(WinnerRow[1]);
        if(GServer->Config.LottoType == 1)
        {
            GiveZuliePrize(name, number);
        }
    }
    GServer->DB->QExecute("UPDATE Lotto_Config SET active = 0");
    GServer->Config.LottoActive = 0;
}

void DeleteLottoRecords()
{
    GServer->DB->QExecute("DELETE FROM LottoNumbers");
    GServer->DB->QExecute("DELETE FROM LottoEntries");
    GServer->DB->QExecute("DELETE FROM LottoWinners");
    //GServer->DB->QExecute("DELETE FROM LottoPrize");
}

void swap(int& a,int& b, bool& swapped)
{
    int t = a;
    a = b;
    b = t;
    swapped = true;
}

void change(int& c,bool& MayChange)
{
    srand(time(0));
    c = rand() % 30 + 1;
    MayChange = true;
}

void LottoAnn()
{
    GServer->Config.lastLastLottoAnn = clock();

    BEGINPACKET( pak, 0x702 );

    if(GServer->Config.LottoType == 1)
    {
        ADDSTRING( pak, "There is a Zulie lottery active!" );
    }
    ADDSTRING( pak, "Type /lottohelp for more info! " );
    ADDBYTE( pak, 0x00);
    GServer->SendToAll( &pak );

}

void LoadLottoConfig()
{
    MYSQL_ROW LottoConfigRow;
    MYSQL_RES *LottoConfig = NULL;

    LottoConfig = GServer->DB->QStore("SELECT accesslevel,active,type,ZulieCost,ZulieLimit FROM lotto_config");
    GServer->DB->QFree();
    LottoConfigRow = mysql_fetch_row(LottoConfig);

    if(LottoConfig == NULL || LottoConfigRow == NULL)
    {
        GServer->Config.Command_Lotto = 299;
        GServer->Config.LottoActive = 0;
        GServer->Config.LottoType = 1;
        GServer->Config.ZulieCost = 1000000;
        GServer->Config.ZulieLimit = 100000000;
    }else
    {
        GServer->Config.Command_Lotto = atoi(LottoConfigRow[0]);
        GServer->Config.LottoActive = atoi(LottoConfigRow[1]);
        GServer->Config.LottoType = atoi(LottoConfigRow[2]);
        GServer->Config.ZulieCost = atoi(LottoConfigRow[3]);
        GServer->Config.ZulieLimit = atoi(LottoConfigRow[4]);
    }
}

void GiveZuliePrize(char name[20], int number)
{
    long long int prize;
    prize=GServer->Config.ZuliePrize;

    CPlayer* otherclient = GServer->GetClientByCharName( name );
    if(number == 6)
    {
        if(otherclient != NULL)
        {
            otherclient->CharInfo->Zulies += prize;

            BEGINPACKET( pak, 0x71d );
            ADDQWORD( pak, otherclient->CharInfo->Zulies );
            otherclient->client->SendPacket( &pak );

            BEGINPACKET( pak1, 0x702 );
            ADDSTRING( pak1, name );
            ADDSTRING( pak1, " won the Zulie lottery!" );
            ADDBYTE( pak1, 0x00);
            GServer->SendToAll( &pak1 );
            GServer->DB->QExecute("DELETE FROM LottoPrize");
        }

    }else
    {
        if(otherclient != NULL)
        {
            if(number == 3)
            {
                otherclient->CharInfo->Zulies += (prize*3/100);
                BEGINPACKET( pak, 0x71d );
                ADDQWORD( pak, otherclient->CharInfo->Zulies );
                otherclient->client->SendPacket( &pak );
            }else if(number == 4)
            {
                otherclient->CharInfo->Zulies += (prize*1/10);
                BEGINPACKET( pak, 0x71d );
                ADDQWORD( pak, otherclient->CharInfo->Zulies );
                otherclient->client->SendPacket( &pak );
            }else if(number == 5)
            {
                otherclient->CharInfo->Zulies += (prize*2/10);
                BEGINPACKET( pak, 0x71d );
                ADDQWORD( pak, otherclient->CharInfo->Zulies );
                otherclient->client->SendPacket( &pak );
            }
        }
    }
}

void UpdateZuliePrize(long long int amount)
{
    MYSQL_ROW Prizerow;
    MYSQL_RES *Prize = NULL;

    long long int PrizeAmount;

    Prize = GServer->DB->QStore("SELECT Zulie FROM LottoPrize");
    Prizerow = mysql_fetch_row(Prize);
    GServer->DB->QFree();

    if(Prize == NULL || Prizerow == NULL)
    {
        PrizeAmount = amount;
        GServer->DB->QExecute("INSERT INTO LottoPrize (Zulie) VALUES (%i)",PrizeAmount);
    }else
    {
        PrizeAmount = atoi(Prizerow[0]) + amount;
        if(PrizeAmount > GServer->Config.ZulieLimit)
        {
            PrizeAmount = GServer->Config.ZulieLimit;
        }
        GServer->DB->QExecute("UPDATE LottoPrize SET Zulie = %i",PrizeAmount);
    }
}

bool get_grandPrize()
{
    char tempChar[20];

    MYSQL_ROW GrandPrizeRow;
    MYSQL_RES* GrandPrize = NULL;

    GrandPrize = GServer->DB->QStore("SELECT Zulie FROM lottoprize");
    GrandPrizeRow = mysql_fetch_row(GrandPrize);
    GServer->DB->QFree();

    sprintf(tempChar, "%i", atoi(GrandPrizeRow[0]));

    if(GrandPrize != NULL || GrandPrizeRow != NULL)
    {
        BEGINPACKET( pak2, 0x702 );
        ADDSTRING(pak2, "The lottery grand prize is ");
        ADDSTRING(pak2, tempChar);
        ADDSTRING(pak2, " zulie!!!");
        ADDBYTE( pak2, 0x00);
        GServer->SendToAll( &pak2 );
    }

    return true;
}

bool Advertise()
{
	Log(MSG_INFO,"Ad started");
    if(GServer->Config.lastAd < GServer->Config.numAds && GServer->Config.numAds > 0 )
    {
        BEGINPACKET( pak, 0x0784 );
        ADDSTRING( pak, "" );
        ADDBYTE( pak, 0 );
        ADDSTRING( pak, GServer->Config.Adverd[GServer->Config.lastAd].c_str() );
        ADDBYTE( pak, 0 );
        GServer->SendToAll(&pak);

		GServer->Config.lastAd += 1;

    }else if(GServer->Config.numAds > 0)
    {
        GServer->Config.lastAd = 0;

        BEGINPACKET( pak, 0x0784 );
        ADDSTRING( pak, "" );
        ADDBYTE( pak, 0 );
        ADDSTRING( pak, GServer->Config.Adverd[GServer->Config.lastAd].c_str() );
        ADDBYTE( pak, 0 );
        GServer->SendToAll(&pak);
    }

    return true;
}

bool LoadAdverds()
{
    #ifndef ADS
    #define ADS
    ifstream in_ads;
    in_ads.open("ads.dat");
    #endif

    int i = 0;

    if(in_ads.fail())
    {
        GServer->Config.AdvertiseActive = 0;
        Log(MSG_ERROR,"Cant open ads.dat! Ads deactivated for this session!");
        return true;
    }

    while(!in_ads.eof())
    {
        getline(in_ads, GServer->Config.Adverd[i], '\n');
        i++;
    }

    GServer->Config.numAds = i;
    Log(MSG_INFO,"%i Ads loaded!",i);

    //in_ads.close();

    return true;
}

