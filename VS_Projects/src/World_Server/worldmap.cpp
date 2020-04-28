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

#include "worldmap.h"

CMap::CMap( )
{
    id = 0;
    dayperiod = 0;
    morningtime = 0;
    daytime = 0;
    eveningtime = 0;
    nighttime = 0;
    allowpvp = 0;
    allowpat = 0;
    MapTime = 0;
    LastUpdate = clock( );
    CurrentTime = 0;
    RespawnList.clear();
    MonsterSpawnList.clear();
    MobGroupList.clear();
    MonsterList.clear();
    DropsList.clear();
    PlayerList.clear();
    NPCList.clear();
    TeleGateList.clear();
    MonsterMutex = PTHREAD_MUTEX_INITIALIZER;
    DropMutex = PTHREAD_MUTEX_INITIALIZER;
}

CMap::~CMap( )
{
    for(UINT i=0;i<RespawnList.size();i++)
        delete RespawnList.at(i);
    for(UINT i = 0; i < MobGroupList.size(); i++) {
      CMobGroup *thisgroup = MobGroupList.at(i);
      for (UINT j = 0; j < thisgroup->basicMobs.size(); j++)
        delete thisgroup->basicMobs.at(j);
      for (UINT j = 0; j < thisgroup->tacMobs.size(); j++)
        delete thisgroup->tacMobs.at(j);
      delete thisgroup;
    }
    for(UINT i=0;i<MonsterSpawnList.size();i++)
        delete MonsterSpawnList.at(i);
    for(UINT i=0;i<MonsterList.size();i++)
        delete MonsterList.at(i);
    for(UINT i=0;i<DropsList.size();i++)
        delete DropsList.at(i);
    for(UINT i=0;i<NPCList.size();i++)
        delete NPCList.at(i);
    for(UINT i=0;i<TeleGateList.size();i++)
        delete TeleGateList.at(i);
}

bool CMap::AddPlayer( CPlayer* player )
{
    PlayerList.push_back( player );
    return false;
}

bool CMap::RemovePlayer( CPlayer* player, bool clearobject )
{
    GServer->ClearClientID( player->clientid );
    if(clearobject)
    {
        BEGINPACKET( pak, 0x794 );
        ADDWORD    ( pak, player->clientid );
        GServer->SendToVisible( &pak, player, false );
    }
    for(UINT i=0;i<PlayerList.size();i++)
    {
        if(PlayerList.at(i)==player)
        {
            PlayerList.erase( PlayerList.begin()+i );
            return true;
        }
    }
    Log(MSG_WARNING, "Player not founded %s", player->CharInfo->charname );
    return false;
}

// add a new monster to this map
CMonster* CMap::AddMonster( UINT montype, fPoint position, UINT owner, CMDrops* MonsterDrop, CMDrops* MapDrop, UINT spawnid , bool GetDropData, bool is_tactic)
{
    // check if is a valid monster
    CNPCData* thisnpc = GServer->GetNPCDataByID( montype );
    if(thisnpc==NULL)
    {
        Log( MSG_WARNING, "Add Monster:: Invalid montype %u", montype );
        return NULL;
    }

    CMonster* monster = new (nothrow) CMonster( position, montype, this->id, owner, spawnid  );
    if(monster==NULL)
    {
        Log( MSG_WARNING, "Add Monster %u:: Error allocing memory",montype);
        return NULL;
    }
    monster->thisnpc = thisnpc;
    monster->life_time=0;
    monster->SetStats( );
    monster->Stats->HP = monster->Stats->MaxHP;
    monster->Stats->MP = monster->Stats->MaxMP;
    monster->is_tactical=is_tactic;
    monster->suicide=false;

    //LMA: test for quest hack (stackable).
    #ifdef QHACK
    monster->die_quest=thisnpc->die_quest;
    #endif
    //LMA END

    for(int i=0;i<20;i++)
    {
        monster->AIVar[i] = 0;
    }

    //LMA: no agressivity in Santa's planetoid ;)
    if(monster->Position->Map==38)
    {
        monster->thisnpc->aggresive=0;
        monster->thisnpc->helpless=1;
        //Log( MSG_WARNING, "Add Monster %u:: to Santa's planetoid",montype); // PY code added to read out monsters spawning on Santa's planetoid
    }

    //LMA: Little check, for now we "only" have a DWORD for monster's HP so there is a limit
    //broken by some monsters (Turak boss)
    if(monster->Stats->HP>MAXHPMOB)
    {
        monster->Stats->HP=(long long) MAXHPMOB;
    }

    monster->Status->spawnid = spawnid; //this way we can easily find which spawn a mob belongs to
    if(GetDropData && owner==0)
    {
        monster->MonsterDrop->mobdrop = GServer->GetDropData( monster->thisnpc->dropid );
        monster->MonsterDrop->mapdrop = GServer->GetDropData( id );
    }
    else
    {
        monster->MonsterDrop->mobdrop = MonsterDrop;
        monster->MonsterDrop->mapdrop = MapDrop;
    }
    MonsterList.push_back( monster );

    //LMA: daynight
    monster->daynight=0;
    monster->sp_aip=0;

    //LMA: team:
    monster->team=0;

    if(monster->IsSummon()&&owner!=0)
    {
        //Trying to get the owner.
        CPlayer* tempplayer=GServer->GetClientByID(owner,monster->Position->Map);
        if (tempplayer!=NULL)
        {
            monster->team=tempplayer->pvp_id;
            Log(MSG_INFO,"ADDMONSTER, overwriting team with owner's pvp_id %i",tempplayer->pvp_id);

            //Tomiz : Get Owner lvl For Summon (the one from job, setup to fix normal atk formula because summon got lvl <= 100 in STB)
            UINT ownerlevel = 0;
            ownerlevel += tempplayer->Stats->Level;
            monster->Stats->Level = ownerlevel;
             Log(MSG_INFO,"Summon %i, got lvl %i by this owner",thisnpc->id,ownerlevel);
         }

        //LMA: adding a summon :)
        if (monster->IsSummon())
        {
            nb_summons++;
            Log(MSG_INFO,"Adding a summon in map %i (now %u)",id,nb_summons);
        }

     }
    //Log( MSG_WARNING, "AddMonster:: about to do some crap with groups" );
    /* if(spawnid!=0)  //PY: commenting out this bit cuz it crashes with my new IFO spawns
    {
        CMobGroup* thisgroup = GServer->GetMobGroup( spawnid, this->id );
        if(thisgroup!=NULL)
        {
            //LMA: only if not a tactical.
            if(!monster->is_tactical)
            {
                thisgroup->active++;
            }

        }

        //LMA: daynight
        monster->daynight=thisgroup->daynight;
    }
    */
    monster->SpawnTime = clock( );
    monster->lastDegenTime=clock();
    monster->OnSpawn( false );
    monster->lastAiUpdate = clock();
    monster->nextAi_attacked=clock();
    monster->hitcount = 0xFF;

    monster->hitcount=0;
    monster->maxhitcount=0;
    monster->stay_still=false;

    if(monster->thisnpc->id==659)
    {
       monster->maxhitcount=3;   //LMA: MoonChild
       monster->stay_still=true;
    }

    if(monster->thisnpc->id==201)
    {
       monster->maxhitcount=1;   //LMA: Worm dragon
    }

    if(monster->thisnpc->id==1572)
    {
       monster->maxhitcount=1;   //rl2171: Cursed Ant Vagabond
    }

    if(monster->thisnpc->id==662 || monster->thisnpc->id==667 || monster->thisnpc->id==673 || monster->thisnpc->id==678)
    {
       monster->maxhitcount=3;   //rl2171: Dragon Egg for Clan Wars  - 662, 667, 673, 678
       monster->stay_still=true;
    }

     if(monster->thisnpc->id==558)
    {
       monster->maxhitcount=1;   //rl2171: 1st Turak
    }

     if(monster->thisnpc->id==559)
    {
       monster->maxhitcount=1;   //rl2171: 2nd Turak
    }

    /*if(monster->thisnpc->id==560)
    {
       monster->maxhitcount=1;   //rl2171: 3rd Turak
    }
    */
    //Sunrise, Sunset and Dusk Crystal in Junon Cartel
    if (monster->thisnpc->id>=431&&monster->thisnpc->id<=433)
       monster->stay_still=true;

    //Rune Stone, Green, Blue, Red, White Crystal in Luna
    //    if (monster->thisnpc->id>=325&&monster->thisnpc->id<=329)
    //       monster->stay_still=true;

        //Seal Stone
    //    if (monster->thisnpc->id>=451&&monster->thisnpc->id<=457)
    //       monster->stay_still=true;

        //Clan Wars gate - wall
    //    if (monster->thisnpc->id>=1022&&monster->thisnpc->id<=1023)
    //       monster->stay_still=true;

    //LMA: bonfire (and salamender...) don't move and don't attack ;)
    if (monster->IsBonfire())
    {
       monster->stay_still=true;
       monster->Battle->target=owner;
    }

    monster->bonushp=0;
    monster->bonusmp=0;
    monster->maxvalue=0;
    monster->minvalue=0;
    monster->skillid=0;
    monster->range=0;
    monster->buffid=0;
    monster->char_montype=montype;

    //Log(MSG_INFO,"XCID, Addmonster %i in map %i",montype,monster->Position->Map);


    return monster;
}

// Delete a monster
bool CMap::DeleteMonster( CMonster* monster, bool clearobject, UINT i )
{
    if(monster == NULL) return false;
    //LMA: deleting a summon :)
    if (monster->IsSummon())
    {
        if (nb_summons > 0)
        {
            nb_summons --;
        }
        Log(MSG_INFO,"Deleting a summon in map %i (now %u)",id,nb_summons);
    }

    //LMA TEST
    bool lma_debug = true;
    if(monster->Position->respawn == 4589)
    {
       lma_debug = true;
       Log(MSG_INFO,"DeleteMonster Spawn %u CID %u",monster->Position->respawn,monster->clientid);
    }

    GServer->ClearClientID( monster->clientid );
    if(monster->Position->respawn !=0 ) //it has a spawn. not generated by AIP or QSD or whatever
    {
        //PY: not using mobgroups so need to comment out all that stuff and use this instead
        CSpawnArea* thisspawn = new (nothrow) CSpawnArea;
        for(UINT i=0;i<MonsterSpawnList.size();i++)
        {
            thisspawn = MonsterSpawnList.at(i);
            if (thisspawn->id == monster->Position->respawn)
            {
                //found the spawn so exit the loop
                break;
            }
        }
        //count how many remaining monsters are in this spawn and store teh value in the spawn
        UINT MCounter = 0;
        for(UINT i=0;i<MonsterList.size();i++)
        {
            if(MonsterList.at(i)->Position->respawn == monster->Position->respawn )
            {
                MCounter++;
            }
        }
        thisspawn->MCount = MCounter-1; //remember we haven't deleted this monster yet
    }

        /*
        CMobGroup* thisgroup = GServer->GetMobGroup( monster->Position->respawn, monster->Position->Map );
        if(thisgroup != NULL)
        {
            //if(thisgroup->active >= thisgroup->limit)// reset spawn timer if the spawn is full
            //    thisgroup->lastRespawnTime = clock();

            //LMA: only if the monster isn't tactical...

           if(lma_debug)
           {
               Log(MSG_INFO,"DeleteMonster Spawn %u CID %u:: group found, is tactical? %i",monster->Position->respawn,monster->clientid,monster->is_tactical);
           }

            if(!monster->is_tactical)
            {
                thisgroup->active --;
                thisgroup->basicKills ++;
                thisgroup->lastKills ++;

               if(lma_debug)
               {
                   Log(MSG_INFO,"DeleteMonster Spawn %u CID %u:: not tactical, active %u, basickills %u, lastkills %u",monster->Position->respawn,monster->clientid,thisgroup->active,thisgroup->basicKills,thisgroup->lastKills);
               }

            }

        }
        else
        {
            Log(MSG_WARNING,"Spawn %u not found in map %u",monster->Position->respawn,monster->Position->Map);
        }

    }*/

    if(clearobject)
    {
        BEGINPACKET( pak, 0x799 );
        ADDWORD    ( pak, monster->clientid );
        ADDWORD    ( pak, monster->clientid );
        ADDDWORD   ( pak, monster->Stats->HP );
        ADDDWORD   ( pak, 16 );
        GServer->SendToVisible( &pak, monster );

       if(lma_debug)
       {
           Log(MSG_INFO,"DeleteMonster Spawn %u CID %u:: clearing",monster->Position->respawn,monster->clientid);
       }

    }

    if(i!=0)
    {

       if(lma_debug)
       {
           Log(MSG_INFO,"DeleteMonster Spawn %u CID %u:: erasing with %u",monster->Position->respawn,monster->clientid,i);
       }

        MonsterList.erase( MonsterList.begin()+i );
        delete monster;
        return true;
    }

    for(UINT i=0;i<MonsterList.size();i++)
    {
        CMonster* othermon = MonsterList.at(i);
        if(othermon==monster)
        {

           if(lma_debug)
           {
               Log(MSG_INFO,"DeleteMonster Spawn %u CID %u:: erasing",monster->Position->respawn,monster->clientid);
           }

            MonsterList.erase( MonsterList.begin()+i );
            delete monster;
            return true;
        }

    }

   if(lma_debug)
   {
       Log(MSG_INFO,"DeleteMonster Spawn %u CID %u:: erasing, not found in monsterlist",monster->Position->respawn,monster->clientid);
   }

    delete monster;
    return false;
}

//add a new drop
bool CMap::AddDrop( CDrop* drop )
{
    DropsList.push_back( drop );
    return true;
}

bool CMap::DeleteDrop( CDrop* drop )
{
    BEGINPACKET( pak, 0x794 );
	ADDWORD    ( pak, drop->clientid );
	GServer->SendToVisible( &pak, drop );
    GServer->ClearClientID( drop->clientid );
    for(UINT j=0;j<DropsList.size();j++)
    {
        if(drop==DropsList.at(j))
        {
            DropsList.erase( DropsList.begin()+j );
            delete drop;
            return true;
        }
    }
    delete drop;
    return false;
}

bool CMap::AddNPC( CNPC* npc )
{
    NPCList.push_back( npc );

    //LMA: We keep a track with a more general map as well since we'll need it
	//in some QSD...
    if (GServer->ListAllNpc.find(npc->npctype)==GServer->ListAllNpc.end())
    {
        //not found yet.
        GServer->ListAllNpc[npc->npctype].push_back(npc->posMap);
        //Log(MSG_WARNING,"Adding NPC %i (didn't exist) in map %i",npc->npctype,npc->posMap);
        return true;
    }

    //found, let's see if we already got a NPC in this map.
    for (int k=0;k<GServer->ListAllNpc[npc->npctype].size();k++)
    {
        if(GServer->ListAllNpc[npc->npctype].at(k)==npc->posMap)
        {
            //Log(MSG_WARNING,"there is already a NPC %i in map %i",npc->npctype,npc->posMap);
            return true;
        }

    }

    //let's add it :)
    GServer->ListAllNpc[npc->npctype].push_back(npc->posMap);
    //Log(MSG_WARNING,"Adding NPC %i (already in other maps) in map %i as well",npc->npctype,npc->posMap);


    return true;
}

bool CMap::DeleteNPC( CNPC* npc )
{
    BEGINPACKET( pak, 0x794 );
    ADDWORD    ( pak, npc->clientid );
    GServer->SendToMap( &pak, npc->posMap );
    GServer->ClearClientID( npc->clientid );
    for(UINT j=0;j<NPCList.size();j++)
    {
        if(npc==NPCList.at(j))
        {
            NPCList.erase( NPCList.begin()+j );
            delete npc;
            return true;
        }
    }
    delete npc;
    return false;
    return true;
}
