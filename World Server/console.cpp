#include "WorldServer.h"

bool CWorldServer::handleCommand( char* buffer )
{
	char* command = strtok( buffer , " ");
	char* tmp;
	if (command==NULL) return true;
	if(strcasecmp( command, "closeserver" )==0)
	{
        Log( MSG_CONSOLE, "Closing server..." );
        isActive = false;
        return false;
    }
    if(strcasecmp( command, "ann" )==0)
    {
        char* from = strtok( NULL , " ");
        if(from==NULL)
        {
            Log (MSG_CONSOLE, "'ann' command: ann <name> <message>" );
            return true;
        }
        unsigned msgoffset = strlen(from)+5;
        return pakConsoleAnn( from, (char*)&buffer[msgoffset] );
    }
    if(strcasecmp( command, "kick" )==0)
    {
        char* name = strtok( NULL , " ");
        return pakConsoleKick( name );
    }
    if(strcasecmp( command, "hhOn" )==0)
    {
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0;
        int hhOn=atoi(tmp);

        if(hhOn == 1)
        {
            doubleExpMapActivate();
            Log(MSG_CONSOLE, "HH activated");
			Log(MSG_GMACTION, "Server :: /hhOn active");
        }
		if(hhOn == 0)
		{
			doubleExpMapDeactivate();
			GServer->Config.doubleExpActive = 0;
			Log(MSG_CONSOLE, "HH deactivated");
			Log(MSG_GMACTION, "Server :: /hhOn deactive");
		}
    }
    else
    if(strcasecmp( command, "adver" )==0)
    {
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0;
        int adver=atoi(tmp);

        if(adver == 1)
        {
            Config.AdvertiseActive = 1;
            DB->QExecute("UPDATE  list_config SET advertise_active = 1");
            Config.AdvertiseTime = clock();
            Advertise();

            Log (MSG_CONSOLE, "Advertise system active" );
        }else
        {
            Config.AdvertiseActive = 0;
            DB->QExecute("UPDATE list_config SET advertise_active = 0");
            Log (MSG_CONSOLE, "Advertise system deactivated" );
        }
        return true;
    }
    else
    if(strcasecmp( command, "loadads" )==0)
    {
            LoadAdverds();
            Log (MSG_CONSOLE, "Adverds loaded");
            return true;
    }
    else
    if(strcasecmp( command, "help" )==0)
    {
            Log (MSG_CONSOLE, "Console Commands Available:" );
            Log (MSG_CONSOLE, "'ann' command: ann <name> <message>" );
			Log (MSG_CONSOLE, "'kick' command: kick <char name> kicks a player." );
			Log (MSG_CONSOLE, "'hhon' command: hhon <0/1> turns HH on or off." );
            Log (MSG_CONSOLE, "'help' command: returns this list" );
            return true;
    }
    else

        Log( MSG_CONSOLE, "Command not handled" );
    return true;
}

// CONSOLE: Announcment
bool CWorldServer::pakConsoleAnn( char* from, char* message )
{
	BEGINPACKET( pak, 0x702 );
	ADDSTRING( pak, from );
	ADDSTRING( pak, "> " );
	ADDSTRING( pak, message );
	ADDBYTE( pak, 0x00);
	SendToAll( &pak );
	Log( MSG_CONSOLE, "Announcment sent" );
	return true;
}

bool CWorldServer::pakConsoleKick(char* name)
{
    Log( MSG_GMACTION, " Server : /kick %s" , name);
    CPlayer* otherclient = GetClientByCharName( name );
    if(otherclient==NULL)
        return true;
    BEGINPACKET( pak, 0x702 );
    ADDSTRING( pak, "You were disconnected from the server !" );
    ADDBYTE( pak, 0 );
    otherclient->client->SendPacket( &pak );

    RESETPACKET( pak, 0x707 );
    ADDWORD( pak, 0 );
    otherclient->client->SendPacket( &pak );

    otherclient->client->isActive = false;

    return true;
}
