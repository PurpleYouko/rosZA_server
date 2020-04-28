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

// Props to ExJam for this code :D Ported to OSpRose by PY
#include "../worldserver.h"

//Check Quest
QUESTCOND(000)
{
	GETCONDDATA(000);

	if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(000):: Check Quest %i", data->iQuestSN);
	for(dword i = 0; i < 10; i++)
	{
		if(client->quest.quests[i].QuestID == data->iQuestSN)
		{
            client->CheckQuest = i;
            //LMA: We select this quest, else problems on getquestvars.
            client->ActiveQuest=data->iQuestSN;
            if( client->questdebug )
                Log(MSG_QUESTDEBUG, "QUEST(000):: Quest found");
            return QUEST_SUCCESS;
        }
	}
	if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(000):: Quest NOT found");

	return QUEST_FAILURE;
}

//Check Quest Variable
QUESTCOND(001)
{
    GETCONDDATA(001);
	int retVal = 1;
	word nValue;
	word tempValue;
	for(int i = 0; i < data->iDataCnt; i++)
	{
		dword address = i * sizeof(STR_QUEST_DATA);
		address += (dword)data;
		address += 4;
		STR_QUEST_DATA* curQst = (STR_QUEST_DATA*)address;
        nValue = curQst->nValue;
		tempValue = client->GetQuestVar(curQst->m_wVarTYPE, curQst->m_wVarNO);
		retVal = OperateValues<word>(curQst->btOp, &tempValue, curQst->nValue);
        if( client->questdebug )
            Log(MSG_QUESTDEBUG, "QUESTCOND(001):: Check Quest Var. [%i][%i]needed %i found %i)",curQst->m_wVarTYPE,curQst->m_wVarNO,curQst->nValue,tempValue);
        //if(retVal == 0) return QUEST_FAILURE;
    }
	 if( client->questdebug )
	{
        if(retVal == 0)
        {
            Log(MSG_QUESTDEBUG, "QUESTCOND(001):: returned FAIL");
        }
        else
        {
            Log(MSG_QUESTDEBUG, "v(001):: returned SUCCESS");
        }
	}
	return (retVal == 0)?QUEST_FAILURE:QUEST_SUCCESS;
}

//Check Quest Variable
QUESTCOND(002)
{
	if( client->questdebug )
          Log(MSG_QUESTDEBUG, "QUESTCOND(002):: Running QUESTCOND(001) because that's what QUEST(002) does");
	return QUEST_COND_001(server, client, raw);
}

//Check Stats
QUESTCOND(003)
{
	GETCONDDATA(003);
	for(int i = 0; i < data->iDataCnt; i++)
	{
		dword address = i * 0x0C;
		address += (dword)data;
		address += 4;
		STR_ABIL_DATA* curAbil = (STR_ABIL_DATA*)address;
        if( client->questdebug )
          server->SendPM(client, "Check Stat[%i] - %i (Op: %i)", curAbil->iType, curAbil->iValue, curAbil->btOp);
		switch( curAbil->iType )
		{
		    case sPvp:
		    {
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->pvp_id, curAbil->iValue))
                    return QUEST_FAILURE;
                    if( client->questdebug )
                        Log(MSG_QUESTDEBUG, "QUESTCOND(003):: checking pvp %i %i",curAbil->btOp,curAbil->iValue);
		    }
		    break;
            case sGender:
            {
                //LMA: We do this because Sex is a BYTE and if we do a (int*) the cast is wrong.
                int temp_sex = client->CharInfo->Sex;
                if( client->questdebug )
                    Log(MSG_QUESTDEBUG, "QUESTCOND(003):: checking gender");
                if(!OperateValues<int>(curAbil->btOp, (int*)&temp_sex, curAbil->iValue))
                {
                    return QUEST_FAILURE;
                }

            }
            break;
            case sJob:
            {
                word tempValue = client->CharInfo->Job / 100;
                if( client->questdebug )
                    Log(MSG_QUESTDEBUG, "QUESTCOND(003):: checking job");
                if(!OperateValues<word>(curAbil->btOp, (word*)&tempValue, curAbil->iValue))
                    return QUEST_FAILURE;
            }
            break;
            case sUnion:
            {
                //Log(MSG_INFO,"[Union] ? QUESTCOND(003) btOp %i, value: %i",curAbil->btOp,curAbil->iValue);
                if( client->questdebug )
                    Log(MSG_QUESTDEBUG, "QUESTCOND(003):: checking Union");
                if(!OperateValues<int>(curAbil->btOp, (int*)&client->CharInfo->unionid, curAbil->iValue))
                return QUEST_FAILURE;
                //Log(MSG_INFO,"[Union] OK QUESTCOND(003) GET union %i",curAbil->iValue);
            }
            break;
            case 81:    //LMA: Union Points (no break, it's NOT a mistake)...
            {
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->CharInfo->union01, curAbil->iValue))
                        return QUEST_FAILURE;
            }
            break;
            case 82:
            {
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->CharInfo->union02, curAbil->iValue))
                        return QUEST_FAILURE;
            }
            break;
            case 83:
            {
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->CharInfo->union03, curAbil->iValue))
                        return QUEST_FAILURE;
            }
            break;
            case 84:
            {
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->CharInfo->union04, curAbil->iValue))
                        return QUEST_FAILURE;
            }
            break;
            case 85:
            {
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->CharInfo->union05, curAbil->iValue))
                        return QUEST_FAILURE;
            }
            break;
            case sStrength:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->Attr->Str, curAbil->iValue))
                    return QUEST_FAILURE;
                break;

            case sDexterity:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->Attr->Dex, curAbil->iValue))
                    return QUEST_FAILURE;
                break;

            case sIntelligence:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->Attr->Int, curAbil->iValue))
                    return QUEST_FAILURE;
                break;

            case sConcentration:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->Attr->Con, curAbil->iValue))
                    return QUEST_FAILURE;
                break;

            case sCharm:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->Attr->Cha, curAbil->iValue))
                    return QUEST_FAILURE;
                break;

            case sSensibility:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->Attr->Sen, curAbil->iValue))
                    return QUEST_FAILURE;
                break;

            case sLevel:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->Stats->Level, curAbil->iValue))
                    return QUEST_FAILURE;
                break;

            case sStatPoints:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->CharInfo->StatPoints, curAbil->iValue))
                    return QUEST_FAILURE;
                break;
            case sMoney:
                    if(!OperateValues<int>(curAbil->btOp, (int*)&client->CharInfo->Zulies, curAbil->iValue))
                    return QUEST_FAILURE;
                break;
            case sEXPRate:
                if(!OperateValues<int>(curAbil->btOp, (int*)&client->bonusxp, curAbil->iValue))
                            return QUEST_FAILURE;
            break;
            default:
                if( client->questdebug )
                    Log(MSG_QUESTDEBUG, "QUESTCOND(003):: Type Unknown: '%i'", curAbil->iType);
            break;
		}
	}
	return QUEST_SUCCESS;
}

//Check Items
QUESTCOND(004)
{
	GETCONDDATA(004);
	for(int i = 0; i < data->iDataCnt; i++)
	{
		dword address = i * 0x10;
		address += (dword)data;
		address += 4;
		STR_ITEM_DATA* curItem = (STR_ITEM_DATA*)address;

        if( client->questdebug )
          Log(MSG_QUESTDEBUG, "QUESTCOND(004):: Check Item - [Slot:%i][Type:%i]", curItem->iWhere, curItem->uiItemSN);

        // Check quest items for a specific amount
        int ch_itemtype=GServer->gi(curItem->uiItemSN,0);
        int ch_itemnum=GServer->gi(curItem->uiItemSN,1);

        //Log(MSG_INFO,"QSD CDT4:: (%s) Checking item (%u::%u)",client->CharInfo->charname,ch_itemtype,ch_itemnum);

        //LMA: where not reliable.
		//if (curItem->iWhere==13)
		if(ch_itemtype==13)
		{
          // Ok, check the current Check'd quest.
          if (client->CheckQuest < 0) return QUEST_SUCCESS;
          unsigned int j = client->CheckQuest;
          for (unsigned int k = 0; k < 5; k++)
          {
              //LMA: Correct way.
              //if (client->quest.quests[j].Items[k].itemnum == (curItem->uiItemSN % 1000))
              if (client->quest.quests[j].Items[k].itemnum == (GServer->gi(curItem->uiItemSN,1)))
              {
                if( client->questdebug )
                  Log(MSG_QUESTDEBUG, "QUESTCOND(004):: Operate - btOp: %i Val1: %i Val2: %i", curItem->btOp, client->quest.quests[j].Items[k].count, curItem->iRequestCnt);

                if(!OperateValues<int>(curItem->btOp, (int*)(&client->quest.quests[j].Items[k].count), curItem->iRequestCnt))
                {
                    return QUEST_FAILURE;
                }
                 else
                 {
                    return QUEST_SUCCESS;
                 }

               }

          }

          // Since the item doesn't exist, treat it as if there's 0 of them.
          int tempInt = 0;
          if(!OperateValues<int>(curItem->btOp, &tempInt, curItem->iRequestCnt))
          {
            return QUEST_FAILURE;
          }
          else
          {
            return QUEST_SUCCESS;
          }

        }
        else if(ch_itemtype!=0&&ch_itemtype!=13&&curItem->uiItemSN!=0)
        {
            //else if(curItem->iWhere!=0&&curItem->iWhere!=13&&curItem->uiItemSN!=0)

            //LMA: We check also some other items...
            //curItem->iWhere is actually the itemtype...
            //int ch_itemtype=GServer->gi(curItem->uiItemSN,0);
            //int ch_itemnum=GServer->gi(curItem->uiItemSN,1);
            int ch_nb_items=0;
            bool player_has_item=false;
            for (int k=0;k<140;k++)
            {
                if(client->items[k].itemtype!=ch_itemtype||client->items[k].itemnum!=ch_itemnum)
                {
                    continue;
                }

                player_has_item=true;

                ch_nb_items=client->items[k].count;
                if(OperateValues<int>(curItem->btOp, &ch_nb_items, curItem->iRequestCnt))
                {
                    return QUEST_SUCCESS;
                }

            }

            //LMA: Special case where they test if the player has less than "x" items.
            //It can be where the player does NOT have any...
            if(!player_has_item&&curItem->btOp==3)
            {
                return QUEST_SUCCESS;
            }


            return QUEST_FAILURE;
        }

        // Check equipped items for a specific item? - Drakia
        //LMA: usually it's to test if a player has something equipped or not (reset skill and so on).
		if(curItem->uiItemSN == 0 && curItem->iWhere != 0)
		{
			int itemHead = client->items[curItem->iWhere].itemtype;
			if(!OperateValues<int>(curItem->btOp, &itemHead, curItem->uiItemSN))
				return QUEST_FAILURE;
		}

	}


	return QUEST_SUCCESS;
}

//Check Party
//QUESTCOND(005)
//{
//	return QUEST_SUCCESS;
//}

//Check Party
QUESTCOND(005)
{
	//byte btIsLeader    pos 0x00
	//int iLevel         pos 0x04
	//byte btReversed    pos 0x08

	GETCONDDATA(005);

	if( client->questdebug )
          Log(MSG_QUESTDEBUG, "QUESTCOND(005):: Check Party");
	if(client->Party->party == NULL)	//client not in a party
        return QUEST_FAILURE;
	if(data->btReversed)
	{
		if(data->btIsLeader)										//isleader set to true
		{
			if(!client->Party->IsMaster)							//player is NOT the party leader
				return QUEST_SUCCESS;								// success as btreversed is set
		}
		else														//isleader NOT set so we don't care if player is leader or not
		{
			if(client->Party->party->PartyLevel < data->iLevel)		//party level is less than iLevel
				return QUEST_SUCCESS;								//Success
		}
	}
	else															//btreversed NOT set
	{
		if(data->btIsLeader)										//isleader set to true
		{
			if(client->Party->IsMaster)							    //player is the party leader
				return QUEST_SUCCESS;								//success
		}
		else														//isleader NOT set so we don't care if player is leader or not
		{
			if(client->Party->party->PartyLevel >= data->iLevel)	//party level is >= iLevel
				return QUEST_SUCCESS;								//Success
		}
	}
    return QUEST_FAILURE;											//NOPE. Utterly failed this time
}

//Near Point
QUESTCOND(006)
{
	GETCONDDATA(006);
	client->UpdatePosition(false);
	if( client->questdebug )
          Log(MSG_QUESTDEBUG, "QUESTCOND(006):: Check near point");
	if(data->iZoneSN != 0){
		if(client->Position->Map != data->iZoneSN)
			return QUEST_FAILURE;
	}

    //float dx = client->Position->current.x - (float)data->iX;
    //float dy = client->Position->current.y - (float)data->iY;
    float dx = client->Position->current.x - (float)(data->iX / 100);
    float dy = client->Position->current.y - (float)(data->iY / 100);
	float distance = sqrt((dx*dx) + (dy*dy));
	return (distance < data->iRadius)?QUEST_SUCCESS:QUEST_FAILURE;
}

//World Time
QUESTCOND(007)
{
    if( client->questdebug )
          Log(MSG_QUESTDEBUG, "QUESTCOND(007):: Check World Time");
    return QUEST_SUCCESS;
}

//Quest Time
QUESTCOND(008)
{
    GETCONDDATA(008);
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(008):: QuestTime - ulTime: %i btOp: %i", data->ulTime, data->btOp);
    if (client->CheckQuest < 0)
    {
        LogDebug("QUESTCOND::No quest to check");
        return QUEST_FAILURE; // Not checking a quest, don't return success
    }

    long int Time = 0;
    if (server->STB_QUEST.rows[client->quest.quests[client->CheckQuest].QuestID][1] > 0)
    {
        Time += client->quest.quests[client->CheckQuest].StartTime; // Start time
        LogDebug("QUESTCOND::start time %li",Time);
        Time += server->STB_QUEST.rows[client->quest.quests[client->CheckQuest].QuestID][1] * 10; // Time to finish
        LogDebug("QUESTCOND::with time to finish %li",Time);
        Time -= time(NULL); // Current time
        if (Time < 0) Time = 0; // Time is up
        Time /= 10; // Divide to get 10's of seconds
        LogDebug("QUESTCOND::quest time %li",Time);
    }

    if (!OperateValues<dword>(data->btOp, (dword*)&Time, data->ulTime))
        return QUEST_FAILURE;
    else
        return QUEST_SUCCESS;
}

//Check Skill
QUESTCOND(009)
{
    GETCONDDATA(009);
	// Line not needed, never used - Drakia
	//int checkVal = (data->btOp)?QUEST_SUCCESS:QUEST_FAILURE;
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(009):: Check Skill: %i", data->iSkillSN1);

	for (dword i = 0; i < MAX_ALL_SKILL; i++) {
        if(client->cskills[i].id == data->iSkillSN1)
        	return (data->btOp)?QUEST_SUCCESS:QUEST_FAILURE;
	}
	return (data->btOp)?QUEST_FAILURE:QUEST_SUCCESS;
}

//percentage success chance
QUESTCOND(010)
{
    //PY random chance check for quest drops. generates a random percentage. if between btLowPcnt and btHighPcnt then return success
    GETCONDDATA(010);
    //byte btLowPcnt;        pos 0x00
	//byte btHighPcnt;       pos 0x01
	UINT thischance = GServer->RandNumber(0,100);
	if(thischance >= data->btLowPcnt and thischance <= data->btHighPcnt)
	{
	    if( client->questdebug )
            Log(MSG_QUESTDEBUG, "QUESTCOND(009):: Percent chance SUCCESS %i < %i", thischance,data->btHighPcnt);
	    return QUEST_SUCCESS;
	}
	if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(009):: Percent chance FAILURE %i > %i", thischance,data->btHighPcnt);
	return QUEST_FAILURE;
}

//Object Variable
QUESTCOND(011)
{
	//LMA: Like AIP.
    GETCONDDATA(011);

	int refNPC = client->quest.RefNPC;
	if(refNPC == 0)
	{
	    Log(MSG_WARNING,"QUESTCOND::Np NPC selected");
	    return QUEST_FAILURE;
	}

	int ObjvarIndex = data->nVarNo;
	int tempval = GServer->ObjVar[refNPC][ObjvarIndex];
	int orgvalue = 0;

    //PY WTF is all this stuff for? This function is supposed to be a simple comparrison. Why all the special cases.
    /*
    //LMA: WarpGate or standard NPC?
    if(refNPC>10000&&refNPC==GServer->WarpGate.virtualNpctypeID)
    {
        //WarpGate.
        if(ObjvarIndex>19)
            return QUEST_FAILURE;
        tempval = GServer->WarpGate.IfoObjVar[ObjvarIndex];
    }
    else
    {
        if(refNPC>=MAX_NPC)
        {
            Log(MSG_WARNING,"It seems NPC %i>=%u , change MAX_NPC value!",refNPC,MAX_NPC);
            return QUEST_FAILURE;
        }

        tempval = GServer->ObjVar[refNPC][ObjvarIndex];
    }

    //LMA: We force the nb players required for UW.
    orgvalue=data->iValue;
    //LMA: 2010/07/30: naRose changed its requirements from 30/33 to 20/23.
    //if(ObjvarIndex==2&&refNPC>=1088&&refNPC<=1091&&(orgvalue==30||orgvalue==33))
    if(ObjvarIndex==2&&refNPC>=1088&&refNPC<=1091&&(orgvalue==20||orgvalue==23))
    {
        //if(orgvalue==30&&data->btOp==2)
        if(orgvalue==20&&data->btOp==2)
        {
            orgvalue=GServer->UWNbPlayers;
            LogDebug("QSDC11::UW We trapped NPC %i[%i]>=%i (replaces >= %i)",refNPC,ObjvarIndex,orgvalue,data->iValue);
        }

        //if(orgvalue==33&&data->btOp==4)
        if(orgvalue==23&&data->btOp==4)
        {
            orgvalue=GServer->UWNbPlayers+3;
            LogDebug("QSDC11::UW We trapped NPC %i[%i]<=%i (replaces <= %i)",refNPC,ObjvarIndex,orgvalue,data->iValue);
        }

    }
  */

    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(011):: NPC %i, op=%i, value=%i (real=%i)",refNPC,data->btOp,data->iValue,tempval);

	switch(data->btOp)
	{
        case 0:
             if(tempval == data->iValue)
                 return QUEST_SUCCESS;
             break;
        case 1:
             if(tempval > data->iValue)
                 return QUEST_SUCCESS;
             break;
        case 2:
             if(tempval >= data->iValue)
                 return QUEST_SUCCESS;
             break;
        case 3:
             if(tempval < data->iValue)
                 return QUEST_SUCCESS;
             break;
        case 4:
             if(tempval <= data->iValue)
                 return QUEST_SUCCESS;
             break;
        case 10:
             if(tempval != data->iValue)
                 return QUEST_SUCCESS;
             break;
        default:
            //LogDebug("QUESTCOND(011) FAILED, switch default");
             return QUEST_FAILURE;
             break;
    }

    //LogDebug("QUESTCOND(011) FAILED, compare false");


	return QUEST_FAILURE;
}

//Select an IFO Object.
QUESTCOND(012)
{
    /*
    GETCONDDATA(012);

    //LMA: For now we do only this stuff for the warp gate :)
    bool is_ok=false;
    LogSp(MSG_INFO,"\t\t\t\t\t CDT %.3i: Select Object %i in map %i, IFO %i_%i.IFO (NOT CODED)",opcode,data->iEventID,data->iZone,data->iX,data->iY);
    if(GServer->WarpGate.id!=data->iEventID||GServer->WarpGate.IfoX!=data->iX||GServer->WarpGate.IfoY!=data->iY||GServer->WarpGate.mapid!=data->iZone)
    {
        return QUEST_FAILURE;
    }

    //We select as refNPC the WarpGate ^_^
    if(GServer->WarpGate.Npc==NULL)
    {
        return QUEST_FAILURE;
    }

    client->quest.selectedNpc = GServer->WarpGate.Npc;
    Log(MSG_INFO,"CDTQ 012 Warp Gate Selected");
    */
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(012):: Select an IFO object. NOT CODED");


	return QUEST_SUCCESS;
}


//Select NPC
QUESTCOND(013)
{
    //LMA: like AIP.
    GETCONDDATA(013);
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(013):: Select an NPC with ID %i", data->iNpcNo);
	client->quest.RefNPC=data->iNpcNo; // sets the reference variable for the correct ObjVar
	return QUEST_SUCCESS;
}



//Check Quest Flag
QUESTCOND(014){
    GETCONDDATA(014);
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(014):: Check QFlag[%i] == %i", data->nSN, data->btOp);
	bool dataOp = (data->btOp)?true:false;
	return (client->quest.GetFlag(data->nSN) == dataOp)?QUEST_SUCCESS:QUEST_FAILURE;
}

//Party check 2
QUESTCOND(015)
{
    //word nNumber1       pos 0x00
	//word nNumber2       pos 0x02
	GETCONDDATA(015);
	if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(015):: Party Check");
	if(client->Party->party == NULL)	//client not in a party
        return QUEST_FAILURE;
    CParty* party = client->Party->party;
	int MemberCount = 0;
	if(party->Members.size()>1)
    {
        for(UINT i=0;i<party->Members.size();i++)
        {
			CPlayer* thismember = party->Members.at(i);
			if(thismember != NULL)
				MemberCount ++;
		}
	}
	else
		return QUEST_FAILURE; //nobody in the party
	if(MemberCount >- data->nNumber1 && MemberCount <= data->nNumber2)
		return QUEST_SUCCESS;
	return QUEST_FAILURE;
}

//Zone Time
QUESTCOND(016)
{
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(016):: Check Zone Time");
    return QUEST_SUCCESS;
}

//LMA: Compare two NPC & Obj Variables.
QUESTCOND(017)
{
    GETCONDDATA(017);

    int var1=0;
    int var2=0;

    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(017)::  We compare NPC %i[%i] with (%i) NPC %i[%i]",data->NpcVar1.iNpcNo,data->NpcVar1.nVarNo,data->btOp,data->NpcVar2.iNpcNo,data->NpcVar2.nVarNo);

    if(data->NpcVar1.iNpcNo>=MAX_NPC||data->NpcVar2.iNpcNo>=MAX_NPC||data->NpcVar1.nVarNo>=20||data->NpcVar2.nVarNo>=20)
    {
        if(data->NpcVar1.iNpcNo>=MAX_NPC)
        {
            Log(MSG_WARNING,"It seems NPC %i>=%u , change MAX_NPC value!",data->NpcVar1.iNpcNo,MAX_NPC);
            return QUEST_FAILURE;
        }

        if(data->NpcVar2.iNpcNo>=MAX_NPC)
        {
            Log(MSG_WARNING,"It seems NPC %i>=%u , change MAX_NPC value!",data->NpcVar2.iNpcNo,MAX_NPC);
            return QUEST_FAILURE;
        }

        //LogDebug("QD17 Failure 1");
        return QUEST_FAILURE;
    }

    var1=GServer->ObjVar[data->NpcVar1.iNpcNo][data->NpcVar1.nVarNo];
    var1=GServer->ObjVar[data->NpcVar2.iNpcNo][data->NpcVar2.nVarNo];

    if(!OperateValues<int>(data->btOp, (int*)&var1, var2))
    {
        //LogDebug("QD17 Failure 2");
        return QUEST_FAILURE;
    }


    //LogDebug("QD17 Success");

    return QUEST_SUCCESS;
}

//Time on Date (credits PY)
QUESTCOND(018)
{
    //Returns SUCCESS if the system time of the server is between the defined start and end times. Else returns FAILURE.
    //Allows for time dependent quests
    //byte btDate;       pos 0x00
	//byte btHour1;      pos 0x01
	//byte btMin1;       pos 0x02
	//byte btHour2;      pos 0x03
	//byte btMin2;       pos 0x04
	GETCONDDATA(018);
	if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(018):: Check time on Date");
	SYSTEMTIME sTIME;
    GetLocalTime(&sTIME);

    if(data->btDate != 0)
    {
    	if(sTIME.wDay != data->btDate)
    		return QUEST_FAILURE;
    }
    word wMin = ((sTIME.wHour * 60) + sTIME.wMinute);
    word wFrom = (data->btHour1 * 60) + data->btMin1;
    word wTo = (data->btHour2 * 60) + data->btMin2;
    if(wMin >= wFrom && wMin <= wTo)
    	return QUEST_SUCCESS;
	return QUEST_FAILURE;
}

//Time on Day
QUESTCOND(019)
{
    //byte btWeekDay;     pos 0x00
	//byte btHour1;       pos 0x01
	//byte btMin1;        pos 0x02
	//byte btHour2;       pos 0x03
	//byte btMin2;        pos 0x04
	GETCONDDATA(019);
	if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(019):: Check time on Day");
	SYSTEMTIME sTIME;
    GetLocalTime(&sTIME);
    if(data->btWeekDay != 0)
    {
        if(sTIME.wDayOfWeek != data->btWeekDay)
            return QUEST_FAILURE;
    }
    word wMin = ((sTIME.wHour * 60) + sTIME.wMinute);
    word wFrom = (data->btHour1 * 60) + data->btMin1;
    word wTo = (data->btHour2 * 60) + data->btMin2;
    if(wMin >= wFrom && wMin <= wTo)
    	return QUEST_SUCCESS;
	return QUEST_FAILURE;
}

//Team number between iNo1 and iNo2
QUESTCOND(020)
{
    //int iNo1           pos 0x00
	//int iNo2           pos 0x02
	GETCONDDATA(020);
	if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(020):: Check Team number");
	int team = client->pvp_id;
	if( team >= data->iNo1 && team <= data->iNo2 )
		return QUEST_SUCCESS;
	return QUEST_FAILURE;
}

//LMA: In range from a NPC.
QUESTCOND(021)
{
    GETCONDDATA(021);
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(021):: Check in range from selected NPC");
	int refNPC = client->quest.RefNPC;
	if(refNPC==0)
	{
	    //Log(MSG_WARNING,"QSDC21::No NPC selected.");
	    return QUEST_FAILURE;
	}

	//Getting coordinates for this NPC.
	if(client->Position->Map==0||client->Position->Map>=GServer->MapList.max)
	{
	    //Log(MSG_WARNING,"QSDC21: Player %s is in an incorrect map %i",client->CharInfo->charname,client->Position->Map);
	    return QUEST_FAILURE;
	}

	CMap* thisMap = GServer->MapList.Index[client->Position->Map];
	CNPC* thisNpc = thisMap->GetNPCInMapQSD(refNPC);
	if(thisNpc==NULL)
	{
	    //Log(MSG_WARNING,"QSDC21::Incorrect NPC %u",refNPC);
	    return QUEST_FAILURE;
	}

    int iDistance = (int) GServer->distance( thisNpc->pos, client->Position->current );

    if (iDistance>data->iRadius)
    {
        //Log(MSG_WARNING,"Player %s is too far from NPC %i",client->CharInfo->charname,refNPC);
        return QUEST_FAILURE;
    }

    //Log(MSG_INFO,"Player %s is in good distance from NPC %i",client->CharInfo->charname,refNPC);


    return QUEST_SUCCESS;
}

//Check Server/Channel
QUESTCOND(022)
{
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(022):: Check Server channel (not coded)");
    return QUEST_SUCCESS;
}

//In Clan (credits PY)
QUESTCOND(023)
{
    //byte btReg         pos 0x00
    GETCONDDATA(023);
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(023):: Check Clan. Am I in one?");
    int Clanid = client->Clan->clanid;
    if(data->btReg == 0)
    {
        if(Clanid == 0)return QUEST_SUCCESS;
        else return QUEST_FAILURE;
    }
    else
    {
        if(Clanid != 0)return QUEST_SUCCESS;
        else return QUEST_FAILURE;
    }

    //btReg should only ever be 1 or 0 but just in case......


    return QUEST_FAILURE;
}

//Clan Position (credits PY)
QUESTCOND(024)
{
    //check my current rank in the clan against data->nPOS using btOP
    GETCONDDATA(024);
    //word nPOS;         pos 0x00
	//byte btOP;         pos 0x02
	int myRank = client->Clan->clanrank;
	if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(024):: Check Clan Rank");
    switch(data->btOP)
	{
        case 0:
             if(myRank == data->nPOS)
                 return QUEST_SUCCESS;
             break;
        case 1:
             if(myRank > data->nPOS)
                 return QUEST_SUCCESS;
             break;
        case 2:
             if(myRank >= data->nPOS)
                 return QUEST_SUCCESS;
             break;
        case 3:
             if(myRank < data->nPOS)
                 return QUEST_SUCCESS;
             break;
        case 4:
             if(myRank <= data->nPOS)
                 return QUEST_SUCCESS;
             break;
        case 10:
             if(myRank != data->nPOS)
                 return QUEST_SUCCESS;
             break;
        default:
             return QUEST_FAILURE;
             break;
    }


    return QUEST_FAILURE;
}

//Clan Contribution
QUESTCOND(025)
{
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(026):: Check Clan Contribution");
    return QUEST_SUCCESS;
}

//Clan Grade (credits PY)
QUESTCOND(026)
{
    //possibly compare client's current clan grade with data->nGRD using data->btOp?
    GETCONDDATA(026);
    //word nGRD;
	//byte btOP;
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(026):: Check Clan Grade");
	//LMA: Getting real clan grade.
	client->Clan->grade=GServer->getClanGrade(client->Clan->clanid);
	int myGrade = client->Clan->grade;
	switch(data->btOP)
	{
        case 0:
             if(myGrade == data->nGRD)
                 return QUEST_SUCCESS;
             break;
        case 1:
             if(myGrade > data->nGRD)
                 return QUEST_SUCCESS;
             break;
        case 2:
             if(myGrade >= data->nGRD)
                 return QUEST_SUCCESS;
             break;
        case 3:
             if(myGrade < data->nGRD)
                 return QUEST_SUCCESS;
             break;
        case 4:
             if(myGrade <= data->nGRD)
                 return QUEST_SUCCESS;
             break;
        case 10:
             if(myGrade != data->nGRD)
                 return QUEST_SUCCESS;
             break;
        default:
             return QUEST_FAILURE;
             break;
    }


    return QUEST_FAILURE;
}

//LMA: Check Clan Points amount (?)
QUESTCOND(027){
    GETCONDDATA(027);
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(027):: Check Clan points");
    //LMA: Getting real clan points amount.
    client->Clan->CP=GServer->getClanPoints(client->Clan->clanid);
	UINT myCP = client->Clan->CP;
	switch(data->btOP)
	{
        case 0:
             if(myCP == data->nPOINT)
                 return QUEST_SUCCESS;
             break;
        case 1:
             if(myCP > data->nPOINT)
                 return QUEST_SUCCESS;
             break;
        case 2:
             if(myCP >= data->nPOINT)
                 return QUEST_SUCCESS;
             break;
        case 3:
             if(myCP < data->nPOINT)
                 return QUEST_SUCCESS;
             break;
        case 4:
             if(myCP <= data->nPOINT)
                 return QUEST_SUCCESS;
             break;
        case 10:
             if(myCP != data->nPOINT)
                 return QUEST_SUCCESS;
             break;
        default:
             return QUEST_FAILURE;
             break;
    }


    return QUEST_FAILURE;
}

//Clan Funds
QUESTCOND(028)
{
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(028):: Check Clan Funds");
    return QUEST_SUCCESS;
}

//Clan Members
QUESTCOND(029)
{
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(029):: Check Clan Members");
    return QUEST_SUCCESS;
}

//Clan Skills
QUESTCOND(030)
{
    if( client->questdebug )
        Log(MSG_QUESTDEBUG, "QUESTCOND(030):: Check Clan Skills");
    return QUEST_SUCCESS;
}

