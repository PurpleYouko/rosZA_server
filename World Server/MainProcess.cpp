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

// Map Process
PVOID MapProcess( PVOID TS )
{
    bool ok_cont=false;
    UINT loopcount=0;
    clock_t time_skill=0;
    bool only_npc=false;    //LMA: AIP is done by NPC even when no player in map.
    bool only_summon=false;  //LMA: test for summons when no one's around (sad is a summon's life ^_^).
    UINT nb_summons_map=0;  //LMA: test for summons when no one's around (sad is a summon's life ^_^).

    //LMA: temp monster used for NPCs.
    fPoint tempPos;
    tempPos.x=0;
    tempPos.y=0;
    tempPos.z=0;
    CMonster* NPCmonster = new (nothrow) CMonster( tempPos, 0, 0, 0, 0  );

    while(GServer->ServerOnline)
    {
        loopcount++;            //geobot: refresh only every 100 cycles
        if (loopcount<100)
        {
            continue;
        }

        loopcount=0;

        pthread_mutex_lock( &GServer->PlayerMutex );
        pthread_mutex_lock( &GServer->MapMutex );

        for(UINT i=0;i<GServer->MapList.Map.size();i++)
        {
            CMap* map = GServer->MapList.Map.at(i);
            //PY spawn TD Mobs
            if(map->id < 100)  //too many damn maps when we include Pedion.
            {
                //PY TD stuff
                if(GServer->WPList[map->id][1].WPType != 0) //this map has waypoints defined
                {
                    if(GServer->TDMap == map->id && GServer->TDSpawnCount > 0) //this is the current TD map and there are unspawned monsters
                    {
                        clock_t etime = clock() - GServer->TDSpawnTimer;
                        if(etime > GServer->TDSpawnRate)
                        {
                            //spawn a monster
                            CMonster* thismon = map->AddMonster(GServer->TDSpawnType, GServer->WPList[map->id][1].pos, 0, NULL, NULL, 0,true );
                            GServer->TDSpawnCount --; //decrement the spawn counter
                            thismon->sp_aip = 9999; //Don't want this mob doing any AIP stuff
                            thismon->TD = true;
                            thismon->NextWayPoint = 2;
                            //set monster's destiny position to first (non-start) waypoint'
                            thismon->Position->destiny.x = GServer->WPList[map->id][2].pos.x;
                            thismon->Position->destiny.y = GServer->WPList[map->id][2].pos.y;
                            thismon->thisnpc->stance = 0;
                            thismon->Stats->Move_Speed = thismon->GetMoveSpeed();
                            thismon->SetStats(false);
                            GServer->TDMonCount ++;
                            Log(MSG_WARNING,"TD Monster spawned with mspd %i. Current TD Monster Count is %i",thismon->Stats->Move_Speed,GServer->TDMonCount);
                            Log(MSG_WARNING,"moving to next position %f %f",thismon->Position->destiny.x,thismon->Position->destiny.y);
                            GServer->TDSpawnTimer = clock();
                            //send a move packet
                            BEGINPACKET( pak, 0x797 );
                            ADDWORD    ( pak, thismon->clientid );
                            ADDWORD    ( pak, 0x0000 );
                            ADDWORD    ( pak, thismon->Stats->Move_Speed );
                            ADDFLOAT   ( pak, thismon->Position->destiny.x*100 );
                            ADDFLOAT   ( pak, thismon->Position->destiny.y*100 );
                            ADDWORD    ( pak, 0xcdcd );
                            ADDBYTE    ( pak, thismon->thisnpc->stance );
                            GServer->SendToVisible(&pak, thismon);
                        }
                    }
                }
            }

            //LMA: test for Union.
            only_npc=false;
            only_summon=false;  //LMA: test for summons when no one's around (sad is a summon's life ^_^).
            nb_summons_map=0;
            if( map->PlayerList.size()<1 )
            {
                only_npc=true;    //LMA: AIP is done by NPC even when no player in map.
                //LMA: doing summons too if there are any in map.
                if(map->nb_summons>0)
                {
                    only_summon=true;
                }
                //continue;
            }

            if (!only_npc||only_summon)
            {
                // Player update //------------------------
                for(UINT j=0;j<map->PlayerList.size();j++)
                {
                    CPlayer* player = map->PlayerList.at(j);
                    if(!player->Session->inGame) continue;

                    if(player->IsDead( ))
                    {
                          player->lastRegenTime=0;
                          player->lastShowTime=0;
                          continue;
                     }

                     //Log(MSG_INFO,"Player %s in map %i, position->map %i",player->CharInfo->charname,map->id,player->Position->Map);

                     #ifdef LMA_SPAWNM
                     //LMA: For use with /spawnmonsters gm command (for drop me).
                     if(player->mdeb!=0&&player->mend!=0&&player->playertime!=0)
                     {
                         if(player->playertime < (UINT)GServer->round((clock( ) - player->lastSpawnUpdate)))
                         {
                             CMap* map = GServer->MapList.Index[player->Position->Map];

                             //let's delete the old one :)
                             if(player->last_monstercid!=0)
                             {
                                CMonster* thismon = GServer->GetMonsterByID(player->last_monstercid, player->Position->Map);
                                if (thismon!=NULL)
                                {
                                    thismon->Stats->HP = -1;
                                    map->DeleteMonster( thismon );
                                }

                                GServer->SendPM(player,"Killing monster %u.",player->last_monstercid);
                                player->ClearObject(player->last_monstercid);

                                /*
                                thismon->Stats->HP = -1;
                                BEGINPACKET( pak, 0x799 );
                                ADDWORD    ( pak, thismon->clientid );
                                ADDWORD    ( pak, thismon->clientid );
                                ADDDWORD   ( pak, thismon->thisnpc->hp*thismon->thisnpc->level );
                                ADDDWORD   ( pak, 16 );
                                GServer->SendToVisible( &pak, thismon );
                                map->DeleteMonster( thismon );
                                */
                                player->playertime=1000;
                                player->last_monstercid=0;
                             }
                             else
                             {
                                 if(player->last_monster==0)
                                 {
                                     player->last_monster=player->mdeb;
                                 }
                                 else
                                 {
                                     player->last_monster++;
                                 }

                                 if (player->last_monster<=player->mend)
                                 {
                                     player->lastSpawnUpdate=clock();
                                    fPoint position;
                                    position.x=player->xx;
                                    position.y=player->yy;
                                    position.z=0;

                                    bool is_ok=false;
                                    CMonster* thismonster=NULL;
                                    if(player->last_monster<GServer->maxNPC)
                                    {
                                        if(GServer->NPCData[player->last_monster]->STLId!=0)
                                        {
                                            is_ok=true;
                                            thismonster=map->AddMonster( player->last_monster, position, 0, NULL, NULL, 0 , true );
                                            if (thismonster==NULL)
                                            {
                                                is_ok=false;
                                            }

                                        }

                                    }

                                    if (is_ok)
                                    {
                                        GServer->SendPM(player,"Spawning monster %u (%u).",player->last_monster,thismonster->clientid);
                                        player->last_monstercid=thismonster->clientid;
                                        player->playertime=2000;
                                    }
                                    else
                                    {
                                        GServer->SendPM(player,"Monster %u does not exist.",player->last_monster);
                                        player->last_monstercid=0;
                                        player->playertime=1000;
                                    }

                                 }
                                 else
                                 {
                                     GServer->SendPM(player,"End spawning from %u to %u.",player->mdeb,player->mend);
                                     player->mdeb=0;
                                     player->mend=0;
                                 }

                             }

                         }

                     }
                     #endif

                     player->RefreshHPMP();         //LMA HP / MP Jumping
                    if(player->UpdateValues( )) //Does nothing except for rides... equals to true if player isn't on the back seat
                        player->UpdatePosition(false);
                    if(player->IsOnBattle( ))
                        player->DoAttack( );

                    //LMA: not used anymore.
                    //player->CheckItems( );

                    player->RefreshBuff( );
                    player->PlayerHeal( );
                    player->Regeneration( );
                    player->CheckPlayerLevelUP( );

                    #ifdef PYCUSTOM
                    //LMA: define PYCUSTOM in the .h to enable custom events. Rebuild your server then.
                    player->CheckPortal( );  //Custom Events
                    player->CheckEvents( );  //Custom Events
                    #endif

                    player->CheckDoubleEquip(); //LMA: Core fix for double weapon and shield
                    player->CheckZulies( );

                    //Fuel handling.
                    if (player->Status->Stance==DRIVING&&(player->last_fuel>0)&&(clock()-player->last_fuel>60000))
                    {
                      //We kill some fuel every now and then :)
                      player->TakeFuel();
                      player->last_fuel=clock();
                    }

                    //LMA: mileage coupon checks.
                    time_t etime=time(NULL);
                    if(player->no_exp&&(etime>=player->timer_no_exp))
                    {
                      BEGINPACKET( pak, 0x702 );
                      ADDSTRING( pak, "[Mileage] Null Xp vanished.");
                      ADDBYTE( pak, 0 );
                      player->client->SendPacket(&pak);
                      player->timer_no_exp=0;
                      player->no_exp=false;
                    }
                    if(player->bonusxp>1&&(etime>=player->timerxp))
                    {
                      BEGINPACKET( pak, 0x702 );
                      ADDSTRING( pak, "[Mileage] Bonus Xp vanished.");
                      ADDBYTE( pak, 0 );
                      player->client->SendPacket(&pak);
                      player->bonusxp=1;
                      player->timerxp=0;
                      player->wait_validation=0;
                    }
                    if(player->bonusddrop > 0 && (etime >= player->timerddrop)) //reset stats after medal expires
                    {
                      BEGINPACKET( pak, 0x702 );
                      ADDSTRING( pak, "[Mileage] Medal of Fortune vanished.");
                      ADDBYTE( pak, 0 );
                      player->client->SendPacket(&pak);
                      player->bonusddrop = 0;                                   // default now 0
                      player->timerddrop = 0;
                      player->wait_validation_ddrop = 0;                        // Set to default 0
                    }
                    if(player->bonusstatdrop>1&&(etime>=player->timerstatdrop))
                    {
                      BEGINPACKET( pak, 0x702 );
                      ADDSTRING( pak, "[Mileage] Medal of Excellence vanished.");
                      ADDBYTE( pak, 0 );
                      player->client->SendPacket(&pak);
                      player->bonusstatdrop=1;
                      player->timerstatdrop=0;
                      player->wait_validation_statdrop=0;
                    }
                    if(player->bonusgraydrop>0&&(etime>=player->timergraydrop))
                    {
                      BEGINPACKET( pak, 0x702 );
                      ADDSTRING( pak, "[Mileage] Medal of Retrieval vanished.");
                      ADDBYTE( pak, 0 );
                      player->client->SendPacket(&pak);
                      player->bonusgraydrop=0;
                      player->timergraydrop=0;
                      player->wait_validation_graydrop=0;
                    }
                    if(player->Shop->ShopType>0&&(etime>=player->Shop->mil_shop_time))
                    {
                      BEGINPACKET( pak, 0x702 );
                      ADDSTRING( pak, "[Mileage] Mileage shop expired !");
                      ADDBYTE( pak, 0 );
                      player->client->SendPacket(&pak);
                      player->Shop->ShopType=0;
                      player->Shop->mil_shop_time=0;
                    }
                    if(player->Shop->ShopType>0&&(etime>=player->Shop->mil_shop_time))
                    {
                      BEGINPACKET( pak, 0x702 );
                      ADDSTRING( pak, "[Mileage] Mileage shop expired !");
                      ADDBYTE( pak, 0 );
                      player->client->SendPacket(&pak);
                      player->Shop->ShopType=0;
                      player->Shop->mil_shop_time=0;
                    }
                    if(player->storageExpanded&&(etime>=player->timerexapnsion))
                    {
                      BEGINPACKET( pak, 0x702 );
                      ADDSTRING( pak, "[Item Mall] Storage Expansion expired!");
                      ADDBYTE( pak, 0 );
                      player->client->SendPacket(&pak);
                      player->storageExpanded=false;
                      player->timerexapnsion=0;
                    }

                }


                // Monster update //------------------------
                pthread_mutex_lock( &map->MonsterMutex );

                for(UINT j=0;j<map->MonsterList.size();j++)
                {
                    CMonster* monster = map->MonsterList.at(j);
                    //LMA: only summon ?
                    if (only_summon&&!monster->IsSummon())
                    {
                        continue;
                    }
                    else if(only_summon)
                    {
                        nb_summons_map++;
                    }

                    //PY doing TD stuff
                    if(monster->TD && map->id < 100)  //too many damn maps when we include Pedion.)
                    {
                        //if monster is at the current target waypoint it needs to head to the next one or commit suicide.
                        monster->UpdatePosition(false);
                        if(monster->Position->current.x == monster->Position->destiny.x && monster->Position->current.y == monster->Position->destiny.y)
                        { //reached a waypoint
                            Log(MSG_WARNING,"reached waypoint.current position %f %f",monster->Position->destiny.x,monster->Position->destiny.y);
                            if(GServer->WPList[map->id][monster->NextWayPoint].WPType == 3)
                            {
                                //it's the final point
                                //now remove monster
                                monster->Stats->HP = -1;
                                monster->suicide = true;
                                GServer->TDMonCount --; //decrement TD monster count
                                Log(MSG_WARNING,"TD monster reached target and suicided. Current TD Monster Count is %i",GServer->TDMonCount);
                                //Also need to do stuff to reduce life of target. todo later
                                //delete monster
                                BEGINPACKET( pak, 0x799 );
                                ADDWORD    ( pak, monster->clientid );
                                ADDWORD    ( pak, monster->clientid );
                                ADDDWORD   ( pak, monster->thisnpc->hp * monster->thisnpc->level );
                                ADDDWORD   ( pak, 16 );
                                GServer->SendToVisible( &pak, monster );
                            }
                            else //hasn't arrived at final point. move to next
                            {
                                Log(MSG_WARNING,"TD monster reached waypoint %i",monster->NextWayPoint);
                                monster->NextWayPoint ++; //increment the waypoint
                                monster->Position->destiny.x = GServer->WPList[map->id][monster->NextWayPoint].pos.x;
                                monster->Position->destiny.y = GServer->WPList[map->id][monster->NextWayPoint].pos.y;
                                Log(MSG_WARNING,"moving to next position %f %f",monster->Position->destiny.x,monster->Position->destiny.y);
                            }
                        }
                        //send a move packet whether the monster reached waypoint or not
                        BEGINPACKET( pak, 0x797 );
                        ADDWORD    ( pak, monster->clientid );
                        ADDWORD    ( pak, 0x0000 );
                        ADDWORD    ( pak, monster->Stats->Move_Speed );
                        ADDFLOAT   ( pak, monster->Position->destiny.x*100 );
                        ADDFLOAT   ( pak, monster->Position->destiny.y*100 );
                        ADDWORD    ( pak, 0xcdcd );
                        ADDBYTE    ( pak, monster->thisnpc->stance );
                        GServer->SendToVisible(&pak, monster);
                    }

                    //PY Checking target for dead monster.
                    //if(monster->Stats->HP<0)
                    //{
                        //Log(MSG_DEBUG,"A monster %i is dead in map %i (target %i)",monster->montype,map->id,monster->Battle->target);
                    //}
                    //PY Yup it still has a target at this point.
                    if(only_summon&&nb_summons_map==0)
                    {
                        map->nb_summons=0;
                    }

                    //LMA: AIP CODE
                    if(monster->hitcount == 0xFF && !monster->TD)//this is a delay for new monster spawns this might olso fix invisible monsters(if they attack directly on spawning the client dosn't get the attack packet(its not in it's visible list yet))
                    {                                            //PY don't do this for TD monsters
                        if(1000 < (UINT)GServer->round((clock( ) - monster->lastAiUpdate)))
                        {
                            //LogDebug("DoAIP mainprocess monster loop %i",monster->thisnpc->AI);
                            monster->hitcount = 0;
                            monster->DoAi(monster->thisnpc->AI, 0);
                            monster->lastAiUpdate=clock();
                        }

                    }
                    //END AIP CODE

                    //LMA: maps (using grid now?)
                     ok_cont=false;
                     if (GServer->Config.testgrid!=0)
                     {
                         ok_cont = monster->PlayerInGrid( );
                     }
                     else
                     {
                         ok_cont = monster->PlayerInRange( );
                     }

                     if (!ok_cont)
                     {
                        //LMA: Perhaps not necessary but who knows...
                        if (monster->IsSummon())
                        {
                            monster->SummonUpdate(monster,map, j);
                            continue;
                        }

                        if(monster->IsDead( ))
                        {
                            //LMA: we do it only if the monster didn't commit suicide, for Chief Turak for now...
                            if(monster->montype!=1830)
                            {
                                LogDebugPriority(3);
                                LogDebug("DoAIP mainprocess monster %u is dead %i",monster->montype,monster->thisnpc->AI);
                                LogDebugPriority(4);
                                monster->DoAi(monster->thisnpc->AI, 5);

                            }
                            else
                            {
                                if(monster->suicide)
                                {
                                    LogDebugPriority(3);
                                    LogDebug("We DON'T DoAIP mainprocess monster %u is dead %i, because chief turak committed suicide.",monster->montype,monster->thisnpc->AI);
                                    LogDebugPriority(4);
                                }
                                else
                                {
                                    LogDebugPriority(3);
                                    LogDebug("DoAIP mainprocess monster chief turak %u is dead %i",monster->montype,monster->thisnpc->AI);
                                    LogDebugPriority(4);
                                    monster->DoAi(monster->thisnpc->AI, 5);
                                }

                            }

                            monster->OnDie( );
                        }

                        continue;
                    }

                    //LMA: daynight stuff :) kinda vampire code for spawns ^_^
                    //PY: NOT Necessary. AIP takes care of suicides for night only monsters
                    //if((monster->daynight!=0)&&((monster->daynight==2&&!map->IsNight())||(monster->daynight==1&&map->IsNight())))
                    //{
                        //Bye bye little monster...
                        //map->DeleteMonster( monster, true, j );
                        //continue;
                    //}

                    //General monsters===============================================================
                    //LMA: moved to beginning...
                    //if(!monster->PlayerInRange( )) continue;

                   //PY utterly pointless. UpdateValues always returns true
                    //if(!monster->UpdateValues( ))
                    //{
                     //   continue;
                    //}
                    //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess before update) Enemy->target = %i",monster->Battle->target);
                    monster->UpdatePosition(monster->stay_still);
                    //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess after update) Enemy->target = %i",monster->Battle->target);

                    if(monster->IsOnBattle( )&& !monster->IsDead()) //was sending the dead monster back through the doattack loop and removing its target so AIP 5 was broken
                    {
                        //monster->DoAttack( );
                        //LMA: AIP Timer.
                        //if(2000<(UINT)GServer->round((clock( ) - monster->lastAiUpdate)))

                        if(monster->thisnpc->AiTimer==0)
                        {
                            monster->thisnpc->AiTimer=6000;
                            //Log(MSG_WARNING,"Monster %i hadn't timer, file AI=%i",monster->montype,monster->thisnpc->AI);
                        }

                        if(monster->thisnpc->AiTimer<(UINT)GServer->round((clock( ) - monster->lastAiUpdate)) && !monster->TD)
                        {  //PY no AI stuff for TD monsters
                            /*Log(MSG_INFO,"Sending back monster HP amount");
                            BEGINPACKET( pak, 0x79f );
                            ADDWORD    ( pak, monster->clientid );
                            ADDDWORD   ( pak, monster->Stats->HP );
                            GServer->SendToVisible( &pak, monster );*/

                            //LogDebug("DoAIP mainprocess monster on battle %i,2",monster->thisnpc->AI);

                            if(!monster->IsBonfire())
                            {

                                  //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess before AI 2) Enemy->target = %i",monster->Battle->target);
                                  monster->DoAi(monster->thisnpc->AI, 2);
                                  // if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess after AI 2) Enemy->target = %i",monster->Battle->target);
                            }
                            else
                            {
                                //LMA: Bonfires are never on battle, peace and love ^_^
                                 monster->DoAi(monster->thisnpc->AI, 1);
                                 // if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess thinks it's a bonfire) Enemy->target = %i",monster->Battle->target);
                            }

                            monster->lastAiUpdate = clock();
                            //Log(MSG_INFO,"Monster type: %i current HP: %i",monster->montype, monster->Stats->HP);


                        }
                        else
                        {
                             //Log(MSG_INFO,"Monster doing attack instead of AIP.");
                            //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess before doattack) Enemy->target = %i",monster->Battle->target);
                            if(!monster->TD) //TD monsters do not fight back
                                monster->DoAttack( );
                            //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess after doattack) Enemy->target = %i",monster->Battle->target);
                             //LMA: We clear battle for bonfires.
                             if(monster->IsBonfire())
                             {
                                 ClearBattle(monster->Battle);
                                 //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess isbonfire) Enemy->target = %i",monster->Battle->target);
                             }

                        }
                        //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess onbattle end) Enemy->target = %i",monster->Battle->target);
                    }
                    else if(!monster->IsOnBattle() && !monster->IsDead( ))
                    {
                        //LMA: AIP Timer.
                        //if(2000<(UINT)GServer->round((clock( ) - monster->lastAiUpdate)))

                        if(monster->thisnpc->AiTimer==0)
                        {
                            monster->thisnpc->AiTimer=6000;
                            Log(MSG_WARNING,"monster (2) %i hadn't timer, file AI=%i",monster->montype,monster->thisnpc->AI);
                        }

                        if(monster->thisnpc->AiTimer<(UINT)GServer->round((clock( ) - monster->lastAiUpdate)) && !monster->TD)
                        {  //PY no AI stuff for TD monsters
                            /*Log(MSG_INFO,"Sending back monster HP amount");
                            BEGINPACKET( pak, 0x79f );
                            ADDWORD    ( pak, monster->clientid );
                            ADDDWORD   ( pak, monster->Stats->HP );
                            GServer->SendToVisible( &pak, monster );*/

                            //LogDebug("DoAIP mainprocess monster iddle? %i,1",monster->thisnpc->AI);
                            monster->DoAi(monster->thisnpc->AI, 1);
                            monster->lastAiUpdate = clock();
                        }

                    }
                    else
                    {
                        /*
                        //LMA: Done in summonupdate now.
                        if(monster->IsSummon( ))
                        {// if is summon and is not attacking we reduce his life 1%
                            time_t elapsedTime = time(NULL) - monster->lastLifeUpdate;
                            if(elapsedTime>=5) // every 5 seconds
                            {
                                monster->Stats->HP -= (long int)ceil(monster->GetMaxHP( )/100);
                                Log(MSG_WARNING,"Bye bye life summon :) %I64i",monster->Stats->HP);
                                monster->lastLifeUpdate = time(NULL);
                                if(monster->Stats->HP<=0)
                                {
                                    map->DeleteMonster( monster, true, j ); continue;
                                }

                            }

                        }
                        */

                    }
                    //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess before buff) Enemy->target = %i",monster->Battle->target);
                    monster->RefreshBuff( );
                    //if(monster->IsDead( ))Log(MSG_INFO,"Enemy died. (mainprocess after buff) Enemy->target = %i",monster->Battle->target);

                    //osprose
                    if (monster->IsSummon())
                    {
                        monster->SummonUpdate(monster,map, j);
                        continue;
                    }

                    if(monster->IsDead( ))
                    {
                        //Log(MSG_INFO,"Enemy died. (mainprocess isdead) Enemy->target = %i",monster->Battle->target);  //print out the eney's target
                        //LMA: we do it only if the monster didn't commit suicide, for Chief Turak for now...
                        if(monster->montype!=1830)
                        {
                            LogDebugPriority(3);
                            LogDebug("DoAIP mainprocess monster %u is dead %i",monster->montype,monster->thisnpc->AI);
                            LogDebugPriority(4);
                            monster->DoAi(monster->thisnpc->AI, 5);

                        }
                        else
                        {
                            if(monster->suicide)
                            {
                                LogDebugPriority(3);
                                LogDebug("We DON'T DoAIP mainprocess monster %u is dead %i, because chief turak committed suicide.",monster->montype,monster->thisnpc->AI);
                                LogDebugPriority(4);
                            }
                            else
                            {
                                LogDebugPriority(3);
                                LogDebug("DoAIP mainprocess monster chief turak %u is dead %i",monster->montype,monster->thisnpc->AI);
                                LogDebugPriority(4);
                                monster->DoAi(monster->thisnpc->AI, 5);
                            }

                        }

                        monster->OnDie( );
                    }

                }

            }

            if(only_npc&&!only_summon)
            {
                pthread_mutex_lock( &map->MonsterMutex );
            }

            //LMA: AIP for NPC.
            for(UINT j=0;j<map->NPCList.size();j++)
            {
                CNPC* npc = map->NPCList.at(j);

                //LMA: We don't worry about IFO Objects...
                if(npc->npctype>10000)
                {
                    continue;
                }

                if(npc->thisnpc->AI != 0)
                {
                    //check every minute. Conditions seem to be based on 6 minute segments
                    //LMA: untrue for some NPCs, special case for UW...
                    bool is_time_ok=false;

                    int delay=60000;    //each AIP 60 seconds.
                    //LMA: AIP Timer.
                    delay=npc->thisnpc->AiTimer;

                    if(npc->thisnpc->AiTimer==0)
                    {
                        npc->thisnpc->AiTimer=60000;
                        Log(MSG_WARNING,"NPC %i hadn't timer, file AI=%i",npc->npctype,npc->thisnpc->AI);
                    }

                    //Leum, for Union War (no need to do his stuff always).
                    if(npc->npctype==1113&&GServer->ObjVar[1113][1]>0)
                    {
                        //LogDebug("Doing an update for Leum each 10 seconds since UW is on");
                        delay=10000;
                    }

                    //Walls for map 66 (no need to do his stuff always)
                    if(npc->npctype>=1024&&npc->npctype<=1027&&GServer->ObjVar[1249][2]>0&&GServer->ObjVar[1249][2]<=90)
                    {
                        //LogDebug("Doing an update for Wall %i each second quest from Hope is on",npc->npctype);
                        delay=1000;
                    }

                    //Hope map 66 (no need to do his stuff always)
                    if(npc->npctype==1249&&GServer->ObjVar[1249][2]>0&&GServer->ObjVar[1249][2]<=90)
                    {
                        //LogDebug("Doing an update for Hope each 10 seconds quest from Hope is on",npc->npctype);
                        delay=10000;
                    }

                    //Williams
                    if(npc->npctype==1075)
                    {
                        //Each 5 minutes
                        delay=300000;
                    }

                    //LMA END

                     //if(60000<(UINT)GServer->round((clock( ) - npc->lastAiUpdate)))
                     //if(is_time_ok)
                     if(delay<(UINT)GServer->round((clock( ) - npc->lastAiUpdate)))
                     {
                         //Log(MSG_INFO,"Doing AIP for NPC %i",npc->npctype);

                         //LMA: Debug Log
                         /*LogDebugPriority(3);
                         LogDebug("We do AIP for NPC %i",npc->npctype);
                         LogDebugPriority(4);*/

                         CNPCData* thisnpc = GServer->GetNPCDataByID( npc->npctype );
                         if(thisnpc == NULL)
                         {
                             Log( MSG_WARNING, "Invalid montype %i", npc->npctype );
                             continue;
                         }

                         //LMA: before this temp monster was created and deleted every time, now we only do it once...
                         //new code, we overwrite the temp monster each time rather than creating / deleting him each time.
                        NPCmonster->Position->source = npc->pos;
                        NPCmonster->Position->current = npc->pos;
                        NPCmonster->Position->destiny = npc->pos;
                         NPCmonster->montype=npc->npctype;
                         NPCmonster->Position->Map=map->id;
                         NPCmonster->Position->lastMoveTime = clock( );
                         NPCmonster->SpawnTime = clock( );
                         NPCmonster->lastSighCheck = clock( );
                         NPCmonster->lastLifeUpdate = time(NULL);

                         //old code:
                         //CMonster* NPCmonster = new (nothrow) CMonster( npc->pos, npc->npctype, map->id, 0, 0  );
                         NPCmonster->aip_npctype=npc->npctype;
                         NPCmonster->aip_clientid=npc->clientid;
                         NPCmonster->thisnpc = thisnpc;

                         int lma_previous_eventID=npc->thisnpc->eventid;
                         //Log(MSG_INFO,"XCIDAIBEGIN NPC %i map %i cid %i",npc->npctype,map->id,npc->clientid);

                         NPCmonster->DoAi(NPCmonster->thisnpc->AI, 1);
                         //Log(MSG_INFO,"XCIDAIEND NPC %i map %i cid %i",npc->npctype,map->id,npc->clientid);

                        //Williams (temple of Oblivion)
                        if(npc->npctype==1075)
                        {
                            //each 5 minutes
                            //saving values for him
                            if(GServer->LastTempleAccess[0]!=GServer->ObjVar[npc->npctype][0]||GServer->LastTempleAccess[1]!=GServer->ObjVar[npc->npctype][1])
                            {
                                GServer->DB->QExecute("UPDATE list_npcs SET eventid=%i, extra_param=%i WHERE type=1075",GServer->ObjVar[1075][0],GServer->ObjVar[1075][1]);
                                /*Log(MSG_WARNING,"Doing an update for Williams each 5 minutes, values changed (%i->%i, %i->%i)",
                                GServer->LastTempleAccess[0],GServer->ObjVar[npc->npctype][0],
                                GServer->LastTempleAccess[1],GServer->ObjVar[npc->npctype][1]);*/
                            }
                            /*else
                            {
                                Log(MSG_WARNING,"Doing an update for Williams each 5 minutes.");
                            }*/

                            GServer->LastTempleAccess[0]=GServer->ObjVar[npc->npctype][0];
                            GServer->LastTempleAccess[1]=GServer->ObjVar[npc->npctype][1];
                        }

                         //LMA: check if eventID changed, if we do it in AIP conditions / actions, it just fails...
                         if (lma_previous_eventID!=NPCmonster->thisnpc->eventid)
                         {
                            //Log(MSG_WARNING,"(1)Event ID not the same NPC %i from %i to %i in map %i, npc->thisnpc->eventid=%i !",npc->npctype,lma_previous_eventID,NPCmonster->thisnpc->eventid,map->id,npc->thisnpc->eventid);
                            LogDebugPriority(3);
                            LogDebug("(1)Event ID not the same NPC %i from %i to %i in map %i, npc->thisnpc->eventid=%i !",npc->npctype,lma_previous_eventID,NPCmonster->thisnpc->eventid,map->id,npc->thisnpc->eventid);
                            LogDebugPriority(4);
                            npc->thisnpc->eventid=NPCmonster->thisnpc->eventid;
                            npc->event=npc->thisnpc->eventid;
                            //LMA: We have to change the event ID here since we didn't send the clientID :(
                            BEGINPACKET( pak, 0x790 );
                            ADDWORD    ( pak, npc->clientid );
                            ADDWORD    ( pak, npc->thisnpc->eventid );
                            GServer->SendToAllInMap(&pak,map->id);
                         }

                        //LMA: We don't clear anymore, too much hassle :(
                        //We declare the temporary monster just once...
                        /*GServer->ClearClientID(NPCmonster->clientid);
                         delete NPCmonster;*/
                         npc->lastAiUpdate = clock();
                     }

                }

                //LMA: Sometimes another NPC does the job for you.
                if(npc->thisnpc->eventid!=GServer->ObjVar[npc->npctype][0])
                {
                    int new_event_id=GServer->ObjVar[npc->npctype][0];
                    LogDebugPriority(3);
                    //Log(MSG_WARNING,"(2)Event ID not the same NPC %i from %i to %i in map %i, npc->thisnpc->eventid=%i !",npc->npctype,npc->thisnpc->eventid,new_event_id,map->id,npc->thisnpc->eventid);
                    LogDebug("(2)Event ID not the same NPC %i from %i to %i in map %i, npc->thisnpc->eventid=%i !",npc->npctype,npc->thisnpc->eventid,new_event_id,map->id,npc->thisnpc->eventid);
                    LogDebugPriority(4);
                    npc->thisnpc->eventid=new_event_id;
                    npc->event=new_event_id;
                    //LMA: We have to change the event ID here since we didn't send the clientID :(
                    BEGINPACKET( pak, 0x790 );
                    ADDWORD    ( pak, npc->clientid );
                    ADDWORD    ( pak, npc->thisnpc->eventid );
                    GServer->SendToAllInMap(&pak,map->id);
                }

            }

            pthread_mutex_unlock( &map->MonsterMutex );
        }

        pthread_mutex_unlock( &GServer->MapMutex );
        pthread_mutex_unlock( &GServer->PlayerMutex );

        #ifdef _WIN32
        Sleep(GServer->Config.MapDelay);
        #else
        usleep(GServer->Config.MapDelay);
        #endif
    }
    pthread_exit( NULL );

    //LMA: we delete the temporary monster.
    GServer->ClearClientID(NPCmonster->clientid);
    delete NPCmonster;
}


// Visibility Process
PVOID VisibilityProcess(PVOID TS)
{
    while(GServer->ServerOnline)
    {
        pthread_mutex_lock( &GServer->PlayerMutex );
        pthread_mutex_lock( &GServer->MapMutex );
        for(UINT i=0;i<GServer->MapList.Map.size();i++)
        {
            CMap* map = GServer->MapList.Map.at(i);
            if( map->PlayerList.size()<1 )
                continue;
            for(UINT j=0;j<map->PlayerList.size();j++)
            {
                CPlayer* player = map->PlayerList.at(j);
                if(!player->Session->inGame)
                {
                    continue;
                }
                if(!player->VisiblityList()) Log(MSG_WARNING, "Visibility False: %u", player->clientid );
                if( GServer->Config.AUTOSAVE == 1 )
                {
                    clock_t etime = clock() - player->lastSaveTime;
                    if( etime >= GServer->Config.SAVETIME*1000 )
                    {
                        player->savedata( );
                        player->lastSaveTime = clock();
                    }
                }
                if( GServer->Config.AutoDP == 1)
                {
                    clock_t etime = clock() - player->lastDpTime;
                    if( etime >= GServer->Config.TimeDP*1000 )
                    {
                        player->DP( );
                        player->lastDpTime = clock();
                    }
                }
            }
        }

        if(GServer->Config.warningSysActive == 1)
        {

            clock_t etime = clock() - GServer->Config.lastWCountRed;
            if(etime >= GServer->Config.ban_timer*1000)
            {
                BanTracker();
                GServer->Config.lastWCountRed = clock();

            }
        }

        if(GServer->Config.doubleExpActive == 1) // Double exp for map
        {
            clock_t last = clock() - GServer->Config.lastMapTime;
            clock_t lastAnn = clock() - GServer->Config.lastAnnTime;
            if( last >= GServer->Config.doubleExpTime*1000)
            {
                Log(MSG_INFO, "Double exp for a map activation pending...");
                doubleExpMapActivate();
            }else if(last >= GServer->Config.mapTimer*1000 && GServer->Config.mapFlag == false)
            {
                Log(MSG_INFO, "Double exp for a map deactivation pending...");
                doubleExpMapDeactivate();
            }

            if(lastAnn >= 600000 && GServer->Config.mapFlag == false)
            {
                doubleExpAnnounce();
            }
        }

		if(GServer->Config.LottoActive == 1) // Lotto Ann
        {
            clock_t lastLottoAnnTime = clock() - GServer->Config.lastLastLottoAnn;
            if(lastLottoAnnTime >= 600000)
            {
                LottoAnn();
                get_grandPrize();
            }
        }

        if(GServer->Config.AdvertiseActive == 1)
        {
            clock_t lastAdverTime = clock() - GServer->Config.AdvertiseTime;
            if(lastAdverTime >= GServer->Config.AdvertiseClock*1000)
            {
                Advertise();
                GServer->Config.AdvertiseTime = clock();
            }
        }

        pthread_mutex_unlock( &GServer->MapMutex );
        pthread_mutex_unlock( &GServer->PlayerMutex );
        #ifdef _WIN32
        Sleep(GServer->Config.VisualDelay);
        #else
        usleep(GServer->Config.VisualDelay);
        #endif
    }
pthread_exit(NULL);
}

// World Process
PVOID WorldProcess( PVOID TS )
{
    while( GServer->ServerOnline )
    {
        pthread_mutex_lock( &GServer->MapMutex );
        for(UINT i=0;i<GServer->MapList.Map.size();i++)
        {
            CMap* map = GServer->MapList.Map.at(i);
            if( map->PlayerList.size()<1 )
                continue;
            map->UpdateTime( );
            pthread_mutex_lock( &map->DropMutex );
            map->CleanDrops( );
            pthread_mutex_unlock( &map->DropMutex );
            pthread_mutex_lock( &map->MonsterMutex );
            map->RespawnMonster( );
            pthread_mutex_unlock( &map->MonsterMutex );
        }
        pthread_mutex_unlock( &GServer->MapMutex );
        GServer->RefreshFairy( );
        #ifdef _WIN32
        Sleep(GServer->Config.WorldDelay);
        #else
        usleep(GServer->Config.WorldDelay);
        #endif
    }
    pthread_exit(NULL);
}

// Shutdown Server Process
PVOID ShutdownServer(PVOID TS)
{
    int minutes = (int)TS;
    #ifdef _WIN32
    Sleep(minutes*60000);
    #else
    usleep(minutes*60000);
    #endif
    Log( MSG_INFO, "Saving User Information... " );
    GServer->DisconnectAll();
    Log( MSG_INFO, "Waiting Process to ShutDown... " );
    GServer->ServerOnline = false;
    int status,res;
    res = pthread_join( GServer->WorldThread[0], (PVOID*)&status );
    if(res)
    {
        Log( MSG_WARNING, "World thread can't be joined" );
    }
    else
    {
        Log( MSG_INFO, "World thread closed." );
    }
    res = pthread_join( GServer->WorldThread[1], (PVOID*)&status );
    if(res)
    {
        Log( MSG_WARNING, "Visibility thread can't be joined" );
    }
    else
    {
        Log( MSG_INFO, "Visibility thread closed." );
    }
    res = pthread_join( GServer->MapThread[0], (PVOID*)&status );
    if(res)
    {
        Log( MSG_WARNING, "Map thread can't be joined" );
    }
    else
    {
        Log( MSG_INFO, "Map thread closed." );
    }
    Log( MSG_INFO, "All Threads Closed." );
    GServer->isActive = false;
    pthread_exit(NULL);
}
