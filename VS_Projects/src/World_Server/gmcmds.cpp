/*
    Rose Online Server Emulator
    Copyright (C) 2006,2007,2008,2009 OSRose Team http://www.dev-osrose.com

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

    developed with Main erose/hrose source server + some change from the original eich source
*/
#include "worldserver.h"
#include "player.h"
#include "datatypes.h"

// Parse our commands to their appropriate function
bool CWorldServer::pakGMCommand( CPlayer* thisclient, CPacket* P )
{

	char* tmp;
	char* command = strtok( (char*)&(*P).Buffer[1] , " ");
	if (command==NULL) return true;

	if(strcmp(command, "addquest") == 0) {
        if(Config.Command_Addquest > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        unsigned int questnum =atoi(tmp);
        Log( MSG_GMACTION, "add quest %i" , questnum);
        for(int i=0;i<10;i++){
            if (thisclient->quest.quests[i].QuestID != 0) continue;
            thisclient->quest.quests[i].QuestID = questnum;
            thisclient->quest.quests[i].StartTime = clock_t();
            Log( MSG_GMACTION, "quest added %i" , thisclient->quest.quests[i].QuestID);
            break;
        }
		return true;
    }
	if (strcmp(command, "allskill")==0) /* Give all Skill to a Player - By CrAshInSiDe */
    {
        if(Config.Command_AllSkill > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        Log( MSG_GMACTION, " %s : /allskill %s", thisclient->CharInfo->charname, name);
        return pakGMAllSkill(thisclient, name);
    }
	if (strcmp(command, "ani")==0){
        if(Config.Command_Ani > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        unsigned anid = atoi(tmp);
        Log( MSG_GMACTION, " %s : /ani %i" , thisclient->CharInfo->charname, anid);
        pakGMDoEmote( thisclient, anid );
		return true;
    }
    if (strcmp(command, "ann")==0){ // *** SEND A ANNOUNCEMENT ***
        if(Config.Command_Ann > thisclient->Session->accesslevel)
	       return true;
        Log( MSG_GMACTION, " %s : /ann %s" , thisclient->CharInfo->charname, &P->Buffer[5] );
		return pakGMAnn(thisclient, P);
    }
    if(strcmp(command, "atkModif")==0)
    {
        if(Config.Command_AtkModif > thisclient->Session->accesslevel)
           return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned attkSpeedModif=atoi(tmp);
	    Log( MSG_GMACTION, " AttkSpeedModif changed to %i by %s" , attkSpeedModif, thisclient->CharInfo->charname);
	    return pakGMChangeAtkSpeedModif(thisclient, attkSpeedModif);
	}
	if (strcmp(command, "b")==0)
    {
        if(Config.Command_Broadcast > thisclient->Session->accesslevel)
           return true;
        time_t seconds;
        seconds = time (NULL);
        int levelfee;
        if(thisclient->Stats->Level < 20)
        {
            levelfee = Config.Command_Fee1;

        }else if(thisclient->Stats->Level > 20 && thisclient->Stats->Level <= 50)
        {
            levelfee = Config.Command_Fee2;

        }else if(thisclient->Stats->Level > 50 && thisclient->Stats->Level <= 100)
        {
            levelfee = Config.Command_Fee3;

        }else if(thisclient->Stats->Level > 100 && thisclient->Stats->Level <= 200)
        {
            levelfee = Config.Command_Fee4;

        }else
        {
            levelfee = Config.Command_Fee5;
        }
        if(((thisclient->CharInfo->LastGlobal+Config.Command_GlobalTime) <= seconds && thisclient->CharInfo->Zulies >= levelfee) || thisclient->Session->accesslevel >= 300 )
        {
            thisclient->CharInfo->LastGlobal = time (NULL);
            char outputmsg[200];
            sprintf( outputmsg, "[broadcast] %s", &P->Buffer[3] );
            Log( MSG_INFO, "%s> %s %s", thisclient->CharInfo->charname, Config.Command_GlobalPrefix, &P->Buffer[3]);
            SendGlobalMSG(thisclient, outputmsg);
            if(thisclient->Session->accesslevel == 100 && thisclient->Stats->Level > 20)
			{
				char deduct[200];
				thisclient->CharInfo->Zulies = thisclient->CharInfo->Zulies - levelfee;
				sprintf (deduct, "Broadcast fee of %i zulie removed!", levelfee);
				SendSysMsg(thisclient, deduct);
				BEGINPACKET( pak, 0x71d );
                ADDQWORD( pak, thisclient->CharInfo->Zulies );
                thisclient->client->SendPacket( &pak );
			}
        }
        else
        {
            long int remaining = (Config.Command_GlobalTime-(seconds-thisclient->CharInfo->LastGlobal));
            char buffer2[200];
			char zulieneed[200];
            if(thisclient->CharInfo->Zulies < levelfee)
            {
                sprintf (zulieneed, "You need %i zulie to broadcast.", levelfee);
				SendSysMsg(thisclient, zulieneed);
            }
            else
            {
                sprintf ( buffer2, "Please wait %i seconds before sending another global message.", remaining );
				SendPM(thisclient, buffer2);
            }
        }
        return true;
    }
	if (strcmp(command, "ban")==0)
    {
        if(Config.Command_Ban > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
        {
        Log( MSG_GMACTION, " %s : /ban NOT ALLOWED" , thisclient->CharInfo->charname);
        char buffer[200];
        sprintf ( buffer, "ban NOT ALLOWED");
        SendPM(thisclient, buffer);
	    return true;
        }
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        Log( MSG_GMACTION, " %s : /ban %s" , thisclient->CharInfo->charname, name);
        return pakGMBan( thisclient, name );
	}
    if (strcmp(command, "exportstbstl")==0)
    {
        if(Config.Command_Ban > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
        {
            return true;
        }

        return pakGMExportSTBSTL(thisclient);
	}
    if (strcmp(command, "bonusxp")==0)
	{
         if(Config.Command_BonusXp > thisclient->Session->accesslevel)
	        return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        char* name = tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        unsigned int bonus =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        unsigned int time_min =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        unsigned int to_save =atoi(tmp);

        CPlayer* otherclient = GetClientByCharName(name);
        if (otherclient == NULL)
            return true;

        char buffer2[200];
        if (bonus==0)
        {
           otherclient->bonusxp=1;
           otherclient->timerxp=0;
           otherclient->wait_validation=0;
           otherclient->Saved=false;
            sprintf ( buffer2, "Your Bonus Xp has been deactivated.");
            SendPM(otherclient, buffer2);
        }
        else
        {
           otherclient->bonusxp=bonus;
           otherclient->wait_validation=0;
           //otherclient->timerxp=clock()+time_min*60*1000;
           otherclient->timerxp=time(NULL)+time_min*60;
           otherclient->Saved=false;
           if (to_save!=0)
              otherclient->Saved=true;
            sprintf ( buffer2, "You have a bonus xp (*%i) for %i minutes.",bonus,time_min);
            SendPM(otherclient, buffer2);
        }

        Log( MSG_GMACTION, " %s : /bonusxp %s %i %i %i" ,thisclient->CharInfo->charname, otherclient->CharInfo->charname,bonus,time_min,to_save );
        return true;
    }
	 //Example for Cart.
    if (strcmp(command, "cart")==0)     // get all castlegear parts
    {
        if(Config.Command_cart > thisclient->Session->accesslevel)
           return true;
        Log( MSG_GMACTION, " %s : /cart", thisclient->CharInfo->charname);
        BEGINPACKET( pak, 0);
        {
            MYSQL_ROW row;
            MYSQL_RES *result=NULL;
            result = DB->QStore("SELECT itemnumber, itemtype FROM list_cart_cg WHERE Cart_CG='Cart' AND isactive=1");
            if(result==NULL) return true;

            int good_slot=102;
            vector<int> slot_list;

            while( row = mysql_fetch_row(result) )
            {
                UINT itemtype=atoi(row[1]);
                UINT itemnum=atoi(row[0]);
                if(itemtype!=14||itemnum<=0)
                       continue;

                bool is_ok=false;
                for(int k=good_slot;k<=131;k++)
                {
                    if (thisclient->items[k].itemnum!=0)
                    {
                        continue;
                    }
                    else
                    {
                        good_slot=k;
                        slot_list.push_back(k);
                        is_ok=true;
                        break;
                    }

                }

                if(!is_ok)
                {
                    SendPM(thisclient,"Not enough place in PAT inventory!");
                    break;
                }

                thisclient->items[good_slot].itemnum = itemnum;
                thisclient->items[good_slot].itemtype = itemtype;
                thisclient->items[good_slot].refine = 0;
                thisclient->items[good_slot].durability = 40;
                thisclient->items[good_slot].lifespan = 100;
                thisclient->items[good_slot].count = 1;
                thisclient->items[good_slot].stats = 0;
                thisclient->items[good_slot].socketed = false;
                thisclient->items[good_slot].appraised = true;
                thisclient->items[good_slot].gem = 0;
                thisclient->UpdateInventory( good_slot,0xffff,false);   //We don't want to save now, mysql mutex...
                //Log(MSG_INFO,"Adding in slot %i item %i::%i",good_slot,itemtype,itemnum);
                RESETPACKET( pak, 0x7a5);
                ADDWORD( pak, thisclient->clientid );
                ADDWORD( pak, good_slot);
                ADDWORD( pak, itemnum);  // ITEM NUM
                ADDWORD( pak, BuildItemRefine( thisclient->items[good_slot] ));   // REFINE
                ADDWORD( pak, thisclient->Stats->Move_Speed );  // REFINE 2602
                SendToVisible( &pak,thisclient );
            }

            DB->QFree( );

            //LMA: we save the slots afterwards...
            for(int k=0;k<slot_list.size();k++)
            {
                thisclient->SaveSlot41(slot_list.at(k));
            }

            slot_list.clear();

            SendPM(thisclient, "Got Cart Parts!");
        }

        thisclient->SetStats( );
        return true;
    }
	 //Example for CG.
    if (strcmp(command, "cg")==0)     // get all castlegear parts
    {
        if(Config.Command_cg > thisclient->Session->accesslevel)
           return true;
        Log( MSG_GMACTION, " %s : /cg", thisclient->CharInfo->charname);
        BEGINPACKET( pak, 0);
        {
            MYSQL_ROW row;
            MYSQL_RES *result=NULL;
            result = DB->QStore("SELECT itemnumber, itemtype FROM list_cart_cg WHERE Cart_CG='CG' AND isactive=1");
            if(result==NULL) return true;

            int good_slot=102;
            vector<int> slot_list;

            while( row = mysql_fetch_row(result) )
            {
                UINT itemtype=atoi(row[1]);
                UINT itemnum=atoi(row[0]);
                if(itemtype!=14||itemnum<=0)
                       continue;

                bool is_ok=false;
                for(int k=good_slot;k<=131;k++)
                {
                    if (thisclient->items[k].itemnum!=0)
                    {
                        continue;
                    }
                    else
                    {
                        good_slot=k;
                        slot_list.push_back(k);
                        is_ok=true;
                        break;
                    }

                }

                if(!is_ok)
                {
                    SendPM(thisclient,"Not enough place in PAT inventory!");
                    break;
                }

                thisclient->items[good_slot].itemnum = itemnum;
                thisclient->items[good_slot].itemtype = itemtype;
                thisclient->items[good_slot].refine = 0;
                thisclient->items[good_slot].durability = 40;
                thisclient->items[good_slot].lifespan = 100;
                thisclient->items[good_slot].count = 1;
                thisclient->items[good_slot].stats = 0;
                thisclient->items[good_slot].socketed = false;
                thisclient->items[good_slot].appraised = true;
                thisclient->items[good_slot].gem = 0;
                thisclient->UpdateInventory( good_slot,0xffff,false);   //We don't want to save now, mysql mutex...
                //Log(MSG_INFO,"Adding in slot %i item %i::%i",good_slot,itemtype,itemnum);
                RESETPACKET( pak, 0x7a5);
                ADDWORD( pak, thisclient->clientid );
                ADDWORD( pak, good_slot);
                ADDWORD( pak, itemnum);  // ITEM NUM
                ADDWORD( pak, BuildItemRefine( thisclient->items[good_slot] ));   // REFINE
                ADDWORD( pak, thisclient->Stats->Move_Speed );  // REFINE 2602
                SendToVisible( &pak,thisclient );
            }

            DB->QFree( );

            //LMA: we save the slots afterwards...
            for(int k=0;k<slot_list.size();k++)
            {
                thisclient->SaveSlot41(slot_list.at(k));
            }

            slot_list.clear();

            SendPM(thisclient, "get CastleGear Parts!");
        }

        thisclient->SetStats( );
        return true;
    }
    if(strcmp(command, "CharInfo")==0)
    {
        if(Config.Command_Item > thisclient->Session->accesslevel)
	        return true;
		if((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
		Log( MSG_GMACTION, " %s : /CharInfo %s" , thisclient->CharInfo->charname, name);
		return pakGMInfo(thisclient, name);
	}
    if(strcmp(command, "cfmode")==0)
    {
        if(Config.Command_Cfmode > thisclient->Session->accesslevel)
	       return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned mode=atoi(tmp);
	    Log( MSG_GMACTION, " Clan field mode changed to %i by %s" , mode, thisclient->CharInfo->charname);
	    return pakGMChangeCfmode(thisclient, mode);
	}
    if (strcmp(command, "cha")==0)
    {
        if(Config.Command_Cha > thisclient->Session->accesslevel)
	       return true;
        int slot;
        int tipo;
        int id;
        int stats;
        int socket;
        int refine;
        if ((tmp = strtok(NULL, " "))==NULL)return true;
        if(strcmp(tmp, "mask")==0)
        {
            slot=1;
            tipo=1;
        }
        else
        if(strcmp(tmp, "cap")==0)
        {
            slot=2;
            tipo=2;
        }
        else
        if(strcmp(tmp, "suit")==0)
        {
            slot=3;
            tipo=3;
        }
        else
        if(strcmp(tmp, "back")==0)
        {
            slot=4;
            tipo=6;
        }
        else
        if(strcmp(tmp, "glov")==0)
        {
            slot=5;
            tipo=4;
        }
        else
        if(strcmp(tmp, "shoe")==0)
        {
            slot=6;
            tipo=5;
        }
        else
        if(strcmp(tmp, "weap")==0)
        {
            slot=7;
            tipo=8;
        }
        else
        if(strcmp(tmp, "shield")==0)
        {
            slot=8;
            tipo=9;
        }
        else
            return true;
        if ((tmp = strtok(NULL, " "))==NULL)return true;
            id=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL)
            stats=0;
        else
            stats=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL)
            socket=0;
        else
            socket=1;
        if ((tmp = strtok(NULL, " "))==NULL)
            refine=0;
        else
            refine=atoi(tmp);
        thisclient->items[slot].itemnum = id;
        thisclient->items[slot].itemtype = tipo;
        thisclient->items[slot].refine = refine;
        thisclient->items[slot].durability = 50;
        thisclient->items[slot].lifespan = 100;
        thisclient->items[slot].count = 1;
        thisclient->items[slot].stats = stats;
        thisclient->items[slot].socketed = socket;
        thisclient->items[slot].appraised = true;
        thisclient->items[slot].gem = 0;
        thisclient->items[slot].sp_value = 0;
        BEGINPACKET( pak, 0x7a5);
        ADDWORD( pak, thisclient->clientid );
        ADDWORD( pak, slot);
        ADDWORD( pak, id);	// ITEM NUM
        ADDWORD( pak, BuildItemRefine(  thisclient->items[slot] ));	// REFINE
        ADDWORD( pak, thisclient->Stats->Move_Speed );	// REFINE 2602
        SendToVisible( &pak,thisclient );

        thisclient->UpdateInventory( slot );
        thisclient->SetStats( );
        SendPM( thisclient, "GM command Cha Type %i ID %i", tipo, id);
		return true;
    }
    if (strcmp(command, "class")==0)
    {
        if(Config.Command_Class > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL)
            return true;
        char* classid=(char*)tmp;
        return pakGMClass( thisclient, classid );
    }
    if (strcmp(command, "configreset")==0)    // configreset - by PurpleYouko
   // *** RELOAD DATA FILES ******
    {
         if(Config.Command_ConfigReset > thisclient->Session->accesslevel)
            return true;
         char* action;
         bool is_ok=false;
         UINT newval=0;
         //char* buffer;
         char buffer[200];
         if ((tmp = strtok(NULL, " ")) == NULL)return true; action = tmp;
         if(strcmp(action, "maxlevel")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.MaxLevel = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET maxlevel=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "xprate")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.EXP_RATE = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET exp_rate=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "itemdroprate")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.ItemDropRate = newval;
                GServer->DB->QExecute("UPDATE list_config SET item_drop_rate = %i WHERE conf = '%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "droprate")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.DROP_RATE = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET drop_rate=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "droptype")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.DROP_TYPE = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET drop_type=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "zulyrate")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.ZULY_RATE = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET zuly_rate=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "savetime")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.SaveTime = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET savetime=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "partygap")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.Partygap = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET Partygap=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "maxstat")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.MaxStat = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET MaxStat=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "playerdmg")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.PlayerDmg = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET player_damage=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "monsterdmg")==0)
         {
			if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
			{
				Config.MonsterDmg = newval;
				//char* confname = "default";
				GServer->DB->QExecute("UPDATE list_config SET monster_damage=%i WHERE conf='%s'",newval,"default");
				is_ok=true;
			}
         }
         if(strcmp(action, "cfmode")==0)
         {
            if ((tmp = strtok(NULL, " ")) == NULL) return true; newval = atoi(tmp);
            {
                Config.Cfmode = newval;
                //char* confname = "default";
                GServer->DB->QExecute("UPDATE list_config SET cfmode=%i WHERE conf='%s'",newval,"default");
                is_ok=true;
            }
         }
         if(strcmp(action, "cmd")==0)
         {
            LoadConfigurations( "commands.ini" );
            is_ok=true;
         }

         if(!is_ok)
         {
             Log( MSG_INFO, "Unrecognized configreset command by GM %s" , thisclient->CharInfo->charname);
             //sprintf ( buffer, "/configreset %s is not a valid command", tmp );
             sprintf ( buffer, "configreset %s is not a valid command", action );
             SendPM(thisclient, buffer);
             return true;
         }

         Log( MSG_GMACTION, " %s : /configreset %s" , thisclient->CharInfo->charname, action);
         sprintf ( buffer, "Configreset: %s has been set to %i", action, newval  );
         SendPM(thisclient, buffer);
         return true;
    }
    if(strcmp(command, "convert")==0)
    {
        if(Config.Command_Convert > thisclient->Session->accesslevel)
	       return true;
	    if(thisclient->Battle->target==0) return true;
        if ((tmp = strtok(NULL, " " ))==NULL)
            return true;
        UINT newmon = atoi(tmp);
        if(newmon==0) return true;
        CMonster* thismon = GetMonsterByID( thisclient->Battle->target, thisclient->Position->Map );
        if(thismon==NULL)
        {
            CNPC* thisnpc = GetNPCByID( thisclient->Battle->target, thisclient->Position->Map );
            if(thisnpc==NULL) return true;
            MapList.Index[thisclient->Position->Map]->ConverToMonster( thisnpc, newmon );
        }
        else
        {
            MapList.Index[thisclient->Position->Map]->ConverToMonster( thismon, newmon );
        }
        return true;
    }
    if ( strcmp(command, "debuff")==0)
    {
        if(Config.Command_Debuff > thisclient->Session->accesslevel)
           return true;
        Log(MSG_GMACTION, "debuff : character [ %s ]", thisclient->CharInfo->charname);
        pakGMDebuff(thisclient);
        return true;
    }
    if(strcmp(command, "serverdebug")==0)
    {
        //devs only
        if(thisclient->Session->accesslevel < 900)
	       return true;
        char buffer2[200];
        if( GServer->ServerDebug )
        {
            GServer->ServerDebug = false;

            sprintf ( buffer2, "Global Debug Mode disabled" );
            SendPM(thisclient, buffer2);
        }
        else
        {
            GServer->ServerDebug = true;
            sprintf ( buffer2, "Global Debug Mode enabled" );
            SendPM(thisclient, buffer2);
        }
        return true;
    }
    if(strcmp(command, "debugmode")==0)
    {
        //if(Config.Command_debugmode > thisclient->Session->accesslevel)
        if(thisclient->Session->accesslevel <= 100)
	       return true;
        char buffer2[200];
        if( thisclient->Session->codedebug )
        {
            thisclient->Session->codedebug = false;

            sprintf ( buffer2, "Debug Mode disabled" );
            SendPM(thisclient, buffer2);
        }
        else
        {
            thisclient->Session->codedebug = true;
            sprintf ( buffer2, "Debug Mode enabled" );
            SendPM(thisclient, buffer2);
        }
        return true;
    }
    if (strcmp(command, "delskills")==0) /* Remove all Skill from a Player - modified by rl2171 */
    {
        if(Config.Command_DelSkills > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        Log( MSG_GMACTION, " %s : /delskills %s", thisclient->CharInfo->charname, name);
        return pakGMDelSkills(thisclient, name);
    }
    if(strcmp(command, "DELETESPAWN")==0)
    {
         if(Config.Command_DelSpawn > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
           {
           Log( MSG_GMACTION, " %s : /DELETESPAWN NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "DELETESPAWN NOT ALLOWED");
           SendPM(thisclient, buffer);
	        return true;
           }
        if ((tmp = strtok(NULL, " "))==NULL)
             return true;
        int id=atoi(tmp);
        DB->QExecute("DELETE from list_spawnareas where id=%i",id);
//		BEGINPACKET( pak, 0x702 );
        char buffer[200];
        sprintf( buffer, "DELETESPAWN %i", id);
        BEGINPACKET ( pak, 0x702 );
        ADDSTRING( pak, buffer );
//		ADDSTRING( pak, "DELETESPAWN" );
		ADDBYTE( pak, 0 );
        thisclient->client->SendPacket(&pak);
        CSpawnArea* thisspawn = GetSpawnArea( id );
        if(thisspawn==NULL)
            return true;
        CMap* map = MapList.Index[thisspawn->map];
        for(UINT i=0;i<map->MonsterList.size();i++)
        {
            CMonster* thismon = map->MonsterList.at(i);
            BEGINPACKET( pak, 0x794 );
            ADDWORD    ( pak, thismon->clientid );
            SendToVisible( &pak, thismon );
            MapList.Index[thisspawn->map]->DeleteMonster( thismon );
        }
        DeleteSpawn( thisspawn );
        Log( MSG_GMACTION, " %s : /deletespawn %i" , thisclient->CharInfo->charname, id);
		return true;
    }
    if (strcmp(command, "dev")==0)
    {
        if(Config.Command_Dev > thisclient->Session->accesslevel)
            return true;
        char buffer[200];
		sprintf ( buffer, "osRose Version %s", Config.osRoseVer);
		SendPM(thisclient, buffer);
		return true;
    }
    if(strcmp(command, "dquest")==0)
    {
         if(Config.Command_DQuest > thisclient->Session->accesslevel )
           {
           Log( MSG_GMACTION, " %s : /dquest NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "dquest NOT ALLOWED");
           SendPM(thisclient, buffer);
	        return true;
           }
        char line0[200];

        if( thisclient->questdebug )
        {
            Log(MSG_INFO,"Quest Debug deactivated");
            sprintf(line0, "Quest Debug deactivated");
            thisclient->questdebug = false;
            GServer->questdebug = false;
        }
        else
        {
            Log(MSG_INFO,"Quest Debug activated");
            sprintf(line0, "Quest Debug activated");
            thisclient->questdebug = true;
            GServer->questdebug = true;
        }

        SendPM(thisclient, line0 );
		return true;
    }
    if (strcmp(command, "aiwatch")==0) // set a skill for mobs to use. testing
    {
        //SendPM(thisclient, "trying to use aiwatch");
        if(thisclient->Session->accesslevel < 900)
            return true;
        if ((tmp = strtok(NULL, " "))==NULL)return true;
        UINT newval = atoi(tmp);
        Config.AIWatch = newval;
        SendPM(thisclient, "AI script watching set to %i",newval);
		return true;
    }
    if(strcmp(command, "qdrop")==0) // Sends a packet to the client to add/subtract a quest drop.
    {   //be extremely careful to enter the correct quest trigger. Use only from QN files.
        char* qname;
        char buffer[200];
        if(thisclient->Session->accesslevel >= 900) //dev only.
        {
            if ((tmp = strtok(NULL, " ")) == NULL)
            {
                sprintf ( buffer, "%s is not a valid trigger name",tmp);
                SendPM(thisclient, buffer);
                return true;
            }
            else
            {
                qname = tmp;
            }
            Log( MSG_GMACTION, " %s : used /Qdrop" , thisclient->CharInfo->charname);

            sprintf ( buffer, "0x730 packet sent for quest trigger %s",qname);
            SendPM(thisclient, buffer);
            int success = 5;
            dword hash = MakeStrHash(qname);

            BEGINPACKET ( pak, 0x730);
            ADDBYTE ( pak, success);
            ADDBYTE ( pak, 0);
            ADDDWORD( pak, hash);
            thisclient->client->SendPacket(&pak);
            return true;
        }
        sprintf ( buffer, "%s : /qdrop not allowed",thisclient->CharInfo->charname);
        SendPM(thisclient, buffer);
        return true;
    }
    if (strcmp(command, "drop")==0)
    {
         if(Config.Command_Drop > thisclient->Session->accesslevel)
            return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned itemtype=atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned itemid=atoi(tmp);
        Log( MSG_GMACTION, " %s : /drop %i,%i" , thisclient->CharInfo->charname, itemtype, itemid);

		CDrop* thisdrop = new CDrop;
		assert(thisdrop);
		thisdrop->clientid = GetNewClientID();
		thisdrop->type = 2;
		thisdrop->pos.x = thisclient->Position->current.x;
		thisdrop->pos.y = thisclient->Position->current.y;
		thisdrop->posMap = thisclient->Position->Map;
		thisdrop->droptime = time(NULL);

		ClearItem(thisdrop->item);
		thisdrop->item.itemnum = itemid;
		thisdrop->item.itemtype = itemtype;
		thisdrop->item.count = 1;
		thisdrop->item.refine = 0;
		thisdrop->item.durability = 35;
		thisdrop->item.lifespan = 100;
		thisdrop->item.appraised = true;
		thisdrop->item.socketed = false;
		thisdrop->item.stats = 0;
		thisdrop->item.gem = 0;

		thisdrop->amount = 1;
		thisdrop->owner = 0;
		CMap* map = MapList.Index[thisdrop->posMap];
		map->AddDrop( thisdrop );
		return true;
	}
    if(strcmp(command, "testmon")==0)
    {
        //LMA: just a test.
        if ((tmp = strtok(NULL, " "))==NULL)
        {
            //summoning monster
            Log(MSG_INFO,"testmon time");
            return pakGMMon( thisclient, 2, 1,0 );
        }
        else
        {
            UINT monster_clientid=atoi(tmp);
            tmp = strtok(NULL, " ");
            UINT type=atoi(tmp);
            CCharacter* Enemy=GetMonsterByID(monster_clientid,0);
            if(Enemy==NULL)
            {
                Log(MSG_INFO,"ClientID %i not found!",monster_clientid);
                return true;
            }

            //We set his HP to 0 and kill it (yeah die you scum...)
            if (type==1)
            {
                BEGINPACKET( pak, 0x79f );
                ADDWORD    ( pak, Enemy->clientid );
                ADDDWORD   ( pak, 1);
                GServer->SendToVisible( &pak, Enemy );

                RESETPACKET( pak, 0x799 );
                ADDWORD    ( pak, thisclient->clientid );
                ADDWORD    ( pak, Enemy->clientid );
                ADDDWORD   ( pak, 2 );
                ADDDWORD   ( pak, 16 );
                GServer->SendToVisible( &pak, Enemy );
                Log(MSG_INFO,"is monster still there?");
            }
            else
            {
                BEGINPACKET( pak, 0x79f );
                ADDWORD    ( pak, Enemy->clientid );
                ADDDWORD   ( pak, 1);
                GServer->SendToVisible( &pak, Enemy );

                RESETPACKET( pak, 0x799 );
                ADDWORD    ( pak, 0 );
                ADDWORD    ( pak, Enemy->clientid );
                ADDDWORD   ( pak, 2 );
                ADDDWORD   ( pak, 16 );
                GServer->SendToVisible( &pak, Enemy );
                Log(MSG_INFO,"is monster still there 2?");
            }

        }
		return true;
    }
    if (strcmp(command, "event")==0) //==== Trigger Events (credit Welson)
    {
        //SendPM(thisclient, "trying to use event command");
        if(Config.Command_Event > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        int npctype=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        int dialog=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        long int type=atoi(tmp);
        Log( MSG_GMACTION, " %s : /event %i %i %i" ,thisclient->CharInfo->charname, npctype,dialog,type);
        SendPM(thisclient, "Event set. NPC [%i] dialog [%i] set to value %i", npctype,dialog,type);
        return pakGMEventType(thisclient,npctype,dialog,type);
    }
    if (strcmp(command, "eventname")==0) //==== Trigger Events (credit Welson)
    {
        if(Config.Command_EventName > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        char* name=tmp;
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        int is_on=atoi(tmp);
        if (is_on!=0)
        {
            is_on=1;
        }

        Log( MSG_GMACTION, " %s : /eventname %s %i" ,thisclient->CharInfo->charname, name,is_on);

        return pakGMEventName(thisclient,name,is_on);
    }
    if (strcmp(command,"readobjvar")==0) //PY code added to read values in NPC Object Variables (dialogs) Set access level very high
    {
        //SendPM(thisclient, "trying to use readobjvar");
        if(thisclient->Session->accesslevel < 900)
            return true;
        if ((tmp = strtok(NULL, " ")) == NULL)
            return true;
        int objVar = atoi(tmp);
        //if (objVar > 1999)return true;
        if ((tmp = strtok(NULL, " ")) == NULL)
            return true;
        int thisIndex = atoi(tmp);
        if (thisIndex > 19)return true;
        int thisValue = GServer->ObjVar[objVar][thisIndex];
        SendPM(thisclient, "Global Object Variable [%i][%i] current value is %i" , objVar,thisIndex,thisValue);
		return true;
    }
    if (strcmp(command,"setobjvar")==0) //PY code added to set values in NPC Object Variables (dialogs) Set access level very high. Effects duplicate the event command
    {
        //SendPM(thisclient, "trying to use setobjvar");
        if(thisclient->Session->accesslevel < 900)
            return true;
        if ((tmp = strtok(NULL, " ")) == NULL)
            return true;
        int objVar = atoi(tmp);
        //if (objVar > 1999)return true;
        if ((tmp = strtok(NULL, " ")) == NULL)
            return true;
        int thisIndex = atoi(tmp);
        if (thisIndex > 19)return true;
        if ((tmp = strtok(NULL, " ")) == NULL)
            return true;
        int thisValue = atoi(tmp);
        SendPM(thisclient, "Global Object Variable [%i][%i] set to value %i" , objVar,thisIndex,thisValue);
        GServer->ObjVar[objVar][thisIndex] = thisValue;
		return true;
    }
    if (strcmp(command, "eventifo")==0) //LMA: For IFO Objects
    {
        if(Config.Command_EventIfo > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        int npctype=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        int eventID=atoi(tmp);
        Log( MSG_GMACTION, " %s : /eventifo %i %i" ,thisclient->CharInfo->charname, npctype,eventID);
        return pakGMEventIFO(thisclient, npctype,eventID);
    }
    if (strcmp(command, "exp")==0)
    {
        if(Config.Command_Exp > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned exp=atoi(tmp);
        char* name;
        if ((tmp = strtok(NULL, " "))==NULL)
            name = thisclient->CharInfo->charname;
        else
            name = tmp;
        CPlayer* otherclient = GetClientByCharName( name );
        if (otherclient == NULL)
            return true;
        otherclient->CharInfo->Exp += exp;
        BEGINPACKET( pak, 0x79b );
        ADDDWORD   ( pak, otherclient->CharInfo->Exp );
        ADDWORD    ( pak, otherclient->CharInfo->stamina );
        ADDWORD    ( pak, 0 );
        otherclient->client->SendPacket( &pak );
        Log( MSG_GMACTION, " %s : /exp %i %s" , thisclient->CharInfo->charname, exp, name);
		return true;
    }
    if(strcmp(command, "face")==0)
    {
        if(Config.Command_Face > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " ")) == NULL) return true; UINT face=atoi(tmp);
        if (face > 12) return true;
        UINT style;
        if ((tmp = strtok(NULL, " ")) == NULL) style = 0; else style = atoi(tmp);
        if (style > 3) style = 3;
        thisclient->CharInfo->Face = ((face*7) + style + 1);
        BEGINPACKET(pak, 0x721);
        ADDWORD(pak, 8);
        ADDWORD(pak, thisclient->CharInfo->Face);
        ADDWORD(pak, 0);
        thisclient->client->SendPacket(&pak);
        RESETPACKET(pak, 0x0730);
        ADDWORD(pak, 5);
        ADDWORD(pak, 0xa24d);
        ADDWORD(pak, 0x40b3);
        thisclient->client->SendPacket(&pak);
        SendPM(thisclient, "Face changed!");
        DB->QExecute("UPDATE characters SET face=%i WHERE id=%i", thisclient->CharInfo->Face, thisclient->CharInfo->charid);
		return true;
    }
    if(strcmp(command, "fairymode")==0)
    {
        if(Config.Command_ManageFairy > thisclient->Session->accesslevel)
	       return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned mode=atoi(tmp);
	    Log( MSG_GMACTION, " %s : /fairymode mode: %i" , thisclient->CharInfo->charname, mode);
	    return pakGMManageFairy(thisclient, mode);
	}
    if(strcmp(command, "fairystay")==0)
    {
        if(Config.Command_ChangeFairyStay > thisclient->Session->accesslevel)
	       return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned value=atoi(tmp);
	    Log( MSG_GMACTION, " %s : /fairystay value: %i" , thisclient->CharInfo->charname, value);
	    return pakGMChangeFairyStay(thisclient, value);
	}
    if(strcmp(command, "fairytestmode")==0)
    {
        if(Config.Command_ChangeFairyTestMode > thisclient->Session->accesslevel)
	       return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned mode=atoi(tmp);
	    Log( MSG_GMACTION, " %s : /fairytestmode mode: %i" , thisclient->CharInfo->charname, mode);
	    return pakGMChangeFairyTestMode(thisclient, mode);
	}
    if(strcmp(command, "fairywait")==0)
    {
        if(Config.Command_ChangeFairyWait > thisclient->Session->accesslevel)
	       return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned value=atoi(tmp);
	    Log( MSG_GMACTION, " %s : /fairywait value: %i" , thisclient->CharInfo->charname, value);
	    return pakGMChangeFairyWait(thisclient, value);
	}
    if (strcmp(command, "fskill")==0)
    {
         //forcing a skill (for test)
         if(Config.Command_fskill > thisclient->Session->accesslevel)
	        return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
            unsigned monstertype =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true;
            unsigned skilltype =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true;
            unsigned skillvalue =atoi(tmp);
        fmmonstertype=monstertype;         //montype
        ftypeskill=skilltype;              //1=attack, 2=debuff, 3=self buff
        fskill=skillvalue;                 //skill value.
        Log( MSG_GMACTION, " %s : /fskill monter %i type %i skill %i" , thisclient->CharInfo->charname, monstertype,skilltype,skillvalue);
        return true;
    }
    if (strcmp(command, "give2")==0)
    {
        if(Config.Command_Item > thisclient->Session->accesslevel)
           return true;
        UINT itemrefine, itemstats, itemls, itemsocket;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true; UINT itemid =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true; UINT itemtype =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true; UINT itemamount =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL)
            itemrefine =0;
        else
            itemrefine = atoi(tmp)<10?atoi(tmp)*16:9*16;
        if ((tmp = strtok(NULL, " "))==NULL)
            itemls =100;
        else
            itemls = atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL)
            itemsocket =0;
        else
            itemsocket =atoi(tmp)==0?false:true;
        if ((tmp = strtok(NULL, " "))==NULL)
            itemstats =0;
        else
            itemstats =atoi(tmp);
        Log( MSG_GMACTION, " %s : /give2 %s,%i,%i,%i,%i,%i,%i,%i" , thisclient->CharInfo->charname, name, itemid , itemtype , itemamount , itemrefine , itemls, itemstats , itemsocket);
        return pakGMItemtoplayer( thisclient , name , itemid , itemtype , itemamount , itemrefine , itemls, itemstats , itemsocket );
    }
    if(strcmp(command, "giveclanp")==0)
    {
        if(Config.Command_GiveClanp > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
	    if((tmp = strtok(NULL, " "))==NULL) return true; int points=atoi(tmp);
	    Log( MSG_GMACTION, " %s : /giveclanp %s, %i" , thisclient->CharInfo->charname, name, points);
	    return pakGMClanPoints(thisclient, name, points);
	}
    if(strcmp(command, "giveclanrp")==0)
    {
        if(Config.Command_GiveClanRp > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
	    if((tmp = strtok(NULL, " "))==NULL) return true; int points=atoi(tmp);
	    Log( MSG_GMACTION, " %s : /giveclanrp %s, %i" , thisclient->CharInfo->charname, name, points);
	    return pakGMClanRewardPoints(thisclient, name, points);
	}
    if(strcmp(command, "givefairy")==0)
    {
        if(Config.Command_GiveFairy > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned mode=atoi(tmp);
	    Log( MSG_GMACTION, " %s : /givefairy %s, %i" , thisclient->CharInfo->charname, name, mode);
	    return pakGMFairyto(thisclient, name, mode);
	}
    if(strcmp(command, "givezuly")==0)
    {
        if(Config.Command_GiveZuly > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
		//if((tmp = strtok(NULL, " "))==NULL) return true; int zuly=atoi(tmp);
		if((tmp = strtok(NULL, " "))==NULL) return true; long long zuly=atoll(tmp);
        Log( MSG_GMACTION, " %s : /givezuly %s, %I64i" , thisclient->CharInfo->charname, name, zuly);
		return pakGMZulygive(thisclient, name, zuly);
	}
    if(strcmp(command, "gmlist")==0) /* GM List {By CrAshInSiDe} */
    {
        if(Config.Command_GmList > thisclient->Session->accesslevel)
           return true;
        SendPM(thisclient, "Currently connected GMs:");
        int count=1;
        int nb_gms=0;
        int hiddenam=0;
        char line0[200];
        while(count <= (ClientList.size()-1))
        {
            char line1[200];
            CPlayer* whoclient = (CPlayer*)ClientList.at(count)->player;
            if(whoclient->Session->accesslevel > 100)
            {
                sprintf(line1, "%s - GM[%i]", whoclient->CharInfo->charname, whoclient->Session->accesslevel);
                if(whoclient->isInvisibleMode2 != true)
                {
                    SendPM(thisclient, line1 );
                    nb_gms++;
                }
                else
                {
                    hiddenam++;
                }
            }
            count++;
        }
        sprintf(line0, "There are currently %i GM connected!",nb_gms);
        Log( MSG_GMACTION, " %s : /gmlist" , thisclient->CharInfo->charname);
        SendPM(thisclient, line0 );
        return true;
    }
    if (strcmp(command, "gmskills")==0)
    {
        if(Config.Command_GMSkills > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
        {
           Log( MSG_GMACTION, " %s : /gmskills NOT ALLOWED" , thisclient->CharInfo->charname);
           return true;
        }

        Log(MSG_INFO,"gmskills: %s:: config_smskills=%u, player access level=%u, is he GM? %u",thisclient->CharInfo->charname,Config.Command_GMSkills,thisclient->Session->accesslevel,thisclient->CharInfo->isGM);


           /*
           char buffer[200];
           sprintf ( buffer, "gmskills NOT ALLOWED");
           SendPM(thisclient, buffer);
	        return true;
	        */
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        Log( MSG_GMACTION, " %s : /gmskills %s", thisclient->CharInfo->charname, name);
        return pakGMGMSkills(thisclient, name);
    }
    if (strcmp(command, "go")==0) // AtCommandGo
    {
        if(Config.Command_go > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; 
			int loc = atoi(tmp);
        if(Config.Command_Go > thisclient->Session->accesslevel)
            return true;
        int x = 0;
        int y = 0;
        int map = 0;
        if(loc == 1) // Adventure Plains
        {
            map = 22;
            x = 5800; //old map
            y = 5200; // old map
            //x = 5098; // new map
            //y = 5322; // new map
        }
        else if(loc == 2) // Canyon City of Zant
        {
            map = 1;
            x = 5240;
            y = 5192;
        }
        else if (loc == 3) // Junon Polis
        {
            map = 2;
            x = 5513;
            y = 5368;
        }
        else if (loc == 4) // Magic City of Eucar - Luna
        {
            if (thisclient->Stats->Level<50){
				SendPM(thisclient, "You need to be a least Level 50 to visit Eucar!");
				return true;
            }
			map = 51;
			x = 5357;
			y = 5013;
        }
        else if (loc == 5) // Xita Refuge - Eldeon
        {
             if (thisclient->Stats->Level<120)
            {
				SendPM(thisclient, "You need to be a least Level 120 to visit Eldeon!");
				return true;
            }
            map = 61;
            x = 5434;
            y = 4569;
        }
        else if (loc == 6) // Training Grounds
        {
            map = 6;
            x = 5199;
            y = 5280;
        }
        else if (loc == 7) // Lions Plains
        {
            if(thisclient->Session->accesslevel > 299)
            {
                map = 8;
                x = 5160;
                y = 5080;
            } else {
                SendPM(thisclient,"You need to be GM to access this map");
                return true;
            }
        }
        else if (loc == 8) // Luna Clan Field
        {
             //if (thisclient->Stats->Level<120)
             if (thisclient->Stats->Level<120 && thisclient->Stats->Level>160)
             // Luna Clan Field actually restricted to from 120 - 160 only, will above work?
             {
             SendPM(thisclient, "You need to be between Level 120 and 160 to visit Luna Clan Field!");
             return true;
             }
            map = 59;
            x = 5095;
            y = 5128;
		}
        else if (loc == 9) // Desert of the Dead
        {
            map = 29;
            x = 5093;
            y = 5144;
        }
        //Added By Thriel
        else if (loc == 10) // Goblin Caves
        {
            map = 31;
            x = 5516;
            y = 5437;
        }
        else if (loc == 11) // Sikuku Underground Prison
        {
             if (thisclient->Stats->Level<160) // by Terr0risT
             {
                 SendPM(thisclient, "You need to be a least Level 160 to visit Sikuku Underground Prison!");
                 return true;
             }
             else
             {
                map = 65;
                x = 5485;
                y = 5285;
             }
        }
        //Added By Thriel
        else if (loc == 12) // Gorge Of Silence
        {
            map = 28;
            x = 5205;
            y = 4911;
        }
        //Added By Thriel
        else if (loc == 13) // Forgotten Temple
        {
             if (thisclient->Stats->Level<120)
             {
                 SendPM(thisclient, "You need to be a least Level 120 to visit Forgotten Temple!");
                 return true;
             }
             else
             {
                map = 56;
                x = 5035;
                y = 5200;
             }
        }
        //Added By Thriel
        else if (loc == 14) // Oblivion Temple
        {
             if (thisclient->Stats->Level<140)
             {
                 SendPM(thisclient, "You need to be a least Level 140 to visit Oblivion Temple!");
                 return true;
             }
             else
             {
                map = 41;
                x = 5062;
                y = 5201;
             }
        }
/*        else if (loc == 10) // Pyramid Tombs
        {
            map = 41;
            x = 5105; // 5165 if jRose Client
            y = 5246; // 5207 if jRose Client
        }
*/
        //Orlo Added By Thriel
        else if (loc == 15) // Orlo. Dessert city of Muris
        {
             if (thisclient->Stats->Level<160)
             {
                 SendPM(thisclient, "You need to be a least Level 160 to visit Oro!");
                 return true;
             }
             else
             {
                map = 71;
                x = 5168;
                y = 5227;
             }
        }

         //Sikuku Ruins Added By Thriel
        else if (loc == 16) // Sikuku Ruins
        {
             if (thisclient->Stats->Level<150)
             {
                 SendPM(thisclient, "You need to be a least Level 150 to visit Sikuku Ruins!");
                 return true;
             }
             else
             {
                map = 66;
                x = 5997;
                y = 5255;
             }
        }

        //Cave of Ulverick Entrance Added By Thriel
        else if (loc == 17) // Cave of Ulverick
        {
             if (thisclient->Stats->Level<150)
             {
                 SendPM(thisclient, "You need to be a least Level 150 and leader of party to enter Cave of Ulverick!");
                 return true;
             }
             else
             {
                map = 29;
                x = 5843;
                y = 5105;
             }
        }


/*        else if (loc == 13) // Union Wars - Attack
        {
             if (thisclient->Stats->Level<100)
             {
                 SendPM(thisclient, "You need to be a least Level 100 to visit Union Wars!");
                 return true;
             }
             else
             {
                map = 9;
                x = 5199.91;
                y = 4784.5;
             }
        }
*/
/*        else if (loc == 14) // Union Wars - Defense
        {
             if (thisclient->Stats->Level<100)
             {
                 SendPM(thisclient, "You need to be a least Level 100 to visit Union Wars!");
                 return true;
             }
             else
             {
                map = 9;
                x = 5199.91;
                y = 5365.37;
             }
        }
*/
        else
        {
            SendPM(thisclient, "Please input a number after the go command, below is a list of places and their appropriate number");
            SendPM(thisclient, "1 = Adventurers Plains");
            SendPM(thisclient, "2 = The City of Zant");
            SendPM(thisclient, "3 = Junon Polis");
            SendPM(thisclient, "4 = Magic City of Eucar");
            SendPM(thisclient, "5 = Xita Refuge");
            SendPM(thisclient, "6 = Training grounds");
            SendPM(thisclient, "7 = Lions Plains");
            SendPM(thisclient, "8 = Luna Clan Field");
            SendPM(thisclient, "9 = Desert of the Dead");
            SendPM(thisclient, "10 = Goblin Caves");
//            SendPM(thisclient, "10 = Pyramid Tombs - ElVerloon");
            SendPM(thisclient, "11 = Sikuku Underground Prison");
            SendPM(thisclient, "12 = Gorge of Silence");
            SendPM(thisclient, "13 = Forgotten Temple");
            SendPM(thisclient, "14 = Oblivion Temple");
            //Orlo Added By Thriel
            SendPM(thisclient, "15 = Orlo");
            SendPM(thisclient, "16 = Sikuku Ruins"); // Sikuku ruins added by thriel
            SendPM(thisclient, "17 = Cave of Ulverick entrance"); // Sikuku ruins added by thriel

//            SendPM(thisclient, "13 = Union Wars-Atk");
//            SendPM(thisclient, "14 = Union Wars-Def");
            SendPM(thisclient, "Example; /go 3");
			return true;
        }
        if ( thisclient->Stats->HP < (thisclient->Stats->MaxHP / 2) || thisclient->Stats->HP < 1 || thisclient->Session->inGame == false )
        {
             SendPM(thisclient, "You need at least 50% HP in order to warp");
             return true;
        }

        if( (x != 0) && (y != 0) && (map != 0) )
        {
            fPoint coord;
            coord.x = x;
            coord.y = y;
            SendPM(thisclient, "teleport to map: %i",map);
            MapList.Index[map]->TeleportPlayer( thisclient, coord, false );

            if(thisclient->Session->accesslevel > 100)//Prevents go gm log from showing when normal players use it
            Log( MSG_GMACTION, " %s : /go %i" , thisclient->CharInfo->charname, loc);
        }
        return true;
    }
    if (strcmp(command, "goto")==0)
    {
        if(Config.Command_Goto > thisclient->Session->accesslevel)
	       return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        // added to prevent map changes with less hp than 1/2
        if ( thisclient->Stats->HP < (thisclient->Stats->MaxHP / 2) || thisclient->Session->inGame == false )
        {
             SendPM(thisclient, "You need at least 50% HP in order to warp");
             return true;
        }
		Log( MSG_GMACTION, " %s : /goto %s" , thisclient->CharInfo->charname, name);
		return pakGMTeleToPlayer(thisclient, name);
	}
    if (strcmp(command, "gotomap")==0) // *** TELEPORT WITH MAP ID *****
	{	 // credits to Blackie
	   if(Config.Command_GoToMap > thisclient->Session->accesslevel)
	      return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned map=atoi(tmp);

        //LMA: jumping qsd_zone.
        bool no_qsd_zone=false;
        if ((tmp = strtok(NULL, " "))!=NULL)
        {
            no_qsd_zone=true;
        }

        // added to prevent map changes with less hp than 1/2
        if ( thisclient->Stats->HP < (thisclient->Stats->MaxHP / 2) || thisclient->Session->inGame == false )
        {
             SendPM(thisclient, "You need at least 50% HP in order to warp");
             return true;
        }
        SendPM(thisclient, "Go to map: %i",map);
        Log( MSG_GMACTION, " %s : /gotomap %i %i" , thisclient->CharInfo->charname, map,no_qsd_zone);
		return pakGMGotomap(thisclient, map,no_qsd_zone);
    }
    if(strcmp(command, "grid")==0)
    {
        if(Config.Command_grid > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
           {
           Log( MSG_GMACTION, " %s : /grid NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "grid NOT ALLOWED");
           SendPM(thisclient, buffer);
	        return true;
           }
        if((tmp = strtok(NULL, " "))!=NULL)
        {
            if (Config.testgrid==0)
               SendPM(thisclient,"[Info] We are in Range Mode.");
            else
               SendPM(thisclient,"[Info] We are in Grid Mode.");

            return true;
        }
        if (Config.testgrid!=0)
        {
           Config.testgrid=0;
           SendPM(thisclient,"We are in Range Mode.");
        }
        else
        {
           Config.testgrid=1;
           SendPM(thisclient,"We are in Grid Mode.");
        }

	    Log( MSG_GMACTION, " Test Grid set at %i by %s" , Config.testgrid, thisclient->CharInfo->charname);
	    return true;
	}
    if(strcmp(command, "hair")==0)
    {
        if(Config.Command_Hair > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " ")) == NULL) return true; UINT hair=atoi(tmp);
        //if (hair > 6) return true;
        thisclient->CharInfo->Hair = (hair*5);
        BEGINPACKET(pak, 0x721);
        ADDWORD(pak, 9);
        ADDWORD(pak, thisclient->CharInfo->Hair);
        ADDWORD(pak, 0);
        thisclient->client->SendPacket(&pak);
        RESETPACKET(pak, 0x0730);
        ADDWORD(pak, 5);
        ADDWORD(pak, 0xa24d);
        ADDWORD(pak, 0x40b3);
        thisclient->client->SendPacket(&pak);
        DB->QExecute("UPDATE characters SET hairStyle=%i WHERE id=%i", thisclient->CharInfo->Hair, thisclient->CharInfo->charid);
        SendPM(thisclient, "Hair changed!");
		return true;
    }
    if(strcmp(command, "heal")==0)
    {
        if(Config.Command_Heal > thisclient->Session->accesslevel)
	       return true;
        Log( MSG_GMACTION, " %s : /heal", thisclient->CharInfo->charname );
        return pakGMHeal( thisclient );
    }
    if (strcmp(command, "here")==0)
	{
        if(Config.Command_Here > thisclient->Session->accesslevel)
           return true;

        if( thisclient->Shop->open)
            return true;

        return pakGMTele(thisclient, thisclient->Position->Map, thisclient->Position->current.x, thisclient->Position->current.y);
    }
    if (strcmp(command, "hide")==0)
    {
        if(Config.Command_Hide > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL)
            return true;
        int mode= atoi( tmp );
        return pakGMHide( thisclient, mode );
    }
    if(strcmp(command, "hurthim")==0)
    {
        if(Config.Command_HurtHim > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
	    Log( MSG_GMACTION, " %s : /hurthim %s" , thisclient->CharInfo->charname, name);
	    return pakGMHurtHim(thisclient, name);
	}
    if (strcmp(command, "info")==0)
    {
        if(Config.Command_Info > thisclient->Session->accesslevel)
	       return true;
        Log( MSG_GMACTION, " %s : /info" , thisclient->CharInfo->charname);
        thisclient->GetPlayerInfo( );
        return true;
	}
    if(strcmp(command, "iquest")==0)
    {
         if(Config.Command_IQuest > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
           {
           Log( MSG_GMACTION, " %s : /iquest NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "iquest NOT ALLOWED");
           SendPM(thisclient, buffer);
	        return true;
           }
        int n=1;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        unsigned int itemquest =atoi(tmp);
        if ((tmp = strtok(NULL, " "))!=NULL)
            n=atoi(tmp);
        if( thisclient->questdebug )
        {
            for(int i=0;i<n;i++)
            {
                BEGINPACKET( pak, 0x731 )
                ADDWORD    ( pak, itemquest );
                thisclient->client->SendPacket( &pak );
            }
        }
		return true;
    }
    if(strcmp(command, "hitModif")==0)
    {
        if(Config.Command_HitModif > thisclient->Session->accesslevel)
           return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned hitDelayModif=atoi(tmp);
	    Log( MSG_GMACTION, " HitDelayModif changed to %i by %s" , hitDelayModif, thisclient->CharInfo->charname);
	    return pakGMChangeHitDelayModif(thisclient, hitDelayModif);
	}
    if (strcmp(command, "item")==0)//Modified by Hiei (added refine/socket/stats)
    {
        if(Config.Command_Item > thisclient->Session->accesslevel)
	       return true;
        UINT itemrefine, itemstats, itemls, itemsocket;
        if ((tmp = strtok(NULL, " "))==NULL) return true; UINT itemid =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true; UINT itemtype =atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true; UINT itemamount =atoi(tmp);

        //LMA: new naRose's refine system (2010/05)
        if ((tmp = strtok(NULL, " "))==NULL)
        {
            itemrefine =0;
        }
        else
        {
            #ifdef REFINENEW
            itemrefine = atoi(tmp)<16?atoi(tmp)*16:15*16;
            #else
            itemrefine = atoi(tmp)<10?atoi(tmp)*16:9*16;
            #endif
        }

        if ((tmp = strtok(NULL, " "))==NULL)
            itemls =100;
        else
            itemls = atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL)
            itemsocket =0;
        else
            itemsocket =atoi(tmp)==0?false:true;
        if ((tmp = strtok(NULL, " "))==NULL)
            itemstats =0;
        else
            itemstats =atoi(tmp);
        // Remove below if you want GM to socket anything but Armor, Jewelery or Weapons - code by lmame
        if(itemtype!=3&&itemtype!=7&&itemtype!=8)
        {
            itemsocket=0;
            if (itemstats>=300)
            {
                itemstats=0;
            }

        }

        //LMA: check on item amount
        if(itemtype<10||itemtype>12)
        {
            itemamount=1;
        }

        Log( MSG_GMACTION, " %s : /item %i,%i,%i,%i,%i,%i" , thisclient->CharInfo->charname, itemid, itemtype, itemamount , itemrefine , itemsocket ,itemstats);
        return pakGMItem( thisclient , itemid , itemtype , itemamount , itemrefine , itemls, itemstats , itemsocket );
    }
    if (strcmp(command, "itemstat")==0)
    {
//        if(thisclient->Session->accesslevel < 300)
        if(Config.Command_ItemStat > thisclient->Session->accesslevel)
            return true;
        if ((tmp = strtok(NULL, " "))==NULL)return true;
        int slot = 7; //defaults to weapon
        int tipo;
        int itemstat;
        if(strcmp(tmp, "mask")==0)
        {
            slot=1;
        }
        else
        if(strcmp(tmp, "cap")==0)
        {
            slot=2;
        }
        else
        if(strcmp(tmp, "suit")==0)
        {
            slot=3;
        }
        else
        if(strcmp(tmp, "back")==0)
        {
            slot=4;
        }
        else
        if(strcmp(tmp, "glov")==0)
        {
            slot=5;
        }
        else
        if(strcmp(tmp, "shoe")==0)
        {
            slot=6;
        }
        else
        if(strcmp(tmp, "weap")==0)
        {
            slot=7;
        }
        else
        if(strcmp(tmp, "shield")==0)
        {
            slot=8;
        }
        if ((tmp = strtok(NULL, " "))==NULL)
            itemstat = 0;
        else
            itemstat = atoi(tmp);
        thisclient->items[slot].stats= itemstat;
        if(itemstat > 300)
            thisclient->items[slot].gem = itemstat;
        else
            thisclient->items[slot].gem = 0;
        thisclient->items[slot].appraised = true;

        BEGINPACKET( pak,0x7a5 );
        ADDWORD( pak, thisclient->clientid );
        ADDWORD( pak, slot );
        ADDDWORD( pak, BuildItemShow(thisclient->items[slot])); // ITEM
        ADDWORD ( pak, thisclient->Stats->Move_Speed);
        SendToVisible( &pak, thisclient );

        thisclient->UpdateInventory( slot );
        thisclient->SetStats( );
        return true;
    }

    if (strcmp(command, "job")==0) // *** Change Job *****
    {
        if(Config.Command_Job > thisclient->Session->accesslevel)
	       return true;
        char* tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* job=tmp;
        Log( MSG_GMACTION, " %s : /job %s" , thisclient->CharInfo->charname, job);
        if(thisclient->CharInfo->Job != 0 || thisclient->Stats->Level<10)
            return true;
        if(strcmp(job,"soldier")==0)
        {
            BEGINPACKET(pak, 0x721);
            ADDWORD    (pak, 0x0004);
            ADDWORD    (pak, 0x006f);
            ADDWORD    (pak, 0x0000);
            thisclient->client->SendPacket(&pak);

            RESETPACKET( pak, 0x730);
            ADDBYTE(pak, 0x05);
            ADDBYTE(pak, 0x00);
            ADDWORD(pak, 0xf52f);
            ADDWORD(pak, 0x2964);
            thisclient->client->SendPacket( &pak );
            thisclient->CharInfo->Job=111;
        }
        else
        if(strcmp(job,"muse")==0)
        {
            BEGINPACKET(pak, 0x721);
            ADDWORD    (pak, 0x0004);
            ADDWORD    (pak, 0x00D3);
            ADDWORD    (pak, 0x0000);
            thisclient->client->SendPacket(&pak);

            RESETPACKET( pak, 0x730);
            ADDBYTE(pak, 0x05);
            ADDBYTE(pak, 0x00);
            ADDWORD(pak, 0xdc3b);
            ADDWORD(pak, 0x20cd);
            thisclient->client->SendPacket( &pak );
            thisclient->CharInfo->Job=211;
        }
        else
        if(strcmp(job,"hawker")==0)
        {
                BEGINPACKET(pak, 0x721);
                ADDWORD    (pak, 0x0004);
                ADDWORD    (pak, 0x0137);
                ADDWORD    (pak, 0x0000);
                thisclient->client->SendPacket(&pak);

                RESETPACKET( pak, 0x730);
                ADDBYTE(pak, 0x05);
                ADDBYTE(pak, 0x00);
                ADDWORD(pak, 0x1104);
                ADDWORD(pak, 0x5150);
                thisclient->client->SendPacket( &pak );
                thisclient->CharInfo->Job=311;
        }
        else
        if(strcmp(job,"dealer")==0)
        {
            BEGINPACKET(pak, 0x721);
            ADDWORD    (pak, 0x0004);
            ADDWORD    (pak, 0x019b);
            ADDWORD    (pak, 0x0000);
            thisclient->client->SendPacket(&pak);

            RESETPACKET( pak, 0x730);
            ADDBYTE(pak, 0x05);
            ADDBYTE(pak, 0x00);
            ADDWORD(pak, 0x4c1c);
            ADDWORD(pak, 0xef69);
            thisclient->client->SendPacket( &pak );
            thisclient->CharInfo->Job=411;
		}
		return true;
    }
    if (strcmp(command, "kick")==0)
    {
        if(Config.Command_Kick > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
           {
           Log( MSG_GMACTION, " %s : /kick NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "kick NOT ALLOWED");
           SendPM(thisclient, buffer);
	        return true;
           }
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        Log( MSG_GMACTION, " %s : /kick %s" , thisclient->CharInfo->charname, name);
        return pakGMKick( thisclient, name );
    }
    if (strcmp(command, "killinrange")==0)
    {
        if(Config.Command_KillInRange > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned range=atoi(tmp);
        Log( MSG_GMACTION, " %s : /killinrange %i" , thisclient->CharInfo->charname, range);
        return pakGMKillInRange( thisclient, range );
    }
    if (strcmp(command, "learnskill")==0)
    {
        //LMA: Used for tests.
         if(Config.Command_Listqflag > thisclient->Session->accesslevel)
            return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned skillid=atoi(tmp);
        Log( MSG_GMACTION, " %s : /learnskill %i" , thisclient->CharInfo->charname,skillid);
        if (LearnSkill( thisclient, skillid,false))
        {
            SendPM(thisclient,"Skill %i learned with success.",skillid);
        }
        else
        {
            SendPM(thisclient,"Skill %i wasn't learned.",skillid);
        }

        return true;
    }
    if (strcmp(command, "level")==0)
    {
         if(Config.Command_Level > thisclient->Session->accesslevel)
            return true;
        char* name;
        if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned level=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) name = thisclient->CharInfo->charname; else name = tmp;
        Log( MSG_GMACTION, " %s : /level %i %s" , thisclient->CharInfo->charname, level, name);
        return pakGMLevel( thisclient, level, name );
    }
    if (strcmp(command, "levelup")==0)
    {
        if(Config.Command_LevelUp > thisclient->Session->accesslevel)
	       return true;
        Log( MSG_GMACTION, " %s : /levelup" , thisclient->CharInfo->charname);
		thisclient->CharInfo->Exp += thisclient->GetLevelEXP();
		return true;
	}
	if(strcmp(command, "settestquests")==0)
    {
        //LMA: only for tests! Don't use it.
        for (int k=0;k<10;k++)
        {
            if(thisclient->quest.quests[4].Variables[k]==0)
            {
                thisclient->quest.quests[4].Variables[k]=k+1;
            }

        }

        for (int k=0;k<4;k++)
        {
            thisclient->quest.quests[4].Switches[k]=k+1;
        }

        thisclient->quest.quests[4].Items[0].refine=15;
        thisclient->quest.quests[4].Items[0].lifespan=100;
        thisclient->quest.quests[4].Items[0].durability=99;
        thisclient->quest.quests[4].Items[0].socketed=true;
        thisclient->quest.quests[4].Items[0].appraised=true;
        thisclient->quest.quests[4].Items[0].stats=111;
        thisclient->quest.quests[4].Items[0].gem=301;
        thisclient->quest.quests[4].Items[0].durabLeft=98;
        thisclient->quest.quests[4].Items[0].sig_head=1111;
        thisclient->quest.quests[4].Items[0].sig_data=2222;
        thisclient->quest.quests[4].Items[0].sig_gem=3333;
        thisclient->quest.quests[4].Items[0].sp_value=444;
        thisclient->quest.quests[4].Items[0].last_sp_value=555;


        thisclient->quest.quests[4].Items[4].itemtype=2;
        thisclient->quest.quests[4].Items[4].itemnum=222;
        thisclient->quest.quests[4].Items[4].count=99;
        thisclient->quest.quests[4].Items[4].refine=15;
        thisclient->quest.quests[4].Items[4].lifespan=100;
        thisclient->quest.quests[4].Items[4].durability=99;
        thisclient->quest.quests[4].Items[4].socketed=true;
        thisclient->quest.quests[4].Items[4].appraised=true;
        thisclient->quest.quests[4].Items[4].stats=111;
        thisclient->quest.quests[4].Items[4].gem=301;
        thisclient->quest.quests[4].Items[4].durabLeft=98;
        thisclient->quest.quests[4].Items[4].sig_head=1111;
        thisclient->quest.quests[4].Items[4].sig_data=2222;
        thisclient->quest.quests[4].Items[4].sig_gem=3333;
        thisclient->quest.quests[4].Items[4].sp_value=444;
        thisclient->quest.quests[4].Items[4].last_sp_value=555;

        for (int k=0;k<6;k++)
        {
            thisclient->quest.quests[4].unknown[k]=k+1;
        }


        return true;
    }
    if(strcmp(command, "dumpquests")==0)
    {
        //dump quest informations to server side;
        if(Config.Command_Listqflag > thisclient->Session->accesslevel)
           return true;

        Log(MSG_INFO,"Quest data dump for player %u::%s",thisclient->CharInfo->charid,thisclient->CharInfo->charname);
        for (int k=0;k<5;k++)
        {
            Log(MSG_INFO,"EpisodeVar[%i]=%u",k,thisclient->quest.EpisodeVar[k]);
        }

        for (int k=0;k<3;k++)
        {
            Log(MSG_INFO,"JobVar[%i]=%u",k,thisclient->quest.JobVar[k]);
        }

        for (int k=0;k<7;k++)
        {
            Log(MSG_INFO,"PlanetVar[%i]=%u",k,thisclient->quest.PlanetVar[k]);
        }

        for (int k=0;k<10;k++)
        {
            Log(MSG_INFO,"UnionVar[%i]=%u",k,thisclient->quest.UnionVar[k]);
        }

        //quests here.
        for (int k=0;k<10;k++)
        {
            Log(MSG_INFO,"Quest %i::",k);
            Log(MSG_INFO,"-> QuestID=%u",thisclient->quest.quests[k].QuestID);
            Log(MSG_INFO,"-> StartTime=%u",thisclient->quest.quests[k].StartTime);

            for(int j=0;j<10;j++)
            {
                Log(MSG_INFO,"-> Variables[%i]=%u",j,thisclient->quest.quests[k].Variables[j]);
            }

            for(int j=0;j<4;j++)
            {
                Log(MSG_INFO,"-> Switches[%i]=%u",j,thisclient->quest.quests[k].Switches[j]);
            }

            //items here!
            for(int j=0;j<5;j++)
            {
                Log(MSG_INFO,"-> Item %i::",j);
                Log(MSG_INFO,"--> %i * %u::%u",thisclient->quest.quests[k].Items[j].count,thisclient->quest.quests[k].Items[j].itemtype,thisclient->quest.quests[k].Items[j].itemnum);
                Log(MSG_INFO,"--> ref=%u, life=%u, dur=%u, sock=%u, appr=%u",thisclient->quest.quests[k].Items[j].refine,thisclient->quest.quests[k].Items[j].lifespan,thisclient->quest.quests[k].Items[j].durability,thisclient->quest.quests[k].Items[j].socketed,thisclient->quest.quests[k].Items[j].appraised);
                Log(MSG_INFO,"--> St=%u, gem=%u, duraL=%u",thisclient->quest.quests[k].Items[j].stats,thisclient->quest.quests[k].Items[j].gem,thisclient->quest.quests[k].Items[j].durabLeft);
                Log(MSG_INFO,"--> sig_head=%u, sig_data=%u, sig_gem=%u",thisclient->quest.quests[k].Items[j].sig_head,thisclient->quest.quests[k].Items[j].sig_data,thisclient->quest.quests[k].Items[j].sig_gem);
                Log(MSG_INFO,"--> sp=%u, lastsp=%u",thisclient->quest.quests[k].Items[j].sp_value,thisclient->quest.quests[k].Items[j].last_sp_value);
            }
            //end of items.

            for(int j=0;j<6;j++)
            {
                Log(MSG_INFO,"-> unknown[%i]=%u",j,thisclient->quest.quests[k].unknown[j]);
            }

        }
        //end of quests

        //quest flags
        for (int k=0;k<512;k++)
        {
            Log(MSG_INFO,"flags[%i]=%u",k,thisclient->quest.GetFlag(k));
        }

        for (int k=0;k<5;k++)
        {
            Log(MSG_INFO,"ClanVar[%i]=%u",k,thisclient->quest.ClanVar[k]);
        }

        Log(MSG_INFO,"RefNPC=%u",thisclient->quest.RefNPC);

        Log(MSG_INFO,"END OF Quest data dump for player %u::%s",thisclient->CharInfo->charid,thisclient->CharInfo->charname);


        return true;
    }
    if(strcmp(command, "listqflag")==0)
    {
        if(Config.Command_Listqflag > thisclient->Session->accesslevel)
           return true;
        SendPM(thisclient, "Quest Flags");
        string buffer = "";

        for (dword i = 0; i < 512; i++)
        {
			char buf2[5];
			sprintf(buf2, "%i ", thisclient->quest.GetFlag(i));
			buffer.append(buf2);
	//      if (i > 0 && i%10 == 0) {
			if ((i + 1) % 10 == 0)
			{
			  SendPM(thisclient, (char*)buffer.c_str());
			  buffer = "";
			}
		}

      SendPM(thisclient, (char*)buffer.c_str());
	  return true;
    }
    if(strcmp(command, "listquest")==0)
    {
        if(Config.Command_Listquest > thisclient->Session->accesslevel)
	        return true;
        SendPM( thisclient, "Current Quests:" );
        for(int i=0;i<10;i++)
        {
            if (thisclient->quest.quests[i].QuestID == 0) continue;
            SendPM( thisclient, "Quest[%i]: %i[%i] %i[%i] %i[%i] %i[%i] %i[%i]" , thisclient->quest.quests[i].QuestID, thisclient->quest.quests[i].Items[0].itemnum, thisclient->quest.quests[i].Items[0].count, thisclient->quest.quests[i].Items[1].itemnum, thisclient->quest.quests[i].Items[1].count, thisclient->quest.quests[i].Items[2].itemnum,thisclient->quest.quests[i].Items[2].count,thisclient->quest.quests[i].Items[3].itemnum,thisclient->quest.quests[i].Items[3].count,thisclient->quest.quests[i].Items[4].itemnum,thisclient->quest.quests[i].Items[4].count);
        }
		return true;
    }
    if(strcmp(command, "listqvar")==0)
    {
        if(Config.Command_Listqvar > thisclient->Session->accesslevel)
           return true;
        SendPM( thisclient, "Quest Variables");

        string buffer = "Episode: ";
        for(dword i = 0; i < 5; i++) {
          char buf2[5];
          sprintf(buf2, "%02x ", thisclient->quest.EpisodeVar[i]);
          buffer.append(buf2);
        }

        SendPM( thisclient, (char*)buffer.c_str() );

        buffer = "Job: ";
        for(dword i = 0; i < 3; i++) {
          char buf2[5];
          sprintf(buf2, "%02x ", thisclient->quest.JobVar[i]);
          buffer.append(buf2);
        }

        SendPM( thisclient, (char*)buffer.c_str());

        buffer = "Planet: ";
        for(dword i = 0; i < 7; i++) {
          char buf2[5];
          sprintf(buf2, "%02x ", thisclient->quest.PlanetVar[i]);
          buffer.append(buf2);
        }
        SendPM( thisclient, (char*)buffer.c_str());

        buffer = "Union: ";
        for(dword i = 0; i < 10; i++) {
          char buf2[5];
          sprintf(buf2, "%02x ", thisclient->quest.UnionVar[i]);
          buffer.append(buf2);
            }
        SendPM( thisclient, (char*)buffer.c_str());

        //LMA: Clan Var.
        buffer = "Clan: ";
        for(dword i = 0; i < 5; i++) {
          char buf2[5];
          sprintf(buf2, "%02x ", thisclient->quest.ClanVar[i]);
          buffer.append(buf2);
            }
        SendPM( thisclient, (char*)buffer.c_str());

        for (dword j = 0; j < 10; j++)
        {
            if (thisclient->quest.quests[j].QuestID == 0) continue;
            buffer="-> Vars: ";
            SendPM(thisclient,"Quest %i/10 (%u): ",j,thisclient->quest.quests[j].QuestID);

              for(dword i = 0; i < 10; i++)
              {
                char buf2[20];
                sprintf(buf2, "[%i]=%i, ",i,thisclient->quest.quests[j].Variables[i]);
                buffer.append(buf2);
              }

              SendPM( thisclient, (char*)buffer.c_str());

              //quest objects.
              for (int k=0;k<5;k++)
              {
                  if(thisclient->quest.quests[j].Items[k].itemtype==0||thisclient->quest.quests[j].Items[k].itemnum==0)
                  {
                      continue;
                  }

                  SendPM(thisclient,"-> Object slot %i:: %i*(%i::%i)",k,thisclient->quest.quests[j].Items[k].count,thisclient->quest.quests[j].Items[k].itemtype,thisclient->quest.quests[j].Items[k].itemnum);
              }
        }
		return true;
    }
    if(strcmp(command, "listqvars")==0)
    {
        //server side.
        if(Config.Command_Listqvar > thisclient->Session->accesslevel)
           return true;

        string buffer = "";
        for(dword i = 0; i < 5; i++)
        {
          char buf2[20];
          sprintf(buf2, "[%i]=%i, ",i,thisclient->quest.EpisodeVar[i]);
          buffer.append(buf2);
        }

        Log(MSG_INFO,"%s Episode vars: %s",thisclient->CharInfo->charname,buffer.c_str());

        buffer = "";
        for(dword i = 0; i < 3; i++) {
          char buf2[20];
          sprintf(buf2, "[%i]=%i, ",i,thisclient->quest.JobVar[i]);
          buffer.append(buf2);
        }

        Log(MSG_INFO,"%s Job vars: %s",thisclient->CharInfo->charname,buffer.c_str());

        buffer = "";
        for(dword i = 0; i < 7; i++) {
          char buf2[20];
          sprintf(buf2, "[%i]=%i, ",i,thisclient->quest.PlanetVar[i]);
          buffer.append(buf2);
        }

        Log(MSG_INFO,"%s Planet vars: %s",thisclient->CharInfo->charname,buffer.c_str());

        buffer = "";
        for(dword i = 0; i < 10; i++) {
          char buf2[20];
          sprintf(buf2, "[%i]=%i, ",i,thisclient->quest.UnionVar[i]);
          buffer.append(buf2);
            }

        Log(MSG_INFO,"%s Union vars: %s",thisclient->CharInfo->charname,buffer.c_str());

        //LMA: Clan Var.
        buffer = "";
        for(dword i = 0; i < 5; i++) {
          char buf2[20];
          sprintf(buf2, "[%i]=%i, ",i, thisclient->quest.ClanVar[i]);
          buffer.append(buf2);
            }

        Log(MSG_INFO,"%s Clan vars: %s",thisclient->CharInfo->charname,buffer.c_str());

        //Quest flags.
        buffer="Quest Flags: ";
        for (int i=0;i<512;i++)
        {
            int bit=thisclient->quest.GetFlag(i);
            if(bit==0)
            {
                continue;
            }

          char buf2[20];
          sprintf(buf2, "[%i]=%i, ",i, bit);
          buffer.append(buf2);
        }

        Log(MSG_INFO,"%s Quest Flags: %s",thisclient->CharInfo->charname,buffer.c_str());

        for (dword j = 0; j < 10; j++)
        {
            if (thisclient->quest.quests[j].QuestID == 0) continue;
            buffer="-> Vars: ";
            Log(MSG_INFO,"Quest slot %i/10 (%u): ",j,thisclient->quest.quests[j].QuestID);
            for(dword i = 0; i < 10; i++)
            {
            char buf2[20];
            sprintf(buf2, "[%i]=%i, ",i,thisclient->quest.quests[j].Variables[i]);
            buffer.append(buf2);
            }

            Log(MSG_INFO,"%s",buffer.c_str());

            //quest objects.
            for (int k=0;k<5;k++)
            {
                if(thisclient->quest.quests[j].Items[k].itemtype==0||thisclient->quest.quests[j].Items[k].itemnum==0)
                {
                    continue;
                }

                Log(MSG_INFO,"-> Object slot %i:: %i*(%i::%i)",k,thisclient->quest.quests[j].Items[k].count,thisclient->quest.quests[j].Items[k].itemtype,thisclient->quest.quests[j].Items[k].itemnum);
            }
        }
		return true;
    }
    if(strcmp(command, "maxstats")==0)    // MaxStats - by rl2171
    {
        if(Config.Command_MaxStats > thisclient->Session->accesslevel)
           return true;
        Log( MSG_GMACTION, " %s : /MaxStats", thisclient->CharInfo->charname );
        return pakGMMaxStats( thisclient );
    }
    if(strcmp(command, "mdmg")==0)
    {
        if(Config.Command_Mdmg > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
        {
			Log( MSG_GMACTION, " %s : /mdmg NOT ALLOWED" , thisclient->CharInfo->charname);
			char buffer[200];
			sprintf ( buffer, "mdmg NOT ALLOWED");
			SendPM(thisclient, buffer);
			return true;
        }
	    if((tmp = strtok(NULL, " "))==NULL) 
			return true; 
		unsigned rate=atoi(tmp);
	    Log( MSG_GMACTION, " Rate for Monster Dmg is now set at %i by %s" , rate, thisclient->CharInfo->charname);
	    return pakGMChangeMonsterDmg(thisclient, rate);
	}
	if (strcmp(command, "tdwave")==0) //create a wave of monsters in current map
	{
        if(thisclient->Session->accesslevel < 900) //DEV only
	        return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned montype=atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned moncount=atoi(tmp);
		int monaip = -1;
        if (moncount > Config.monmax) moncount = Config.monmax; //max monsters from config

        //check if there are waypoints for this map. If not don't set the globals
        if(GServer->WPList[thisclient->Position->Map][1].WPType == 0)
        {
            SendPM(thisclient,"Cannot create spwan. Map %i has no defined waypoints",thisclient->Position->Map);
        }
        else
        {
            GServer->TDMap = thisclient->Position->Map;
            GServer->TDSpawnType = montype;
            GServer->TDSpawnCount = moncount;
            SendPM(thisclient,"TD Spawn started on Map",GServer->TDMap);

            BEGINPACKET( pak, 0x702 );
            ADDSTRING( pak, "TD Wave started");
            ADDBYTE( pak, 0x00);
            SendToAll( &pak );
        }
		Log( MSG_GMACTION, " %s : /tdspawn %i,%i", thisclient->CharInfo->charname, montype, moncount);
		return true;
    }
    if (strcmp(command, "tdspawnrate")==0) //create a wave of monsters in current map
	{
         if(thisclient->Session->accesslevel < 900) //DEV only
	        return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned thisrate=atoi(tmp);
        GServer->TDSpawnRate = thisrate;
        //check if there are waypoints for this map. If not don't set the globals
		Log( MSG_GMACTION, " %s : /tdspawnrate %i", thisclient->CharInfo->charname, thisrate);
		return true;
    }
    if (strcmp(command, "mon")==0)
    {
         if(Config.Command_Mon > thisclient->Session->accesslevel)
	        return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned montype=atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned moncount=atoi(tmp);
		unsigned monteam=0;
		if ((tmp = strtok(NULL, " "))!=NULL) monteam=atoi(tmp);
		int monaip=-1;
		if ((tmp = strtok(NULL, " "))!=NULL) monaip=atoi(tmp);
        if (moncount > Config.monmax) moncount = Config.monmax; //max monsters from config
		Log( MSG_GMACTION, " %s : /mon %i,%i,%i,%i" , thisclient->CharInfo->charname, montype, moncount,monteam,monaip);
		return pakGMMon( thisclient, montype, moncount,monteam,monaip);
	}
    if (strcmp(command, "mon2")==0)
    {    //Spawn "x" monsters
         if(Config.Command_Mon2 > thisclient->Session->accesslevel)
	        return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned montype=atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned moncount=atoi(tmp);

		for (int k=0;k<moncount;k++)
	    {
             Log( MSG_GMACTION, " %s : /mon2 %i,%i" , thisclient->CharInfo->charname, montype+k, moncount);
             pakGMMon( thisclient, montype+k, 1 );
        }

		return true;
	}
    if (strcmp(command, "move")==0)
    {
        if(Config.Command_Move > thisclient->Session->accesslevel)
	        return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned map=atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true; float x=(float)atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true; float y=(float)atoi(tmp);
		Log( MSG_GMACTION, " %s : /move %s,%i,%i,%i" , thisclient->CharInfo->charname, name, map, x, y);
		return pakGMTeleOtherPlayer(thisclient, name, map, x, y);
	}
    if(strcmp(command, "moveto")==0)
    {
        if(Config.Command_Moveto > thisclient->Session->accesslevel)
           return true;
        fPoint position;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        position.x = atof(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        position.y = atof(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        position.z = atof(tmp);
        pakGMMoveTo( thisclient, position );
		return true;
    }
    if (strcmp(command, "mute")==0) //==== Mute a player ==== [by Paul_T]
    {
        if(Config.Command_Mute > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        char* id=tmp;
        if ((tmp = strtok(NULL, " "))==NULL)
                return true;
        int min=atoi(tmp);
        Log( MSG_GMACTION, " %s : /mute %s %i" , thisclient->CharInfo->charname, id, min);
        return pakGMMute(thisclient, id, min);
    }
    if (strcmp(command, "myloc")==0)    // myloc - by PurpleYouko
    {
        char buffer[200];
        sprintf ( buffer, "My location. Map = %i X = %f Y = %f", thisclient->Position->Map, thisclient->Position->current.x, thisclient->Position->current.y);
        SendPM(thisclient, buffer);
		return true;
    }
    if(strcmp(command, "mystat")==0)    // mystat - by PurpleYouko
    {
        if ((tmp = strtok(NULL, " "))==NULL)
        {
            SendPM(thisclient, "Please input after the mystat command, below is a list of commands");
            SendPM(thisclient, "ap = Attack Power");
            SendPM(thisclient, "acc = Accuracy");
            SendPM(thisclient, "dodge = Dodge");
            SendPM(thisclient, "def = Defense");
            SendPM(thisclient, "crit = Critical");
            SendPM(thisclient, "mspd = Movement Speed");
            SendPM(thisclient, "team = PvP Team");
            SendPM(thisclient, "Example; /mystat ap");
         return true;
		}
        char buffer[200];
        if(strcmp(tmp, "ap")==0)
        {
            sprintf ( buffer, "My Attack Power is %i", thisclient->Stats->Attack_Power );
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "acc")==0)
        {
            sprintf ( buffer, "My Accuracy is %i", thisclient->Stats->Accuracy );
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "team")==0)
        {
            //LMA: pvp team.
            sprintf ( buffer, "My team is %i, pvp status is %i", thisclient->pvp_id,thisclient->pvp_status);
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "dodge")==0)
        {
            sprintf ( buffer, "My dodge is %i", thisclient->Stats->Dodge);
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "def")==0)
        {
            sprintf ( buffer, "My defense is %i", thisclient->Stats->Defense);
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "crit")==0)
        {
            sprintf ( buffer, "My critical is %i", thisclient->Stats->Critical);
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "mspd")==0)
        {
            sprintf ( buffer, "My move speed is %i", thisclient->Stats->Move_Speed);
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "pos")==0)
        {
            sprintf ( buffer, "pos: %i (%.2f,%.2f)", thisclient->Position->Map,thisclient->Position->current.x,thisclient->Position->current.y);
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "BR")==0)
        {
            sprintf(buffer, "BR: %f",thisclient->Stats->Block_Rate);
            SendPM(thisclient, buffer);
        }
        else if(strcmp(tmp, "BlockedDmg")==0)
        {
            sprintf(buffer, "BlockedDamage: %i",thisclient->Stats->Blocked_dmg);
            SendPM(thisclient, buffer);
        }
		return true;
    }
    if(strcmp(command, "mystat2")==0)    // mystat2 - by PurpleYouko - from osprose to test
    {
         if ((tmp = strtok(NULL, " "))==NULL)return true;
         if(strcmp(tmp, "ap")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My Attack Power is %i", thisclient->Stats->Attack_Power );
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "acc")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My Accuracy is %i", thisclient->Stats->Accuracy );
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "hp")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My current HP is %I64i", thisclient->Stats->HP );
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "mp")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My current MP is %u", thisclient->Stats->MP );
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "maxhp")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My Maximum HP is %I64i", thisclient->GetMaxHP());
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "maxmp")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My maximum MP is %u", thisclient->GetMaxMP());
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "dodge")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My dodge is %i", thisclient->Stats->Dodge);
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "def")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My defense is %i", thisclient->Stats->Defense);
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "mdef")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My Magic defense is %i", thisclient->Stats->Magic_Defense);
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "crit")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My critical is %i", thisclient->Stats->Critical);
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "mspd")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My move speed is %i", thisclient->Stats->Move_Speed);
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "aspd")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My attack speed is %f", thisclient->Stats->Attack_Speed);
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "aspdp") == 0)
         {
              SendPM(thisclient, "\nMy attack speed percent is %f", thisclient->Stats->Attack_Speed_Percent);
         }
         else if(strcmp(tmp, "stamina")==0)
         {
             char buffer2[200];
             sprintf ( buffer2, "My Stamina is %i", thisclient->CharInfo->stamina);
             SendPM(thisclient, buffer2);
         }
         else if(strcmp(tmp, "clan")==0)
         {
             SendPM(thisclient, "My clan id is: %i My clan rank is: %i", thisclient->Clan->clanid, thisclient->Clan->clanrank);
         }
         else if(strcmp(tmp, "map")==0)
         {
             SendPM(thisclient, "My current map is: %i ", thisclient->Position->Map);
         }
         else if(strcmp(tmp, "cha")==0)
         {
             SendPM(thisclient, "Cha: %i ", thisclient->Attr->Cha + thisclient->Attr->Echa);
         }
		 return true;
    }
    if(strcmp(command, "npc")==0)
    {
        if(Config.Command_Npc > thisclient->Session->accesslevel)
	       return true;
		if((tmp = strtok(NULL, " "))==NULL) return true; unsigned npcid=atoi(tmp);
		unsigned npcdialog = 0;
		unsigned eventid=0;
		if((tmp = strtok(NULL, " "))!=NULL)
            npcdialog=atoi(tmp);
		if((tmp = strtok(NULL, " "))!=NULL)
            eventid=atoi(tmp);
        Log( MSG_GMACTION, " %s : /npc %i, %i" , thisclient->CharInfo->charname, npcid, npcdialog);
        return pakGMNpc(thisclient, npcid, npcdialog,eventid);
	}
    if(strcmp(command, "npcobjvar")==0)
    {
        if(Config.Command_NpcObjVar > thisclient->Session->accesslevel)
	       return true;
		if((tmp = strtok(NULL, " "))==NULL) return true;
		int npctype=atoi(tmp);
        Log( MSG_GMACTION, " %s : /npcobjvar %i" , thisclient->CharInfo->charname, npctype);
        return pakGMObjVar(thisclient,npctype);
	}
    if(strcmp(command, "npcsetobjvar")==0)
    {
        if(Config.Command_NpcSetObjVar > thisclient->Session->accesslevel)
	       return true;
		if((tmp = strtok(NULL, " "))==NULL) return true;
		int npctype=atoi(tmp);
        if((tmp = strtok(NULL, " "))==NULL) return true;
		int index=atoi(tmp);
        if((tmp = strtok(NULL, " "))==NULL) return true;
		int value=atoi(tmp);
        Log( MSG_GMACTION, " %s : /npcsetobjvar %i %i %i" , thisclient->CharInfo->charname, npctype,index,value);
        return pakGMSetObjVar(thisclient,npctype,index,value);
	}
    if (strcmp(command, "npcltb")==0) /* LMA:npc shouts or announces with LTB string */
	{
        if(Config.Command_NpcLtb > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* shan=tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* aipqsd=tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true; int npctype=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true; int ltbid=atoi(tmp);
        return pakGMnpcshout(thisclient,shan,aipqsd,npctype,ltbid);
    }
	if (strcmp(command, "prize")==0)
    {
        #ifdef PYCUSTOM
         if ((tmp = strtok(NULL, " "))==NULL)return true;
         {
             UINT prizeid = atoi(tmp);
             thisclient->PrizeExchange(thisclient, prizeid);
             return true;
         }
         #else
         SendPM(thisclient,"PY custom events are not in this build. Check source code.");
         #endif
		 return true;
    }
    if(strcmp(command, "p")==0)  //*** READ THE PACKET.TXT AND SEND IT
    {
        if(Config.Command_Pak > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
			{
			Log( MSG_GMACTION, " %s : /p NOT ALLOWED" , thisclient->CharInfo->charname);
			char buffer[200];
			sprintf ( buffer, "p NOT ALLOWED");
			SendPM(thisclient, buffer);
				return true;
			}
        if ((tmp = strtok(NULL, " "))==NULL)
             return true;
		char buffer;
		std::string filename = "packet/packet";
		filename += tmp;
		filename += ".txt";
		FILE *packet1 = fopen(filename.c_str(),"r");
		if(packet1==NULL)
		{
			printf("Error opening packet.txt!\n");
			return true;
		}
		unsigned int command = 0;
		fread( &command, 1, 2, packet1 );
		BEGINPACKET(pak,command);
		while((fscanf(packet1,"%c",&buffer))!=EOF)
				ADDBYTE(pak,buffer);
		thisclient->client->SendPacket( &pak );
		fclose(packet1);
		return true;
    }
    if (strcmp(command, "partylvl")==0)
    {
        if(Config.Command_Partylvl > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; int level= atoi( tmp );
        return pakGMPartylvl( thisclient, level );
    }
    if(strcmp(command, "pdmg")==0)
    {
        if(Config.Command_Pdmg > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
           {
           Log( MSG_GMACTION, " %s : /pdmg NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "pdmg NOT ALLOWED");
           SendPM(thisclient, buffer);
	         return true;
           }
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned rate=atoi(tmp);
	    Log( MSG_GMACTION, " Rate for Player Dmg is now set at %i by %s" , rate, thisclient->CharInfo->charname);
	    return pakGMChangePlayerDmg(thisclient, rate);
	}
    if(strcmp(command, "pvp")==0)
    {
        if(Config.Command_Pvp > thisclient->Session->accesslevel)
	       return true;
        CMap* map = MapList.Index[thisclient->Position->Map];
        BEGINPACKET(pak, 0x721);
        ADDWORD(pak, 34);
        if(map->allowpvp!=0)
        {
            map->allowpvp = 0;
            ADDWORD(pak, 2);
            SendSysMsg(thisclient, "PVP off");
        }
        else
        {
            map->allowpvp = 1;
            ADDWORD(pak, 51);
            SendSysMsg(thisclient, "PVP on");
        }
        ADDWORD(pak, 0);
        SendToMap(&pak, thisclient->Position->Map);
        RESETPACKET(pak, 0x0730);
        ADDWORD(pak, 5);
        ADDWORD(pak, 0xa24d);
        ADDWORD(pak, 0x40b3);
        SendToMap(&pak, thisclient->Position->Map);
		return true;
    }
    if(strcmp(command, "pvpnew")==0)
    {
        if(Config.Command_Pvp > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; int testvalue=atoi(tmp);
        CMap* map = MapList.Index[thisclient->Position->Map];

        switch (testvalue)
        {
           case 0:
           {
            BEGINPACKET(pak, 0x0730);
            ADDWORD(pak, 5);
            ADDWORD(pak, 0xa24d);
            ADDWORD(pak, 0x40b3);
            SendSysMsg(thisclient, "PVP 0");
            SendToMap(&pak, thisclient->Position->Map);
            }
            break;
            case 1:
            {
                BEGINPACKET(pak, 0x0730);
                ADDWORD(pak, 0x0101);
                ADDWORD(pak, 0x0b03);
                ADDWORD(pak, 0x0000);
                 SendSysMsg(thisclient, "PVP 1");
                 SendToMap(&pak, thisclient->Position->Map);
             }
             break;
            case 2:
            {
                //0x813 - 05 00
                //0x814 - 04 00
                BEGINPACKET(pak, 0x0813);
                ADDWORD(pak, 0x0005);
                 SendToMap(&pak, thisclient->Position->Map);
                 RESETPACKET(pak, 0x0814);
                ADDWORD(pak, 0x0004);
                 SendSysMsg(thisclient, "PVP 2");
                 SendToMap(&pak, thisclient->Position->Map);
             }
             break;
            default:
            {
                    SendSysMsg(thisclient, "Wrong value");
            }
            break;
        }
		return true;
    }
    if(strcmp(command, "raisecg")==0)
    {
        if(Config.Command_RaiseCG > thisclient->Session->accesslevel)
	       return true;
	    return pakGMRaiseCG(thisclient);
	}
    if(strcmp(command, "rate")==0) //incomplete
    {
        if(Config.Command_Rate > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        char* type = tmp;
        if(strcmp(type,"drop")==0)
        {
            if ((tmp = strtok(NULL, " "))==NULL) return true;
            Config.DROP_RATE = atoi(tmp);
        }
        else
        if(strcmp(type,"exp")==0)
        {
            if ((tmp = strtok(NULL, " "))==NULL) return true;
            Config.EXP_RATE = atoi(tmp);
        }
		return true;
    }
    if (strcmp(command, "reborn")==0) //==== Reborn ==== (By Core)
	{
         if(Config.Command_Reborn > thisclient->Session->accesslevel)
            return true;

        return pakGMReborn(thisclient);
	}
    if (strcmp(command, "refine")==0)
    {
        if(Config.Command_Refine > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL)return true;
        int slot = 7; //defaults to weapon
        int tipo;
        int itemrefine;
        if(strcmp(tmp, "mask")==0)
        {
            slot=1;
        }
        else
        if(strcmp(tmp, "cap")==0)
        {
            slot=2;
        }
        else
        if(strcmp(tmp, "suit")==0)
        {
            slot=3;
        }
        else
        if(strcmp(tmp, "back")==0)
        {
            slot=4;
        }
        else
        if(strcmp(tmp, "glov")==0)
        {
            slot=5;
        }
        else
        if(strcmp(tmp, "shoe")==0)
        {
            slot=6;
        }
        else
        if(strcmp(tmp, "weap")==0)
        {
            slot=7;
        }
        else
        if(strcmp(tmp, "shield")==0)
        {
            slot=8;
        }
        if ((tmp = strtok(NULL, " "))==NULL)
            itemrefine =0;
        else
           itemrefine = atoi(tmp)<16?atoi(tmp)*16:9*16;
        thisclient->items[slot].refine = itemrefine;

        BEGINPACKET( pak, 0x7a5);
	    ADDWORD( pak, thisclient->clientid );
	    ADDWORD( pak, slot);
	    ADDWORD( pak, thisclient->items[slot].itemnum);	// ITEM NUM
	    ADDWORD( pak, BuildItemRefine(  thisclient->items[slot] ));	// REFINE
	    ADDWORD( pak, thisclient->Stats->Move_Speed );	// REFINE 2602
	    SendToVisible( &pak,thisclient );

        thisclient->UpdateInventory( slot );
        thisclient->SetStats( );
		return true;
    }
    if (strcmp(command, "reload")==0) // *** RELOAD CONFIG.INI ******
    {
           if(Config.Command_Reload > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
           {
           Log( MSG_GMACTION, " %s : /reload NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "reload NOT ALLOWED");
           SendPM(thisclient, buffer);
	         return true;
           }
         if ((tmp = strtok(NULL, " "))==NULL)return true;
         else if(strcmp(tmp, "config")==0)
//         {
             LoadConfigurations( (char*)filename.c_str()  );
//             GServer->LoadConfig( );
//         }
         else if(strcmp(tmp, "mobs")==0)
             GServer->LoadNPCData( );
         else if(strcmp(tmp, "drops")==0)
         {
             //LMA: And system.
            if(!Config.drop_rev)
            {
                GServer->LoadPYDropsData( );
            }
            else
            {
                GServer->LoadPYDropsDataAnd();
            }

         }
         else if(strcmp(tmp, "tdef")==0)
             LoadWayPoints( );
         else if(strcmp(tmp, "cmd")==0)
             LoadConfigurations( "commands.ini" );
         else if(strcmp(tmp, "cquests")==0)
         {
             #ifdef PYCUSTOM
             LoadCustomEvents( );
             LoadCustomTeleGate( );
             #else
             SendPM(thisclient,"the PY custom events are not enabled in this build, check code.");
             #endif
         }
         else if(strcmp(tmp, "events")==0)
         {
             #ifdef PYCUSTOM
             GServer->LoadCustomEvents( );
             GServer->LoadCustomTeleGate( );
             #else
             SendPM(thisclient,"the PY custom events are not enabled in this build, check code.");
             #endif
         }
         else if(strcmp(tmp, "quests")==0)
         {
             GServer->QuestList.clear( );
             GServer->LoadQuestData( );
         }
         else if(strcmp(tmp, "spawns")==0)
         {
             //LMA: outdated.
             SendPM(thisclient,"GM command outdated.");
             //GServer->LoadMonsterSpawn();
         }
         //else if(strcmp(tmp, "equip")==0)
         //{
         //    GServer->LoadEquip();
         //}
         else
         {
             Log( MSG_INFO, "Unrecognized reload command by GM %s" , thisclient->CharInfo->charname);
             return true;
         }
         Log( MSG_GMACTION, " %s : /reload %s" , thisclient->CharInfo->charname, tmp);
         char buffer2[200];
         sprintf ( buffer2, "%s data has been reloaded", tmp );
         SendPM(thisclient, buffer2);
         return true;
    }
    if (strcmp(command,"setwp")==0)  //PY new waypoint system added
    {
        // SYNTAX::  /setwp[wptype]
        // Sets a waypoint of the selected type at the current possition of the GM
        // 1 = Start point
        // 2 = regular
        // 3 = end point
        if(thisclient->Session->accesslevel < 900 )
            return true;
        if ((tmp = strtok(NULL, " "))==NULL)
            return true;
        unsigned int wptype = atoi(tmp);
        if(wptype > 3 )
        {
            SendPM(thisclient, "Data not saved. WP Type cannot be greater than 3");
            return true;
        }
        if(wptype == 0)
        {
            SendPM(thisclient, "Data not saved. WP Type cannot be zero");
            return true;
        }
        SendPM(thisclient, "REMINDER. Changes made here are not permanent. please use /savewp to make the change in the DB too");
        UINT nextentry = 1;
        bool firstentry = false;
        bool finalentry = false;
        UINT mapid = thisclient->Position->Map;
        for(int i=1;i<100;i++) //let's see what is already set for this map
        {
            int j = GServer->WPList[mapid][i].WPType;
            if(j > 0)
            {
                nextentry++;
                if(j == 1)
                    firstentry = true;
                if(j == 3)
                    finalentry = true;
            }
        }
        if(wptype == 1) //first Way Point
        {
            if(firstentry == true)
            {
                SendPM(thisclient, "The path for this map already contains a start point. You will need to delete the list before adding a new one");
                return true;
            }
            GServer->WPList[mapid][nextentry].WPType = 1;
            GServer->WPList[mapid][nextentry].X = thisclient->Position->current.x;
            GServer->WPList[mapid][nextentry].Y = thisclient->Position->current.y;
            SendPM(thisclient, "A new TDef Start point has been added for this map");
            return true;
        }
        if(wptype == 2) //regular Way Point
        {
            if(firstentry == false)
            {
                SendPM(thisclient, "The path for this map does not contain a start point. You will need to add one before you can add regular waypoints");
                return true;
            }
            if(finalentry == true)
            {
                SendPM(thisclient, "The path for this map already contains a Final destination point. You cannot add more regular way points until you delete it");
                return true;
            }
            GServer->WPList[mapid][nextentry].WPType = 2;
            GServer->WPList[mapid][nextentry].X = thisclient->Position->current.x;
            GServer->WPList[mapid][nextentry].Y = thisclient->Position->current.y;
            SendPM(thisclient, "A new TDef Way Point has been added for this map");
            return true;
        }
        if(wptype == 3) //Final Way Point
        {
            if(finalentry == true)
            {
                SendPM(thisclient, "The path for this map already contains a Final destination point. You will need to delete the list before adding a new one");
                return true;
            }
            GServer->WPList[mapid][nextentry].WPType = 3;
            GServer->WPList[mapid][nextentry].X = thisclient->Position->current.x;
            GServer->WPList[mapid][nextentry].Y = thisclient->Position->current.y;
            SendPM(thisclient, "A new TDef Final Point has been added for this map");
            return true;
        }
        SendPM(thisclient, "Unrecognised SetWP command");
        return true; //just to catch any non handled stuff
    }
    if (strcmp(command,"savewp")==0)
    {
        if(thisclient->Session->accesslevel < 900)
            return true;
        bool firstentry = false;
        bool finalentry = false;
        UINT mapid = thisclient->Position->Map;
        for(int i=1;i<100;i++) //let's see what is already set for this map
        {
            int j = GServer->WPList[mapid][i].WPType;
            if(j > 0)
            {
                if(j == 1)
                    firstentry = true;
                if(j == 3)
                    finalentry = true;
            }
        }
        if(firstentry == false)
        {
            SendPM(thisclient, "Cannot save to the database. The WayPoint list for this map contains no start point.");
            return true;
        }
        if(finalentry == false)
        {
            SendPM(thisclient, "Cannot save to the database. The WayPoint list for this map contains no final destination point.");
            return true;
        }
        //save the data
        GServer->DB->QExecute("DELETE FROM list_waypoints WHERE mapid=%i", mapid);
        for(int j=1;j<100;j++)
        {
            if(GServer->WPList[mapid][j].WPType != 0)
            GServer->DB->QExecute("INSERT INTO list_waypoints (mapid,wpnum,mapx,mapy,wptype) VALUES(%i,%i,%f,%f,%i)",
	            mapid, j, GServer->WPList[mapid][j].X, GServer->WPList[mapid][j].Y,GServer->WPList[mapid][j].WPType);
        }
        SendPM(thisclient, "The Waypoint list for this map has been saved to the database.");
        return true;
    }
    if (strcmp(command,"delwp")==0)
    {
        if(thisclient->Session->accesslevel < 900)
            return true;
        int counter = 0;
        for(int i=0;i<100;i++)
        {

            if(GServer->WPList[thisclient->Position->Map][i].WPType != 0)
            {
                GServer->WPList[thisclient->Position->Map][i].WPType = 0;
                GServer->WPList[thisclient->Position->Map][i].X = 0;
                GServer->WPList[thisclient->Position->Map][i].Y = 0;
                counter++;
            }
        }
        if(counter > 0)
        {
            SendPM(thisclient, "All WayPoints in this map have been deleted. Use /savewp to make the change in the DB too");
            return true;
        }
        SendPM(thisclient, "No Waypoints were found for this map");
        return true;
    }
    if (strcmp(command, "reloadquest")==0)
    {
        if(Config.Command_ReloadQuest > thisclient->Session->accesslevel)
	       return true;
        QuestList.clear( );
        LoadQuestData( );
        Log( MSG_GMACTION, " %s : /reloadquest" , thisclient->CharInfo->charname);
        return true;
    }
    if(strcmp(command, "rules")==0)  // Rules Command by Matt
    {
        if(Config.Command_Rules > thisclient->Session->accesslevel)
	       return true;
		SendPM(thisclient, "Please follow the following rules to ensure you have a fun time on this server!");
		SendPM(thisclient, "Rule 1: Hacking will be strictly dealt with!");
		SendPM(thisclient, "Rule 2: Do not abuse any bugs in the game");
		SendPM(thisclient, "Rule 3: Be respectful to all players and GM's");
		SendPM(thisclient, "Rule 4: Do not beg for items");
		SendPM(thisclient, "Rule 5: Do not harass any players or GM's");
		SendPM(thisclient, "Rule 6: No Botting or cheating allowed");
		SendPM(thisclient, "Rule 7: Do not abuse broadcast");
		return true;
    }
    if (strcmp(command, "save")==0) // *** SAVE USER DATA *****
    {
        if(Config.Command_Save > thisclient->Session->accesslevel)
	       return true;
        thisclient->savedata();
        return true;
	}
    if (strcmp(command, "savetown")==0) /// geo edit for /savetown // 30 sep 07
    {
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; int loc=atoi(tmp);
        unsigned int spot = 0;
        switch(loc)
        {
            case 1:  //AP
            {
                spot = 54;  // 1
                break;
            }
            case 2:  //Zant
            {
                spot = 1;  // 2
                break;
            }
            case 3:  //Junon
            {
                spot = 4;  // 4
                break;
            }
            case 4:  //Eucar
            {
                spot = 105;  // 5
                break;
            }
            case 5:  //Xita
            {
                spot = 122;  //68
                break;
            }
        }
        if(spot>0)
        {
            thisclient->Position->saved = spot;
            SendPM(thisclient, "Saved in Town");
        }
        else
        {
            SendPM(thisclient, "Please input a number after the savetown command, below is a list of places and their appropriate number");
            SendPM(thisclient, "1 = Adventurers Plains");
            SendPM(thisclient, "2 = The City of Zant");
            SendPM(thisclient, "3 = Junon Polis");
            SendPM(thisclient, "4 = The City of Eucar");
            SendPM(thisclient, "5 = Xita Refuge");
            SendPM(thisclient, "Example; /savetown 3");
        }

        return true;
    }
    if(strcmp(command, "serverinfo")==0)
    {
        if(Config.Command_ServerInfo > thisclient->Session->accesslevel)
	       return true;
        Log( MSG_GMACTION, " %s : /serverinfo" , thisclient->CharInfo->charname);
		return pakGMServerInfo( thisclient );
    }
    if (strcmp(command, "set") == 0)
    {
        if(Config.Command_Set > thisclient->Session->accesslevel)
           return true;
        int refine;
        if ((tmp = strtok(NULL, " ")) == NULL)
            return true;
        int id = atoi(tmp);
        if ((tmp = strtok(NULL, " ")) == NULL)
            refine = 0;
        else
            refine = atoi(tmp);
        //LMA: why *=16??? (because).
        refine *= 16;
        BEGINPACKET( pak, 0);
        int hatid = id;
        if(id > 200)hatid = id + 100; //for values above 200 the hats STB is off by 100 places
        if(EquipList[2].Index[hatid] != NULL)
        {
            thisclient->items[2].itemnum = hatid;
    		thisclient->items[2].itemtype = 2;
    		thisclient->items[2].refine = refine;
    		thisclient->items[2].durability = 50;
    		thisclient->items[2].lifespan = 100;
    		thisclient->items[2].count = 1;
    		thisclient->items[2].stats = 0;
    		thisclient->items[2].socketed = false;
    		thisclient->items[2].appraised = true;
        	thisclient->items[2].gem = 0;
        	thisclient->items[2].sp_value = 0;
        	thisclient->items[2].last_sp_value = 0;
            thisclient->UpdateInventory( 2 );

    	    RESETPACKET( pak, 0x7a5);
    	    ADDWORD( pak, thisclient->clientid );
    	    ADDWORD( pak, 0x0002);
    	    ADDWORD( pak, hatid);	// ITEM NUM
    	    ADDWORD( pak, BuildItemRefine( thisclient->items[2] ) );	// REFINE
    	    ADDWORD( pak, thisclient->Stats->Move_Speed );	// REFINE
    	    SendToVisible( &pak,thisclient );
        }
    	if(EquipList[3].Index[id] != NULL)
    	{
    		thisclient->items[3].itemnum = id;
    		thisclient->items[3].itemtype = 3;
    		thisclient->items[3].refine = refine;
    		thisclient->items[3].durability = 50;
    		thisclient->items[3].lifespan = 100;
    		thisclient->items[3].count = 1;
    		thisclient->items[3].stats = 0;
    		thisclient->items[3].socketed = false;
    		thisclient->items[3].appraised = true;
        	thisclient->items[3].gem = 0;
        	thisclient->items[3].sp_value = 0;
        	thisclient->items[3].last_sp_value = 0;
            thisclient->UpdateInventory( 3 );

	        RESETPACKET( pak, 0x7a5);
    	    ADDWORD( pak, thisclient->clientid );
    	    ADDWORD( pak, 0x0003);
    	    ADDWORD( pak, id);	// ITEM NUM
    	    ADDWORD( pak, BuildItemRefine( thisclient->items[3] ));	// REFINE
    	    ADDWORD( pak, thisclient->Stats->Move_Speed );	// REFINE
    	    SendToVisible( &pak,thisclient );
        }
        if(EquipList[4].Index[id]!=NULL)
        {
    		thisclient->items[5].itemnum = id;
    		thisclient->items[5].itemtype = 4;
    		thisclient->items[5].refine = refine;
    		thisclient->items[5].durability = 50;
    		thisclient->items[5].lifespan = 100;
    		thisclient->items[5].count = 1;
    		thisclient->items[5].stats = 0;
    		thisclient->items[5].socketed = false;
    		thisclient->items[5].appraised = true;
        	thisclient->items[5].gem = 0;
        	thisclient->items[5].sp_value = 0;
        	thisclient->items[5].last_sp_value = 0;
            thisclient->UpdateInventory( 5 );

    	    RESETPACKET( pak, 0x7a5);
     	    ADDWORD( pak, thisclient->clientid );
	        ADDWORD( pak, 0x0005);
    	    ADDWORD( pak, id);	// ITEM NUM
    	    ADDWORD( pak, BuildItemRefine( thisclient->items[5] ));	// REFINE
    	    ADDWORD( pak, thisclient->Stats->Move_Speed );	// REFINE 2602
    	    SendToVisible( &pak,thisclient );
        }
        if(EquipList[5].Index[id]!=NULL)
        {
    		thisclient->items[6].itemnum = id;
    		thisclient->items[6].itemtype = 5;
    		thisclient->items[6].refine = refine;
    		thisclient->items[6].durability = 50;
    		thisclient->items[6].lifespan = 100;
    		thisclient->items[6].count = 1;
      		thisclient->items[6].stats = 0;
    		thisclient->items[6].socketed = false;
    		thisclient->items[6].appraised = true;
        	thisclient->items[6].gem = 0;
        	thisclient->items[6].sp_value = 0;
        	thisclient->items[6].last_sp_value = 0;
            thisclient->UpdateInventory( 6 );

    	    RESETPACKET( pak, 0x7a5);
    	    ADDWORD( pak, thisclient->clientid );
    	    ADDWORD( pak, 0x0006);
    	    ADDWORD( pak, id);	// ITEM NUM
    	    ADDWORD( pak, BuildItemRefine( thisclient->items[6] ));	// REFINE
    	    ADDWORD( pak, thisclient->Stats->Move_Speed);	// REFINE
    	    SendToVisible( &pak,thisclient );
        }
        thisclient->SetStats( );
        SendPM( thisclient, "GM command Set %i", id);
		return true;
    }
    if(strcmp(command, "Setqflag")==0)
    {
        if(Config.Command_Setqflag > thisclient->Session->accesslevel)
           return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true;
			unsigned int var = atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true;
			unsigned int val = atoi(tmp);
		if (var >= (0x40 * 8)) return true;
		thisclient->quest.SetFlag(var, (val > 0) ? true : false);
		SendPM( thisclient, "Set Flag[%i] = %i", var, val);
		return true;
    }
    if(strcmp(command, "setqvar")==0)
    {
        if(Config.Command_Setqvar > thisclient->Session->accesslevel)
            return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true;
            unsigned int vartype = atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true;
            unsigned int var = atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true;
            unsigned int val = atoi(tmp);
		if (vartype > 3) return true;
		switch (vartype) {
			case 0:
			  if (var >= 5) return true;
			  thisclient->quest.EpisodeVar[var] = val;
			break;
			case 1:
			  if (var >= 3) return true;
			  thisclient->quest.JobVar[var] = val;
			break;
			case 2:
			  if (var >= 7) return true;
			  thisclient->quest.PlanetVar[var] = val;
			break;
			case 3:
			  if (var >= 10) return true;
			  thisclient->quest.UnionVar[var] = val;
			break;
		}
		SendPM( thisclient, "Set Var[%i][%i] = %i", vartype, var, val);
		return true;
    }
    if(strcmp(command, "settime")==0)
    {
        if(Config.Command_Settime > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        unsigned int time = atoi(tmp);
        if(MapList.Index[thisclient->Position->Map]!=NULL)
        {
            switch(time)
            {
                case MORNING:
                    MapList.Index[thisclient->Position->Map]->MapTime = MapList.Index[thisclient->Position->Map]->morningtime;
                    SendSysMsg( thisclient, "Time changed!, rejoin to see change [MORNING]" );
                break;
                case DAY:
                    MapList.Index[thisclient->Position->Map]->MapTime = MapList.Index[thisclient->Position->Map]->daytime;
                    SendSysMsg( thisclient, "Time changed!, rejoin to see change [DAY]" );
                break;
                case EVENING:
                    MapList.Index[thisclient->Position->Map]->MapTime = MapList.Index[thisclient->Position->Map]->eveningtime;
                    SendSysMsg( thisclient, "Time changed!, rejoin to see change [EVENING]" );
                break;
                case NIGHT:
                    MapList.Index[thisclient->Position->Map]->MapTime = MapList.Index[thisclient->Position->Map]->nighttime;
                    SendSysMsg( thisclient, "Time changed!, rejoin to see change [NIGHT]" );
                break;
                default:
                    SendSysMsg( thisclient, "Time: 0=MORNING | 1=DAY | 2=EVENING | 3=NIGHT" );
            }
        }
		return true;
    }
    if(strcmp(command, "gemquest")==0)
    {
        if(Config.Command_SetGemQuest > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
         if(strcmp(tmp, "reset")==0)
         {
             if(GServer->ObjVar[1104][0]!=1)
             {
                 SendPM(thisclient, "Gem quest isn't active.");
                 return true;
             }

             GServer->ObjVar[1104][13]=0;
             SendPM(thisclient, "Gem quest spawn counter reseted.");
             return true;
         }
         else if(strcmp(tmp, "stop")==0)
         {
             if(GServer->ObjVar[1104][0]!=1)
             {
                 SendPM(thisclient, "Gem quest isn't active.");
                 return true;
             }

             GServer->ObjVar[1104][0]=0;
             GServer->ObjVar[1104][12]=0;
             GServer->ObjVar[1104][13]=0;
             GServer->GemQuestForce=0;
             GServer->GemQuestReset=0;
             //changing the dialogID.
             pakGMSetObjVar(thisclient,1104,0,0);
             SendPM(thisclient, "Gem quest spawn stopped.");
             return true;
         }
         else if(strcmp(tmp, "info")==0)
         {
             if(GServer->ObjVar[1104][0]!=1)
             {
                 SendPM(thisclient, "Gem quest isn't active.");
                 return true;
             }

             SendPM(thisclient, "Gem quest active, spawn counter: %i, auto reset set to %i",GServer->ObjVar[1104][13],GServer->GemQuestReset);
             return true;
         }
         else if(strcmp(tmp, "start")==0)
         {
             if(GServer->ObjVar[1104][0]!=0)
             {
                 SendPM(thisclient, "Gem quest is already active.");
                 return true;
             }

            if ((tmp = strtok(NULL, " "))==NULL)
            {
                return true;
            }

            int time = atoi(tmp);
            if (time<0)
            {
                return true;
            }

            GServer->GemQuestReset=0;
            if ((tmp = strtok(NULL, " "))!=NULL)
            {
                GServer->GemQuestReset=atoi(tmp);
                if (GServer->GemQuestReset<=0)
                {
                    GServer->GemQuestReset=0;
                }

            }

             SendPM( thisclient, "gem quest will forced to start in %i minutes, auto reset set to %i",time,GServer->GemQuestReset);
             pakGMForceGemQuest(thisclient,time);


             return true;
         }


        return true;
    }
    if(strcmp(command, "setuw")==0)
    {
        if(Config.Command_SetUW > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        int time = atoi(tmp);

        if (time<0)
            return true;

        if (time==0)
        {
            SendPM( thisclient, "%s: /setuw %i (UW forcing deactivated)",thisclient->CharInfo->charname,time);
        }
        else
        {
            SendPM( thisclient, "%s: /setuw %i (UW will forced to start in %i minutes)",thisclient->CharInfo->charname,time,time);
        }


        return pakGMForceUW(thisclient,time);
    }
    if(strcmp(command, "setuwnb")==0)
    {
        if(Config.Command_SetUWnb > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        int nb_players = atoi(tmp);

        if (nb_players<0)
            return true;

        if (nb_players==0)
        {
            SendPM( thisclient, "%s: /setuwnb %i (UW nb players forcing deactivated)",thisclient->CharInfo->charname,nb_players,nb_players);
        }
        else
        {
            SendPM( thisclient, "%s: /setuwnb %i (UW needs %i players on each side to begin)",thisclient->CharInfo->charname,nb_players,nb_players);
        }


        return pakGMForceUWPlayers(thisclient,nb_players);
    }
    if (strcmp(command, "shoptype")==0)
	{
        if(Config.Command_ShopType > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
            unsigned int shoptype =atoi(tmp);
        thisclient->Shop->ShopType = shoptype;
        Log( MSG_GMACTION, " %s : /shoptype %i" , thisclient->CharInfo->charname, shoptype);
		return true;
    }
    if (strcmp(command, "skillp")==0)
    {
        if (Config.Command_Stat > thisclient->Session->accesslevel)
            return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; int skillp = atoi(tmp);
        char* name;
        if ((tmp = strtok(NULL, " "))==NULL)
            name = thisclient->CharInfo->charname;
        else
            name = tmp;
        CPlayer* otherclient = GetClientByCharName(name);
        if (otherclient == NULL)
            return true;
        if ((int)otherclient->CharInfo->SkillPoints + skillp < 0)
            otherclient->CharInfo->SkillPoints = 0;
        else
            otherclient->CharInfo->SkillPoints += skillp;
        BEGINPACKET( pak, 0x720 );
        ADDWORD( pak, 37 );
        ADDWORD( pak, skillp );
        ADDWORD( pak, 0 );
        otherclient->client->SendPacket( &pak );
        RESETPACKET( pak, 0x0730 );
        ADDWORD( pak, 5 );
        ADDWORD( pak, 0xa24d );
        ADDWORD( pak, 0x40b3 );
        otherclient->client->SendPacket( &pak );
        Log(MSG_GMACTION, " %s : /skillp %i %s", thisclient->CharInfo->charname, skillp, name);
        return true;
    }
    if (strcmp(command, "shutdown")==0)
	{
        if(Config.Command_Shutdown > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
           {
           Log( MSG_GMACTION, " %s : /showdown NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "shutdown NOT ALLOWED");
           SendPM(thisclient, buffer);
	         return true;
           }
        if ((tmp = strtok(NULL, " "))==NULL) return true;
            unsigned int minutes =atoi(tmp);
        char text[200];
        sprintf( text, "Server will shutdown in %i minutes, Please logout NOW to be sure your information is saved correctly.",
           minutes );
        BEGINPACKET( pak, 0x702 );
	    ADDSTRING  ( pak, thisclient->CharInfo->charname );
    	ADDSTRING  ( pak, "> " );
    	ADDSTRING  ( pak, text );
    	ADDBYTE    ( pak, 0x00 );
    	SendToAll  ( &pak );
        pthread_create( &WorldThread[SHUTDOWN_THREAD], NULL, ShutdownServer, (PVOID)minutes);
        Log( MSG_GMACTION, " %s : /shutdown %u" , thisclient->CharInfo->charname, shutdown);
		return true;
    }
    if(strcmp(command, "skill")==0)
    {
        if (Config.Command_KillInRange > thisclient->Session->accesslevel)
            return true;

		if ((tmp = strtok(NULL, " "))==NULL) return true;
		unsigned skillid = atoi(tmp);
		Log(MSG_GMACTION, "%s cast skill %i", thisclient->CharInfo->charname, skillid);
		SendPM(thisclient, "Casting skill %i", skillid);
		CCharacter* Target = thisclient->GetCharTarget( );
		CSkills* thisskill = GetSkillByID(skillid);
		thisclient->UseSkill(thisskill, Target);
		return true;
    }
    if(strcmp(command, "spawnlist")==0)
    {
        if (Config.Command_SpawnList > thisclient->Session->accesslevel)
            return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; int range=atoi(tmp);
	    if (range<0)
	    {
	        return true;
	    }

	    Log( MSG_GMACTION, "%s:: spawnlist %i" ,thisclient->CharInfo->charname,range);
	    return pakGMSpawnList(thisclient, range);
	}
    if(strcmp(command, "spawndetail")==0)
    {
        if (Config.Command_SpawnDetail > thisclient->Session->accesslevel)
            return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; int map=atoi(tmp);
	    if((tmp = strtok(NULL, " "))==NULL) return true; int id=atoi(tmp);
	    if (map<0||id<0)
	    {
	        return true;
	    }

	    Log( MSG_GMACTION, "%s:: spawndetail %i %i" ,thisclient->CharInfo->charname,map,id);
	    return pakGMSpawnDetail(thisclient,id,map);
	}
	#ifdef LMA_SPAWNM
        else if(strcmp(command, "spawnmonsters")==0)
        {
            if((tmp = strtok(NULL, " "))==NULL) return true; int mdeb=atoi(tmp);
            if((tmp = strtok(NULL, " "))==NULL) return true; int mend=atoi(tmp);
            if((tmp = strtok(NULL, " "))==NULL) return true; int xx=atoi(tmp);
            if((tmp = strtok(NULL, " "))==NULL) return true; int yy=atoi(tmp);

            if(mdeb==mend&&mdeb==0)
            {
                //this is the mire, save it if you need to setup a grid in drop_me
                SendPM(thisclient,"Spawning monster 0123456789 (mire).");
                return true;
            }

            SendPM(thisclient,"Doing from %u to %u at (%u;%u).",mdeb,mend,xx,yy);

            thisclient->lastSpawnUpdate=clock();
            thisclient->last_monster=0;
            thisclient->last_monstercid=0;
            thisclient->mdeb=mdeb;
            thisclient->mend=mend;
            thisclient->xx=xx;
            thisclient->yy=yy;
            thisclient->playertime=3000;
            return true;
        }
	#endif
    if(strcmp(command, "spawnrefresh")==0)
    {
        if (Config.Command_SpawnRefresh > thisclient->Session->accesslevel)
            return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; int map=atoi(tmp);
	    if((tmp = strtok(NULL, " "))==NULL) return true; int id=atoi(tmp);
	    if (map<0||id<0)
	    {
	        return true;
	    }

	    Log( MSG_GMACTION, "%s:: spawnrefresh %i %i" ,thisclient->CharInfo->charname,map,id);
	    return pakGMSpawnForceRefresh(thisclient,id,map);
	}
    if(strcmp(command, "SpeedModif")==0)
    {
        if (Config.Command_SpeedModif > thisclient->Session->accesslevel)
            return true;
	    if((tmp = strtok(NULL, " "))==NULL) return true; unsigned mSpeedModif=atoi(tmp);
	    Log( MSG_GMACTION, " mSpeedModif changed to %i by %s" , mSpeedModif, thisclient->CharInfo->charname);
	    return pakGMChangeMSpeedModif(thisclient, mSpeedModif);
	}
    if (strcmp(command, "stat")==0) // Code By Minoc
    {
        if(Config.Command_Stat > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* statname =(char*)tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true; int statvalue    = atoi(tmp);
        if (statvalue > Config.MaxStat)  // code by PurpleYouko for setting max stat to 300
            statvalue = Config.MaxStat;  // extra code to match setting in worldserver.conf by Atomsk
        Log( MSG_GMACTION, " %s : /stat %s %i" , thisclient->CharInfo->charname, statname, statvalue);
        return pakGMStat( thisclient , statname , statvalue);
    }
    if (strcmp(command, "statp")==0)
    {
        if (Config.Command_Stat > thisclient->Session->accesslevel)
            return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; int statvalue = atoi(tmp);
        char* name;
        if ((tmp = strtok(NULL, " "))==NULL)
            name = thisclient->CharInfo->charname;
        else
            name = tmp;
        CPlayer* otherclient = GetClientByCharName(name);
        if (otherclient == NULL)
            return true;
        if ((int)otherclient->CharInfo->StatPoints + statvalue < 0)
            otherclient->CharInfo->StatPoints = 0;
        else
            otherclient->CharInfo->StatPoints += statvalue;
        BEGINPACKET( pak, 0x720 );
        ADDWORD( pak, 32 );
        ADDWORD( pak, statvalue );
        ADDWORD( pak, 0 );
        otherclient->client->SendPacket( &pak );
        RESETPACKET( pak, 0x0730 );
        ADDWORD( pak, 5 );
        ADDWORD( pak, 0xa24d );
        ADDWORD( pak, 0x40b3 );
        otherclient->client->SendPacket( &pak );
        Log(MSG_GMACTION, " %s : /stats %i %s", thisclient->CharInfo->charname, statvalue, name);
        return true;
    }
    if(strcmp(command,"summon")==0)
    {
        if(Config.Command_Summon > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true;
        unsigned int summon = atoi(tmp);

        unsigned int lma_aip=0;
        unsigned int lma_hp=0;
        if ((tmp = strtok(NULL, " "))!=NULL)
        {
            lma_aip = atoi(tmp);

            if ((tmp = strtok(NULL, " "))!=NULL)
            {
                lma_hp = atoi(tmp);
            }

        }

        Log( MSG_GMACTION, " %s : /summon %i %i %u" , thisclient->CharInfo->charname, summon,lma_aip,lma_hp);
        fPoint position = RandInCircle( thisclient->Position->current, 5 );
        CMap* map = MapList.Index[thisclient->Position->Map];
        CMonster* thismonster=map->AddMonster( summon, position, thisclient->clientid );

        //LMA: giving a special AIP to a monster...
        if (thismonster!=NULL&&lma_aip!=0)
        {
            thismonster->sp_aip=lma_aip;
        }

        if(thismonster!=NULL&&lma_hp!=0)
        {
            thismonster->Stats->MaxHP=lma_hp;
            thismonster->Stats->HP=lma_hp;
        }
        else
        {
            //LMA: new formula ;)
            thismonster->Stats->MaxHP=SummonFormula(thisclient,thismonster);
            thismonster->Stats->HP=thismonster->Stats->MaxHP;
            //Log(MSG_WARNING,"New HP value for summon %I64i",thismonster->Stats->HP);
        }

        //Start Animation
        BEGINPACKET( pak, 0x7b2 );
        ADDWORD    ( pak, thisclient->clientid );
        ADDWORD    ( pak, 2802 );
        SendToVisible( &pak, thisclient );
        //Finish Animation
        RESETPACKET( pak, 0x7bb );
        ADDWORD    ( pak, thisclient->clientid );
        SendToVisible( &pak, thisclient );
        //????
        RESETPACKET( pak, 0x7b9);
        ADDWORD    ( pak, thisclient->clientid);
        ADDWORD    ( pak, 1286 );
	    SendToVisible( &pak, thisclient );
	    // Add our Mob to the mobs list
		return true;
    }
    if(strcmp(command, "targetinfo")==0)
    {
        if(Config.Command_TargetInfo > thisclient->Session->accesslevel)
	       return true;
        return GMShowTargetInfo( thisclient );
    }
    if (strcmp(command, "tele")==0) // **** TELEPORT TO MAX AND X Y POINTS *****
    {
        if(Config.Command_Tele > thisclient->Session->accesslevel)
	       return true;
        int noqsd=0;
		if ((tmp = strtok(NULL, " "))==NULL) return true; unsigned map=atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true; float x=(float)atoi(tmp);
		if ((tmp = strtok(NULL, " "))==NULL) return true; float y=(float)atoi(tmp);

		if ((tmp = strtok(NULL, " "))!=NULL)
		{
		    noqsd =atoi(tmp);
		}

        Log( MSG_GMACTION, " %s : /tele %i,%.2f,%.2f" , thisclient->CharInfo->charname, map, x, y);
        return pakGMTele(thisclient, map, x, y,noqsd);
    }
    if (strcmp(command, "teletome")==0)
    {
        if(Config.Command_TeleToMe > thisclient->Session->accesslevel)
	       return true;
		if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
		Log( MSG_GMACTION, " %s : /teletome %s" , thisclient->CharInfo->charname, name);
        if(strcmp(name,"all")==0)
        {
            return pakGMTeleAllHere(thisclient);
        }
		return pakGMTelePlayerHere(thisclient, name);
    }
    if (strcmp(command, "temple")==0)
    {
        if(Config.Command_Temple > thisclient->Session->accesslevel)
	       return true;

		if ((tmp = strtok(NULL, " "))==NULL) return true; int value=atoi(tmp);
		Log( MSG_GMACTION, " %s : /temple %i" , thisclient->CharInfo->charname, value);
		if(value==0)
		{
		    //Temple is closed.
		    ObjVar[1075][0]=0;
		    ObjVar[1075][1]=0;

		    SendPM(thisclient,"Oblivion temple is closed now.");
		}
		else if(value>=50)
		{
		    //Temple is opened
		    ObjVar[1075][0]=8650;
		    ObjVar[1075][1]=0;
		    SendPM(thisclient,"Oblivion temple is opened now.");
		}
		else
		{
		    //Some money remain to be collected.
		    ObjVar[1075][0]=0;
		    ObjVar[1075][1]=value;
		    if(value>=10)
		    {
		        ObjVar[1075][0]=1;
		    }
		    if(value>=20)
		    {
		        ObjVar[1075][0]=2;
		    }
		    if(value>=30)
		    {
		        ObjVar[1075][0]=3;
		    }
		    if(value>=40)
		    {
		        ObjVar[1075][0]=4;
		    }

		    SendPM(thisclient,"Oblivion temple has %i millions.",value);
		}

        LastTempleAccess[0]=ObjVar[1075][0];
        LastTempleAccess[1]=ObjVar[1075][1];
        DB->QExecute("UPDATE list_npcs SET eventid=%i, extra_param=%i WHERE type=1075",ObjVar[1075][0],ObjVar[1075][1]);
		return true;
    }
    if(strcmp(command, "tquest")==0)
    {
        if(Config.Command_tquest > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
       {
           Log( MSG_GMACTION, " %s : /tquest NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "tquest NOT ALLOWED");
           SendPM(thisclient, buffer);
             return true;
       }

        if ((tmp = strtok(NULL, " "))==NULL) return true; int action= atoi( tmp );
        if ((tmp = strtok(NULL, " "))==NULL) return true; int headerid= atoi( tmp );

       //810-01 00 00 00 fd 1a 00 00 66 00 d0 07
        BEGINPACKET( pak, 0x810);
        ADDBYTE    (pak, headerid);
        ADDBYTE    (pak, 0x00);
        ADDBYTE    (pak, 0x00);
        ADDBYTE    (pak, 0x00);
        ADDBYTE    (pak, thisclient->CharInfo->unionid);
        ADDBYTE    (pak, 0x00);
        ADDBYTE    (pak, 0x00);
        ADDBYTE    (pak, 0x00);
        ADDBYTE    (pak, 0x9);  //Map
        ADDBYTE    (pak, 0x00);
        if (action==1)
        {
            ADDBYTE    (pak, 0xd0);
            ADDBYTE    (pak, 0x07);
        }
        else
        {
            ADDBYTE    (pak, 0xE8);
            ADDBYTE    (pak, 0x03);
        }
        thisclient->client->SendPacket( &pak );
        return true;
	}
    if(strcmp(command, "transx")==0)
    {
        if(Config.Command_Transx > thisclient->Session->accesslevel)
           return true;
        thisclient->CharInfo->Sex = thisclient->CharInfo->Sex==0?1:0;
        BEGINPACKET( pak, 0x720 );
        ADDWORD( pak, 2 );
        ADDWORD( pak, thisclient->CharInfo->Sex );
        ADDWORD( pak, 0 );
        thisclient->client->SendPacket( &pak );
        RESETPACKET( pak, 0x0730 );
        ADDWORD( pak, 5 );
        ADDWORD( pak, 0xa24d );
        ADDWORD( pak, 0x40b3 );
        thisclient->client->SendPacket( &pak );
        GServer->DB->QExecute("UPDATE characters SET sex=%i WHERE id=%i", thisclient->CharInfo->Sex, thisclient->CharInfo->charid);
		return true;
    }
    if (strcmp(command, "union")==0)
    {
        if(Config.Command_Union > thisclient->Session->accesslevel)
	       return true;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true; int which_union= atoi( tmp );
        return pakGMUnion(thisclient,name,which_union);
    }
    if(strcmp(command, "unionpoints")==0)
    {
        if(Config.Command_UnionPoints > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
       {
           Log( MSG_GMACTION, " %s : /unionpoints NOT ALLOWED" , thisclient->CharInfo->charname);
           char buffer[200];
           sprintf ( buffer, "unionpoints NOT ALLOWED");
           SendPM(thisclient, buffer);
             return true;
       }

        if ((tmp = strtok(NULL, " "))==NULL) return true; char* namemode=tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true; int value= atoi( tmp );
        return pakGMUnionPoints(thisclient, namemode, value);
	}
    if(strcmp(command, "who")==0)
    {
        if(Config.Command_Who > thisclient->Session->accesslevel)
           return true;
        int count=1;
        int gmcount=0;
        int playercount=0;
        char line0[200];
        while(count <= (ClientList.size()-1))
        {
            CPlayer* whoclient = (CPlayer*)ClientList.at(count)->player;
            if(whoclient->isInvisibleMode2 != true)
            {
                    if(whoclient->Session->accesslevel > 100)
                    {
                        gmcount++;
                    }
                    else
                    {
                        playercount++;
                    }
            }
            count++;
        }
        sprintf(line0, "There are currently %i players and %i game moderators currently connected.", playercount, gmcount);
        Log( MSG_GMACTION, " %s : /who" , thisclient->CharInfo->charname);
        SendPM(thisclient, line0 );
        return true;
    }
    if(strcmp(command, "who2")==0)
    {
        if(Config.Command_Who2 > thisclient->Session->accesslevel)
           return true;
        SendPM(thisclient, "The following players are currently connected;");
        int count=0;
        int hiddenam=0;
        char line0[200];
        while(count <= (ClientList.size()-1))
        {
            CPlayer* whoclient = (CPlayer*)ClientList.at(count)->player;
            if(whoclient->Session->accesslevel > 100)
            {
                //sprintf(line0, "%s - GM[%i]", whoclient->CharInfo->charname, whoclient->Session->accesslevel);
                sprintf(line0, "%s - (GM %i, lvl %u), map %u[%.2f,%.2f]", whoclient->CharInfo->charname, whoclient->Session->accesslevel,
                whoclient->Stats->Level,
                whoclient->Position->Map,
                whoclient->Position->current.x,
                whoclient->Position->current.y);
            }
            else
            {
                //sprintf(line0, "%s", whoclient->CharInfo->charname);
                sprintf(line0, "%s (job %u, lvl %u), map %u[%.2f,%.2f]", whoclient->CharInfo->charname,
                whoclient->CharInfo->Job,
                whoclient->Stats->Level,
                whoclient->Position->Map,
                whoclient->Position->current.x,
                whoclient->Position->current.y);
            }

            if(!whoclient->isInvisibleMode || thisclient->Session->accesslevel >= whoclient->Session->accesslevel)
            {
                SendPM(thisclient, line0 );
            }
            else
            {
                hiddenam++;
            }

            count++;
        }

        sprintf(line0, "There are currently %i players connected!", ((ClientList.size()-1)-hiddenam));
        SendPM(thisclient, line0 );
        Log( MSG_GMACTION, " %s : /who2" , thisclient->CharInfo->charname);
        return true;
    }
    if(strcmp(command, "whoattacksme")==0)
	{
	    return pakGMWhoAttacksMe(thisclient);
    }
    if(strcmp(command, "writeqvar")==0)
    {
        if(Config.Command_Listqvar > thisclient->Session->accesslevel)
           return true;

        if ((tmp = strtok(NULL, " "))==NULL) return true; int type=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true; int index=atoi(tmp);
        if ((tmp = strtok(NULL, " "))==NULL) return true; int value= atoi( tmp );

        if(value<0)
            return true;

        switch(type)
        {
            case 3:
            {
              if(index >= 5) return true;
              thisclient->quest.EpisodeVar[index] = value;
            }
            break;
            case 4:
            {
              if(index >= 3) return true;
              thisclient->quest.JobVar[index] = value;
            }
            break;
            case 5:
            {
              if(index >= 7) return true;
              thisclient->quest.PlanetVar[index] = value;
            }
            break;
            case 6:
            {
              if(index >= 10) return true;
              thisclient->quest.UnionVar[index] = value;
            }
            break;
            case 7:
            {
                if(index >= 5) return true;
                thisclient->quest.ClanVar[index] = value;
            }
            break;
            case 8:
            {
                if(index >= 512) return true;
                if(value!=0&&value!=1) return true;
                thisclient->quest.SetFlag(index,value);
            }
            break;
            default:
            {
                SendPM(thisclient,"Incorrect type, only 3 to 7 available");
                return true;
            }

        }

        SendPM(thisclient,"type %i[%i] set to %i, please relog!",type,index,value);
        Log(MSG_GMACTION,"%s has used /writeqvars %i %i %i",thisclient->CharInfo->charname,type,index,value);
		return true;
    }
	if(strcmp(command, "warning")==0){  // warning command
        if(Config.Command_Warning > thisclient->Session->accesslevel)
        {
            SendPM(thisclient, "YOU DON'T HAVE THE RIGHT PRIVILEDGES TO USE THE WARNING COMMAND");
            return true;
        }
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* name=tmp;
        if ((tmp = strtok(NULL, " "))==NULL) return true; char* reason=tmp;

        Log( MSG_GMACTION, " %s : /warning %s REASON:%s" , thisclient->CharInfo->charname, name, reason);

        return pakGMWarning( thisclient, name, reason );

	}
	if(strcmp(command, "warningcount") == 0){

	    return pakWarningCount(thisclient);
    }
	if(strcmp(command, "warningreason") ==0){

        return pakWarningReason(thisclient);
    }
	if(strcmp(command, "seewarning") == 0){

        if(Config.Command_SeeWarning > thisclient->Session->accesslevel)
        {
            SendPM(thisclient, "YOU DON'T HAVE THE RIGHT PRIVILEDGES TO USE THE COMMAND");
            return true;
        }
        if((tmp = strtok(NULL, " ")) == NULL) return true; char* name=tmp;
        if(!name){

            SendPM(thisclient,"Please enter the name of the player who's warnings you view");
            return true;

        }else{
            Log( MSG_GMACTION, " %s: /seewarning %s",thisclient->CharInfo->charname, name);

            //return pakGMSeeWarning(thisclient, name);
        }
		return true;
    }
	if(strcmp(command,"delwarning")==0){


        if(Config.Command_DelWarning > thisclient->Session->accesslevel)
        {
            SendPM(thisclient, "YOU DON'T HAVE THE RIGHT PRIVILEDGES TO USE THE COMMAND");
        }
        if((tmp = strtok(NULL, " ")) == NULL) return true; char* name=tmp;
        if((tmp = strtok(NULL, " ")) == NULL) return true; int warning=atoi(tmp);

        if(!name){
            SendPM(thisclient, "Please make sure you enter of the player who's warning you wish to delete");
        }else{
            if(!warning){
                SendPM(thisclient, "Please make sure you enter the number of the warning you wish to delete");
            }else{
                //pakGMDelWarning(thisclient, name, warning);
            }
        }
		return true;
    }
	if (strcmp(command, "ticket")==0) // Submit ticket
    {
        char ticketTest[200];
        char date[9];
        MYSQL_ROW sqlticketrow;
        MYSQL_RES *sqlticketresult = NULL;
        sprintf( ticketTest, "%s", &P->Buffer[8] );
        int tickFind = 0;
        string ticket = ticketTest;

        tickFind = ticket.find("'");

        while(tickFind != -1)
        {
            ticket.erase(tickFind,1);
            tickFind = ticket.find("'",tickFind);
        }

        if(ticketTest != NULL)
        {
            GServer->DB->QExecute("INSERT INTO tickets (char_name,date,message) VALUES('%s','%s','%s')",thisclient->CharInfo->charname,_strdate(date), ticket.c_str());
            SendPM(thisclient, "Your ticket has been sent. We will contact you as soon as possible.");

            sqlticketresult = DB->QStore("SELECT id FROM tickets WHERE char_name = '%s' AND date = '%s' AND message = '%s'",thisclient->CharInfo->charname,_strdate(date), ticket.c_str());
            sqlticketrow = mysql_fetch_row(sqlticketresult);
            DB->QFree();

            if(sqlticketrow == NULL)
            {
                SendPM(thisclient, "Ticket error, please try again or contact a GM");
            }else
            {
                SendPM(thisclient, "Your ticket id is %i, please write it down.",atoi(sqlticketrow[0]));
            }

            Log( MSG_GMACTION, " %s : /ticket %s", thisclient->CharInfo->charname, ticket.c_str());
        } else
        {
            SendPM(thisclient, "Please enter a valid message. Example: /ticket I am stuck.");
        }
		return true;
    }
    if (strcmp(command, "deltick")==0) // Delete Ticket
    {
        if(Config.Command_DelTicket > thisclient->Session->accesslevel)
           return true;
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; int tick=atoi(tmp);
        GServer->DB->QExecute("DELETE FROM tickets WHERE id=%i", tick);
        SendPM(thisclient,"Ticket %i deleted.", tick);
        Log( MSG_GMACTION, " %s : /deltick %i", thisclient->CharInfo->charname, tick);
		return true;
    }
    if (strcmp(command, "readtick")==0) // Read Tickets
    {
        if(Config.Command_ReadTick > thisclient->Session->accesslevel)
           return true;

        MYSQL_ROW row;
        MYSQL_RES *result = NULL;

        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0;
        int readtick=atoi(tmp);
        if(readtick == 0)
        {
            SendPM(thisclient, "Please choose a ticket number from list below:");
            result = DB->QStore("SELECT id,char_name,date,opened FROM tickets WHERE id>0");
            while(row = mysql_fetch_row(result)) SendPM(thisclient, "%i - %s - %s - %s",atoi(row[0]),row[1],row[2],row[3]);
            DB->QFree();
            SendPM(thisclient, "Format: Ticket ID - Character Name - Date(M/D/Y)");
            SendPM(thisclient, "Please input /readtick id");
            Log(MSG_GMACTION," %s : /readtick",thisclient->CharInfo->charname);
        } else
        {
            result = DB->QStore("SELECT id,message FROM tickets WHERE id=%i", readtick);
            row = mysql_fetch_row(result);

            if(result == NULL)
            {
                SendPM(thisclient,"Please choose a correct ticket id");
                DB->QFree();
                return true;
            }

            if(row == NULL)
            {
                SendPM(thisclient,"Please choose a correct ticket id");
                DB->QFree();
                return true;
            }

            SendPM(thisclient, "%s - %s",row[0],row[1]);
            DB->QFree();
            GServer->DB->QExecute("UPDATE tickets SET opened = 'Y', viewed = '%s' WHERE id=%i",thisclient->CharInfo->charname, readtick);
            Log(MSG_GMACTION," %s : /readtick %i",thisclient->CharInfo->charname,readtick);
        }
		return true;
    }
    if (strcmp(command, "time")==0) // show time
    {
        time_t rawtime;
        struct tm * timeinfo;

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );

        SendPM(thisclient,"The current server time is: %s",asctime(timeinfo));
        Log(MSG_INFO,"%s checked the time",thisclient->CharInfo->charname);
		return true;
    }
    if (strcmp(command, "happyhour")==0) // show hh map
    {
        if(GServer->Config.doubleExpMap != 0)
		{
			SendPM(thisclient, "You will receive double exp on map - %s - Map ID %i", GServer->Config.mapName.c_str(), GServer->Config.doubleExpMap);
		}else
		{
			SendPM(thisclient, "Sorry no happy hour map active");
		}
		return true;
    }
    if (strcmp(command, "hhexp")==0) // change hh exp
    {
        if(Config.Command_hhExp > thisclient->Session->accesslevel)
           return true;

        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0;
        int expAmount=atoi(tmp);

        if(expAmount != 0)
        {
            GServer->Config.doubleExpAmount = expAmount;
            SendPM(thisclient, "HH exp now: %i", GServer->Config.doubleExpAmount);
			Log(MSG_GMACTION, " %s : /hhexp %i", thisclient->CharInfo->charname,expAmount);
        }else
        {
            SendPM(thisclient, "Please choose an amount bigger than 0");
        }
		return true;
    }
    if(strcmp(command, "hhOn")==0) // Activate HH
    {
        if(Config.Command_hhExp > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
        {
            SendPM(thisclient, "You are not allowed to activate Happy Hour");
            Log(MSG_GMACTION," %s : /hhOn NOT ALLOWED", thisclient->CharInfo->charname);
            return true;
        }

        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0;
        int hhOn=atoi(tmp);

        if(hhOn == 1)
        {
            doubleExpMapActivate();
            SendPM(thisclient, "HH activated");
			Log(MSG_GMACTION, " %s : /hhOn active", thisclient->CharInfo->charname);
        }
		if(hhOn == 0)
		{
			doubleExpMapDeactivate();
			GServer->Config.doubleExpActive = 0;
			SendPM(thisclient, "HH deactivated");
			Log(MSG_GMACTION, " %s : /hhOn deactive", thisclient->CharInfo->charname);
		}
		return true;
    }
    if(strcmp(command, "lottobegin")==0)
    {
        if(GServer->Config.Command_Lotto > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
        {
            SendPM(thisclient, "You are not allowed begin the Lotto");
            Log(MSG_GMACTION," %s : /lottobegin NOT ALLOWED", thisclient->CharInfo->charname);
            return true;
        }

        if(GServer->Config.LottoActive == 1)
        {
            SendPM(thisclient, "There is already a lottery active!");
            return true;
        }

        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0;
        int Ltype=atoi(tmp);

        MYSQL_ROW CostAmountRow;
        MYSQL_RES *CostAmount;

        if(Ltype == 1)
        {
            GServer->Config.LottoType = Ltype;
            GServer->Config.LottoActive = 1;
            if(Ltype == 1)
            {
                SendPM(thisclient,"Zulie lotto is now active!");
                CostAmount = DB->QStore("SELECT ZulieCost FROM lotto_config");
                CostAmountRow = mysql_fetch_row(CostAmount);
                DB->QFree();
                GServer->Config.ZulieCost = atoi(CostAmountRow[0]);
            }
            LottoAnn();
            DeleteLottoRecords();
            GServer->DB->QExecute("UPDATE lotto_config SET active = 1");
            GServer->DB->QExecute("UPDATE lotto_config SET type = %i", Ltype);
            Log(MSG_GMACTION," %s : /lottobegin %i", thisclient->CharInfo->charname, Ltype);
        }else
        {
            SendPM(thisclient,"/lottobegin 1 for Zulie lotto");
        }
        return true;

    }
    if(strcmp(command, "lottostart")==0)
    {
        if(GServer->Config.Command_Lotto > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
        {
            SendPM(thisclient, "You are not allowed start the Lotto");
            Log(MSG_GMACTION," %s : /lottostart NOT ALLOWED", thisclient->CharInfo->charname);
            return true;
        }

        if(GServer->Config.LottoActive == 0)
        {
            SendPM(thisclient,"There is no Lotto active, please use the lottobegin command!");
            SendPM(thisclient,"/lottobegin 1 for Zulie lotto");
            return true;
        }
        Log(MSG_GMACTION," %s : /lottostart", thisclient->CharInfo->charname);

        MYSQL_ROW PrizeRow;
        MYSQL_RES *PrizeWinner = NULL;

        if(GServer->Config.LottoType == 1)
        {
            PrizeWinner = GServer->DB->QStore("SELECT Zulie FROM LottoPrize");
            PrizeRow = mysql_fetch_row(PrizeWinner);
            GServer->DB->QFree();

			if(PrizeWinner == NULL || PrizeRow == NULL)
			{
				SendPM(thisclient,"Error, no zulie prize!");
				return true;
			}

            GServer->Config.ZuliePrize = atoi(PrizeRow[0]);
        }

        Lotto();
        GivePrizes();
        return true;
    }
    if(strcmp(command, "lotto")==0)
    {
        int GuessNumber[6]={0,0,0,0,0,0};
        char Guess[100];
        int DonationPoints;

        if(GServer->Config.LottoActive == 0)// See if lotto is active
        {
            SendPM(thisclient,"There is no lottery active at the moment!");
            return true;
        }

        if(thisclient->CharInfo->Zulies < GServer->Config.ZulieCost && GServer->Config.LottoType == 1)
        {
            SendPM(thisclient,"You need atleast %i zulie to enter!",1000000);
            return true;
        }

        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; GuessNumber[0]=atoi(tmp);
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; GuessNumber[1]=atoi(tmp);
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; GuessNumber[2]=atoi(tmp);
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; GuessNumber[3]=atoi(tmp);
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; GuessNumber[4]=atoi(tmp);
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0; GuessNumber[5]=atoi(tmp);

        for(int i =0;i<6;i++)
        {
            if(GuessNumber[i] <= 0)
            {
                SendPM(thisclient, "Your entery is invalid please try again!");
                SendPM(thisclient, "Example: /lotto 1 2 3 4 5 6");
                return true;
            }

            if(GuessNumber[i] > 30)
            {
                SendPM(thisclient, "You entered a number above 30! Please try again");
                return true;
            }
        }

        //------------------------------Numbers check
        if (GuessNumber[0] == GuessNumber[1]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[0] == GuessNumber[2]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[0] == GuessNumber[3]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[0] == GuessNumber[4]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[0] == GuessNumber[5]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}

        if (GuessNumber[1] == GuessNumber[2]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[1] == GuessNumber[3]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[1] == GuessNumber[4]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[1] == GuessNumber[5]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}

        if (GuessNumber[2] == GuessNumber[3]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[2] == GuessNumber[4]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[2] == GuessNumber[5]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}

        if (GuessNumber[3] == GuessNumber[4]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}
        if (GuessNumber[3] == GuessNumber[5]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}

        if (GuessNumber[4] == GuessNumber[5]) {SendPM(thisclient, "You entered two of the same numbers!"); return true;}

        bool bSwapped = true;
        while (bSwapped)
        {
            bSwapped = FALSE;
            if (GuessNumber[0] > GuessNumber[1]) swap(GuessNumber[0], GuessNumber[1],bSwapped);
            if (GuessNumber[1] > GuessNumber[2]) swap(GuessNumber[1], GuessNumber[2],bSwapped);
            if (GuessNumber[2] > GuessNumber[3]) swap(GuessNumber[2], GuessNumber[3],bSwapped);
            if (GuessNumber[3] > GuessNumber[4]) swap(GuessNumber[3], GuessNumber[4],bSwapped);
            if (GuessNumber[4] > GuessNumber[5]) swap(GuessNumber[4], GuessNumber[5],bSwapped);
        }
        //--------------------------END Numbers check

        sprintf(Guess,"%i - %i - %i - %i - %i - %i",GuessNumber[0],GuessNumber[1],GuessNumber[2],GuessNumber[3],GuessNumber[4],GuessNumber[5]);

        if(GServer->Config.LottoType == 1)//Zulie Lotto
        {
            thisclient->CharInfo->Zulies -= GServer->Config.ZulieCost;
            BEGINPACKET( pak, 0x71d );
            ADDQWORD( pak, thisclient->CharInfo->Zulies );
            thisclient->client->SendPacket( &pak );

            UpdateZuliePrize(GServer->Config.ZulieCost);

            SendPM(thisclient, "You paid %i zulie to enter the lottery!",GServer->Config.ZulieCost);

        }

        GServer->DB->QExecute("INSERT INTO LottoEntries (char_name, number1, number2, number3, number4, number5, number6) VALUES ('%s',%i,%i,%i,%i,%i,%i)",thisclient->CharInfo->charname,GuessNumber[0],GuessNumber[1],GuessNumber[2],GuessNumber[3],GuessNumber[4],GuessNumber[5]);

        SendPM(thisclient, "You are now entered into the lottery!");
        SendPM(thisclient, "Your Lotto Numbers are: %s",Guess);
		Log(MSG_GMACTION," %s : /lotto %s",thisclient->CharInfo->charname, Guess);
		return true;
    }
    if(strcmp(command, "lottohelp")==0)
    {
        SendPM(thisclient, "To enter the lottery type /lotto followed by 6 numbers between 1 and 30");
        SendPM(thisclient, "Exapmle: /lotto 3 23 22 45 12 5");

        if(GServer->Config.LottoActive == 1)
        {
            if(GServer->Config.LottoType == 1)
            {
                SendPM(thisclient, "One lottery ticket will cost you %i zulie!", GServer->Config.ZulieCost);
            }
        }else
        {
            SendPM(thisclient,"There is no lottery active!");
        }

        return true;
    }
    if(strcmp(command, "lottor")==0)
    {
        int GuessNumber[6]={0,0,0,0,0,0};
        char Guess[100];
        int DonationPoints;

        if(GServer->Config.LottoActive == 0)// See if lotto is active
        {
            SendPM(thisclient,"There is no lottery active at the moment!");
            return true;
        }

        if(thisclient->CharInfo->Zulies < GServer->Config.ZulieCost && GServer->Config.LottoType == 1)
        {
            SendPM(thisclient,"You need atleast %i zulie to enter!",1000000);
            return true;
        }

        srand(time(0));

        for(int i = 0; i < 6;i++)
        {
            GuessNumber[i] = rand() % 30 + 1;
        }

        bool MayContinue = true;
        while(MayContinue)
        {
            MayContinue = false;
            if (GuessNumber[0] == GuessNumber[1]) change(GuessNumber[0],MayContinue);
            if (GuessNumber[0] == GuessNumber[2]) change(GuessNumber[0],MayContinue);
            if (GuessNumber[0] == GuessNumber[3]) change(GuessNumber[0],MayContinue);
            if (GuessNumber[0] == GuessNumber[4]) change(GuessNumber[0],MayContinue);
            if (GuessNumber[0] == GuessNumber[5]) change(GuessNumber[0],MayContinue);

            if (GuessNumber[1] == GuessNumber[2]) change(GuessNumber[1],MayContinue);
            if (GuessNumber[1] == GuessNumber[3]) change(GuessNumber[1],MayContinue);
            if (GuessNumber[1] == GuessNumber[4]) change(GuessNumber[1],MayContinue);
            if (GuessNumber[1] == GuessNumber[5]) change(GuessNumber[1],MayContinue);

            if (GuessNumber[2] == GuessNumber[3]) change(GuessNumber[2],MayContinue);
            if (GuessNumber[2] == GuessNumber[4]) change(GuessNumber[2],MayContinue);
            if (GuessNumber[2] == GuessNumber[5]) change(GuessNumber[2],MayContinue);

            if (GuessNumber[3] == GuessNumber[4]) change(GuessNumber[3],MayContinue);
            if (GuessNumber[3] == GuessNumber[5]) change(GuessNumber[3],MayContinue);

            if (GuessNumber[4] == GuessNumber[5]) change(GuessNumber[4],MayContinue);
        }

        bool bSwapped = true;
        while (bSwapped)
        {
        bSwapped = FALSE;
            if (GuessNumber[0] > GuessNumber[1]) swap(GuessNumber[0], GuessNumber[1],bSwapped);
            if (GuessNumber[1] > GuessNumber[2]) swap(GuessNumber[1], GuessNumber[2],bSwapped);
            if (GuessNumber[2] > GuessNumber[3]) swap(GuessNumber[2], GuessNumber[3],bSwapped);
            if (GuessNumber[3] > GuessNumber[4]) swap(GuessNumber[3], GuessNumber[4],bSwapped);
            if (GuessNumber[4] > GuessNumber[5]) swap(GuessNumber[4], GuessNumber[5],bSwapped);
        }

        sprintf(Guess,"%i - %i - %i - %i - %i - %i",GuessNumber[0],GuessNumber[1],GuessNumber[2],GuessNumber[3],GuessNumber[4],GuessNumber[5]);

        if(GServer->Config.LottoType == 1)//Zulie Lotto
        {
            thisclient->CharInfo->Zulies -= GServer->Config.ZulieCost;
            BEGINPACKET( pak, 0x71d );
            ADDQWORD( pak, thisclient->CharInfo->Zulies );
            thisclient->client->SendPacket( &pak );

            UpdateZuliePrize(GServer->Config.ZulieCost);

            SendPM(thisclient, "You paid %i zulie to enter the lottery!",GServer->Config.ZulieCost);

        }

        GServer->DB->QExecute("INSERT INTO LottoEntries (char_name, number1, number2, number3, number4, number5, number6) VALUES ('%s',%i,%i,%i,%i,%i,%i)",thisclient->CharInfo->charname,GuessNumber[0],GuessNumber[1],GuessNumber[2],GuessNumber[3],GuessNumber[4],GuessNumber[5]);

        SendPM(thisclient, "You are now entered into the lottery!");
        SendPM(thisclient, "Your Lotto Numbers are: %s",Guess);
		Log(MSG_GMACTION," %s : /lottor %s",thisclient->CharInfo->charname, Guess);
		return true;
    }
    if(strcmp(command, "vinfo")==0)
    {
        int number;
        if ((tmp = strtok(NULL, " ")) == NULL)
        {
            tmp = 0;
            number = 13;

        }else
        {
            number = atoi(tmp);
        }

        if(number > 12)
        {
            SendPM(thisclient,"Type - Description");
            SendPM(thisclient,"0 - Zulie");
            SendPM(thisclient,"1 - Face Items");
            SendPM(thisclient,"2 - Head Items");
            SendPM(thisclient,"3 - Body Items");
            SendPM(thisclient,"4 - Arm Items");
            SendPM(thisclient,"5 - Foot Items");
            SendPM(thisclient,"6 - Back Items");
            SendPM(thisclient,"7 - Jewels");
            SendPM(thisclient,"8 - Weapons");
            SendPM(thisclient,"9 - Sub Weapons");
            SendPM(thisclient,"10 - Consumables");
            SendPM(thisclient,"11 - Gems");
            SendPM(thisclient,"12 - Materials");
            SendPM(thisclient,"Type /vinfo [Type] for more info");

            return true;
        }
		else
        {
            return voteShopInfo(thisclient, number);
        }
		return true;
    }
    if(strcmp(command, "vbuy")==0)
    {
        int number;
        if ((tmp = strtok(NULL, " ")) == NULL) tmp = 0;

        number = atoi(tmp);

        if(number < 0)
        {
            SendPM(thisclient,"Invalid ID, type /vinfo for more info");
            return true;
        }

        return voteShopBuy(thisclient, number);
    }
    if(strcmp(command, "vpoints")==0)
    {
        SendPM(thisclient, "You currently have %i vote points", GetVotePoints(thisclient));
        return true;
    }
    if(strcmp(command, "loadads")==0)
    {
        if(GServer->Config.Command_Lotto > thisclient->Session->accesslevel || thisclient->CharInfo->isGM == false)
        {
            SendPM(thisclient, "You are not allowed to load the adverds!");
            Log(MSG_GMACTION," %s : /loadads NOT ALLOWED", thisclient->CharInfo->charname);
            return true;
        }
        LoadAdverds();
        SendPM(thisclient,"Adverds loaded!");
        Log(MSG_GMACTION," %s : /loadads ", thisclient->CharInfo->charname);
        return true;
    }
    
    
	//if it ever gets here then the command didn't rtigger any of the conditionals
	Log( MSG_WARNING, "Invalid GM Command '%s' by '%s'", command, thisclient->CharInfo->charname);
	//Wrong Command Alert {By CrAshInSiDe}
    SendPM(thisclient, "Invalid Command");
	
	return true;
}

// GM: Announcment
bool CWorldServer::pakGMAnn( CPlayer* thisclient, CPacket *P )
{
	BEGINPACKET( pak, 0x702 );
	ADDSTRING( pak, thisclient->CharInfo->charname );
	ADDSTRING( pak, "> " );
	ADDSTRING( pak, (&P->Buffer[5]));
	ADDBYTE( pak, 0x00);
	SendToAll( &pak );

	return true;
}

// GM: Spawn x mobs
bool CWorldServer::pakGMMon( CPlayer* thisclient, int montype, int moncount,int monteam, int lma_aip)
{
    //thisclient->Position->current->x
    fPoint position=thisclient->Position->current;
    CMap* map = MapList.Index[thisclient->Position->Map];
    SendPM(thisclient,"%i monsters of type %i summoned near position %.2f %.2f Map: %i (team %i)",moncount,montype,position.x,position.y,map->id,monteam);

	for (int i=0; i<moncount; i++)
    {
        position = RandInCircle( thisclient->Position->current, 10 );
        CMonster* thismonster=map->AddMonster( montype, position, 0, NULL, NULL, 0 , true );

        //LMA: adding team to monster (used for PVP for example).
        if(thismonster!=NULL)
        {
            thismonster->team=monteam;

            if (lma_aip!=-1)
            {
                thismonster->sp_aip=lma_aip;
                Log(MSG_INFO,"pakGMMon %s:: monster %i, cid %i, aip %i",thisclient->CharInfo->charname,montype,thismonster->clientid,lma_aip);
            }
            else
            {
                Log(MSG_INFO,"pakGMMon %s:: monster %i, cid %i",thisclient->CharInfo->charname,montype,thismonster->clientid);
            }

        }

	}
	return true;
}

// GM: Teleport somewhere
bool CWorldServer::pakGMTele( CPlayer* thisclient, int map, float x, float y,int no_qsd)
{
    fPoint coord;

    //LMA: no qsd zone this time.
    if(no_qsd!=0)
    {
        thisclient->skip_qsd_zone=true;
    }

    //LMA: check to avoid stupid stuff.
    if(map<=0||map>=maxZone)
    {
        return true;
    }

    coord.x = x;
    coord.y = y;
    MapList.Index[map]->TeleportPlayer( thisclient, coord, false );
	return true;
}

// [by Paul_T] [Thanks to AridTag for the packet :D]
bool CWorldServer::pakGMMute( CPlayer* thisclient, char* name, int time)
{
    CPlayer* otherclient = GetClientByCharName ( name );
    if(otherclient==NULL)
        return true;

    BEGINPACKET( pak, 0x70d );
    ADDBYTE    ( pak, 0 );
    ADDWORD    ( pak, time );
    ADDSTRING  ( pak, thisclient->CharInfo->charname );
    ADDBYTE    ( pak, 0 );
    otherclient->client->SendPacket( &pak );
    return true;
}

// GM: Item   - Modified by Hiei (added refine/socket/stats)
bool CWorldServer::pakGMItem( CPlayer* thisclient, UINT itemid, UINT itemtype, UINT itemamount, UINT itemrefine, UINT itemdura, UINT itemstats, UINT itemsocket )
{
    CItem item;
    item.count            = itemamount;
    item.durability        = itemdura;
    item.itemnum        = itemid;
    item.itemtype        = itemtype;
    item.lifespan        = 100;  // changed from itemls to 100, no need for lifespan on a item that is spawned - rl2171
    item.refine            = itemrefine;
    item.stats            = itemstats;
    item.socketed        = itemsocket;
    item.appraised        = 1;
    item.gem = 0;
    item.sp_value=0;
    item.last_sp_value=0;
    unsigned newslot = thisclient->GetNewItemSlot( item );

    //LMA: PAT:
    if (itemtype==14)
    {
        item.sp_value=item.lifespan*10;
    }

    if (newslot != 0xffff)
    {
        thisclient->items[newslot] = item;
        thisclient->UpdateInventory( newslot );
        char buffer[200];
        sprintf( buffer, "Item added! (ID: %i) (Type: %i) (Refine: %i) (Socket: %i)", item.itemnum, item.itemtype, item.refine, item.socketed );
        BEGINPACKET ( pak, 0x702 );
        ADDSTRING( pak, buffer );
        ADDBYTE( pak, 0 );
        thisclient->client->SendPacket( &pak );

    }else{
        BEGINPACKET( pak, 0x7a7 );
        ADDWORD( pak, thisclient->clientid );
        ADDBYTE( pak, 5 );
        thisclient->client->SendPacket( &pak );

        RESETPACKET ( pak, 0x702 );
        ADDSTRING( pak, "No free slot !" );
        ADDBYTE( pak, 0 );
        thisclient->client->SendPacket( &pak );
    }
    return true;
}

// LMA: We give an item from a Quest...
bool CWorldServer::pakGMItemQuest( CPlayer* thisclient, UINT itemid, UINT itemtype, UINT itemamount, UINT itemrefine, UINT itemls, UINT itemstats, UINT itemsocket, char buffer2[200] )
{
    CItem item;
    item.count            = itemamount;
    item.durability        = 40;
    item.itemnum        = itemid;
    item.itemtype        = itemtype;
    item.lifespan        = 100;  // changed from items to 100, no need for lifespan on a item that is spawned - rl2171
    item.refine            = itemrefine;
    item.stats            = itemstats;
    item.socketed        = itemsocket;
    item.appraised        = 1;
    item.gem = 0;
    unsigned newslot = thisclient->GetNewItemSlot( item );
    if (newslot != 0xffff)
    {
        thisclient->items[newslot] = item;
        thisclient->UpdateInventory( newslot );
        char buffer[200];
        sprintf( buffer, "%s", buffer2);
        BEGINPACKET ( pak, 0x702 );
        ADDSTRING( pak, buffer );
        ADDBYTE( pak, 0 );
        thisclient->client->SendPacket( &pak );

    }else{
        BEGINPACKET( pak, 0x7a7 );
        ADDWORD( pak, thisclient->clientid );
        ADDBYTE( pak, 5 );
        thisclient->client->SendPacket( &pak );

        RESETPACKET ( pak, 0x702 );
        ADDSTRING( pak, "No slot available in inventory!" );
        ADDBYTE( pak, 0 );
        thisclient->client->SendPacket( &pak );
    }
    return true;
}

// GM: Kick
bool CWorldServer::pakGMKick( CPlayer* thisclient, char* name )
{
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

// GM: Ban
bool CWorldServer::pakGMBan( CPlayer* thisclient, char* name )
{
    CPlayer* otherclient = GetClientByCharName( name );
    if(otherclient==NULL)
        return true;
    otherclient->Session->accesslevel = -1;
    DB->QExecute( "UPDATE accounts SET accesslevel='0' WHERE id=%i", otherclient->Session->userid);
    BEGINPACKET( pak, 0x702 );
    ADDSTRING( pak, "You were banned from the server !" );
    ADDBYTE( pak, 0 );
    otherclient->client->SendPacket( &pak );

    RESETPACKET( pak, 0x707 );
    ADDWORD( pak, 0 );
    otherclient->client->SendPacket( &pak );

    otherclient->client->isActive = false;

    return true;
}

// Reborn command credits Core
bool CWorldServer::pakGMReborn(CPlayer* thisclient)
{

    if(thisclient->Party->party!=NULL)
    {
        Log(MSG_INFO,"Player %s tried to use reborn but he was in a party.");
        return true;
    }

     if(thisclient->Stats->Level < GServer->Config.MaxLevel) //Level 260 can be changed to any level you want
     {
        GServer->SendPM(thisclient, "You have to be lvl %i to reborn !", GServer->Config.MaxLevel);
     }
     else
     {

        thisclient = GServer->GetClientByCharName( thisclient->CharInfo->charname ); // adding this allows the server to retrieve the players client info for item removal
        //starting the item removal process from slots 1 through 11
        /*
            slot 1, Face
            slot 2, Head
            slot 3, Body
            slot 4, Back
            slot 5, Hand
            slot 6, Foot
            slot 7, Weapon
            slot 8, Offhand
            slot 9, Necklace
            slot 10, Ring
            slot 11, Earring
        */
        for(int j=1; j<12; j++){
            if(thisclient->items[j].itemnum == 0){ //prevents unknown itemtype error
                continue;
            }

            UINT newslot = thisclient->GetNewItemSlot(thisclient->items[j]); // finds the first free slot in inventory
            if(newslot==0xffff)
            {
                Log(MSG_INFO,"Unable to find empty slot");
                SendPM(thisclient,"Your inventory is full");
                return false;
            }

            thisclient->items[newslot] = thisclient->items[j];
            ClearItem( thisclient->items[j] );
            thisclient->UpdateInventory( newslot, j );
            BEGINPACKET( pak, 0x7a5 );
            ADDWORD    ( pak, thisclient->clientid );
            ADDWORD    ( pak, j );
            ADDWORD    ( pak, thisclient->items[j].itemnum );
            ADDWORD    ( pak, GServer->BuildItemRefine( thisclient->items[j] ) );
            ADDWORD    ( pak, thisclient->Stats->Move_Speed );
            GServer->SendToVisible( &pak, (CCharacter*)this);
            thisclient->SetStats( );
        }
        (CPlayer *)thisclient;
        //LMA: We remove all but basic skills.
        for(int i=0;i<320;i++)  //For keeping the skills, remove this line
        {  //For keeping the skills, remove this line
            thisclient->cskills[i].id = 0;  //For keeping the skills, remove this line
            thisclient->cskills[i].level = 1;  //For keeping the skills, remove this line
            thisclient->cskills[i].thisskill=NULL;
        }  //For keeping the skills, remove this line

        for(int i=0;i<MAX_QUICKBAR;i++)
            thisclient->quickbar[i] = 0;

         thisclient->p_skills = 0;  //For keeping the skills, remove
         thisclient->CharInfo->SkillPoints = 0;
         thisclient->CharInfo->StatPoints = 0;
         thisclient->CharInfo->Job = 0;

         thisclient->Stats->Level = 1;
         thisclient->CharInfo->Exp = 0;

         thisclient->ActiveQuest = 0;

        /*Update Reborn Command {By CrAshInSiDe*/
        int x = 5098;
        int y = 5322;
        int map = 22;
        if( (x != 0) && (y != 0) && (map != 0) )
        {
            fPoint coord;
            coord.x = x;
            coord.y = y;
            MapList.Index[map]->TeleportPlayer( thisclient, coord, false );
        }

         // Uncomment below if you want to use the Nobles part
    /*
        if(Reborn < 1)
        {
            int Reborn;
            char newcharname[65];
            strcpy (newcharname,"[Noble]");
            strcat (newcharname, thisclient->CharInfo->charname);
            GServer->DB->QExecute(" UPDATE characters SET char_name = '%s' WHERE id = '%i' ",newcharname, thisclient->CharInfo->charid);
        }
    */
        GServer->DB->QExecute("UPDATE characters SET reborn = reborn + 1 WHERE id = '%i'", thisclient->CharInfo->charid);

         BEGINPACKET( pak, 0x702 );
         ADDSTRING( pak, "You were disconnected from the server !" );
         ADDBYTE( pak, 0 );
         thisclient->client->SendPacket( &pak );

         RESETPACKET( pak, 0x707 );
         ADDWORD( pak, 0 );
         thisclient->client->SendPacket( &pak );
         Log( MSG_GMACTION, " %s : /reborn" , thisclient->CharInfo->charname);

         //saving skills.
         thisclient->saveskills();
         thisclient->ResetSkillOffset();
         thisclient->client->isActive = false;
         thisclient->clearquest(thisclient);
      }


     return true;
}

//LMA: To change Ifo stuff, for now only for the warpgate.
bool CWorldServer::pakGMEventIFO(CPlayer* thisclient, int ifoType,int eventID)
{
    if (ifoType!=GServer->WarpGate.id)
        return true;

    if (eventID<0||eventID>1||eventID==GServer->WarpGate.IfoObjVar[0])
    {
        return true;
    }

    char buffer[200];
    sprintf( buffer, "Event %i for IFO: %i", eventID, ifoType);

    GServer->WarpGate.IfoObjVar[0]=eventID;
    if(eventID==0)
    {
        GServer->WarpGate.hidden=true;
    }
    else
    {
        GServer->WarpGate.hidden=false;
    }

    BEGINPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[Server]" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE    ( pak, 0 );
    thisclient->client->SendPacket(&pak);

    RESETPACKET( pak, 0x790 );
    ADDWORD    ( pak, GServer->WarpGate.clientID);
    ADDWORD    ( pak, eventID);
    //thisclient->client->SendPacket(&pak);
    GServer->SendToAllInMap(&pak,GServer->WarpGate.mapid);

    //We have to refresh the stuff.
    //Is that really needed?
    //pakSpawnIfoObject(thisclient,GServer->WarpGate.virtualNpctypeID,true);



	return true;
}

//Event function credits Welson
bool CWorldServer::pakGMEventType(CPlayer* thisclient, int npctype, int dialog, long int type)
{
    CNPC* thisnpc = GetNPCByType(npctype);
    if(thisnpc == NULL)
    {
        delete thisnpc;
        return true;
    }
    if(type<0) type = 0;
    char buffer[200];

    sprintf( buffer, "Event %i for NPC: %i, Dialog: %i (if dialog has changed, type /here to force refresh)", type, npctype, dialog);
    thisnpc->dialog = dialog;
    thisnpc->event = type;

    BEGINPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[Server]" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE    ( pak, 0 );
    thisclient->client->SendPacket(&pak);

    RESETPACKET( pak, 0x790 );
    ADDWORD    ( pak, thisnpc->clientid );
    ADDWORD    ( pak, thisnpc->event );	  //LMA: Welcome in the real Word ^_^
    //thisclient->client->SendPacket(&pak);
    GServer->SendToAllInMap(&pak,thisnpc->posMap);

    //changing objvar as well.
    GServer->ObjVar[npctype][0]=thisnpc->event;

    //Saving in database
    DB->QExecute("UPDATE list_npcs SET tempdialogid=%i, eventid=%i WHERE type=%i", dialog, type,npctype);


	return true;
}

//Event function (by event name)
bool CWorldServer::pakGMEventName(CPlayer* thisclient, char* eventname, int is_on)
{
    if(!CheckEscapeMySQL(eventname,-1,true))
    {
        Log(MSG_WARNING,"%s:: Wrong event name %s",thisclient->CharInfo->charname,eventname);
        return true;
    }

    int npctype=0;

    MYSQL_ROW row;
    MYSQL_RES *result = DB->QStore("SELECT type,dialogid,eventid FROM list_events WHERE activekeyword='%s' ",eventname);
    if(result==NULL) return false;
    while(row = mysql_fetch_row(result))
    {
        npctype=atoi(row[0]);
        CNPC* thisnpc = GetNPCByType(npctype);
        if(thisnpc == NULL)
        {
            Log(MSG_WARNING,"%s:: Wrong NPC %u for event name %s",thisclient->CharInfo->charname,npctype,eventname);
            DB->QFree( );
            delete thisnpc;
            return true;
        }

        if(is_on!=1)
        {
            //deactivation of the event.
            thisnpc->dialog=thisnpc->thisnpc->dialogid;
            thisnpc->event=0;
            BEGINPACKET( pak, 0x790 );
            ADDWORD    ( pak, thisnpc->clientid );
            ADDWORD    ( pak, thisnpc->event );
            GServer->SendToAllInMap(&pak,thisnpc->posMap);
            //changing objvar as well.

            SendPM(thisclient,"Deactivating Event %s, NPC %u",eventname,npctype);
        }
        else
        {
            thisnpc->dialog = atoi(row[1]);
            thisnpc->event = atoi(row[2]);

            BEGINPACKET( pak, 0x790 );
            ADDWORD    ( pak, thisnpc->clientid );
            ADDWORD    ( pak, thisnpc->event );	  //LMA: Welcome in the real Word ^_^
            GServer->SendToAllInMap(&pak,thisnpc->posMap);

            //changing objvar as well.
            SendPM(thisclient,"Activating Event %s, NPC %u",eventname,npctype);
        }

        GServer->ObjVar[npctype][0]=thisnpc->event;

    }

    DB->QFree( );

    if (npctype==0)
    {
        return true;
    }

    //Deactivating or actvating all the event.
    DB->QExecute("UPDATE list_events SET Active='%u' WHERE activekeyword='%s' ",is_on,eventname);


	return true;
}

// GM: Add/Remove/Drop/Set zuly
bool CWorldServer::pakGMZuly( CPlayer* thisclient, int mode, int amount, char* charname )
{
	CPlayer* otherclient = GetClientByCharName(charname);
	if(otherclient==NULL)
	   return true;
	if (mode == 1)
	{
		//ADD
		otherclient->CharInfo->Zulies += amount;
		BEGINPACKET( pak, 0x71d );
		ADDQWORD( pak, otherclient->CharInfo->Zulies );
		otherclient->client->SendPacket( &pak );
	}
	if (mode == 2)
	{
		//REMOVE
		otherclient->CharInfo->Zulies -= amount;
		BEGINPACKET( pak, 0x71d );
		ADDQWORD( pak, otherclient->CharInfo->Zulies );
		otherclient->client->SendPacket( &pak );
	}
	if (mode == 3)
	{
		//DROP
		CDrop* thisdrop = new CDrop;
		assert(thisdrop);
		thisdrop->clientid = GetNewClientID();
		thisdrop->type = 1; // ZULY
		thisdrop->pos.x = otherclient->Position->current.x;
		thisdrop->pos.y = otherclient->Position->current.y;
		thisdrop->posMap = otherclient->Position->Map;
		thisdrop->droptime = time(NULL);
		thisdrop->amount = amount;
		thisdrop->owner = 0;
		CMap* map = MapList.Index[thisdrop->posMap];
		map->AddDrop( thisdrop );
	}
	if (mode == 4)
	{
		//SET
		otherclient->CharInfo->Zulies = amount;
		BEGINPACKET( pak, 0x71d );
		ADDQWORD( pak, otherclient->CharInfo->Zulies );
		otherclient->client->SendPacket( &pak );
	}
	return true;
}

// Change player Level
bool CWorldServer::pakGMLevel( CPlayer* thisclient, int level, char* name )
{
    CPlayer* otherclient = GetClientByCharName( name );
    if (otherclient == NULL)
    {
        return true;
    }

    if (((int)otherclient->Stats->Level + level) > 0)
    {
        otherclient->Stats->Level += level;
    }
    else
    {
        otherclient->Stats->Level = 1;
    }

    if (otherclient->Stats->Level > Config.MaxLevel)
    {
        otherclient->Stats->Level = Config.MaxLevel;
    }

    //if (otherclient->Stats->Level > 250)
    //    otherclient->Stats->Level = 250;
    SendPM (thisclient, "Relogin To Get New Level");
    otherclient->CharInfo->Exp = 0;

    if (level < 0)
    {
        otherclient->CharInfo->StatPoints = 0;
        otherclient->CharInfo->SkillPoints = 0;

        if(otherclient->Stats->Level >= 1)
        {
            //LMA: Rescudo's new formula for Stat points (adapted).
            /*for(int s = 2; s <= otherclient->Stats->Level; s++)
            {
                otherclient->CharInfo->StatPoints += 10;
                otherclient->CharInfo->StatPoints += s / 2;

                if (s >= 10)
                {
                    otherclient->CharInfo->SkillPoints++;
                }

            }*/

            for (int s = 0; s <= otherclient->Stats->Level; s++ )
            {
                if(s<level)
                {
                    otherclient->CharInfo->StatPoints +=9;
                }

                otherclient->CharInfo->StatPoints += s - ((s - 1) / 5);

                if (s >= 10)
                {
                    otherclient->CharInfo->SkillPoints++;
                }

            }

        }

        pakGMStat(otherclient, "str", 15);
        pakGMStat(otherclient, "dex", 15);
        pakGMStat(otherclient, "con", 15);
        pakGMStat(otherclient, "int", 15);
        pakGMStat(otherclient, "cha", 10);
        pakGMStat(otherclient, "sen", 10);
    }
    else
    {
        //LMA: Rescudo's new formula for Stat points (adapted).
        /*for(int s = (otherclient->Stats->Level - level + 1); s <= otherclient->Stats->Level; s++)
        {
            otherclient->CharInfo->StatPoints += 10;
            otherclient->CharInfo->StatPoints += s / 2;

            if (s >= 10)
            {
                otherclient->CharInfo->SkillPoints++;
            }

        }*/

        for (int s = (otherclient->Stats->Level - level + 1); s <= otherclient->Stats->Level; s++ )
        {
            otherclient->CharInfo->StatPoints +=9;
            otherclient->CharInfo->StatPoints += s - ((s - 1) / 5);

            if (s >= 10)
            {
                otherclient->CharInfo->SkillPoints++;
            }

        }

    }

    BEGINPACKET( pak, 0x79e );
    ADDWORD( pak, otherclient->clientid );
    ADDWORD( pak, otherclient->Stats->Level );
    ADDDWORD( pak, otherclient->CharInfo->Exp );
    ADDWORD( pak, otherclient->CharInfo->StatPoints );
    ADDWORD( pak, otherclient->CharInfo->SkillPoints );
    otherclient->client->SendPacket( &pak );

    RESETPACKET( pak, 0x79e );
    ADDWORD( pak, otherclient->clientid );
    SendToVisible( &pak, otherclient, false );

    otherclient->SetStats( );
    otherclient->Stats->HP = otherclient->Stats->MaxHP;
    otherclient->Stats->MP = otherclient->Stats->MaxMP;
    return true;
}


// Reload Mob Spawn
// PY temporarily disabled pending a rewrite. Currently conflicts with fPoint structure changes
/*
bool CWorldServer::ReloadMobSpawn( CPlayer* thisclient, int id )
{
	CSpawnArea* thisspawn = new (nothrow) CSpawnArea;
	if(thisspawn==NULL)
	{
        Log(MSG_WARNING, "Error allocing memory" );
        return true;
    }
    thisspawn->thisnpc = GetNPCDataByID( thisclient->GMRespawnPoints.mobID );
    if(thisspawn->thisnpc==NULL)
    {
        SendSysMsg( thisclient, "Invalid Respawn" );
        return true;
    }
	thisspawn->id = id;
	thisspawn->map = thisclient->GMRespawnPoints.map;
	thisspawn->montype = thisclient->GMRespawnPoints.mobID;
	thisspawn->min = thisclient->GMRespawnPoints.min;
	thisspawn->max = thisclient->GMRespawnPoints.max;
	thisspawn->respawntime = thisclient->GMRespawnPoints.respawntime;
	thisspawn->pcount = thisclient->GMRespawnPoints.n;
	thisspawn->points = thisclient->GMRespawnPoints.points;
    CMap* map = MapList.Index[thisspawn->map];
	for(int j=0; j<thisspawn->max; j++)
    {
        fPoint position = RandInPoly( thisspawn->points, thisspawn->pcount );
        map->AddMonster( thisspawn->montype, position, 0, thisspawn->mobdrop, thisspawn->mapdrop, thisspawn->id, true );
	}
	thisspawn->lastRespawnTime=clock();
    return true;
}*/

// Teleport To player to other place
bool CWorldServer::pakGMTeleOtherPlayer( CPlayer *thisclient, char* name, int map, float x, float y )
{
	CPlayer* otherclient = GetClientByCharName( name );
	if (otherclient != NULL)
	{
        fPoint coord;
        coord.x = x;
        coord.y = y;
        MapList.Index[map]->TeleportPlayer( otherclient, coord, false );
	}
    else
	{
        SendSysMsg( thisclient, "User does not exist or is not online!" );
	}
	return true;
}

// Teleport Me to player
bool CWorldServer::pakGMTeleToPlayer( CPlayer* thisclient, char* name )
{
	CPlayer* otherclient = GetClientByCharName( name );
	if (otherclient != NULL)
	{
        MapList.Index[otherclient->Position->Map]->TeleportPlayer( thisclient, otherclient->Position->current, false );
	}
    else
	{
        SendSysMsg( thisclient, "User does not exist or is not online!" );
	}
	return true;
}

// Teleport Player Here
bool CWorldServer::pakGMTelePlayerHere( CPlayer* thisclient, char* name )
{
	CPlayer* otherclient = GetClientByCharName( name );
	if (otherclient != NULL)
	{
        CMap* map = MapList.Index[thisclient->Position->Map];
        map->TeleportPlayer( otherclient, thisclient->Position->current, false );
	}
    else
	{
        SendSysMsg( thisclient, "User does not exist or is not online!");
	}
	return true;
}

// Show Player info
bool CWorldServer::pakGMInfo(CPlayer* thisclient, char* name)
{
	CPlayer* otherclient = GetClientByCharName(name);
    if(otherclient==NULL)
        return true;
	char  buffer[200];
	char* jobname;

	if(otherclient != 0) {
		if((otherclient->CharInfo->Job - 0) == 0) { jobname = (char*) "Visitor"; }
		else if((otherclient->CharInfo->Job - 0) == 111) { jobname = (char*) "Soldier"; }
		else if((otherclient->CharInfo->Job - 0) == 121) { jobname = (char*) "Knight"; }
		else if((otherclient->CharInfo->Job - 0) == 122) { jobname = (char*) "Champion"; }
		else if((otherclient->CharInfo->Job - 0) == 211) { jobname = (char*) "Muse"; }
		else if((otherclient->CharInfo->Job - 0) == 221) { jobname = (char*) "Mage"; }
		else if((otherclient->CharInfo->Job - 0) == 222) { jobname = (char*) "Cleric"; }
		else if((otherclient->CharInfo->Job - 0) == 311) { jobname = (char*) "Hawker"; }
		else if((otherclient->CharInfo->Job - 0) == 321) { jobname = (char*) "Raider"; }
		else if((otherclient->CharInfo->Job - 0) == 322) { jobname = (char*) "Scout"; }
		else if((otherclient->CharInfo->Job - 0) == 411) { jobname = (char*) "Dealer"; }
		else if((otherclient->CharInfo->Job - 0) == 421) { jobname = (char*) "Bourgeois"; }
		else if((otherclient->CharInfo->Job - 0) == 422) { jobname = (char*) "Artisan"; }
		else { jobname = (char*) "Unknown"; }

		BEGINPACKET(pak, 0x702);

		sprintf(buffer, "Info about <%s>" , otherclient->CharInfo->charname);
		ADDSTRING(pak, buffer);
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);

		RESETPACKET(pak, 0x702);
		sprintf(buffer, "Account: %s | ID: %i" , otherclient->Session->username, otherclient->Session->userid);
		ADDSTRING(pak, buffer);
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);

		RESETPACKET(pak, 0x702);
		sprintf(buffer, "Level: %i | Job: %s" , (otherclient->Stats->Level-0) , jobname);
		ADDSTRING(pak, buffer);
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);

		RESETPACKET(pak, 0x702);
		sprintf(buffer,"Map: %i , X: %i , Y: %i", (otherclient->Position->Map-0), (int)otherclient->Position->current.x, (int)otherclient->Position->current.y);
		ADDSTRING(pak, buffer);
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);
	}
	else {
		BEGINPACKET(pak, 0x702);
		ADDSTRING(pak, "User does not exist or is not online!");
		ADDBYTE(pak, 0)
		thisclient->client->SendPacket(&pak);
	}
	return true;
}

//LMA: Give reward clan points to somebody
//also used when clan shopping.
bool CWorldServer::pakGMClanRewardPoints(CPlayer* thisclient, char* name, int points)
{
  	CPlayer* otherclient = GetClientByCharName (name);
	if(otherclient==NULL){
        BEGINPACKET(pak, 0x702);
		ADDSTRING(pak, "User does not exist or is not online.");
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);
        return true;
    }

     if (otherclient->Clan->clanid==0)
     {
        Log(MSG_WARNING,"pakGMClanRewardPoints:: User %s does not have a clan",thisclient->CharInfo->charname);
        return true;
     }

     //adding points if needed
     //Asking CharServer to refresh the player's informations.
    if (points>0)
    {
        thisclient->CharInfo->rewardpoints+=points;
        char buffer[200];
        sprintf( buffer, "You received %i Clan Reward Points !!", points);
        BEGINPACKET ( pak, 0x702 );
        ADDSTRING( pak, buffer );
        ADDBYTE( pak, 0 );
        otherclient->client->SendPacket( &pak );

        RESETPACKET( pak, 0x7e0 );
     	ADDBYTE    ( pak, 0xff );
    	//ADDWORD    ( pak, otherclient->CharInfo->charid);  //charid
    	ADDDWORD    ( pak, otherclient->CharInfo->charid);  //charid
    	ADDDWORD    ( pak, thisclient->CharInfo->rewardpoints);  //reward points (TOTAL)
    	cryptPacket( (char*)&pak, GServer->cct );
    	send( csock, (char*)&pak, pak.Size, 0 );
    }
    else
    {
        BEGINPACKET( pak, 0x7e0 );
 	    ADDBYTE    ( pak, 0xff );
    	//ADDWORD    ( pak, otherclient->CharInfo->charid);  //charid
    	ADDDWORD    ( pak, otherclient->CharInfo->charid);  //charid
    	ADDDWORD    ( pak, thisclient->CharInfo->rewardpoints);  //reward points (TOTAL)
    	cryptPacket( (char*)&pak, GServer->cct );
    	send( csock, (char*)&pak, pak.Size, 0 );
    }


     return true;
}

//LMA: We raise the Clan Grade (as a test).
bool CWorldServer::pakGMRaiseCG(CPlayer* thisclient)
{
    thisclient->Clan->grade=GServer->getClanGrade(thisclient->Clan->clanid);
    thisclient->Clan->grade++;

    BEGINPACKET( pak, 0x7e0 );
    ADDBYTE    ( pak, 0xfb );
    //ADDWORD    ( pak, thisclient->CharInfo->charid);  //charid
    ADDDWORD    ( pak, thisclient->CharInfo->charid);  //charid
    ADDDWORD    ( pak, thisclient->Clan->grade);  //new grade.
    cryptPacket( (char*)&pak, GServer->cct );
    send( csock, (char*)&pak, pak.Size, 0 );

    //Send to other players
    RESETPACKET( pak, 0x7e0 );
    ADDBYTE    ( pak, 0x35 );
    ADDWORD    ( pak, thisclient->clientid );
    ADDWORD    ( pak, thisclient->Clan->clanid);
    ADDWORD    ( pak, 0x0000 );//???
    ADDWORD    ( pak, thisclient->Clan->back );
    ADDWORD    ( pak, thisclient->Clan->logo );
    ADDBYTE    ( pak, thisclient->Clan->grade );//clan grade
    ADDBYTE    ( pak, 0x06 );//clan rank
    ADDSTRING  ( pak, thisclient->Clan->clanname );
    ADDBYTE    ( pak, 0x00 );
    GServer->SendToVisible( &pak, thisclient );


     return true;
}

//LMA: Give clan points to somebody
bool CWorldServer::pakGMClanPoints(CPlayer* thisclient, char* name, int points)
{
  	CPlayer* otherclient = GetClientByCharName (name);
	if(otherclient==NULL){
        BEGINPACKET(pak, 0x702);
		ADDSTRING(pak, "User does not exist or is not online.");
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);
        return true;
    }

     if (otherclient->Clan->clanid==0)
     {
        Log(MSG_WARNING,"pakGMClanPoints:: User %s does not have a clan",thisclient->CharInfo->charname);
        return true;
     }

     //adding points if needed
     //Asking CharServer to refresh the player's informations.
    otherclient->Clan->CP=GServer->getClanPoints(otherclient->Clan->clanid);
    if(otherclient->Clan->CP+points<0)
    {
        otherclient->Clan->CP=0;
    }
    else
    {
        otherclient->Clan->CP+=points;
    }

    if (points>0)
    {
        char buffer[200];
        sprintf( buffer, "You received %i Clan Points !!", points);
        BEGINPACKET ( pak, 0x702 );
        ADDSTRING( pak, buffer );
        ADDBYTE( pak, 0 );
        otherclient->client->SendPacket( &pak );

        RESETPACKET( pak, 0x7e0 );
     	ADDBYTE    ( pak, 0xfe );
    	//ADDWORD    ( pak, otherclient->CharInfo->charid);  //charid
    	ADDDWORD    ( pak, otherclient->CharInfo->charid);  //charid
    	ADDDWORD    ( pak, points);  //Clan points (to be added)
    	cryptPacket( (char*)&pak, GServer->cct );
    	send( csock, (char*)&pak, pak.Size, 0 );
    }
    else
    {
        BEGINPACKET( pak, 0x7e0 );
 	    ADDBYTE    ( pak, 0xfe );
    	//ADDWORD    ( pak, otherclient->CharInfo->charid);  //charid
    	ADDDWORD    ( pak, otherclient->CharInfo->charid);  //charid
    	ADDDWORD    ( pak, points);  //Clan points (to be added)
    	cryptPacket( (char*)&pak, GServer->cct );
    	send( csock, (char*)&pak, pak.Size, 0 );
    }

    //2do:
    //Add the last packet needed?
    BEGINPACKET( pak, 0x7e0 );
    ADDBYTE    ( pak, 0x35 );
    ADDWORD    ( pak, otherclient->clientid );
    ADDWORD    ( pak, otherclient->Clan->clanid );
    ADDWORD    ( pak, 0x0000 );
    ADDWORD    ( pak, otherclient->Clan->back );
    ADDWORD    ( pak, otherclient->Clan->logo );
    ADDBYTE    ( pak, otherclient->Clan->grade );
    ADDBYTE    ( pak, otherclient->Clan->clanrank);
    ADDSTRING  ( pak, otherclient->Clan->clanname );
    ADDBYTE    ( pak, 0x00 );
    otherclient->client->SendPacket(&pak);


     return true;
}

// Add Fairy
bool CWorldServer::pakGMFairyto(CPlayer* thisclient, char* name, int mode)
{
	if (GServer->Config.FairyMode== 0)
	{
        BEGINPACKET(pak, 0x702);
		ADDSTRING(pak, "Fairy feature is de-activated.");
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);
        return true;
    }

	CPlayer* otherclient = GetClientByCharName (name);
	if(otherclient==NULL)
	{
        BEGINPACKET(pak, 0x702);
		ADDSTRING(pak, "User does not exist or is not online.");
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);
        return true;
    }

    if(mode == 0 && otherclient->Fairy == false)
    {
        BEGINPACKET(pak, 0x702);
		ADDSTRING(pak, "User already not fairied.");
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);
        return true;
    }

    if(mode == 1 && otherclient->Fairy == true)
    {
        BEGINPACKET(pak, 0x702);
		ADDSTRING(pak, "User already fairied.");
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);
        return true;
    }

    //We give a fairy.
    if(!otherclient->Fairy && mode == 1)
    {
          int FairyIndex=GServer->Config.FairyMax+1;
          for (int i=0; i<GServer->Config.FairyMax; i++)
          {
               if (GServer->FairyList.at(i)->assigned == false)
               {
                   FairyIndex=i;
                   //i=GServer->Config.FairyMax;
                   break;
               }

          }

          if (FairyIndex == (GServer->Config.FairyMax+1))
          {
              BEGINPACKET(pak, 0x702);
		      ADDSTRING(pak, "No free Fairy.");
		      ADDBYTE(pak, 0);
		      thisclient->client->SendPacket(&pak);
		      return true;
          }

          BEGINPACKET(pak, 0x702);
		  ADDSTRING(pak, "User fairied.");
		  ADDBYTE(pak, 0);
		  thisclient->client->SendPacket(&pak);

          int ListIndex=0;
          //LMA: New fairy system.
          ListIndex=otherclient->CharInfo->charid;
          /*
          //Old.
          for (int i=0; i<ClientList.size(); i++)
          {
              if (GServer->ClientList.at(i)->player == otherclient)
              {
                 ListIndex = i;
                 ListIndex = otherclient->CharInfo->charid;
                 i = GServer->ClientList.size();
              }

          }*/

          otherclient->Fairy = true;
          otherclient->FairyListIndex = FairyIndex;
          GServer->FairyList.at(FairyIndex)->ListIndex = ListIndex;
          GServer->FairyList.at(FairyIndex)->assigned = true;
          GServer->FairyList.at(FairyIndex)->LastTime = clock();
          GServer->FairyList.at(FairyIndex)->WaitTime = GServer->Config.FairyWait;
          GServer->DoFairyStuff( otherclient, mode );
          otherclient->SetStats();
          Log( MSG_INFO, "HP: %i  MP: %i  ATK: %i   DEF: %i   CRI: %i  MSPD: %i", otherclient->Stats->MaxHP, otherclient->Stats->MaxMP, otherclient->Stats->Attack_Power, otherclient->Stats->Defense, otherclient->Stats->Critical, otherclient->Stats->Move_Speed);
    }

    //We take back a fairy.
    if(otherclient->Fairy && mode == 0)
    {
          GServer->DoFairyFree(otherclient->FairyListIndex);
          GServer->FairyList.at(otherclient->FairyListIndex)->WaitTime = GServer->Config.FairyWait;
          otherclient->Fairy = false;
          otherclient->FairyListIndex = 0;
          GServer->DoFairyStuff( otherclient, mode );
          BEGINPACKET(pak, 0x702);
		  ADDSTRING(pak, "User unfairied.");
		  ADDBYTE(pak, 0);
		  otherclient->client->SendPacket(&pak);
          otherclient->SetStats();
          Log( MSG_INFO, "HP: %i  MP: %i  ATK: %i   DEF: %i   CRI: %i  MSPD: %i", otherclient->Stats->MaxHP, otherclient->Stats->MaxMP, otherclient->Stats->Attack_Power, otherclient->Stats->Defense, otherclient->Stats->Critical, otherclient->Stats->Move_Speed);
    }

    otherclient->SetStats();


	return true;
}

//LMA: let's hurt some people :)
//takes away 90% of HP and MP of a player
bool CWorldServer::pakGMHurtHim(CPlayer* thisclient, char* name)
{
	CPlayer* otherclient = GetClientByCharName (name);
	if(otherclient==NULL){
        BEGINPACKET(pak, 0x702);
		ADDSTRING(pak, "User does not exist or is not online.");
		ADDBYTE(pak, 0);
		thisclient->client->SendPacket(&pak);
        return true;
    }

    otherclient->Stats->HP=(long int) 10*otherclient->Stats->MaxHP/100;
    otherclient->Stats->MP=(long int) 10*otherclient->Stats->MaxMP/100;
    otherclient->SetStats();
	return true;
}

// Activate de-activate Fairy mode in game
bool CWorldServer::pakGMManageFairy(CPlayer* thisclient, int mode)
{
    BEGINPACKET (pak, 0x702);
	if(mode == 0)
	{
        if (GServer->Config.FairyMode != mode)
        {
	        GServer->Config.FairyMode = 0;
            ADDSTRING(pak, "You have de-activated the Fairy mode ingame.");
        }
        else
        {
            ADDSTRING(pak, "The Fairy mode is already de-activated.");
        }

    }
    else
    {
      	if (GServer->Config.FairyMode != mode)
      	{
	        GServer->Config.FairyMode = 1;
            //for (int i=0; i<GServer->Config.FairyMax; i++)
            for (int i=0; i<GServer->FairyList.size(); i++)
            {
                GServer->DoFairyFree(i);
                GServer->FairyList.at(i)->WaitTime = GServer->Config.FairyWait;
            }

            ADDSTRING(pak, "You have activated the Fairy mode ingame.");
        }
        else
        {
            ADDSTRING(pak, "The Fairy mode is already activated.");
        }

    }

    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);


	return true;
}

// Change FairyWait
bool CWorldServer::pakGMChangeFairyWait(CPlayer* thisclient, int newvalue)
{
    GServer->Config.FairyWait = newvalue;
    for (int i=0; i<GServer->FairyList.size(); i++){
        GServer->FairyList.at(i)->WaitTime = newvalue;
    }
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "You have changed the time between each Fairies");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}

// Change FairyStay
bool CWorldServer::pakGMChangeFairyStay(CPlayer* thisclient, int newvalue)
{
    GServer->Config.FairyStay = newvalue;
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "You have changed the time of Fairies for Buffing ");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}

// Change Fairy Test mode.
// 0 -> normal random x * Fairywait
// 1 -> Test mode activated: 1 * Fairywait only
bool CWorldServer::pakGMChangeFairyTestMode(CPlayer* thisclient, int mode)
{
    GServer->Config.FairyTestMode = mode;
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "You have changed the Fairy test mode.");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}

// Give Zuly
bool CWorldServer::pakGMZulygive(CPlayer* thisclient, char* name, long long zuly)
{
    CPlayer* otherclient = GetClientByCharName (name);
    if(otherclient==NULL)
        return true;
	otherclient->CharInfo->Zulies += zuly;
	BEGINPACKET(pak, 0x7a7);
	ADDWORD(pak, otherclient->clientid);
	ADDWORD(pak, 0);
	ADDBYTE(pak, 0);
	ADDDWORD(pak, 0xccccccdf);
	ADDDWORD(pak, zuly);
    ADDDWORD( pak, 0xcccccccc );
    ADDWORD ( pak, 0xcccc );
	otherclient->client->SendPacket(&pak);

	return true;
}


//LMA: setting an union for a player
bool CWorldServer::pakGMUnion(CPlayer* thisclient, char* name, int which_union)
{
    CPlayer* otherclient = GetClientByCharName (name);
    if(otherclient==NULL)
        return true;
    if(which_union<0||which_union>7)
      return true;

    if(which_union==0)
    {
       //leaving union.
       //05 00 00 00 00 00
        BEGINPACKET( pak, 0x721 );
        ADDWORD( pak, 0x05 );
        ADDWORD( pak, 0x0000 );
        ADDWORD( pak, 0x0000 );
        otherclient->client->SendPacket( &pak );

    	RESETPACKET( pak, 0x730 );
        ADDWORD    ( pak, 0x0005 );
        ADDDWORD   ( pak, 0x40b3a24d );
        otherclient->client->SendPacket( &pak );

       char buffer[200];
       sprintf ( buffer, "GM %s made you left the Union.",thisclient->CharInfo->charname);
       SendPM(otherclient, buffer);

       Log( MSG_GMACTION, "Union set to %i for %s by %s" , which_union,name,thisclient->CharInfo->charname);

       thisclient->CharInfo->unionid=0;
       return true;
    }

    //LMA: changing union :)
    BEGINPACKET( pak, 0x721 );
    ADDWORD( pak, 0x05 );
    ADDWORD( pak, which_union );
    ADDWORD( pak, 0x0000 );
    otherclient->client->SendPacket( &pak );
	RESETPACKET( pak, 0x730 );
    ADDWORD    ( pak, 0x0005 );
    ADDDWORD   ( pak, 0x40b3a24d );
    otherclient->client->SendPacket( &pak );

   char buffer[200];
   sprintf ( buffer, "Welcome to union %i, %s",which_union,otherclient->CharInfo->charname);
   SendPM(otherclient, buffer);
   thisclient->CharInfo->unionid=which_union;
   Log( MSG_GMACTION, "Union set to %i for %s by %s" , which_union,name,thisclient->CharInfo->charname);


     return true;
}

//LMA: Giving union points to a player.
bool CWorldServer::pakGMUnionPoints(CPlayer* thisclient, char* name, int nb_points)
{
    CPlayer* otherclient = GetClientByCharName (name);
    if(otherclient==NULL)
    {
        return true;
    }

    if(nb_points<=0||otherclient->CharInfo->unionid==0)
    {
      return true;
    }

    int new_amount=0;
    int new_offset=80+otherclient->CharInfo->unionid;
    switch(new_offset)
    {
        case 81:
        {
            otherclient->CharInfo->union01+=nb_points;
            new_amount=otherclient->CharInfo->union01;
        }
        break;
        case 82:
        {
            otherclient->CharInfo->union02+=nb_points;
            new_amount=otherclient->CharInfo->union02;
        }
        break;
        case 83:
        {
            otherclient->CharInfo->union03+=nb_points;
            new_amount=otherclient->CharInfo->union03;
        }
        break;
        case 84:
        {
            otherclient->CharInfo->union04+=nb_points;
            new_amount=otherclient->CharInfo->union04;
        }
        break;
        case 85:
        {
            otherclient->CharInfo->union05+=nb_points;
            new_amount=otherclient->CharInfo->union05;
        }
        break;
    }

    //otherclient->CharInfo->union05+=nb_points;
    BEGINPACKET( pak, 0x721 );
    ADDWORD( pak, new_offset );
    ADDWORD( pak, new_amount );
    ADDWORD( pak, 0x0000 );
    otherclient->client->SendPacket( &pak );
    RESETPACKET( pak, 0x730 );
    ADDWORD    ( pak, 0x0005 );
    ADDDWORD   ( pak, 0x40b3a24d );
    otherclient->client->SendPacket( &pak );

   char buffer[200];
   sprintf ( buffer, "You have been given %i Faction Points by %s",nb_points,thisclient->CharInfo->charname);
   SendPM(otherclient, buffer);
   Log( MSG_GMACTION, "%i Faction points given to %s by %s" , nb_points,name,thisclient->CharInfo->charname);


     return true;
}

// Spawn a NPC
bool CWorldServer::pakGMNpc(CPlayer* thisclient, int npcid,int dialogid,int eventid)
{
	CNPC* thisnpc = new CNPC;
	assert(thisnpc);
	thisnpc->clientid = GetNewClientID();
	thisnpc->dir = 0;
	thisnpc->npctype = npcid;
	thisnpc->pos = thisclient->Position->current;
	thisnpc->posMap = thisclient->Position->Map;
	thisnpc->thisnpc = GetNPCDataByID( npcid );
	if( thisnpc->thisnpc==NULL ) return true;
	thisnpc->thisnpc->dialogid = dialogid;
	thisnpc->dialog=dialogid;
	thisnpc->event=eventid;
	thisnpc->thisnpc->eventid=eventid;
	CMap* map = MapList.Index[thisclient->Position->Map];
	thisnpc->lastAiUpdate=clock();
	map->AddNPC( thisnpc );
    char buffer[200];
    sprintf( buffer, "NPC Spawned! (NPC: %i) (Dialog: %i) (Event: %i)", npcid, dialogid,eventid );
    BEGINPACKET ( pak, 0x702 );
    ADDSTRING( pak, buffer );
    ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
	return true;
}

// Give Item to Player
bool CWorldServer::pakGMItemtoplayer(CPlayer* thisclient, char* name , UINT itemid, UINT itemtype, UINT itemamount, UINT itemrefine, UINT itemls, UINT itemstats, UINT itemsocket)
{
    CItem item;
    item.count            = itemamount;
    item.durability        = 40;
    item.itemnum        = itemid;
    item.itemtype        = itemtype;
    item.lifespan        = 100; //itemls Set lifespan to 100
    item.refine            = itemrefine;
    item.stats            = itemstats;
    item.socketed        = itemsocket;
    item.appraised        = 1;
    item.gem = 0;
    item.last_sp_value=0;
    item.sp_value=0;

    //LMA: Fuel for PAT.
    if (item.itemnum>=135&&item.itemnum<=136)
    {
        item.sp_value=item.lifespan*10;
    }

   CPlayer* otherclient = GetClientByCharName ( name );

   if(otherclient != NULL) {
      unsigned newslot = otherclient->GetNewItemSlot( item );
      if(newslot != 0xffff) {
         otherclient->items[newslot] = item;
         otherclient->UpdateInventory( newslot );

         BEGINPACKET (pak, 0x702);
         ADDSTRING(pak, "You have recieved an item from a GM !");
         ADDBYTE(pak, 0);
         otherclient->client->SendPacket(&pak);

         RESETPACKET (pak, 0x702);
         ADDSTRING(pak, "Item has been given!");
         ADDBYTE(pak, 0);
         thisclient->client->SendPacket(&pak);
      }
      else {
         BEGINPACKET (pak, 0x702);
         ADDSTRING(pak, "No free slot !");
         ADDBYTE(pak, 0);
         thisclient->client->SendPacket(&pak);
      }
   }

   return true;
}

// Do Emotion
bool CWorldServer::pakGMDoEmote( CPlayer* thisclient, int emotionid )
{
    //LMA: Fix
    if(emotionid<=0||emotionid>300)
    {
        return true;
    }

    ClearBattle( thisclient->Battle );
	BEGINPACKET( pak, 0x781 );
	ADDDWORD( pak, emotionid );
	ADDWORD( pak, thisclient->clientid );
	SendToVisible( &pak, thisclient );
	return true;
}

// Change GM Stats (Coded by Minoc)
bool CWorldServer::pakGMStat( CPlayer* thisclient, const char* statname, int statvalue )
{
    int statid;
    if (strcmp( statname, "Str" )==0 || strcmp( statname, "str" )==0)
    {
        thisclient->Attr->Str = statvalue;
        statid = 0;
    }
    else if (strcmp( statname, "Dex" )==0 || strcmp( statname, "dex" )==0)
    {
        thisclient->Attr->Dex = statvalue;
        statid = 1;
    }
    else if (strcmp( statname, "Int" )==0 || strcmp( statname, "int" )==0)
    {
        thisclient->Attr->Int = statvalue;
        statid = 2;
    }
    else if (strcmp( statname, "Con" )==0 || strcmp( statname, "con" )==0)
    {
        thisclient->Attr->Con = statvalue;
        statid = 3;
    }
    else if (strcmp( statname, "Cha" )==0 || strcmp( statname, "cha" )==0)
    {
        thisclient->Attr->Cha = statvalue;
        statid = 4;
    }
    else if (strcmp( statname, "Sen" )==0 || strcmp( statname, "sen" )==0)
    {
        thisclient->Attr->Sen = statvalue;
        statid = 5;
    }
    else if (strcmp( statname, "team" )==0 || strcmp( statname, "Team" )==0)
    {
        thisclient->pvp_id=statvalue;
        SendPM(thisclient,"Do a /here so other people can see your new team ID");
        return true;
    }
    else
    {
        return true;
    }
    BEGINPACKET( pak, 0x7a9 );
    ADDBYTE( pak, statid );
    ADDWORD( pak, (unsigned short)statvalue );
    thisclient->client->SendPacket( &pak );
    thisclient->SetStats( );
    return true;
}

// GM: Teleport using map id  credits to Blackie
bool CWorldServer::pakGMGotomap( CPlayer* thisclient, int map, bool no_qsd_zone)
{
    //fix by PY / ScionEyez
    if(map<=0||map>=MapList.max)
    {
        return true;
    }

    CMap* thismap = GServer->MapList.Index[map];
    if(thismap == NULL)
    {
        SendPM(thisclient,"Invalid map ID %u",map);
        //SendSysMsg( thisclient, "Invalid map");
        return true;
    }

    //CRespawnPoint* thisrespawn = MapList.Index[map]->GetFirstRespawn( );
    CRespawnPoint* thisrespawn = thismap->GetFirstRespawn( );
    if(thisrespawn==NULL)
    {
        SendSysMsg( thisclient, "This map have no respawn" );
        return true;
    }

    //LMA: QSD Zone jump.
    if(no_qsd_zone)
    {
        thisclient->skip_qsd_zone=true;
    }

    MapList.Index[map]->TeleportPlayer( thisclient, thisrespawn->dest, false );
	return true;
}

// Heal a player ( by rl2171 )
// need to double check function - no errors compiling, but not healing MP and stamina
// playerdata.cpp has this value - CharInfo->MaxStamina = 5000;
// chartype.h has this value -  unsigned int MaxStamina;
bool CWorldServer::pakGMHeal( CPlayer* thisclient )
{
	thisclient->Stats->HP = thisclient->Stats->MaxHP;
	thisclient->Stats->MP = thisclient->Stats->MaxMP;
	thisclient->CharInfo->stamina = thisclient->CharInfo->MaxStamina;
	BEGINPACKET( pak, 0x7ec );
	ADDWORD( pak, thisclient->Stats->HP );
	ADDWORD( pak, thisclient->Stats->MP );
	ADDWORD( pak, thisclient->CharInfo->stamina );
	thisclient->client->SendPacket( &pak );
	return true;
}

// GM: Server Information ( by rl2171 ) modified by me
bool CWorldServer::pakGMServerInfo( CPlayer* thisclient )
{
    char buffer[200];
    // Players Online
	BEGINPACKET( pak, 0x0784 );
	ADDSTRING( pak, "[SYS]ServerInfo" );
	ADDBYTE( pak, 0 );
	ADDSTRING( pak, "SERVER INFORMATION" );
	ADDBYTE( pak, 0 );
	thisclient->client->SendPacket( &pak );
	sprintf( buffer, "Online Players: %i", (int)ClientList.size()-1 );// -1 (we don't count charserver)
	RESETPACKET( pak, 0x0784 );
	ADDSTRING( pak, "[SYS]ServerInfo" );
	ADDBYTE( pak, 0 );
	ADDSTRING( pak, buffer );
	ADDBYTE( pak, 0 );
	thisclient->client->SendPacket( &pak );
    // Exp / Zulies / Drop rates
	sprintf( buffer, "Exp %i | Zulies %i | Drops %i", Config.EXP_RATE, Config.ZULY_RATE, Config.DROP_RATE );
	RESETPACKET( pak, 0x0784 );
	ADDSTRING( pak, "[SYS]ServerInfo" );
	ADDBYTE( pak, 0 );
	ADDSTRING( pak, buffer );
	ADDBYTE( pak, 0 );
	thisclient->client->SendPacket( &pak );
	// Send map time
	if(MapList.Index[thisclient->Position->Map]!=NULL)
	{
    	RESETPACKET( pak, 0x0784 );
    	ADDSTRING( pak, "[SYS]ServerInfo" );
    	ADDBYTE( pak, 0 );
    	switch(MapList.Index[thisclient->Position->Map]->CurrentTime)
    	{
            case MORNING:
                sprintf( buffer, "The Time is: Morning[%i]", MapList.Index[thisclient->Position->Map]->MapTime%MapList.Index[thisclient->Position->Map]->dayperiod );
                ADDSTRING( pak, buffer );
            break;
            case DAY:
                sprintf( buffer, "The Time is: Day[%i]", MapList.Index[thisclient->Position->Map]->MapTime%MapList.Index[thisclient->Position->Map]->dayperiod );
                ADDSTRING( pak, buffer );
            break;
            case EVENING:
                sprintf( buffer, "The Time is: Evening[%i]", MapList.Index[thisclient->Position->Map]->MapTime%MapList.Index[thisclient->Position->Map]->dayperiod );
                ADDSTRING( pak, buffer );
            break;
            case NIGHT:
                sprintf( buffer, "The Time is: Night[%i]", MapList.Index[thisclient->Position->Map]->MapTime%MapList.Index[thisclient->Position->Map]->dayperiod );
                ADDSTRING( pak, buffer );
            break;
            default:
                sprintf( buffer, "Invalid Time is the End of world [%i]", MapList.Index[thisclient->Position->Map]->MapTime );
                ADDSTRING( pak, buffer );
        }
    	ADDBYTE( pak, 0 );
    	thisclient->client->SendPacket( &pak );
    }
	return true;
}

// Show Target Info
bool CWorldServer::GMShowTargetInfo( CPlayer* thisclient )
{
    if(thisclient->Battle->target==0) return true;
    char buffer[200];
    CMonster* monster = GetMonsterByID( thisclient->Battle->target, thisclient->Position->Map );
    if(monster==NULL) return true;
    float dist = distance( thisclient->Position->current, monster->Position->current );
    sprintf( buffer, "Target Position: %.4f, %.4f", monster->Position->current.x, monster->Position->current.y );
    BEGINPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    sprintf( buffer, "Distance: %.0f",	dist );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    sprintf( buffer, "Target Defense: %i", monster->Stats->Defense );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    sprintf( buffer, "Target Level: %i", monster->thisnpc->level );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    sprintf( buffer, "Target Attack Power: %i", monster->Stats->Attack_Power );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    sprintf( buffer, "Target Attack Speed: %.0f", monster->Stats->Attack_Speed );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    sprintf( buffer, "Target Move Speed: %i", monster->Stats->Move_Speed );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    sprintf( buffer, "Target HP/MAXHP: %i/%i", monster->Stats->HP, monster->Stats->MaxHP );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
	ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
// added Database & Monster ID by rl2171 with help from lmame
    sprintf( buffer, "Target Database ID: %i", monster->Position->respawn );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
    ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    sprintf( buffer, "Target Monster ID: %i", monster->montype );
    RESETPACKET( pak, 0x784 );
    ADDSTRING  ( pak, "[SYS]TargetInfo" );
    ADDBYTE    ( pak, 0 );
    ADDSTRING  ( pak, buffer );
    ADDBYTE( pak, 0 );
    thisclient->client->SendPacket( &pak );
    return true;
}

// GM: Make yourself invisible from tomiz
bool CWorldServer::pakGMHide( CPlayer* thisclient, int mode )
{
    BEGINPACKET( pak, 0x702 );
    if ( mode == 1 )
    {
        thisclient -> isInvisibleMode = true;
        ADDSTRING( pak, "You are now invisible, type /here !" );
        Log( MSG_GMACTION, " %s : /hide invisible" , thisclient->CharInfo->charname);
    }
	else if ( mode == 2)
	{
		thisclient -> isInvisibleMode = true;
		thisclient -> isInvisibleMode2 = true;
        ADDSTRING( pak, "You are now invisible and removed from gmlist, type /here !" );
        Log( MSG_GMACTION, " %s : /hide invisible" , thisclient->CharInfo->charname);
	}
    else
    {
        thisclient -> isInvisibleMode = false;
        thisclient -> isInvisibleMode2 = false;
        ADDSTRING( pak, "You are now visible type /here !" );
        Log( MSG_GMACTION, " %s : /hide visible" , thisclient->CharInfo->charname);
    }
    ADDBYTE( pak, 0 );
    thisclient->client->SendPacket ( &pak );
    return true;
}
// GM : Change the party lvl
bool CWorldServer::pakGMPartylvl( CPlayer* partyclient, int level )
{
    if (partyclient->Party->party == NULL) return true;
    if (level < 0 || level >50) return true;
    partyclient->Party->party->PartyLevel = level;
    if( partyclient->Party->party->PartyLevel == 50)
          partyclient->Party->party->Exp = 0;
          //Send Party Level and Party Exp
          BEGINPACKET( pak, 0x7d4 ); //
          ADDBYTE    ( pak, partyclient->Party->party->PartyLevel);
          ADDDWORD   ( pak, partyclient->Party->party->Exp );
          partyclient->Party->party->SendToMembers( &pak );
    return true;
}

// GM: Kill all mobs in a range of x-Fields
bool CWorldServer::pakGMKillInRange( CPlayer* thisclient, int range )
{
    for(UINT j = 0; j < MapList.Index[thisclient->Position->Map]->MonsterList.size(); j++)
    {
        CMonster* thismon = MapList.Index[thisclient->Position->Map]->MonsterList.at(j);
        if( IsMonInCircle( thisclient, thismon, (float)range ))
        {
            //Kill the mob
            thismon->Stats->HP = -1;
            BEGINPACKET( pak, 0x799 );
            ADDWORD    ( pak, thismon->clientid );
            ADDWORD    ( pak, thismon->clientid );
            ADDDWORD   ( pak, thismon->thisnpc->hp*thismon->thisnpc->level );
            ADDDWORD   ( pak, 16 );
            SendToVisible( &pak, thismon );
        	CMap* map = MapList.Index[thisclient->Position->Map];
            map->DeleteMonster( thismon );
        }
    }
    return true;
}


//LMA: NPC shouts or announces with LTB.
bool CWorldServer::pakGMnpcshout( CPlayer* thisclient, char* shan, char* aipqsd, int npctype, int ltbid )
{
    if ( strcmp ( shan , "shout" ) != 0 && strcmp ( shan , "ann" ) != 0)
    {
        return true;
    }

    if ( strcmp ( aipqsd , "aip" ) != 0 && strcmp ( aipqsd , "qsd" ) != 0)
    {
        return true;
    }

    if (strcmp ( aipqsd , "aip" )==0)
    {
        if(ltbid>=maxltbaip)
        {
            return true;
        }

    }

    if (strcmp ( aipqsd , "qsd" )==0)
    {
        if(ltbid>=maxltbqsd)
        {
            return true;
        }

    }

    //checking NPC
    CNPC* thisnpc = GetNPCByType(npctype);
    if(thisnpc==NULL)
    {
        return true;
    }

	if(strcmp ( shan , "shout" ) == 0)
	{
	    if ( strcmp ( aipqsd , "aip" )==0)
	    {
	        //GServer->NPCShout(NULL,GServer->Ltbstring[ltbid]->LTBstring,GServer->GetNPCNameByType(thisnpc->npctype),thisnpc->posMap);
	        GServer->NPCShout(NULL,GServer->Ltbstring[ltbid]->LTBstring, GServer->GetSTLMonsterNameByID(thisnpc->npctype),thisnpc->posMap);
	    }
	    else
	    {
	        //GServer->NPCShout(NULL,GServer->LtbstringQSD[ltbid]->LTBstring,GServer->GetNPCNameByType(thisnpc->npctype),thisnpc->posMap);
	        GServer->NPCShout(NULL,GServer->LtbstringQSD[ltbid]->LTBstring, GServer->GetSTLMonsterNameByID(thisnpc->npctype),thisnpc->posMap);
	    }

	}
	else
	{
	    if ( strcmp ( aipqsd , "aip" )==0)
	    {
	        //GServer->NPCAnnounce(GServer->Ltbstring[ltbid]->LTBstring,GServer->GetNPCNameByType(thisnpc->npctype));
	        GServer->NPCAnnounce(GServer->Ltbstring[ltbid]->LTBstring, GServer->GetSTLMonsterNameByID(thisnpc->npctype));
	    }
	    else
	    {
	        //GServer->NPCAnnounce(GServer->LtbstringQSD[ltbid]->LTBstring,GServer->GetNPCNameByType(thisnpc->npctype));
	        GServer->NPCAnnounce(GServer->LtbstringQSD[ltbid]->LTBstring, GServer->GetSTLMonsterNameByID(thisnpc->npctype));
	    }

	}

	Log( MSG_GMACTION, " %s : /npcltb %s %s %i %i", thisclient->CharInfo->charname,shan,aipqsd,npctype,ltbid);


    return true;
}

// GM: Change Class (from Crash)
// Block Multiclass, except for GMs  (from MonkeyRose)
bool CWorldServer::pakGMClass( CPlayer* thisclient, char* classid )
{
    int classid_new = thisclient->CharInfo->Job;
    int nb_donation;
    // allow GMs to multiclass
    bool GM = ( thisclient->Session->accesslevel >= 300 );

    nb_donation = GetVotePoints(thisclient);

    if ( strcmp ( classid , "Visitor" ) == 0 || strcmp ( classid , "visitor" ) == 0)
    {
        if (GM)
           classid_new = 0;
    }
    else if ( strcmp ( classid , "Soldier" ) == 0 || strcmp ( classid , "soldier" ) == 0)
    {
         if ( GM || ((thisclient->Stats->Level >= 10) && (classid_new == 0 ))) // visitor
             classid_new = 111;
    }
    else if ( strcmp ( classid , "Knight" ) == 0 || strcmp ( classid , "knight" ) == 0)
    {
        if ( GM || ((thisclient->Stats->Level >= 100) && (classid_new == 111)))  // solder
             classid_new = 121;
    }
    else if ( strcmp ( classid , "Champion" ) == 0 || strcmp ( classid , "champion" ) == 0)
    {
        if ( GM || ((thisclient->Stats->Level >= 100) && (classid_new == 111)))  // solder
                classid_new = 122;
    }
    else if ( strcmp ( classid , "Muse" ) == 0 || strcmp ( classid , "muse" ) == 0)
    {
         if ( GM || ((thisclient->Stats->Level >= 10) && (classid_new == 0 ))) // visitor
                classid_new = 211;
    }
    else if ( strcmp ( classid , "Mage" ) == 0 || strcmp ( classid , "mage" ) == 0)
    {
        if ( GM || ((thisclient->Stats->Level >= 100) && (classid_new == 211)))  // muse
                classid_new = 221;
    }
    else if ( strcmp ( classid , "Cleric" ) == 0 || strcmp ( classid , "cleric" ) == 0)
    {
        if ( GM || ((thisclient->Stats->Level >= 100) && (classid_new == 211)))  // muse
                classid_new = 222;
    }
    else if ( strcmp ( classid , "Hawker" ) == 0 || strcmp ( classid , "hawker" ) == 0)
    {
         if ( GM || ((thisclient->Stats->Level >= 10) && (classid_new == 0 ))) // visitor
                classid_new = 311;
    }
    else if ( strcmp ( classid , "Raider" ) == 0 || strcmp ( classid , "raider" ) == 0)
    {
        if ( GM || ((thisclient->Stats->Level >= 100) && (classid_new == 311)))  // hawker
                classid_new = 321;
    }
    else if ( strcmp ( classid , "Scout" ) == 0 || strcmp ( classid , "scout" ) == 0)
    {
        if ( GM || ((thisclient->Stats->Level >= 100) && (classid_new == 311)))  // hawker
                classid_new = 322;
    }
    else if ( strcmp ( classid , "Dealer" ) == 0 || strcmp ( classid , "dealer" ) == 0)
    {
         if ( GM || ((thisclient->Stats->Level >= 10) && (classid_new == 0 ))) // visitor
               classid_new = 411;
    }
    else if ( strcmp ( classid , "Bourgeois" ) == 0 || strcmp ( classid , "bourgeois" ) == 0)
    {
        if ( GM || ((thisclient->Stats->Level >= 100) && (classid_new == 411)))  // dealer
                classid_new = 421;
    }
    else if ( strcmp ( classid , "Artisan" ) == 0 || strcmp ( classid , "artisan" ) == 0)
    {
        if ( GM || ((thisclient->Stats->Level >= 100) && (classid_new == 411)))  // dealer
                classid_new = 422;
    }
    else
    {
        return true;
    }
    /*   add this section?
    // Set the quest vars for the new class by Drakia
    dword jVarSw = classid_new % 10;
    thisclient->quest.JobVar[0] = 0;
    thisclient->quest.JobVar[1] = 0;
    if (jVarSw == 1) {
      thisclient->quest.JobVar[0] = 1;
    } else if (jVarSw == 2) {
      thisclient->quest.JobVar[0] = 1;
      thisclient->quest.JobVar[1] = 2;
    }
    */

    bool changed = true;
    bool donationbool;

    if( nb_donation >= 10 || GM)
        donationbool = true;
    else
        donationbool = false;

    if ( thisclient->CharInfo->Job == classid_new )
       changed = false;

    if ( changed && donationbool )
    {
        int sp_points=0;

		thisclient->CharInfo->Job = classid_new;
		BEGINPACKET(pak, 0x0721);
		ADDWORD(pak,4);
		ADDWORD(pak, thisclient->CharInfo->Job);
		ADDWORD(pak,0);
		thisclient->client->SendPacket(&pak);
		RESETPACKET(pak, 0x0730);
		ADDWORD(pak, 5);
		ADDWORD(pak, 0xa24d);
		ADDWORD(pak, 0x40b3);
		thisclient->client->SendPacket(&pak);

        //LMA: deleting all previous class skills and getting skill points for people
        //fool enough to give this gm command to players :(
        for (int k=0;k<60;k++)
        {
            if(thisclient->cskills[k].thisskill!=NULL&&thisclient->cskills[k].thisskill->sp>0)
            {
                sp_points+=thisclient->cskills[k].level;
            }

            thisclient->cskills[k].id = 0;
            thisclient->cskills[k].level = 1;
            thisclient->cskills[k].thisskill=NULL;
        }

        thisclient->saveskills();
        thisclient->ResetSkillOffset();
        SendPM(thisclient, "You paid 10VP for class change! Please relog now!" );

        if(sp_points>0)
        {
            thisclient->CharInfo->SkillPoints+=sp_points;
            BEGINPACKET( pak, 0x720 );
            ADDWORD( pak, 37 );
            ADDWORD( pak, sp_points );
            ADDWORD( pak, 0 );
            thisclient->client->SendPacket( &pak );
            RESETPACKET( pak, 0x0730 );
            ADDWORD( pak, 5 );
            ADDWORD( pak, 0xa24d );
            ADDWORD( pak, 0x40b3 );
            thisclient->client->SendPacket( &pak );
        }
        if(thisclient->Session->accesslevel < 299)
        GServer->DB->QExecuteUpdate("UPDATE accounts SET vote_points = vote_points - %i WHERE id = %i",10,thisclient->Session->userid);
        Log( MSG_GMACTION, " %s : /class %s" , thisclient->CharInfo->charname, classid);

    }
    else
    {
       if ( thisclient->Stats->Level < 10 )
          SendPM(thisclient, "Class change failed! You must be at least lvl 10 to change your job." );
       else if (classid_new == 0) // visitor
          SendPM(thisclient, "Class change failed! Pick a first job, muse, dealer, hawker, or solder" );
       else if (donationbool < 10)
          SendPM(thisclient, "Need atleast 10VP to change job");
       else
          SendPM(thisclient, "Class change failed!" );
    }

    return true;
}



bool CWorldServer::pakGMTeleAllHere( CPlayer* thisclient )
{
    int count=1;
    while(count <= (ClientList.size()-1))
    {
        CPlayer* otherclient = (CPlayer*)ClientList.at(count)->player;
        if ((otherclient != NULL) && (otherclient != thisclient))
        {
            CMap* map = MapList.Index[thisclient->Position->Map];
            map->TeleportPlayer( otherclient, thisclient->Position->current, false );
        }
        count++;
    }
    return true;
}

bool CWorldServer::pakGMMoveTo( CPlayer* thisclient, fPoint position )
{
    if( thisclient->Shop->open || (!thisclient->Ride->Drive && thisclient->Ride->Ride) || !thisclient->Status->CanMove )
        return true;
    if( thisclient->Status->Stance==1 )
        thisclient->Status->Stance=3;
    thisclient->Position->destiny = position; // PAKGMMOVETO
    ClearBattle( thisclient->Battle );
	BEGINPACKET( pak, 0x79a );
	ADDWORD    ( pak, thisclient->clientid );
	ADDWORD    ( pak, thisclient->Battle->target );

	//LMA: It's distance.
	//ADDWORD    ( pak, thisclient->Stats->Move_Speed );
	ADDWORD    ( pak, (DWORD) (distance(thisclient->Position->current,thisclient->Position->destiny)*100) );

	ADDFLOAT   ( pak, thisclient->Position->destiny.x*100 );
	ADDFLOAT   ( pak, thisclient->Position->destiny.y*100 );
	ADDFLOAT   ( pak, thisclient->Position->destiny.z*100 );
    SendToVisible( &pak, thisclient );
    return true;
}

//change the player dmg rate
bool CWorldServer::pakGMChangePlayerDmg(CPlayer* thisclient, int rate)
{
    GServer->Config.PlayerDmg = rate;
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "You have changed the player dmg rate.");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}
//change the monster dmg rate
bool CWorldServer::pakGMChangeMonsterDmg(CPlayer* thisclient, int rate)
{
    GServer->Config.MonsterDmg = rate;
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "You have changed the Monster dmg rate.");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}
//change the Cfmode
bool CWorldServer::pakGMChangeCfmode(CPlayer* thisclient, int mode)
{
    if(mode<0) mode = 0;
    if(mode>1) mode = 1;
    GServer->Config.Cfmode = mode;
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "You have changed the Clan Field Mode.");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}

bool CWorldServer::pakGMWhoAttacksMe(CPlayer* thisclient)
{
     std::cout << "You are attacked by these Mobs: ";
     CMap* map = GServer->MapList.Index[thisclient->Position->Map];
     for(UINT i=0;i<map->MonsterList.size();i++)
    {
        CMonster* thismon = map->MonsterList.at( i );
		float distance = GServer->distance ( thisclient->Position->current, thismon->Position->current );
		if (thismon->Battle->target == thisclient->clientid)
           std::cout <<"ID: "<<thismon->clientid<< "  / " <<thismon->montype<<" I "<< (GServer->IsVisible(thisclient, thismon) ? "Visible\n":"Invisible\n");
	}
	return true;
}

//change the AtkSpeedModif || For test only
bool CWorldServer::pakGMChangeAtkSpeedModif(CPlayer* thisclient, int modif)
{
    GServer->ATTK_SPEED_MODIF = modif;
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "AttkSpeedModif Changed.");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}

//change the HitDelayModif || For test only
bool CWorldServer::pakGMChangeHitDelayModif(CPlayer* thisclient, int modif)
{
    GServer->HIT_DELAY_MODIF = modif;
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "HitDelayModif Changed.");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}

//change the mSpeedModif || For test only
bool CWorldServer::pakGMChangeMSpeedModif(CPlayer* thisclient, int modif)
{
    GServer->MOVE_SPEED_MODIF = modif;
    BEGINPACKET (pak, 0x702);
    ADDSTRING(pak, "mSpeedModif Changed.");
    ADDBYTE(pak, 0);
    thisclient->client->SendPacket(&pak);
	return true;
}

// GM Debuff players in sight. by Drakia
bool CWorldServer::pakGMDebuff(CPlayer* thisClient)
{
    for(int i = 0; i < 30; i++)
    {
        thisClient->MagicStatus[i].Duration = 0;
        thisClient->MagicStatus[i].BuffTime = 0;
    }
    thisClient->RefreshBuff();
    for (int i = 0; i < thisClient->VisiblePlayers.size(); i++)
    {
        CPlayer* targetClient = thisClient->VisiblePlayers[i];
        if (targetClient->Session->isLoggedIn == false) continue;
        if (targetClient->Stats->HP <= 0) continue;
        for(int j = 0; j < 30; j++)
        {
            targetClient->MagicStatus[j].Duration = 0;
            targetClient->MagicStatus[j].BuffTime = 0;
        }
        targetClient->RefreshBuff();
    }
    return true;
}

// GM Buff players in sight. by Drakia
bool CWorldServer::pakGMBuff( CPlayer* thisClient, int strength )
{
    // Buff the GM
    pakGMDebuff(thisClient);

    pakGMGiveBuff( thisClient, thisClient, 3906, strength); // Attack   (300s) (18)
    pakGMGiveBuff( thisClient, thisClient, 3905, strength); // Defense  (300s) (19)
    pakGMGiveBuff( thisClient, thisClient, 3908, strength); // Accuracy (420s) (20)
    pakGMGiveBuff( thisClient, thisClient, 3907, strength); // MResist  (420s) (21)
    pakGMGiveBuff( thisClient, thisClient, 3909, strength); // Dodge    (420s) (22)
    pakGMGiveBuff( thisClient, thisClient, 3904, strength); // Dash     (300s) (23)
    pakGMGiveBuff( thisClient, thisClient, 3910, strength); // Haste    (420s) (24)
    pakGMGiveBuff( thisClient, thisClient, 3911, strength); // Critical (420s) (26)
    pakGMGiveBuff( thisClient, thisClient, 3900, strength); // Max HP   (420s) (38)
    pakGMGiveBuff( thisClient, thisClient, 3901, strength); // Max MP   (420s) (39)
    pakGMGiveBuff( thisClient, thisClient, 3912, strength); //additional damage (420s) (36)
    // Buff all players visible
//    for (int i = 0; i < thisClient->VisiblePlayers.size(); i++)
//    {
//        CPlayer* target = thisClient->VisiblePlayers[i];
//        if ( target->Session->isLoggedIn == false ) continue;
//        if ( target->Stats->HP <= 0 ) continue;
    for(UINT j=0;j<ClientList.size();j++)
    {
        CPlayer* target = (CPlayer*) ClientList.at( j )->player;
        if ( target == NULL) continue;
        if ( target->Session->isLoggedIn == false ) continue;
        if ( target->Stats->HP <= 0 ) continue;
        if ( IsVisible(thisClient, target))
        {
        pakGMGiveBuff( thisClient, target, 3906, strength); // Attack   (300s) (18)
        pakGMGiveBuff( thisClient, target, 3905, strength); // Defense  (300s) (19)
        pakGMGiveBuff( thisClient, target, 3908, strength); // Accuracy (420s) (20)
        pakGMGiveBuff( thisClient, target, 3907, strength); // MResist  (420s) (21)
        pakGMGiveBuff( thisClient, target, 3909, strength); // Dodge    (420s) (22)
        pakGMGiveBuff( thisClient, target, 3904, strength); // Dash     (300s) (23)
        pakGMGiveBuff( thisClient, target, 3910, strength); // Haste    (420s) (24)
        pakGMGiveBuff( thisClient, target, 3911, strength); // Critical (420s) (26)
        pakGMGiveBuff( thisClient, target, 3900, strength); // Max HP   (420s) (38)
        pakGMGiveBuff( thisClient, target, 3901, strength); // Max MP   (420s) (39)
        pakGMGiveBuff( thisClient, target, 3912, strength); //additional damage (420s) (36)
        }
    }
    return true;
}

// Find the skill, add the buff, and send the packet. This is where the magic happens
// And what took me so freakin long to figure out x.x
bool CWorldServer::pakGMGiveBuff(CPlayer* thisClient, CPlayer* targetClient, int skillID, int strength)
{
    CSkills* skill = GServer->GetSkillByID(skillID);

    GServer->AddBuffs( skill, targetClient, strength );
    BEGINPACKET( pak, 0x7b5 );
    ADDWORD    ( pak, targetClient->clientid );
    ADDWORD    ( pak, thisClient->clientid );
    ADDWORD    ( pak, skillID );
    ADDWORD    ( pak, strength );
    ADDBYTE    ( pak, skill->nbuffs );
    GServer->SendToVisible( &pak, targetClient );
    return true;
}

// GM Give yourself all stats maxed
bool CWorldServer::pakGMMaxStats( CPlayer* thisclient )
{

        pakGMStat(thisclient, "str", Config.MaxStat);
        pakGMStat(thisclient, "dex", Config.MaxStat);
        pakGMStat(thisclient, "con", Config.MaxStat);
        pakGMStat(thisclient, "int", Config.MaxStat);
        pakGMStat(thisclient, "cha", Config.MaxStat);
        pakGMStat(thisclient, "sen", Config.MaxStat);

//changed from fixed 300 to Config.MaxStat controlled from list_config db  - rl2171

    return true;
}


//GM: All Skills {By CrAshInSiDe} - Skills and levels updated by rl2171 & Devilking
//LMA: Mysql now.
bool CWorldServer::pakGMAllSkill(CPlayer* thisclient, char* name)
{
    bool is_ok=false;
    int nb_skills=0;
    CPlayer* otherclient = GetClientByCharName( name );
    if(otherclient==NULL)
        return true;

    int classid = otherclient->CharInfo->Job;


    //LMA: MySQL time.
    MYSQL_ROW row;
    MYSQL_RES *result=NULL;
    //LMA: classid=0 for all classes skills (like uniques, mileages...).
    //LMA: classid=3 for basic skills.
    result = DB->QStore("SELECT skillid, skill_level FROM list_skills WHERE (classid=%i OR classid=3 OR classid=0) AND isactive=1 ORDER BY skillid ASC",classid);
    if(result==NULL) return true;

    if(mysql_num_rows(result)==0)
    {
        Log(MSG_INFO,"No skill to add for class %i",classid);
        DB->QFree( );
        return true;
    }

    //LMA: We delete previous skills to avoid errors (basic, class, unique and mileage ones, so all)...
    //They will be sorted correctly (if needed) at next startup...
    for (int k=0;k<MAX_ALL_SKILL;k++)
    {
        otherclient->cskills[k].id = 0;
        otherclient->cskills[k].level = 0;
        otherclient->cskills[k].thisskill=NULL;
    }

    //LMA: resetting the skill offsets.
    int cur_cskills[5];
    int end[5];
    bool first_pm=true;
    cur_cskills[0]=0;
    cur_cskills[1]=60;
    cur_cskills[2]=320;
    cur_cskills[3]=90;
    cur_cskills[4]=120;
    end[0]=60;
    end[1]=90;
    end[2]=362;
    end[3]=120;
    end[4]=320;

    for (int i=0;i<5;i++)
    {
        otherclient->cur_max_skills[i]=cur_cskills[i];
    }

    while( row = mysql_fetch_row(result) )
    {
        is_ok=true;
        nb_skills++;
        if(nb_skills>=MAX_ALL_SKILL)
        {
            Log(MSG_WARNING,"Too many skills, aborting");
            SendPM(thisclient,"Too many skills, aborting");
            break;
        }

        //LMA: old version.
        /*otherclient->cskills[nb_skills-1].id = atoi(row[0]);
        otherclient->cskills[nb_skills-1].level = atoi(row[1]);*/

        //LMA: New version, we sort the skills from the start...
        int skillid=atoi(row[0]);
        int skilllvl=atoi(row[1]);
        int family=otherclient->GoodSkill(skillid);
        if(family==-1)
        {
            Log(MSG_WARNING,"pakGMAllSkill:: couldn't get the family for skill %i",skillid);

            if(first_pm)
            {
                SendPM(thisclient,"Couldn't get the family for skill %i, check server's logs",skillid);
                first_pm=false;
            }

            continue;
        }

        //looking for the right offset.
        int offset=otherclient->FindSkillOffset(family);
        if(offset==-1)
        {
            if (family>=0&&family<5)
            {
                Log(MSG_WARNING,"pakGMAllSkill:: bad offset for family %i (too many skills? Has %i now, max is %i.)",family,otherclient->cur_max_skills[family],end[family]);
                if(first_pm)
                {
                    SendPM(thisclient,"bad offset for family %i (too many skills? Has %i now, max is %i.), check server's logs",family,otherclient->cur_max_skills[family],end[family]);
                    first_pm=false;
                }

            }
            else
            {
                Log(MSG_WARNING,"pakGMAllSkill:: bad offset for family %i (wrong family)",family);
                if(first_pm)
                {
                    SendPM(thisclient,"bad offset for family %i (wrong family), check server's logs",family);
                    first_pm=false;
                }

            }

            continue;
        }

        otherclient->cskills[offset].id = skillid;
        otherclient->cskills[offset].level = skilllvl;
    }

    DB->QFree( );

    if(is_ok)
    {
        SendPM (otherclient, "Relogin to get all skills");
        otherclient->AttrAllSkills();
        otherclient->saveskills();
        otherclient->ResetSkillOffset();
    }
    else
    {
        SendPM(thisclient, "Can't add skills for this class");
    }


    return true;
}

//GM: DeleteSkills {modified from allskill command - rl2171}
bool CWorldServer::pakGMDelSkills(CPlayer* thisclient, char* name)
{

    CPlayer* otherclient = GetClientByCharName( name );
    if(otherclient==NULL)
        return true;

    //LMA: deleting class skills, unique skills, mileage skills.
    for (int k=0;k<320;k++)
    {
        otherclient->cskills[k].id = 0;
        otherclient->cskills[k].level = 1;
        otherclient->cskills[k].thisskill=NULL;
    }

    SendPM (thisclient, "Relogin to remove All Skills");
    thisclient->ResetSkillOffset();
    thisclient->saveskills();


    return true;
}

// All GM Skills
//LMA: Mysql now.
bool CWorldServer::pakGMGMSkills(CPlayer* thisclient, char* name)
{
    CPlayer* otherclient = GetClientByCharName( name );
    if(otherclient==NULL)
        return true;

    //LMA: Looking for good place to save it now...
    int family=3;
    int nb_gm_skills=0;

    //LMA: MySQL time.
    MYSQL_ROW row;
    MYSQL_RES *result=NULL;
    result = DB->QStore("SELECT skillid, skill_level FROM list_skills WHERE classid=2 AND isactive=1");
    if(result==NULL) return true;

    nb_gm_skills=mysql_num_rows(result);
    if(nb_gm_skills==0)
    {
        Log(MSG_WARNING,"No GM Skill to learn.");
        DB->QFree( );
        return true;
    }

    int index=otherclient->FindSkillOffset(family);
    if(index==-1)
    {
        Log(MSG_WARNING,"No Room anymore for learning GM Skills.");
        DB->QFree( );
        return true;
    }

    if (index+nb_gm_skills>=120)
    {
        Log(MSG_WARNING,"Not enough place to learn all GM skills.");
        DB->QFree( );
        return true;
    }

    int index_ini=index;

    //time to learn...
    while( row = mysql_fetch_row(result) )
    {
        otherclient->cskills[index].id = atoi(row[0]);
        otherclient->cskills[index].level = atoi(row[1]);
        index++;
    }

    DB->QFree( );

    for (int k=index_ini;k<index;k++)
    {
        if (otherclient->cskills[k].id==0)
            continue;

        otherclient->cskills[k].thisskill = GServer->GetSkillByID( otherclient->cskills[k].id+otherclient->cskills[k].level-1 );
        if(otherclient->cskills[k].thisskill==NULL)
        {
            otherclient->cskills[k].id=0;
            otherclient->cskills[k].level=1;
        }

    }

    SendPM (otherclient, "Relogin to get all the GM Skills");
    otherclient->saveskills();
    otherclient->ResetSkillOffset();


    return true;
}

//LMA: We get the Objvar for a NPC.
bool CWorldServer::pakGMObjVar(CPlayer* thisclient, int npctype, int output)
{
    //output==1 for PM.
    //output==2 for debug.

    if(npctype>=MAX_NPC)
    {
        return true;
    }

    CNPC* thisnpc = GetNPCByType(npctype);
    if(thisnpc == NULL)
    {
        delete thisnpc;
        return true;
    }

    if(output==1&&thisclient!=NULL)
    {
        //PM.
        SendPM(thisclient,"Npc %i (%s) [%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i]",npctype,GetSTLMonsterNameByID(npctype),
        ObjVar[npctype][0],ObjVar[npctype][1],ObjVar[npctype][2],ObjVar[npctype][3],ObjVar[npctype][4],ObjVar[npctype][5],ObjVar[npctype][6],ObjVar[npctype][7],ObjVar[npctype][8],ObjVar[npctype][9],
        ObjVar[npctype][10],ObjVar[npctype][11],ObjVar[npctype][12],ObjVar[npctype][13],ObjVar[npctype][14],ObjVar[npctype][15],ObjVar[npctype][16],ObjVar[npctype][17],ObjVar[npctype][18],ObjVar[npctype][19]);
    }
    else
    {
        //debug.
        LogDebug("Npc %i [0]=%i, [1]=%i, [2]=%i, [3]=%i, [4]=%i, [5]=%i, [6]=%i, [7]=%i, [8]=%i, [9]=%i, [10]=%i, [11]=%i, [12]=%i, [13]=%i, [14]=%i, [15]=%i, [16]=%i, [17]=%i, [18]=%i, [19]=%i (%s)",npctype,
        ObjVar[npctype][0],ObjVar[npctype][1],ObjVar[npctype][2],ObjVar[npctype][3],ObjVar[npctype][4],ObjVar[npctype][5],ObjVar[npctype][6],ObjVar[npctype][7],ObjVar[npctype][8],ObjVar[npctype][9],
        ObjVar[npctype][10],ObjVar[npctype][11],ObjVar[npctype][12],ObjVar[npctype][13],ObjVar[npctype][14],ObjVar[npctype][15],ObjVar[npctype][16],ObjVar[npctype][17],ObjVar[npctype][18],ObjVar[npctype][19],
        GetSTLMonsterNameByID(npctype));
    }


    return true;
}

//LMA: We force an ObjVar value for a NPC
bool CWorldServer::pakGMSetObjVar(CPlayer* thisclient, int npctype, int index, int value)
{
    if(npctype>=2000||value<0||index<0||index>19)
    {
        return true;
    }

    CNPC* thisnpc = GetNPCByType(npctype);
    if(thisnpc == NULL)
    {
        delete thisnpc;
        return true;
    }

    ObjVar[npctype][index]=value;
    SendPM(thisclient,"ObjVar[%i][%i] is set to %i",npctype,index,value);

    //Has eventID changed?
    if(index==0)
    {
        thisnpc->event = value;

        BEGINPACKET( pak, 0x790 );
        ADDWORD    ( pak, thisnpc->clientid );
        ADDWORD    ( pak, thisnpc->event );	  //LMA: Welcome in the real Word ^_^
        GServer->SendToAllInMap(&pak,thisnpc->posMap);
    }


    return true;
}


//LMA: We force UnionWar
bool CWorldServer::pakGMForceUW(CPlayer* thisclient, int time)
{
    if (time<=0)
    {
        UWForceFrom=0;
        return true;
    }

    //Checking if UW is already in motion or not...
    if(GServer->ObjVar[1113][1]!=0)
    {
        SendPM(thisclient,"Union War already started!");
        return true;
    }

    SYSTEMTIME sTIME;
    GetLocalTime(&sTIME);
    sTIME.wMinute+=time;
    UWForceFrom=(sTIME.wHour * 60) + sTIME.wMinute;


    return true;
}

//LMA: We force gem quest
bool CWorldServer::pakGMForceGemQuest(CPlayer* thisclient, int time)
{
    if (time<=0)
    {
        GemQuestForce=0;
        return true;
    }

    //Checking if gem quest is already in motion or not...
    //1104=historian Jones
    if(GServer->ObjVar[1104][0]!=0)
    {
        SendPM(thisclient,"Gem quest already started!");
        return true;
    }

    GServer->ObjVar[1104][12]=0;
    GServer->ObjVar[1104][13]=0;

    SYSTEMTIME sTIME;
    GetLocalTime(&sTIME);
    sTIME.wMinute+=time;
    GemQuestForce=(sTIME.wHour * 60) + sTIME.wMinute;


    return true;
}



//LMA: We force Nb players requirement.
bool CWorldServer::pakGMForceUWPlayers(CPlayer* thisclient, int nb_players)
{
    if (nb_players<=0||nb_players>=100)
    {
        UWNbPlayers=0;
        return true;
    }

    UWNbPlayers=nb_players;
    GServer->DB->QExecute("UPDATE list_config SET uwnbplayers=%i",UWNbPlayers);


    return true;
}


//LMA: We export the STB and STL to a .sql file.
bool CWorldServer::pakGMExportSTBSTL(CPlayer* thisclient)
{
    if(GServer->Config.massexport==0)
    {
        return true;
    }

    FILE *filestb=NULL;
    filestb = fopen(LOG_DIRECTORY "stb_stl.sql", "w+" );

    fprintf(filestb,"CREATE TABLE `item_reference` (\r");
    fprintf(filestb,"`itemID` int(11) NOT NULL auto_increment,\r");
    fprintf(filestb,"`Type` int(11) NOT NULL,\r");
    fprintf(filestb,"`ID` int(11) NOT NULL,\r");
    fprintf(filestb,"`Name` tinytext NOT NULL,\r");
    fprintf(filestb,"`Comment` text,\r");
    fprintf(filestb,"PRIMARY KEY (`itemID`)\r");
    fprintf(filestb,") ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;\r");

    //Go... Items from 1 to 9
    for (int k=0;k<9;k++)
    {
        for (int j=0;j<EquipList[k+1].max;j++)
        {
            string name=EscapeMe(GetSTLObjNameByID(k+1,j));
            string comment=EscapeMe(GetSTLObjNameByID(k+1,j,true));
            fprintf(filestb,"INSERT INTO `item_reference` VALUES ('%u', '%u', '%u', '%s', '%s');\r",0,k+1,j,name.c_str(),comment.c_str());
        }

    }

    //Use.
    for (int j=0;j<JemList.max;j++)
    {
        string name=EscapeMe(GetSTLObjNameByID(10,j));
        string comment=EscapeMe(GetSTLObjNameByID(10,j,true));
        fprintf(filestb,"INSERT INTO `item_reference` VALUES ('%u', '%u', '%u', '%s', '%s');\r",0,10,j,name.c_str(),comment.c_str());
    }

    //Jem.
    for (int j=0;j<JemList.max;j++)
    {
        string name=EscapeMe(GetSTLObjNameByID(11,j));
        string comment=EscapeMe(GetSTLObjNameByID(11,j,true));
        fprintf(filestb,"INSERT INTO `item_reference` VALUES ('%u', '%u', '%u', '%s', '%s');\r",0,11,j,name.c_str(),comment.c_str());
    }


    //Natural.
    for (int j=0;j<NaturalList.max;j++)
    {
        string name=EscapeMe(GetSTLObjNameByID(12,j));
        string comment=EscapeMe(GetSTLObjNameByID(12,j,true));
        fprintf(filestb,"INSERT INTO `item_reference` VALUES ('%u', '%u', '%u', '%s', '%s');\r",0,12,j,name.c_str(),comment.c_str());
    }

    //Quest items.
    for (int j=0;j<maxQuestItems;j++)
    {
        string name=EscapeMe(GetSTLObjNameByID(13,j));
        string comment=EscapeMe(GetSTLObjNameByID(13,j,true));
        fprintf(filestb,"INSERT INTO `item_reference` VALUES ('%u', '%u', '%u', '%s', '%s');\r",0,13,j,name.c_str(),comment.c_str());
    }

    //PatList.
    for (int j=0;j<JemList.max;j++)
    {
        string name=EscapeMe(GetSTLObjNameByID(14,j));
        string comment=EscapeMe(GetSTLObjNameByID(14,j,true));
        fprintf(filestb,"INSERT INTO `item_reference` VALUES ('%u', '%u', '%u', '%s', '%s');\r",0,14,j,name.c_str(),comment.c_str());
    }


    fclose(filestb);

    return true;
}


//LMA: returning spawn list around a player.
bool CWorldServer::pakGMSpawnList(CPlayer* thisclient, int range)
{
    UINT map=thisclient->Position->Map;
    UINT nb_found=0;

    if (map>maxZone)
    {
        return true;
    }

    //SendPM(thisclient,"Begin of spawnlist in map %u",map);
    Log(MSG_INFO,"Begin of spawnlist in map %u",map);

    for (unsigned j = 0; j < MapList.Index[map]->MobGroupList.size(); j++)
    {
        CMobGroup* mygroup = MapList.Index[map]->MobGroupList.at(j);
        if(distance(thisclient->Position->current,mygroup->point)>range)
        {
            continue;
        }

        //SendPM(thisclient,"Spawn %u found at (%.2f,%.2f), dist: %.2f",mygroup->id,mygroup->point.x,mygroup->point.y);
        Log(MSG_INFO,"Spawn %u found at (%.2f,%.2f), dist: %.2f",mygroup->id,mygroup->point.x,mygroup->point.y,distance(thisclient->Position->current,mygroup->point));
    }

    //SendPM(thisclient,"End of spawnlist");
    Log(MSG_INFO,"End of spawnlist");


    return true;
}


//LMA: returning a detail of a spawn list.
bool CWorldServer::pakGMSpawnDetail(CPlayer* thisclient, UINT id, UINT map)
{
    if (map>maxZone)
    {
        Log(MSG_INFO,"Map %u does not exist",map);
        return true;
    }

    CMobGroup* thisgroup = GetMobGroup(id,map);
    if (thisgroup==NULL)
    {
        Log(MSG_INFO,"The spawn %u doesn't exist in map %u",id,map);
        return true;
    }

    //Getting last group.
    int last_group=thisgroup->curBasic-1;
    if(last_group<0)
    {
        last_group=thisgroup->basicMobs.size()-1;
        if(last_group<0)
        {
            last_group=0;
        }

    }

    //SendPM(thisclient,"Current spawn information for group %u in map %u:", id,map);
    Log(MSG_INFO,"Current spawn information for group %u in map %u:", id,map);

    //SendPM(thisclient,"-> Current group=%u/%u", last_group,thisgroup->basicMobs.size()-1);
    //SendPM(thisclient,"-> basic deads %u/ %u, is ready? %i",thisgroup->lastKills,thisgroup->basicMobs.at(last_group)->real_amount,thisgroup->group_ready);

    Log(MSG_INFO,"-> Current group=%u/%u", last_group,thisgroup->basicMobs.size()-1);
    Log(MSG_INFO,"-> basic deads %u/ %u, is ready? %i",thisgroup->lastKills,thisgroup->basicMobs.at(last_group)->real_amount,thisgroup->group_ready);

    //Getting the map.
    CMap* mymap= GServer->MapList.Index[map];

    //searching now where are the monsters using this spawnid.
    //doing a mutext to be sure we're alone touching this one.
    pthread_mutex_lock( &mymap->MonsterMutex );
    for(UINT j = 0; j < mymap->MonsterList.size(); j++)
    {
        CMonster* thismon = mymap->MonsterList.at(j);
        if(thismon==NULL)
        {
            continue;
        }

        if (thismon->Position->respawn==id)
        {
            //SendPM(thisclient,"--> Monster %u, type %u is at (%.2f,%.2f), HP %I64i/%I64i, tactical? %i",thismon->clientid,thismon->montype,thismon->Position->current.x,thismon->Position->current.y,thismon->Stats->HP,thismon->Stats->MaxHP,thismon->is_tactical);
            Log(MSG_INFO,"--> Monster %u, type %u is at (%.2f,%.2f), HP %I64i/%I64i, tactical? %i",thismon->clientid,thismon->montype,thismon->Position->current.x,thismon->Position->current.y,thismon->Stats->HP,thismon->Stats->MaxHP,thismon->is_tactical);
        }

    }

    pthread_mutex_unlock( &mymap->MonsterMutex );
    //SendPM(thisclient,"End of monster list");
    Log(MSG_INFO,"End of monster list");


    return true;
}

//LMA: We force the refresh of a group (will load the next one).
//We just kill the monsters from the group and the next spawn should come alone.
bool CWorldServer::pakGMSpawnForceRefresh(CPlayer* thisclient, UINT id, UINT map)
{
    if (map>maxZone)
    {
        Log(MSG_INFO,"Map %u does not exist",map);
        return true;
    }

    CMobGroup* thisgroup = GetMobGroup(id,map);
    if (thisgroup==NULL)
    {
        Log(MSG_INFO,"The spawn %u doesn't exist in map %u",id,map);
        return true;
    }

    //Getting last group.
    int last_group=thisgroup->curBasic-1;
    if(last_group<0)
    {
        last_group=thisgroup->basicMobs.size()-1;
        if(last_group<0)
        {
            last_group=0;
        }

    }

    //Getting the map.
    CMap* mymap= GServer->MapList.Index[map];

    //searching now where are the monsters using this spawnid.
    //doing a mutext to be sure we're alone touching this one.
    vector<UINT> monstertokill;
    pthread_mutex_lock( &mymap->MonsterMutex );
    for(UINT j = 0; j < mymap->MonsterList.size(); j++)
    {
        CMonster* thismon = mymap->MonsterList.at(j);
        if(thismon==NULL)
        {
            continue;
        }

        if (thismon->Position->respawn==id)
        {
            monstertokill.push_back(thismon->clientid);
        }

    }

    pthread_mutex_unlock( &mymap->MonsterMutex );

    //SendPM(thisclient,"Trying to refresh spawn group %u in map %u (Nb to kill %u):", id,map,monstertokill.size());
    Log(MSG_INFO,"Trying to refresh spawn group %u in map %u (Nb to kill %u):", id,map,monstertokill.size());

    //We need to kill them.
    for(UINT j=0;j<monstertokill.size();j++)
    {
        CMonster* thismon=GetMonsterByID(monstertokill.at(j),map);
        if(thismon==NULL)
        {
            Log(MSG_INFO,"Can't find monster %u for delete.",monstertokill.at(j));
            continue;
        }

        //SendPM(thisclient,"--> Killing Monster %u, type %u is at (%.2f,%.2f), HP %I64i/%I64i, tactical? %i",thismon->clientid,thismon->montype,thismon->Position->current.x,thismon->Position->current.y,thismon->Stats->HP,thismon->Stats->MaxHP,thismon->is_tactical);
        Log(MSG_INFO,"--> Killing Monster %u, type %u is at (%.2f,%.2f), HP %I64i/%I64i, tactical? %i",thismon->clientid,thismon->montype,thismon->Position->current.x,thismon->Position->current.y,thismon->Stats->HP,thismon->Stats->MaxHP,thismon->is_tactical);
        //suicide time.
        thismon->Stats->HP = -1;
        BEGINPACKET( pak, 0x799 );
        ADDWORD    ( pak, thismon->clientid );
        ADDWORD    ( pak, thismon->clientid );
        ADDDWORD   ( pak, thismon->thisnpc->hp*thismon->thisnpc->level );
        ADDDWORD   ( pak, 16 );
        SendToVisible( &pak, thismon );
        mymap->DeleteMonster( thismon );
    }

    //SendPM(thisclient,"End of Force refresh, the spawn should refresh on his own.");
    Log(MSG_INFO,"End of Force refresh, the spawn should refresh on his own.");


    return true;
}

bool CWorldServer::pakGMWarning(CPlayer *thisclient, char* name, char* reason)
{
    CPlayer* otherclient = GetClientByCharName( name );

	if (!otherclient){

	    SendPM(thisclient, "The player is currently not online or does not exist");
	    return true;

	}

    MYSQL_ROW row;
    MYSQL_RES *result = GServer->DB->QStore("SELECT warning_no FROM warning_system WHERE account_id = '%i'",otherclient->Session->userid);
    row = mysql_fetch_row(result);

    if(row == NULL){

        DB->QFree();
        DB->QExecute("INSERT INTO warning_system VALUES ('%i','1','0','0')", otherclient->Session->userid);
        DB->QExecute("INSERT INTO warning_reason VALUES ('%i','1','%s','%s')", otherclient->Session->userid, reason, thisclient->CharInfo->charname);
        SendPM(otherclient,"You have received a warning from a GM for misconduct. Be advised that after 3 warnings you will receive a 2 day ban.");

        return true;

    }else{

        int warning = atoi(row[0]);

        if(warning <= 3){

            warning ++;

            if(warning == 4){

                DB->QFree();
                DB->QExecute( "UPDATE warning_system SET ban_type = '1', time_remaining = '0' WHERE account_id=%i", otherclient->Session->userid);
                DB->QExecute("INSERT INTO warning_reason VALUES ('%i','%i','%s','%s)", otherclient->Session->userid, warning, reason, thisclient->CharInfo->charname);

                if(otherclient==NULL)
                    return true;

                otherclient->Session->accesslevel = -1;
                DB->QExecute( "UPDATE accounts SET accesslevel='0' WHERE id=%i", otherclient->Session->userid);

                BEGINPACKET( pak, 0x702 );
                ADDSTRING( pak, "You were banned from the server for 48 hours!" );
                ADDBYTE( pak, 0 );
                otherclient->client->SendPacket( &pak );

                RESETPACKET( pak, 0x707 );
                ADDWORD( pak, 0 );
                otherclient->client->SendPacket( &pak );
                otherclient->client->isActive = false;

                return true;
            }

            DB->QFree();
            DB->QExecute( "UPDATE warning_system SET warning_no = '%i' WHERE account_id=%i", warning, otherclient->Session->userid);
            DB->QExecute("INSERT INTO warning_reason VALUES ('%i','%i','%s','%s)", otherclient->Session->userid, warning, reason, thisclient->CharInfo->charname);
            SendPM(otherclient,"You have received a warning from a GM for misconduct. Be advised that after 3 warnings you will receive a 2 day ban.");

            return true;

        }else if(warning <= 6){

            warning ++;

            if(warning == 7){

                DB->QFree();
                DB->QExecute( "UPDATE warning_system SET ban_type = '2', time_remaining = '0' WHERE account_id=%i", otherclient->Session->userid);
                DB->QExecute("INSERT INTO warning_reason VALUES ('%i','%i','%s','%s)", otherclient->Session->userid, warning, reason, thisclient->CharInfo->charname);

                if(otherclient==NULL)
                    return true;

                otherclient->Session->accesslevel = -1;
                DB->QExecute( "UPDATE accounts SET accesslevel='0' WHERE id=%i", otherclient->Session->userid);

                BEGINPACKET( pak, 0x702 );
                ADDSTRING( pak, "You were banned from the server for 7 days!" );
                ADDBYTE( pak, 0 );
                otherclient->client->SendPacket( &pak );

                RESETPACKET( pak, 0x707 );
                ADDWORD( pak, 0 );
                otherclient->client->SendPacket( &pak );
                otherclient->client->isActive = false;

                return true;
            }

            DB->QFree();
            DB->QExecute( "UPDATE warning_system SET warning_no = '%i' WHERE account_id=%i", warning, otherclient->Session->userid);
            DB->QExecute("INSERT INTO warning_reason VALUES ('%i','%i','%s','%s)", otherclient->Session->userid, warning, reason, thisclient->CharInfo->charname);
            SendPM(otherclient,"You have received a warning from a GM for misconduct. Be advised that after 6 warnings you will receive a 7 day ban.");

            return true;

        }else if(warning <= 9){

            warning ++;

            if(warning == 10){

                DB->QFree();
                DB->QExecute("INSERT INTO warning_reason VALUES ('%i','%i','%s')", otherclient->Session->userid, warning, reason);

                if(otherclient==NULL)
                    return true;

                otherclient->Session->accesslevel = -1;
                DB->QExecute( "UPDATE accounts SET accesslevel='0' WHERE account_id=%i", otherclient->Session->userid);
                DB->QExecute("DELETE FROM warning_system WHERE account_id = '%i'", otherclient->Session->userid);

                BEGINPACKET( pak, 0x702 );
                ADDSTRING( pak, "You were permanently banned from the server!" );
                ADDBYTE( pak, 0 );
                otherclient->client->SendPacket( &pak );

                RESETPACKET( pak, 0x707 );
                ADDWORD( pak, 0 );
                otherclient->client->SendPacket( &pak );
                otherclient->client->isActive = false;

                return true;

            }

            DB->QFree();
            DB->QExecute( "UPDATE warning_system SET warning_no = '%i' WHERE account_id=%i", warning, otherclient->Session->userid);
            DB->QExecute("INSERT INTO warning_reason VALUES ('%i','%i','%s','%s)", otherclient->Session->userid, warning, reason, thisclient->CharInfo->charname);
            SendPM(otherclient,"You have received a warning from a GM for misconduct. Be advised that after 9 warnings you will receive a permanent ban.");
        }
    }
}

bool CWorldServer::pakWarningCount(CPlayer *thisclient)
{
    MYSQL_ROW row;
    MYSQL_RES *result = GServer->DB->QStore("SELECT COUNT(account_id) AS WCount FROM warning_reason WHERE account_id = '%i'",thisclient->Session->userid);
    row = mysql_fetch_row(result);

    if(row == NULL){
        Log(MSG_INFO, "There were no warnings for account id: %i");
        SendPM(thisclient,"You have no warnings on this account");
        GServer->DB->QFree();
        return true;
    }

    SendPM(thisclient,"You have %i warnings already", atoi(row[0]));

    GServer->DB->QFree();

    return true;

}

bool CWorldServer::pakWarningReason(CPlayer *thisclient){

    MYSQL_ROW row;
    MYSQL_RES *result = GServer->DB->QStore("SELECT warning_no, reason, issuer FROM warning_reason WHERE account_id = '%i'",thisclient->Session->userid);

    SendPM(thisclient, "The warnings issued to this account are as follows:");
    SendPM(thisclient, "Warning No | Reason | Issuer:");
    while(row = mysql_fetch_row(result)) SendPM(thisclient, "%i | %s | %s", row[0], row[1], row[2]);
    GServer->DB->QFree();

    if(row == NULL){
        Log(MSG_INFO,"There are no warnings for account: %i", thisclient->Session->userid);
        SendPM(thisclient, "You do not have any warnings on this account.");
        GServer->DB->QFree();
        return true;
    }

    SendPM(thisclient, "If there are any issues with the warnings please contact the GM team and try to rectify the issue");

    return true;

}

/*bool CWorldServer::pakGMSeeWarning(CPlayer *thisclient, char *name)
{
    MYSQL_ROW row, row1,row2;
    MYSQL_RES *result = GServer->DB->QStore("SELECT account_name FROM characters WHERE char_name = '%s'", name);
    row = mysql_fetch_row(result);
    GServer->DB->QFree();

    if(row == NULL){
        SendPM(thisclient, "The player does not exist");
        return true;
    }

    char *account = row[0];

    MYSQL_RES *result1 = GServer->DB->QStore("SELECT id FROM accounts WHERE username ='%s'", account);
    row1 = mysql_fetch_row(result1);
    GServer->DB->QFree();

    int account_id = atoi(row1[0]);

    MYSQL_RES *result2 = GServer->DB->QStore("SELECT warning_no, reason, issuer FROM warning_reason WHERE account_id = %i", account_id);
    SendPM(thisclient, "Warning No | Reason | Issuer:");
    while(row2 = mysql_fetch_row(result2)){
        SendPM(thisclient, "%i | %s | %s",atoi(row[0]),row[1],row[2]);
    }

    if(row2 == NULL){
        SendPM(thisclient, "The player's account does not have any warnings");
        GServer->DB->QFree();
        return true;
    }
    GServer->DB->QFree();
    return true;

}

void CWorldServer::pakGMDelWarning(CPlayer *thisclient, char *name, int warning)
{
    MYSQL_ROW AccountName, AccountID, WarningCount;
    MYSQL_RES *AccountNameR, *AccountIDR, *WarningCountR;
    int warning1, test_warning;

    AccountNameR = GServer->DB->QStore("SELECT account_name FROM characters WHERE char_name = '%s'", name);
    AccountName = mysql_fetch_row(AccountNameR);
    GServer->DB->QFree();

    if(AccountName == NULL){
        SendPM(thisclient, "Make sure you typed the players name correctly");
        return;
    }

    AccountIDR = GServer->DB->QStore("SELECT id FROM accounts WHERE username = '%s'", AccountName[0]);
    AccountID = mysql_fetch_row(AccountIDR);
    GServer->DB->QFree();

    if(AccountID == NULL){
        return;
    }

    Log(MSG_INFO, "Account id: %i, Warning no: %i", atoi(AccountID[0]), warning);

    GServer->DB->QExecute("DELETE FROM warning_reason WHERE account_id = '%i' AND warning_no = '%i'", atoi(AccountID[0]), warning);
    GServer->DB->QExecute("UPDATE warning_system SET warning_no= warning_no-1 WHERE account_id = '%i'",atoi(AccountID[0]));

    WarningCountR = GServer->DB->QStore("SELECT warning_no FROM warning_reason WHERE account_id = '%i'", atoi(AccountID[0]));

    test_warning = 1;

    while(WarningCount=mysql_fetch_row(WarningCountR)){

        warning1 = atoi(WarningCount[0]);

        if(warning1 != test_warning){
            GServer->DB->QFree();
            GServer->DB->QExecute("UPDATE warning_reason SET warning_no = '%i' WHERE account_id ='%i' AND warning_no = '%i'",test_warning, atoi(AccountID[0]), warning1);
            Log(MSG_INFO, "warning1: %i test_warning: %i", warning1, test_warning);
        }
        test_warning++;
        Log(MSG_INFO, "warning1: %i test_warning: %i", warning1, test_warning);
    }

    if(WarningCount[0] == NULL){
        Log(MSG_INFO,"There are no warnings for the player");
        SendPM(thisclient,"There are no warnings for the player");
        GServer->DB->QFree();
    }

    GServer->DB->QFree();
    SendPM(thisclient,"The warning has been successfully deleted");
    Log(MSG_GMACTION, "GM deleted warning for account %i", atoi(AccountID[0]));

}*/

bool CWorldServer::voteShopInfo(CPlayer* thisclient, int type)
{
    #ifndef VSHOPINFO
    #define VSHOPINFO
    MYSQL_RES *itemresult;
    MYSQL_ROW VPitems;
    #endif

    int vp = GetVotePoints(thisclient);
    bool ItemsAvailble = false;

    itemresult = GServer->DB->QStore("SELECT id, name, amount, vp FROM voteshop WHERE itemType = %i AND vp <= %i", type,vp);
    GServer->DB->QFree();

    SendPM(thisclient, "The following items can be bought with your current %i VP:", vp);
    while(VPitems = mysql_fetch_row(itemresult))
    {
        SendPM(thisclient, "ID: %i - Name: %s - Amount: %i - Price: %i",atoi(VPitems[0]),VPitems[1],atoi(VPitems[2]),atoi(VPitems[3]));
        ItemsAvailble = true;
    }

    if(ItemsAvailble == false)
    {
        SendPM(thisclient,"No items availble of this type!");
    }else
    {
        SendPM(thisclient, "Use /vbuy [id] to buy an item using Vote Points");
    }
    Log(MSG_GMACTION," %s : /vinfo %i",thisclient->CharInfo->charname, type);

    return true;
}

bool CWorldServer::voteShopBuy(CPlayer* thisclient, int number)
{
    #ifndef VOTEBUY
    #define VOTEBUY
    MYSQL_RES *RVPBuy;
    MYSQL_ROW VPBuy;
    #endif

    int itemID, itemType, itemDura, vp, PlayerVp;
    long long int itemAmount;
    char name[100];

    RVPBuy = GServer->DB->QStore("SELECT name, itemID, itemType, durability, amount, vp FROM voteshop WHERE id = %i", number);
    if(VPBuy = mysql_fetch_row(RVPBuy))
    {

    }else
    {
        SendPM(thisclient, "Invalid ID, please type /vinfo for more details");
        GServer->DB->QFree();
        return true;
    }
    GServer->DB->QFree();

    sprintf( name,"You bought %s from Vote Shop!" ,VPBuy[0]);
    itemID = atoi(VPBuy[1]);
    itemType = atoi(VPBuy[2]);
    itemDura = atoi(VPBuy[3]);
    itemAmount = atoi(VPBuy[4]);
    vp = atoi(VPBuy[5]);
    PlayerVp = GetVotePoints(thisclient);

    if( PlayerVp < vp)
    {
        SendPM(thisclient, "You need atleast %i VP to buy this item, you only have %i VP",vp, PlayerVp);
        return true;
    }

    if(itemType == 0 || itemID == 0)
    {
        thisclient->CharInfo->Zulies = thisclient->CharInfo->Zulies + itemAmount;
        BEGINPACKET( pak, 0x71d );
        ADDQWORD( pak, thisclient->CharInfo->Zulies );
        thisclient->client->SendPacket( &pak );

        GServer->DB->QExecute("UPDATE accounts SET vote_points = vote_points - %i WHERE id = %i", vp, thisclient->Session->userid);
        Log(MSG_GMACTION," %s : /vbuy %i",thisclient->CharInfo->charname,number);

        return true;
    }

    CItem item;
    item.count            = itemAmount;
    item.durability        = itemDura;
    item.itemnum        = itemID;
    item.itemtype        = itemType;
    item.lifespan        = 100;  // changed from itemls to 100, no need for lifespan on a item that is spawned - rl2171
    item.refine            = 0;
    item.stats            = 0;
    item.socketed        = 0;
    item.appraised        = 1;
    item.gem = 0;
    item.sp_value=0;
    item.last_sp_value=0;
    unsigned newslot = thisclient->GetNewItemSlot( item );

    if (newslot != 0xffff)
    {

        if (item.itemtype>=10&&item.itemtype<=12)
        {
            item.count+=thisclient->items[newslot].count;
        }

        thisclient->items[newslot] = item;
        thisclient->UpdateInventory( newslot );
        BEGINPACKET ( pak, 0x702 );
        ADDSTRING( pak, name);
        ADDBYTE( pak, 0 );
        thisclient->client->SendPacket( &pak );

        GServer->DB->QExecute("UPDATE accounts SET vote_points = vote_points - %i WHERE id = %i", vp, thisclient->Session->userid);
        Log(MSG_GMACTION," %s : /vbuy %i",thisclient->CharInfo->charname ,number);

    }else{
        BEGINPACKET( pak, 0x7a7 );
        ADDWORD( pak, thisclient->clientid );
        ADDBYTE( pak, 5 );
        thisclient->client->SendPacket( &pak );

        RESETPACKET ( pak, 0x702 );
        ADDSTRING( pak, "No free slot !" );
        ADDBYTE( pak, 0 );
        thisclient->client->SendPacket( &pak );
    }
    return true;

}

int CWorldServer::GetVotePoints(CPlayer* thisclient)
{
    #ifndef VOTEPOINTS
    #define VOTEPOINTS
    MYSQL_RES *rvotepoints;
    MYSQL_ROW votepoints;
    #endif

    rvotepoints = GServer->DB->QStore("SELECT vote_points FROM accounts WHERE id = %i", thisclient->Session->userid);
    GServer->DB->QFree();

    votepoints = mysql_fetch_row(rvotepoints);

    return atoi(votepoints[0]);
}
