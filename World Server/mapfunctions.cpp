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

// convert a monster to other
CMonster* CMap::ConverToMonster( CMonster* monster, UINT newmontype, bool heal )
{
    CNPCData* thisnpc = GServer->GetNPCDataByID( newmontype );
    if(thisnpc == NULL)// invalid type
        return NULL;
    int spawnid = monster->Status->spawnid;
    monster->montype = newmontype;
    monster->thisnpc = thisnpc;
    monster->SetStats( );
    if(heal)
        monster->Stats->HP = monster->Stats->MaxHP;
    if(monster->owner == 0)
    {
        monster->MonsterDrop->mobdrop = GServer->GetDropData( monster->thisnpc->dropid );
        monster->MonsterDrop->mapdrop = GServer->GetDropData( id );
    }
    BEGINPACKET( pak, 0x774 );
    ADDWORD    ( pak, monster->clientid );
    ADDWORD    ( pak, newmontype );
    GServer->SendToVisible( &pak, monster );
    monster->OnSpawn( false );
    monster->Status->spawnid = spawnid; //make sure the monster retains the original spawnid. Don't see why it should be lost though.
    return monster;
}

// convert a npc to other [not working]
CMonster* CMap::ConverToMonster( CNPC* npc, UINT newmontype )
{
    CMonster* monster = AddMonster( newmontype, npc->pos, 0, NULL, NULL, 0, true );
    if(monster==NULL) // invalid montype
        return NULL;
    GServer->ClearClientID( monster->clientid );
    monster->clientid = npc->clientid;
    for(UINT i=0;i<NPCList.size();i++)
    {
        if(NPCList.at(i)==npc)
            NPCList.erase( NPCList.begin()+i );
    }
    delete npc;
    BEGINPACKET( pak, 0x774 );
    ADDWORD    ( pak, npc->clientid );
    ADDWORD    ( pak, newmontype );
    GServer->SendToVisible( &pak, monster );
    return monster;
}

// Teleport player to this map and this coord.
bool CMap::TeleportPlayer( CPlayer* player, fPoint coordold, bool TelePassenger )
{
    fPoint coord;
    coord=coordold;

    if(id==9&&player->UWPosition->source.x>0&&player->UWPosition->source.y>0)
    {
        coord.x = player->UWPosition->source.x;
        coord.y = player->UWPosition->source.y;
    }

    GServer->MapList.Index[player->Position->Map]->RemovePlayer( player, false );
    player->Position->Map = id;
    player->Position->current = coord; //TELEPORT ONLY
    player->Position->destiny = coord; //TELEPORT ONLY
    player->Session->inGame = false;
    player->Position->lastMoveTime = clock();
    if(!allowpat || !TelePassenger)
    {
        if(!allowpat)
            player->Status->Stance=0x03;
        player->Ride->Drive = false;
        player->Ride->charid= 0;
        player->Ride->Ride = false;
    }
    AddPlayer( player );
	BEGINPACKET( pak, 0x07a8 );
	ADDWORD    ( pak, player->clientid );
	ADDWORD    ( pak, player->Position->Map );
	ADDFLOAT   ( pak, player->Position->current.x*100 );
	ADDFLOAT   ( pak, player->Position->current.y*100 );
    ADDWORD    ( pak, (player->Status->Stance==0x04?0x0201:0x0001) );

    //LMA: it seems they added some stuff...
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);

    //move speed.
    ADDWORD(pak,player->Stats->Base_Speed);

    //0x00
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDDWORD(pak, 0x00);
    ADDWORD(pak, 0x00);

	player->client->SendPacket( &pak );



    if( player->Ride->Drive && player->Ride->charid!=0 )
    {
            CPlayer* otherclient = GServer->GetClientByCID( player->Ride->charid );
            if( otherclient!=NULL )
            {
                if(TelePassenger)
                {
                    TeleportPlayer( otherclient, coord );
                }
                else
                {
                    otherclient->Ride->Drive = false;
                    otherclient->Ride->charid= 0;
                    otherclient->Ride->Ride = false;
                }
            }
    }
    GServer->pakClearUser( player );
    GServer->ClearClientID( player->clientid );
    player->RestartPlayerVal( );
    return true;
}

// return near respawn [NULL if there are no respawn]
CRespawnPoint* CMap::GetNearRespawn( CPlayer* player )
{
    CRespawnPoint* respawn = NULL;
    float distance = 0xffff;
    for(UINT i=0;i<RespawnList.size();i++)
    {
        float resdist = GServer->distance( player->Position->current, RespawnList.at(i)->dest );
        if( resdist < distance )
        {
            respawn = RespawnList.at(i);
            distance = resdist;
        }
    }
    return respawn;
}

// Return first respawn
CRespawnPoint* CMap::GetFirstRespawn( )
{
    CRespawnPoint* respawn = NULL;
    for(UINT i=0;i<RespawnList.size();i++)
    {
        return RespawnList.at(i);
    }
    return respawn;
}

// Search a client by clientid
CPlayer* CMap::GetPlayerInMap( UINT id )
{
    for(UINT i=0;i<PlayerList.size();i++)
    {
        if(PlayerList.at(i)->clientid == id )
            return PlayerList.at(i);
    }
    return NULL;
}

// Search a client by charid
CPlayer* CMap::GetCharIDInMap( DWORD id )
{
    for(UINT i=0;i<PlayerList.size();i++)
    {
        if(PlayerList.at(i)->CharInfo->charid == id )
            return PlayerList.at(i);
    }
    return NULL;
}

// Search a monster by clientid
CMonster* CMap::GetMonsterInMap( UINT id )
{
    for(UINT i=0;i<MonsterList.size();i++)
    {
        if(MonsterList.at(i)->clientid == id )
            return MonsterList.at(i);
    }
    return NULL;
}

CDrop* CMap::GetDropInMap( UINT id )
{
    for(UINT i=0;i<DropsList.size();i++)
    {
        if(DropsList.at(i)->clientid == id )
            return DropsList.at(i);
    }
    return NULL;
}

CNPC* CMap::GetNPCInMap( UINT id )
{
    for(UINT i=0;i<NPCList.size();i++)
    {
        if(NPCList.at(i)->clientid == id )
            return NPCList.at(i);
    }
    return NULL;
}

//LMA: Used for QSD.
CNPC* CMap::GetNPCInMapQSD( UINT id )
{
    for(UINT i=0;i<NPCList.size();i++)
    {
        CNPC* thisNpc = NPCList.at(i);
        if(thisNpc->npctype == id )
            return thisNpc;
    }

    //LMA: If we didn't find the NPC in current map, let's search elsewhere, let's get the good map.
    Log(MSG_WARNING,"NPC %i not found in map %i",id,this->id);
    if (GServer->ListAllNpc.find(id)==GServer->ListAllNpc.end())
    {
        Log(MSG_WARNING,"NPC %i not found ListAllNPC",id);
        return NULL;
    }

    if(GServer->ListAllNpc[id].size()==0)
    {
        Log(MSG_WARNING,"NPC %i found in ListAllNPC but no entry",id);
        return NULL;
    }

    //We use the first entry...
    CMap* thisMap = GServer->MapList.Index[GServer->ListAllNpc[id].at(0)];
    CNPC* thisNpc = thisMap->GetNPCInMapQSD(id);
    if(thisNpc!=NULL)
    {
        Log(MSG_WARNING,"NPC %i found thanks to ListAllNPC in map %i",thisNpc->npctype,GServer->ListAllNpc[id].at(0));
    }
    else
    {
        Log(MSG_WARNING,"NPC %i NOT found thanks to ListAllNPC in map %i",id,GServer->ListAllNpc[id].at(0));
    }


    return thisNpc;
}

void CMap::UpdateTime( )
{
    if(id==0) return;
    UINT etime = (UINT)round((clock() - LastUpdate)/1000);
    if(etime<10)return;
    MapTime += (UINT)floor(etime/10);
    if(MapTime==0xffffffff)
        MapTime = 0;
    ZoneTime = int(MapTime / 17895697);
    if(dayperiod==0)
        dayperiod=1;
    UINT ctime = MapTime%dayperiod;
    if(ctime>=morningtime && ctime<daytime)
    {
        CurrentTime = MORNING;
    }
    else
    if(ctime>=daytime && ctime<eveningtime)
    {
        CurrentTime = DAY;
    }
    else
    if(ctime>=eveningtime && ctime<nighttime)
    {
        CurrentTime = EVENING;
    }
    else
    if(ctime>=nighttime)
    {
        CurrentTime = NIGHT;
    }
    LastUpdate = clock();
}

// Clean Drops
void CMap::CleanDrops( )
{
    for(UINT j=0;j<DropsList.size();j++)
    {
        CDrop* thisdrop = DropsList.at(j);
        if( time(NULL) - thisdrop->droptime >= 50 )
            DeleteDrop( thisdrop );
    }
}

// Respawn a Monster
void CMap::RespawnMonster( )
{
    //PY new test version. Comment out to use older lmame version
    for(UINT j=0;j<MonsterSpawnList.size();j++)
    {
      CSpawnArea* thisspawn = MonsterSpawnList.at(j);
      //check to see if the spawn is currently empty
      if(thisspawn->MCount <= 0 and thisspawn->ResetSpawn == false)
      {
        //set start of spawn timer. We want it to count from the last death, not last spawn.
        thisspawn->lastRespawnTime = clock();
        thisspawn->ResetSpawn = true;
      }
      //check to see if enough time has passed and that the spawn
      clock_t rtime = clock() - thisspawn->lastRespawnTime;
      if (rtime > thisspawn->respawntime*CLOCKS_PER_SEC && thisspawn->MCount <= 0)
      {
        //spawn has been empty for long enough. Now we need to respawn it.
        //first move the pointer to the next bmob or tmob
        //Yes I know I could simplify this sequence but it's much clearer this way.
        if(thisspawn->MobPointer == 6)
        {
          //tactical mobs cleared. reset to basic mobs again
          thisspawn->MobPointer = 1;
        }
        if(thisspawn->MobPointer == 5)
        {
          //reached the end of bmobs. Time to move onto tmobs
          thisspawn->MobPointer = 6;
        }
        if(thisspawn->MobPointer < 5)
        {
          //increment the pointer to the next basic mob
          thisspawn->MobPointer++;
        }
        thisspawn->MCount = 0; //make sure the count is set to zero
        thisspawn->ResetSpawn = false;
        //OK pointer and counter set. now we spawn em
        if(thisspawn->MobPointer > 5) //Do this only if we are spawning tactical mobs
        {
          //tactical mobs. We spawn both groups at the same time. the monster count becomes the combined total
          if(thisspawn->tcount[1] > 0)
          {
            for (UINT k=0;k<thisspawn->tcount[1];k++) //spawn the tactical monsters within this tmob
            {
              fPoint position = GServer->RandInCircle( thisspawn->points, thisspawn->radius );
              AddMonster( thisspawn->tmob[1], position, 0, NULL, NULL, thisspawn->id );
              thisspawn->MCount++;
            }
          }
          if(thisspawn->tcount[2] > 0)
          {
            for (UINT k=0;k<thisspawn->tcount[2];k++) //spawn the tactical monsters within this tmob
            {
              fPoint position = GServer->RandInCircle( thisspawn->points, thisspawn->radius );
              AddMonster( thisspawn->tmob[2], position, 0, NULL, NULL, thisspawn->id );
              thisspawn->MCount++;
            }
          }
          if(thisspawn->tcount[1] == 0 && thisspawn->tcount[2] == 0)
          {
            //reset Mob pointer to 1 if there are no tactical mobs in the spawn
            thisspawn->MobPointer = 1;
          }

        }
        if(thisspawn->MobPointer <6) //basic mobs
        {
          {
            if(thisspawn->bcount[thisspawn->MobPointer] > 0)
            {
              for (UINT k=0;k<thisspawn->bcount[thisspawn->MobPointer];k++) //spawn the basic monsters within this bmob
              {
                fPoint position = GServer->RandInCircle( thisspawn->points, thisspawn->radius );
                AddMonster( thisspawn->bmob[thisspawn->MobPointer], position, 0, NULL, NULL, thisspawn->id );
                thisspawn->MCount++;
              }
            }
            else //bcount for this spawn is zero so reset mob pointer to first spawn or tactical spawn
            {
              if(thisspawn->tcount[1] == 0 && thisspawn->tcount[2] == 0)
              {
                // No tactical spawns so set pointer to 1
                thisspawn->MobPointer = 1;
              }
              else
              {
                // There are tactical spawns so set pointer to 6. Don't think this will ever happen but added it for completeness
                thisspawn->MobPointer = 6;
              }
            }
          }
        }
      }
    }
    return;
    //This kicks it out before it ever runs the following code.
    //if the spawn didn't contain any entries then MCount will still be zero so the pointer will move on next time around
    //Well that was pretty simple to rewrite.
    //Way more elegant that all that convoluted stuff down below ^_^
    //PY END

    //LMA: daynight
    int last_map = 0;
    bool is_night = false;

    bool lma_debug = false;   //LMA: debugging spawn problem

    for (UINT j = 0; j < MobGroupList.size(); j++)
    {
       CMobGroup* thisgroup = MobGroupList.at(j);

       //LMA: debugging spawn problem
       lma_debug = false;
       if(thisgroup->id == 4589)
       {
           lma_debug=true;
           Log(MSG_INFO,"Spawn %u:: Respawn time?",thisgroup->id);
       }

      clock_t rtime = clock() - thisgroup->lastRespawnTime;
      if (rtime < thisgroup->respawntime*CLOCKS_PER_SEC || thisgroup->active >= thisgroup->limit)
      {
          if(lma_debug)
          {
              Log(MSG_INFO,"Spawn %u:: No respawn (time %u < %u ? or limit %u>=%u ?)",thisgroup->id,rtime,thisgroup->respawntime*CLOCKS_PER_SEC,thisgroup->active,thisgroup->limit);
          }

        continue;
      }

      //LMA: handling day and night this time.
      if(thisgroup->daynight!=0)
      {
        if (last_map!=thisgroup->map)
        {
            CMap* map = GServer->MapList.Index[thisgroup->map];
            last_map=map->id;
            is_night=map->IsNight();
        }

        if((thisgroup->daynight == 2 && !is_night) || (thisgroup->daynight == 1 && is_night))
            continue;
      }

      //LMA: new way.
      //calling in tactictal points if needed.
      CMob *thismob;
      bool groupFull = false;

      if(lma_debug)
      {
          Log(MSG_INFO,"Spawn %u:: Tactical? %u>=%u ?",thisgroup->id,thisgroup->basicKills,thisgroup->tacticalpoints);
      }

      if (thisgroup->tacMobs.size() > 0 && thisgroup->basicKills >= thisgroup->tacticalpoints)
      {
          if(lma_debug)
          {
              Log(MSG_INFO,"Spawn %u:: Tactical time",thisgroup->id);
          }

          //adding ALL tacticals.
          for (UINT k=0;k<thisgroup->tacMobs.size();k++)
          {
            if (thisgroup->curTac >= thisgroup->tacMobs.size())
            {
                thisgroup->curTac = 0;
            }

            thismob = thisgroup->tacMobs.at(thisgroup->curTac);
            thisgroup->basicKills = 0;
            thisgroup->lastKills = 0;
            thisgroup->curTac++;

            //we add the monsters.
            for (UINT i = 0; i < thismob->amount; i++)
            {
                //We load an entire group, not relying to active already there.
                /*
                if (thisgroup->active >= thisgroup->limit)
                {
                    groupFull=true;
                    break;
                }
                */

                //adding the monster as tactical.
                fPoint position = GServer->RandInCircle( thisgroup->point, thisgroup->range );
                AddMonster( thismob->mobId, position, 0, thismob->mobdrop, thismob->mapdrop, thisgroup->id,false,true);
            }

            //We load an entire group, not relying to active already there.
            /*
            if(groupFull)
            {
                break;
            }
            */

          }

      }

      //LMA: basic mobs again? we only add one group.
      //we do it only if enough monsters were killed since last time.
      int last_group = thisgroup->curBasic - 1;
      if(last_group < 0)
      {
          last_group = thisgroup->basicMobs.size() - 1;
          if(last_group < 0)
          {
              last_group = 0;
          }

      }

      if(lma_debug)
      {
          Log(MSG_INFO,"Spawn %u:: Basic group check, this one %u, previous %u, laskills %u>=amount %u ?",thisgroup->id,thisgroup->curBasic,last_group,thisgroup->lastKills,thisgroup->basicMobs.at(last_group)->real_amount);
      }

      //if(thisgroup->lastKills>=thisgroup->basicMobs.at(last_group)->amount)
      if(thisgroup->lastKills >= thisgroup->basicMobs.at(last_group)->real_amount)
      {
          if(!thisgroup->group_ready)
          {
              //extra loop since it's the last dead monster that triggers the respawn time.
              thisgroup->lastRespawnTime = clock();
              thisgroup->group_ready = true;

              if(lma_debug)
              {
                  Log(MSG_INFO,"Spawn %u:: Group wasn't ready to spawn, he is now.",thisgroup->id);
              }


              return;
          }

          thisgroup->group_ready=false;

          thismob = thisgroup->basicMobs.at(thisgroup->curBasic);
          thisgroup->curBasic++;
          if (thisgroup->curBasic >= thisgroup->basicMobs.size())
          {
              thisgroup->curBasic = 0;
          }

          if(lma_debug)
          {
              Log(MSG_INFO,"Spawn %u:: new group will be %u, active for now %u",thisgroup->id,thisgroup->curBasic,thisgroup->active);
          }

        //we add the monsters.
        //LMA: Don't spawn all the mobs at once.
        thismob->real_amount = 1;
        if(thismob->amount != 1)
        {
            thismob->real_amount=GServer->RandNumber(1,thismob->amount);
            if(thismob->real_amount>thismob->amount)
            {
                thismob->real_amount=thismob->amount;
            }

        }

      if(lma_debug)
      {
          Log(MSG_INFO,"Spawn %u:: We'll spawn %u monsters (rand from 1 to %u)",thismob->real_amount,thismob->amount);
      }

        thisgroup->lastKills = 0;
        //LMA: setting the respawn time here.
        thisgroup->lastRespawnTime = clock();

        //for (UINT i = 0; i < thismob->amount; i++)
        for (UINT i = 0; i < thismob->real_amount; i++)
        {
            if (thisgroup->active >= thisgroup->limit)
            {
                  if(lma_debug)
                  {
                      Log(MSG_INFO,"Spawn %u:: Stop spawning monster %u>=%u",thisgroup->id,thisgroup->active,thisgroup->limit);
                  }

                break;
            }

            fPoint position = GServer->RandInCircle( thisgroup->point, thisgroup->range );

            //LMA: We get the monster for test purposes.
            //AddMonster( thismob->mobId, position, 0, thismob->mobdrop, thismob->mapdrop, thisgroup->id );
            CMonster* tempmonster=AddMonster( thismob->mobId, position, 0, thismob->mobdrop, thismob->mapdrop, thisgroup->id );

              if(lma_debug)
              {
                  if (tempmonster != NULL)
                  {
                      Log(MSG_INFO,"Spawn %u:: Spawing monster %u (CID %u) at (%.2f;%.2f) (%u<%u), active now %u",thisgroup->id,thismob->mobId,tempmonster->clientid,position.x,position.y,thismob->mobId,i,thismob->real_amount,thisgroup->active);
                  }
                  else
                  {
                      Log(MSG_WARNING,"Spawn %u:: FAILURE Spawing monster %u at (%.2f;%.2f) (%u<%u), active now %u",thisgroup->id,thismob->mobId,position.x,position.y,thismob->mobId,i,thismob->real_amount,thisgroup->active);
                  }

              }

        }

      }

    }


    return;
}

// Return true if is night
bool CMap::IsNight( )
{
    if(CurrentTime==NIGHT)
        return true;
    return false;
}

CCharacter* CMap::GetCharInMap( unsigned int id )
{
    CMonster* monster = GetMonsterInMap( id );
    if(monster!=NULL)
    {
        //LMA: Setting montype for CCharacter too.
        CCharacter* temp_char=(CCharacter*) monster;
        temp_char->char_montype=monster->montype;
        temp_char->char_owner=monster->owner;
        //return (CCharacter*) monster;
        return temp_char;
    }
    CPlayer* player = GetPlayerInMap( id );
    if(player!=NULL)
        return (CCharacter*) player;
    return NULL;
}
