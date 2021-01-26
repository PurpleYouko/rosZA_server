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
#include "character.h"
#include "worldserver.h"

void CCharacter::DoAttack( )
{
     /*
     if(IsSummon())
       Log(MSG_INFO,"Someone attacks type=%i,skillid=%i",Battle->atktype,Battle->skillid);
     */

    //osptest begin
	CCharacter* Enemy = GetCharTarget( );
    if(IsSummon())
    {
        
        if(Enemy == NULL && Battle->atktype != SKILL_AOE && Battle->atktype != BUFF_AOE)
        {
            //LogDebug("No Enemy found");
            ClearBattle( Battle );
            return;
        }
        //LogDebug("Client id: %i Enemy client id: %i",clientid, Enemy->clientid);
        if(this == Enemy) //summoned monster is attacking itself. It has been observed to happen
        {
            //Log(MSG_INFO,"Clearing Battle for this summon");
            ClearBattle( Battle );
            return;
        }
    }
    //osptest end

    //If we're dead, why are we still fighting?
    if(IsDead())
    {
        ClearBattle(Battle);
        return;
    }
	// Likewise if the Enemy is dead there is no point continuing to pound him
	if (Enemy->IsDead ()) {
		ClearBattle (Battle);
		return;
	}

    if(IsMonster())  //PY Make sure TD monsters don't attack
    {
        CMonster* monster = GServer->GetMonsterByID(clientid, Position->Map);
        if(monster->TD)
            return;
    }

    CMap* map = GServer->MapList.Index[Position->Map];
    switch(Battle->atktype)
    {
        case NORMAL_ATTACK://normal attack
        {
            CCharacter* Enemy = GetCharTarget( );
            if(Enemy == NULL)
            {
                ClearBattle( Battle );
                return;
            }

            if(Enemy == this)
            {
                //Log(MSG_INFO,"WTF?? I AM trying to attack myself");
                ClearBattle( Battle );
            }

            if(IsTargetReached( Enemy ) && CanAttack( ))
            {
                if(GServer->ServerDebug)
                {
                    if(IsMonster()){
                        Log(MSG_INFO,"monster Doing a normal attack");
                    }
                    else {
                        Log(MSG_INFO,"player Doing a normal attack");
                    }
                }

                NormalAttack( Enemy );
                //CSkills* skill = GServer->GetSkillByID( 1117 );     //permafrost chill Mage spell just for giggles. Should do shit loads of damage
                //SkillAttack( Enemy, skill );

                if (Enemy->IsMonster()) // do monster AI script when monster is attacked. NO TIMER here
                {
                    CMonster* monster = GServer->GetMonsterByID(Enemy->clientid, Enemy->Position->Map);

                    if(monster != NULL)
                    {
                        if(!monster->IsDead())
                        {
                            if(GServer->ServerDebug)
                                Log(MSG_INFO,"Monster %i is attacked, doing his AI 3, HP: %I64i",monster->clientid,monster->Stats->HP);
                            if(!monster->TD) //PY no AI stuff for TD monsters
                                monster->DoAi(monster->thisnpc->AI, 3);
						}
                    }
                }
            }
        }
        break;
        case SKILL_ATTACK://skill attack
        {
            CCharacter* Enemy = GetCharTarget( );
            if(Enemy==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            CSkills* skill = GServer->GetSkillByID( Battle->skillid );
            if(skill==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            if(IsTargetReached( Enemy, skill ) && CanAttack( ))
            {
                Log(MSG_INFO,"DoAttack, SKILL_ATTACK, target (%.2f,%.2f)",Position->aoedestiny.x,Position->aoedestiny.y);
                SkillAttack( Enemy, skill );

				//Some skill attacks has aoe effects
				if(skill->aoerange>0)
                {
                    Position->aoedestiny=Enemy->Position->current;
					if(GServer->ServerDebug)
                        Log(MSG_INFO,"[DoAttack] In AOE_TARGET time for AoeSkill");
                    Battle->atktype=AOE_TARGET;
                    AoeSkill( skill, NULL );    //LMA: no specific Enemy in Zone.
                }

                if (Enemy->IsMonster())
                {
                    CMonster* monster = GServer->GetMonsterByID(Enemy->clientid, Enemy->Position->Map);

                    if(monster!=NULL && !monster->TD) //PY No AI stuff for TD monsters
                        monster->DoAi(monster->thisnpc->AI, 3);
                }

            }

        }
        break;
        case MONSTER_SKILL_ATTACK://LMA: monster skill attack
        {
            CCharacter* Enemy = GetCharTarget( );
            if(Enemy==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            CSkills* skill = GServer->GetSkillByID( Battle->skillid );
            if(skill==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            if(IsTargetReached( Enemy, skill ) && CanAttack( ))
                SkillAttack( Enemy, skill );
        }
        break;
        case SUMMON_BUFF://Summon buffs (support)
        {
            //LMA: For summon that buff player :)
            if (Battle->bufftarget==0)
               break;
            CCharacter* master = GetCharBuffTarget( );
            if(master==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            CSkills* skill = GServer->GetSkillByID( Battle->skillid );
            if(skill==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            if(IsTargetReached( master, skill ) && CanAttack( ))
                SummonBuffSkill( master, skill );
        }
        break;
        case MONSTER_SKILL_BUFF://LMA: monster buffs
        {
            CCharacter* Enemy= GetCharTarget( );
            if(Enemy==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            CSkills* skill = GServer->GetSkillByID( Battle->skillid );
            if(skill==NULL)
            {
                ClearBattle( Battle );
                return;
            }

            if(IsTargetReached( Enemy, skill ) && CanAttack( ))
            {
                BuffSkill( Enemy, skill );
            }

        }
        break;
       case SKILL_BUFF://buffs
        {
            //osptest
            CCharacter* Enemy = GetCharTarget( );
            if(Enemy == NULL)
            {
                //LogDebug("this target is NULL");
                ClearBattle( Battle );
                return;
            }

            CSkills* skill = GServer->GetSkillByID( Battle->skillid );

            if(skill == NULL)
            {
                //LogDebug("this skill is NULL");
                //ClearBattle( Battle );
                return;
            }

            //LogDebug("Called skill buff code for skill %i",skill->id);
            if(IsTargetReached( Enemy, skill ) && CanAttack( ))
            {
                //LogDebug("Skill successfully cast");
                //BuffSkill( Enemy, skill );

                if(IsPlayer())
                {
                    CPlayer* plkiller=(CPlayer*) this;
                    //Log(MSG_WARNING,"Before Buff skill Player %i (%s) targets cid %i",clientid,plkiller->CharInfo->charname,plkiller->Battle->target);
                }

                BuffSkill( Enemy, skill );

                if(IsPlayer())
                {
                    CPlayer* plkiller=(CPlayer*) this;
                    //Log(MSG_WARNING,"After Buff skill Player %i (%s) targets cid %i",clientid,plkiller->CharInfo->charname,plkiller->Battle->target);
                }

            }
            //osptest end
        }
        break;  //LMA: break was missing?
        case SKILL_AOE:
        case SKILL_SELF:
        case AOE_TARGET:
        {
            CCharacter* Enemy = NULL;
            CSkills* skill = GServer->GetSkillByID( Battle->skillid );
            if(skill==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            if(Battle->atktype==AOE_TARGET)
            {
                //LMA 2008/09/02: new version, the target is a zone, not a monster... so we stick with aoedestiny ;)
                float distance=GServer->distance( Position->current, Position->aoedestiny );
                //Log(MSG_INFO,"distance %f / range: %i, current: %.2f,%.2f, aoe: %.2f,%.2f",distance,skill->range,Position->current.x,Position->current.y,Position->aoedestiny.x,Position->aoedestiny.y);
                //osprose: canattacl
                if(distance<=skill->range&&CanAttack( ))
                {
                    if(GServer->ServerDebug)
                        Log(MSG_INFO,"[DoAttack] In AOE_TARGET time for AoeSkill");
                    AoeSkill( skill, NULL );    //LMA: no specific Enemy in Zone.
                }
                else
                {
                    //LMA: Too verbose...
                    //Log(MSG_INFO,"[DoAttack] In AOE_TARGET not reached or can't attack");
                }

            }
            else
            {
                if(Enemy==NULL)
                {
                    if(GServer->ServerDebug)
                        Log(MSG_INFO,"player %i: default AOE attack (%i/%i) %i for enemy NULL ", clientid,Battle->atktype,AOE_TARGET,skill->id);
                    if (skill->atkpower == 0)//Like Howl Skill soldier (AOE Def down) or Detect skill have 0 atkpower
                    {
                        if(GServer->ServerDebug)
                            Log(MSG_INFO,"[DoAttack] AoeDebuff");
                        AoeDebuff( skill, NULL );
                    }
                    else
                    {
                        AoeSkill( skill, Enemy );
                    }
                }
                else
                {
                    if(GServer->ServerDebug)
                        Log(MSG_INFO,"player %i: default AOE attack (%i/%i) %i for enemy %i ", clientid,Battle->atktype,AOE_TARGET,skill->id,Enemy->clientid);
                    AoeSkill( skill, Enemy );
                }

                //AoeSkill( skill, Enemy );
            }
        }
        break;
        case BUFF_SELF:
        {
            CSkills* skill = GServer->GetSkillByID( Battle->skillid );
            if(skill==NULL)
            {
                //osprose
                //ClearBattle( Battle );
                return;
            }
            BuffSkill( this, skill );
        }
        break;
        case MONSTER_BUFF_SELF:    //LMA: monster selfs buffs.
        {
            CSkills* skill = GServer->GetSkillByID( Battle->skillid );
            if(skill==NULL)
            {
                ClearBattle( Battle );
                return;
            }
            BuffSkill( this, skill );
        }
        break;
       case BUFF_AOE:
        {
            CSkills* skill = GServer->GetSkillByID( Battle->skillid );
            if(skill==NULL)
            {
                ClearBattle( Battle );
                return;
            }

             //osprose
             AoeBuff( skill );
             //osprose end
        }
        break;
        default:
        {
            return;
        }
        break;
    }

}

// do normal attack
// KT version
// commented for now
/*
void CCharacter::NormalAttack( CCharacter* Enemy )
{
    Enemy->OnBeAttacked( this );
    Position->destiny  = Position->current;
    //Log(MSG_DEBUG,"(NormalAttack) Destiny set to current position X: %f Y: %f.",Position->current.x,Position->current.y);


    // Even newer formula based on Jayce's idea
    //dodge. gives a percentage chance
    unsigned int hitpower = 0;
    bool critical = false;
    unsigned int totalchance = Stats->Accury + Enemy->Stats->Dodge;
    double acdm = Stats->Accury * 100 / totalchance;
    unsigned int hitchance = (unsigned int)floor(acdm);
    if(GServer->RandNumber( 0, 100 ) > hitchance)
    {
        hitpower = 0; // dodged
        //Log(MSG_DEBUG,"accuracy %i dodge %i",Stats->Accury,Enemy->Stats->Dodge);
        //Log(MSG_DEBUG,"Dodged total %i mod %f hit chance %i",totalchance,acdm,hitchance);
    }
    else
    {
        //level adjustment. Not yet implemented
        unsigned int leveltotal = Stats->Level + Enemy->Stats->Level;
        double levelmod = Stats->Level * 200 / leveltotal;

        //calculate damage
        if(Stats->magicattack == 1) //magic attacks such as wands and some monsters
        {
            unsigned int totalpower = Stats->Attack_Power + Enemy->Stats->Magic_Defense;
            double hitmod = Stats->Attack_Power * 100 / totalpower + Stats->Level; //percentage of hitpower to use * 100
            hitpower = (unsigned int)floor((double)(hitmod * Stats->Attack_Power / 100));
        }
        else
        {
            unsigned int totalpower = Stats->Attack_Power + Enemy->Stats->Defense;
            double hitmod = Stats->Attack_Power * 100 / totalpower + Stats->Level; //percentage of hitpower to use * 100
            hitpower = (unsigned int)floor((double)(Stats->Attack_Power * hitmod / 100));
            //Log(MSG_DEBUG,"Normal Attack: AP %i DEF %i",Stats->Attack_Power,Enemy->Stats->Defense);
            //Log(MSG_DEBUG,"Normal Attack: hit total %i hitmod %f hit power %i",totalpower,hitmod,hitpower);
        }
        //add some randomness. + or - 5% of hitpower
        int min = (int)floor((double)(hitpower * 0.85));
        int max = (int)floor((double)(hitpower * 1.15));
        int dif = max - min;
        int mod = GServer->RandNumber( 0, dif );
        hitpower = min + mod;
        //Log(MSG_DEBUG,"min %i max %i dif %i mod %i modified hit power %i", min, max, dif, mod, hitpower);
        if(hitpower <= 5)
        {
            hitpower = 5;
        }
        else
        {
            if(GServer->RandNumber(0,300)<Stats->Critical)
            {
                hitpower = (long int)floor(hitpower * 1.5);
                critical = true;
                //Log(MSG_DEBUG,"CRITICAL hit power %i", hitpower);
            }
        }
        if (hitpower > 0x7ff)//2047 packet size limit.
           hitpower = 0x7ff;
        //Log(MSG_DEBUG,"Successful hit hit chance %i damage %i",hitchance, hitpower);
    }


    //end jayce formula
    Battle->atktarget = Battle->target;
    //if(IsMonster())
    //{
        //Log(MSG_INFO,"Monster hits player for %i damage. Attack target = %i",hitpower,Battle->atktarget);
    //}
    //Log( MSG_INFO, "hitpower %i. Attack %f ", hitpower,attack );
    if(!Enemy->IsSummon( ) && Enemy->IsMonster( ))
    {
        Enemy->AddDamage( this, hitpower );
        Enemy->damagecounter += hitpower;// is for AI
    }
    Enemy->Stats->HP -= hitpower;
    //if(IsPlayer())
    //    Log(MSG_DEBUG,"Player attack did damage %i",hitpower);
    //if(IsMonster())
    //    Log(MSG_DEBUG,"Monster attack did damage %i",hitpower);



    BEGINPACKET( pak, 0x799 );
    ADDWORD    ( pak, clientid );
    ADDWORD    ( pak, Battle->atktarget );

    if(Enemy->IsDead())
    {
        CDrop* thisdrop = NULL;
        ADDWORD ( pak, (hitpower |   (    critical?0xb000:0x8000   )    ));
        if(!Enemy->IsSummon( ) && !Enemy->IsPlayer( ))
        {
            int thisdroprate = Stats->itemdroprate + GServer->Config.DROP_RATE;
            //Log(MSG_DEBUG,"Monster killed. calling drops function %i times",thisdroprate);
            for(int j=0;j<thisdroprate;j++)
            {
                thisdrop = Enemy->GetDrop( );
                if(thisdrop != NULL)
                {
                    CMap* map = GServer->MapList.Index[thisdrop->posMap];
                    map->AddDrop( thisdrop );
                }
            }
        }
        GServer->SendToVisible( &pak, Enemy );
        ClearBattle(Battle);
        //ClearBattle(Enemy->Battle);
        if(Enemy->IsMonster())
        {
            CMonster* monster = reinterpret_cast<CMonster*>(Enemy);
            if(monster == NULL)
            {
                //Log(MSG_DEBUG,"Monster killed. Failed to create monster object");
                return;
            }
            else
            {
                //Log(MSG_DEBUG,"Monster killed. Setting death delay timer");
                monster->DeathDelayTimer = clock();
            }
        }
        OnEnemyDie( Enemy );
        ClearBattle(Battle);
        GServer->SendToVisible( &pak, Enemy );
    }
    else
    {
        ADDWORD   ( pak, (hitpower|(hitpower>0?(critical?0x4000:0):0)));
        //ADDDWORD   ( pak, (hitpower>0?(critical?12:0):0) );
        GServer->SendToVisible( &pak, Enemy );
    }
    ReduceABC( );
    Battle->lastAtkTime = clock( );
    //Battle->lastAtkTime = Battle->NextAtkTime;
}
*/

// do normal attack
void CCharacter::NormalAttack( CCharacter* Enemy )
{
    //LMA: Sometimes it's fired several times, no need to kill several times ;)
    bool is_already_dead = Enemy->IsDead();

    Position->destiny = Position->current;

    //LMA: Log
    if(GServer->ServerDebug)
    {
        if(IsPlayer())
            Log(MSG_INFO,"Forcing destiny to current (%.2f,%.2f), (%.2f,%.2f)",Position->current.x,Position->current.y,Position->destiny .x,Position->destiny .y);
    }
    reduceItemsLifeSpan( false );
    Enemy->OnBeAttacked( this );
    float levelmult = (float) Stats->Level / Enemy->Stats->Level;
    if(levelmult > 2)levelmult = 2;
    float atkdefmult = 0;
    float attack = 0;
    float constant = 4;
    if(Stats->MagicAttack == 1)
    {
        atkdefmult = (float) Stats->Attack_Power / (Enemy->Stats->Magic_Defense);
    }
    else
    {
        atkdefmult = (float) Stats->Attack_Power / (Enemy->Stats->Defense);
    }
    attack = Stats->Attack_Power * levelmult * atkdefmult / constant;

    if(attack < 5) attack = 5;
    float d_attack = attack / 100;
    float mod = GServer->RandNumber( 0, 10 ) * d_attack;
    attack += mod;
    long int hitpower = (long int)floor(attack);
    if(IsPlayer( )) //temp fix to find balance btw monster and player
        hitpower = (long int)floor(attack * (GServer->Config.PlayerDmg/100.00));
    if(IsMonster( )) //temp fix to find balance btw monster and player
        hitpower = (long int)floor(attack * (GServer->Config.MonsterDmg/100.00));

    if(IsPlayer())//Add Dmg
    {
        if( Stats->ExtraDamage_add !=0 )
        {
            long int hitsave = hitpower;
            hitpower += ((hitpower * Stats->ExtraDamage_add) / 100);
            if(GServer->ServerDebug)
                Log(MSG_INFO,"ExtraDmg Normal atk: before %i, after %i (ED: %i)",hitsave,hitpower,Stats->ExtraDamage_add);
        }
    }

    bool critical = false;
    if(hitpower<=0)
    {
        hitpower = 0;
    }
    else
    {
        if(GServer->RandNumber(0,700)<Stats->Critical)
        {
            hitpower = (long int)floor(hitpower*1.5);
            critical = true;
        }
    }
    // dodge
    unsigned int hitvalue = (unsigned int)floor(Stats->Accuracy * 50 / Enemy->Stats->Dodge);
    if(IsMonster( ) && !IsSummon())
        hitvalue = (unsigned int)floor(Stats->Accuracy * 60 / Enemy->Stats->Dodge);
    if(hitvalue>100) hitvalue = 99;
    if(GServer->RandNumber( 0, 100 )>hitvalue)
        hitpower = 0; // dodged
    if(!Enemy->IsSummon( ) && Enemy->IsMonster( ))
    {
        Enemy->AddDamage( this, (long long) hitpower );
        Enemy->damagecounter += (long long) hitpower;// is for AI
    }

    //Block
    if(GServer->RandNumber(0, 100) < Stats->Block_Rate){
        hitpower -= Stats->Blocked_dmg;
        //Log(MSG_DEBUG, "%i Damage has been blocked", Stats->Blocked_dmg);
        if(hitpower <=0){
            hitpower = 1;
        }
    }
	//Damage reflect
    if(Enemy->Status->ShieldDamage_up != 0xff && ((Stats->HP) > ((long long) (hitpower*(Enemy->shield_reflect) / 100))))
    {
        Stats->HP -= (long long) (hitpower*(Enemy->shield_reflect) / 100);
        if(GServer->ServerDebug)
            Log(MSG_INFO,"%i percent Damage Reflected", Enemy->shield_reflect);
		if(Stats->HP < 0)
		{
			Stats->HP = 0;
		}

        //Log(MSG_HACK,"Value = %i",Enemy->shield_reflect);
    }

    Enemy->Stats->HP -=  (long long) hitpower;
	if (hitpower > 0) {
		Enemy->Battle->hitby = clientid; //Enemy was just hit so make sure that we set up the hitby record
	}

    if(GServer->ServerDebug)
    {
        if (Enemy->IsMonster())
        {
            Log(MSG_INFO,"Normal Attack, monster HP %I64i, hitpower %li",Enemy->Stats->HP,hitpower);
        }
        else
        {
            Log(MSG_INFO,"Normal Attack, Player HP %I64i, hitpower %li",Enemy->Stats->HP,hitpower);
        }
    }
    // actually the target was hit, if it was sleeping, set duration of
    // sleep to 0. map process will remove sleep then at next player-update
    if (Enemy->Status->Sleep != 0xff)
    {
        Enemy->MagicStatus[Enemy->Status->Sleep].Duration = 0;
    }

    //if (IsPlayer())
    //printf("Target suffered %i physical damage, %i HP still remain \n", hitpower, Enemy->Stats->HP);
    //else printf("Mob did %i physical damage, %i HP still remain \n", hitpower, Enemy->Stats->HP);
    Enemy->reduceItemsLifeSpan(true);

    //LMA: forcing the HP amount value, only when it's supposed to be dead side.
    if(Enemy->IsMonster()&&Enemy->IsDead())
    {
        BEGINPACKET( pak, 0x79f );
        ADDWORD    ( pak, Enemy->clientid );
        ADDDWORD   ( pak, 1);
        GServer->SendToVisible( &pak, Enemy );
        if(GServer->ServerDebug)
            Log(MSG_INFO,"death atk for %i, amount: %i",Enemy->clientid,hitpower);
    }

    BEGINPACKET( pak, 0x799 );
    ADDWORD    ( pak, clientid );
    //ADDWORD    ( pak, Battle->atktarget );
    ADDWORD    ( pak, Enemy->clientid );
    if(GServer->ServerDebug)
        Log(MSG_INFO,"pak 799 : clientid %i, Enemy->clientid %i",clientid,Enemy->clientid);

    //LMA: little test (see TESTDEATH)
    //ADDDWORD   ( pak, hitpower );

    if(Enemy->IsDead())
    {
        //LMA: UW handling, we add the handling for other quests as well...
        //if (!is_already_dead&&(GServer->MapList.Index[Position->Map]->QSDkilling>0||GServer->MapList.Index[Position->Map]->QSDDeath>0)&&IsPlayer()&&Enemy->IsPlayer())
        if(GServer->ServerDebug)
        {
            if(Enemy->Battle->target == 0)
                Log(MSG_INFO,"Enemy died. Enemy->target = 0");  //should not be 0 at this point
            else
                Log(MSG_INFO,"Enemy died. Enemy->target = %i",Enemy->Battle->target);  //print out the eney's target
        }
        if(GServer->ServerDebug)
            Log(MSG_INFO,"running is_already_dead check 1");

        if (!is_already_dead&&((GServer->MapList.Index[Position->Map]->QSDkilling>0&&IsPlayer()&&Enemy->IsPlayer())||(GServer->MapList.Index[Position->Map]->QSDDeath&&Enemy->IsPlayer())))
        {
            if(GServer->ServerDebug)
                Log(MSG_INFO,"UWKILL begin normal atk");
            UWKill(Enemy);
        }



        //This stuff from LMA doesn't make a lot of sense
        if(GServer->ServerDebug)
            Log(MSG_INFO,"running is_already_dead check 2 for quests");
        //LMA: test for quest hack (stackable).
        #ifdef QHACK
        if(!is_already_dead && Enemy->die_quest != 0 && Enemy->IsMonster() && (IsPlayer() || IsSummon()))
        {
            if(GServer->ServerDebug)
                Log(MSG_INFO,"running questkill");
            QuestKill(Enemy->die_quest);
        }
        #endif
        //LMA END

        //LMA: TESTDEATH :: We try to force the DEATH
        //ADDDWORD   ( pak, Enemy->Stats->MaxHP );
        if(GServer->ServerDebug)
            Log(MSG_INFO,"adding hitpower to packet");
        ADDDWORD   ( pak, hitpower );

        //Logs.
        /*if(IsPlayer()&&Enemy->IsPlayer())
        {
            CPlayer* plkilled=(CPlayer*) Enemy;
            CPlayer* plkiller=(CPlayer*) this;
            Log(MSG_INFO,"NA CID %i (%s) killed %i (%s) (NORMAL_ATTACK).",clientid,plkiller->CharInfo->charname,Enemy->clientid,plkilled->CharInfo->charname);
        }
        else
        {
            Log(MSG_INFO,"NA CID %i killed %i (NORMAL_ATTACK).",clientid,Enemy->clientid);
        }*/
        if(GServer->ServerDebug)
            Log(MSG_INFO,"setting up drop. Why do this if the dead one might be a player?");
        CDrop* thisdrop = NULL;
        //Log(MSG_WARNING,"Dead, critical %i",critical);

        //LMA: Seems that 24 is for critical and skill? We disable critical final hit for now.
        //ADDDWORD   ( pak, critical?28:16 );
        if(GServer->ServerDebug)
            Log(MSG_INFO,"adding 16 (crit) to packet");
        ADDDWORD   ( pak, 16 );
        if(!Enemy->IsSummon( ) && !Enemy->IsPlayer( ))
        {
            //LMA: looping the drops (double drop medal for example).
            int nb_drops = GServer->Config.ItemDropRate;
            if (IsPlayer())
            {
                CPlayer* plkiller=(CPlayer*) this;
                nb_drops += plkiller->bonusddrop + Stats->itemdroprate;
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"Drop time, there should be %i drops",nb_drops);  // should be 1 with default settings (no medals)
            }

            //No drop if already dead and drop done.
            if(Enemy->drop_dead)
            {
                if(GServer->ServerDebug)
                    Log(MSG_WARNING,"Trying to make a monster (CID %u, type %u) drop again but already did.",Enemy->clientid,Enemy->char_montype);
                nb_drops=0;
            }

            for (int k=0;k<nb_drops;k++)
            {
                thisdrop = Enemy->GetDrop( );
                if(thisdrop!=NULL)
                {
                    if(GServer->ServerDebug)
                        Log(MSG_INFO,"Dropping Nb %i",k);
                    CMap* map = GServer->MapList.Index[thisdrop->posMap];
                    map->AddDrop( thisdrop );
                }

            }

        }

        //GServer->SendToVisible( &pak, Enemy, thisdrop );
        if(GServer->ServerDebug)
            Log(MSG_INFO,"sending packet to visible");
        GServer->SendToVisible( &pak, Enemy );

        //LMA: test
        //OnEnemyDie( Enemy );
        //ClearBattle(Battle);  //this line might stop the AIP from finding a target after the monster dies.
        if(GServer->ServerDebug)
            Log(MSG_INFO,"clearing battle");
        if(!is_already_dead && !Enemy->IsPlayer( ))
        {
            if(GServer->ServerDebug)
                Log(MSG_INFO,"I am already dead apparently");
            CMonster* monster = GServer->GetMonsterByID(Enemy->clientid, Enemy->Position->Map);
            if(monster->TD)
            {
                GServer->TDMonCount --;
                Log(MSG_WARNING,"TD monster died. Current TD Monster Count is %i",GServer->TDMonCount);
            }
            TakeExp(Enemy);
        }
        //end of test.
        //Log(MSG_WARNING,"End of dead packet",);


        //LMA: test packet...
        //0x820 - 01 00 00 00
        if(GServer->ServerDebug)
            Log(MSG_INFO,"checking if enemy is a player");
        if(Enemy->IsPlayer())
        {
            if(GServer->ServerDebug)
                Log(MSG_INFO,"yup it's a player");
            CPlayer* plkilled=(CPlayer*) Enemy;
            if(GServer->ServerDebug)
                Log(MSG_INFO,"Uplayer died. sending an 0x820 packet");
            if(plkilled != NULL)
            {
                BEGINPACKET( pak, 0x820 );
                ADDWORD    ( pak, 1 );
                ADDWORD    ( pak, 0 );
                plkilled->client->SendPacket(&pak);
                //Log(MSG_WARNING,"NA Packet 820 sent to %s",plkilled->CharInfo->charname);

				for(int i = 0; i < 30; i++)
				{
					plkilled->MagicStatus[i].Duration = 0;
					plkilled->MagicStatus[i].BuffTime = 0;
				}
				plkilled->RefreshBuff();
            }


        }
        else if (Enemy->IsMonster())
        {
            if(GServer->ServerDebug)
                Log(MSG_INFO,"no it's a monster");
            //LMA let's send a suicide packet so monster should be killed twice (not anymore).
        }
        if(GServer->ServerDebug)
            Log(MSG_INFO,"end of check for is dead");
    }
    else
    {
        //LMA: TESTDEATH

        ADDDWORD   ( pak, hitpower );

        //Logs.
        /*if(IsPlayer()&&Enemy->IsPlayer())
        {
            CPlayer* plkilled=(CPlayer*) Enemy;
            CPlayer* plkiller=(CPlayer*) this;
            Log(MSG_WARNING,"NA hitpower %i, critical %i, %i (%s) hitting %i (%s)",hitpower,critical,clientid,plkiller->CharInfo->charname,Enemy->clientid,plkilled->CharInfo->charname);
            Log(MSG_WARNING,"NA Player %i (%s) targets cid %i",clientid,plkiller->CharInfo->charname,plkiller->Battle->target);
        }
        else
        {
            Log(MSG_WARNING,"NA, Not dead, hitpower %i, critical %i (%i hitting %i)",hitpower,critical,clientid,Enemy->clientid);
        }*/

        ADDDWORD   ( pak, (hitpower>0?(critical?12:0):0) );
        GServer->SendToVisible( &pak, Enemy );
    }

    Battle->lastAtkTime = clock( );
    ReduceABC( );

    //LMA: We take fuel.
    if(IsPlayer()&&Status->Stance==DRIVING)
    {
        if(GServer->ServerDebug)
            Log(MSG_INFO,"taking fuel");
        CPlayer* plkiller=(CPlayer*) this;

        if(plkiller->items[136].itemnum==0||plkiller->items[139].itemnum==0)
        {
            if(GServer->ServerDebug)
                Log(MSG_WARNING,"%s should be riding but doesn't have a motor %i or weapon %i (normal_attack)?",plkiller->CharInfo->charname,plkiller->items[136].itemnum,plkiller->items[139].itemnum);
            return;
        }

        //We take the fuel amount.
        //Log(MSG_INFO,"lifespan before %i, sp_value %i",plkiller->items[136].lifespan,plkiller->items[136].sp_value);
        plkiller->items[136].sp_value-=(int)plkiller->attack_fuel;
        if(plkiller->items[136].sp_value<0)
        {
            plkiller->items[136].sp_value=0;
        }

        plkiller->items[136].lifespan=(int)(plkiller->items[136].sp_value/10);

        if(plkiller->items[136].lifespan<0)
        {
            plkiller->items[136].sp_value=0;
        }

        //Log(MSG_INFO,"lifespan after %i, sp_value %i, attack fuel %.2f",plkiller->items[136].lifespan,plkiller->items[136].sp_value,plkiller->attack_fuel);

        BEGINPACKET( pak,0x7ce );
        ADDWORD    ( pak, 0x88 ); //Slot
        ADDWORD    ( pak, plkiller->items[136].sp_value );
        plkiller->client->SendPacket( &pak );

        //We stop attack if needed
        if(plkiller->items[136].lifespan==0)
        {
            BEGINPACKET( pak, 0x796 );
            ADDWORD    ( pak, plkiller->clientid );
            ADDFLOAT   ( pak, plkiller->Position->current.x*100 );
            ADDFLOAT   ( pak, plkiller->Position->current.y*100 );
            ADDWORD    ( pak, 0x0000 );
            GServer->SendToVisible( &pak, plkiller );

            //clear battle.
            ClearBattle(Battle);
            //Log(MSG_INFO,"No more fuel to attack");
        }

    }


    return;
}

//LMA: Resuming to NormalAttack on some occasions, mainly after skills (direct and AOEs)
//when the player was in NormalAttack mode before skilling.
bool CCharacter::ResumeNormalAttack( CCharacter* Enemy,bool was_aoe)
{
    //LMA: Special case. If the player does a first attack as a skill, he doesn't go to normal attack mode.
    if (IsPlayer())
    {
        //On some AOE we don't have a skilltarget ?
        if (Enemy==NULL&&was_aoe)
        {
            if(GServer->ServerDebug)
                Log(MSG_INFO,"After AOE without specific target.");
        }

        //he was already fighting the monster in normal_attack mode and did a skill, now he has to resume normal_attack.
        if(Battle->skilltarget!=0&&Battle->contatk&&(Battle->atktarget==Battle->skilltarget))
        {
            CCharacter* NAEnemy=NULL;
            if(Enemy!=NULL&&(Enemy->clientid==Battle->atktarget))
            {
                NAEnemy=Enemy;
            }
            else
            {
                NAEnemy = GetCharTarget( );
            }

            if (NAEnemy==NULL)
            {
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"after skill, can't find previous target %u, stopping.",Battle->target);
                ClearBattle(Battle);
                return true;
            }

            //Is the foe dead?
            if(NAEnemy->IsDead())
            {
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"after skill, player is stopping battle with %u because he's dead",Battle->target);
                ClearBattle(Battle);
                return true;
            }
            if(GServer->ServerDebug)
                Log(MSG_INFO,"after skill, player is resuming normal_attack with %u (%u)",Battle->target,NAEnemy->clientid);
            Battle->atktype = NORMAL_ATTACK;
            Battle->skilltarget = 0;
            Battle->atktarget = Battle->target;
            Battle->skillid = 0;

            return true;
        }

        //LMA: this case is mostly after "buffs"
        if(Battle->target!=0&&Battle->contatk&&(Battle->atktarget==Battle->target))
        {
            CCharacter* NAEnemy=NULL;
            if(Enemy!=NULL&&(Enemy->clientid==Battle->atktarget))
            {
                NAEnemy=Enemy;
            }
            else
            {
                NAEnemy = GetCharTarget( );
            }

            if (NAEnemy==NULL)
            {
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"2:: after skill, can't find previous target %u, stopping.",Battle->target);
                ClearBattle(Battle);
                return true;
            }

            //Is the foe dead?
            if(NAEnemy->IsDead())
            {
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"2:: after skill, player is stopping battle with %u because he's dead",Battle->target);
                ClearBattle(Battle);
                return true;
            }
            if(GServer->ServerDebug)
                Log(MSG_INFO,"2:: after skill, player is resuming normal_attack with %u (%u)",Battle->target,NAEnemy->clientid);
            Battle->atktype = NORMAL_ATTACK;
            Battle->skilltarget = 0;
            Battle->atktarget = Battle->target;
            Battle->skillid = 0;

            return true;
        }

        //Stop...
        if(GServer->ServerDebug)
            Log(MSG_INFO,"after skill, player is doing nothing.");
        ClearBattle(Battle);
    }
    else
    {
        if(GServer->ServerDebug)
            Log(MSG_INFO,"after skill, monster is going to normal_attack %u",Battle->target);
        Battle->atktype = NORMAL_ATTACK;
        Battle->skilltarget = 0;
        Battle->atktarget = Battle->target;
        Battle->skillid = 0;
    }


    return true;
}


// do skill attack
bool CCharacter::SkillAttack( CCharacter* Enemy, CSkills* skill )
{
    //Log(MSG_INFO,"SkillAttack, Performing skill %i",skill->skilltype);
    Position->destiny = Position->current;
    if(Battle->castTime == 0)
    {
        BEGINPACKET( pak, 0x7bb );
        ADDWORD    ( pak, clientid );
        GServer->SendToVisible( &pak, this );
        Battle->castTime = clock();
        return true;
    }
    else
    {
        clock_t etime = clock() - Battle->castTime;
        if(etime<SKILL_DELAY)
            return true;
    }
    Battle->castTime = 0;
    UseAtkSkill( Enemy, skill );
	if(skill->costtype[0] == 17)
    Stats->MP -= (skill->mp - (skill->mp * Stats->MPReduction / 100));
    //Some skills uses zulie
    if(skill->costtype[0] == 40)
    {
        CPlayer* thisclient = GServer->GetClientByID(clientid,Position->Map);
        thisclient->CharInfo->Zulies -= skill->costamount[0];
        if(thisclient->CharInfo->Zulies < 0) thisclient->CharInfo->Zulies = 0;
        BEGINPACKET( pak, 0x71d );
        ADDQWORD( pak, thisclient->CharInfo->Zulies );
        thisclient->client->SendPacket( &pak );
    }
    if(skill->costtype[1] == 40)
    {
        CPlayer* thisclient = GServer->GetClientByID(clientid,Position->Map);
        thisclient->CharInfo->Zulies -= skill->costamount[1];
        if(thisclient->CharInfo->Zulies < 0) thisclient->CharInfo->Zulies = 0;
        BEGINPACKET( pak, 0x71d );
        ADDQWORD( pak, thisclient->CharInfo->Zulies );
        thisclient->client->SendPacket( &pak );

    }else if(skill->costtype[1] == 17)
    {
        Stats->MP -= (skill->costamount[1] - (skill->mp * Stats->MPReduction / 100));
    }
    if(Stats->MP<0) Stats->MP=0;
    if(Battle->contatk)
    {
        //LMA: Resuming on NormalAttack?
        ResumeNormalAttack(Enemy);
    }
    else
    {
        //osprose
        //ClearBattle( Battle );
        if(GServer->ServerDebug)
            Log(MSG_INFO,"after skill, nothing special, not in contact / range?...");

        //he was already fighting the monster in normal_attack mode and did a skill, now he has to resume normal_attack.
        if(Battle->skilltarget!=0&&(Battle->atktarget!=Battle->skilltarget))
        {
            //Stop...
            if(GServer->ServerDebug)
                Log(MSG_INFO,"after skill, player is doing nothing (first attack).");
            ClearBattle(Battle);
        }

    }

    GServer->DoSkillScript( this, skill );       //So far only used for summons
    Battle->lastAtkTime = clock( );
    return true;
}

// do buff skill
bool CCharacter::BuffSkill( CCharacter* Target, CSkills* skill )
{
    Position->destiny = Position->current;
    if(Battle->castTime==0)
    {
        BEGINPACKET( pak, 0x7bb );
        ADDWORD    ( pak, clientid );
        GServer->SendToVisible( &pak, (CCharacter*)this );
        Battle->castTime = clock();
        //Log(MSG_INFO,"7bb battle casttime==0");
        return true;
    }
    else
    {
        clock_t etime = clock() - Battle->castTime;
        if(etime<SKILL_DELAY)
        {
            //Log(MSG_INFO,"etime<SKILL_DELAY: %i<%i",etime,SKILL_DELAY);
            return true;
        }

    }

    //Log(MSG_INFO,"new stat MP %i",Stats->MP);
    //Log(MSG_INFO,"%i Really doing skill %i to %i",clientid,skill->id,Target->clientid);

    if(GServer->ServerDebug)
        Log(MSG_INFO,"Doing a simple buff BuffSkill");
    Battle->castTime = 0;
    UseBuffSkill( Target, skill );
	if(skill->costtype[0] == 17)
    Stats->MP -= (skill->mp - (skill->mp * Stats->MPReduction / 100));
    //Some skills uses zulie
    if(skill->costtype[0] == 40)
    {
        CPlayer* thisclient = GServer->GetClientByID(clientid,Position->Map);
        thisclient->CharInfo->Zulies -= skill->costamount[0];
        if(thisclient->CharInfo->Zulies < 0) thisclient->CharInfo->Zulies = 0;
        BEGINPACKET( pak, 0x71d );
        ADDQWORD( pak, thisclient->CharInfo->Zulies );
        thisclient->client->SendPacket( &pak );
    }
    if(skill->costtype[1] == 40)
    {
        CPlayer* thisclient = GServer->GetClientByID(clientid,Position->Map);
        thisclient->CharInfo->Zulies -= skill->costamount[1];
        if(thisclient->CharInfo->Zulies < 0) thisclient->CharInfo->Zulies = 0;
        BEGINPACKET( pak, 0x71d );
        ADDQWORD( pak, thisclient->CharInfo->Zulies );
        thisclient->client->SendPacket( &pak );

    }else if(skill->costtype[1] == 17)
    {
        Stats->MP -= (skill->costamount[1] - (skill->mp * Stats->MPReduction / 100));
    }
    if(Stats->MP<0) Stats->MP=0;

    /*
    ClearBattle( Battle );
    GServer->DoSkillScript( this, skill );
    */

    //osprose
    GServer->DoSkillScript( this, skill );
    if(!IsMonster())
    {
        //LMA: do we have to resume a Normal Attack?
        ResumeNormalAttack(NULL,false);
        /*
        //ClearBattle( Battle ); // clear battle for players when they use buff skills
        Battle->bufftarget = 0;
        Battle->skilltarget = 0;
        Battle->skillid = 0;
        //Battle->atktype = NORMAL_ATTACK;
        Battle->atktype = 0;*/
    }
    else //Monsters need to be reset to normal attack and clear skill attacks.
    {
        Battle->bufftarget = 0;
        Battle->skilltarget = 0;
        Battle->skillid = 0;
        Battle->atktype = NORMAL_ATTACK;
    }
    //osprose end

    Battle->lastAtkTime = clock( );
    return true;
}

// do Debuff skill [netwolf]
bool CCharacter::DebuffSkill( CCharacter* Enemy, CSkills* skill )
{
    Position->destiny = Position->current;
    if(Battle->castTime==0)
    {
        BEGINPACKET( pak, 0x7bb );
        ADDWORD    ( pak, clientid );
        GServer->SendToVisible( &pak, (CCharacter*)this );
        Battle->castTime = clock();
        return true;
    }
    else
    {
        clock_t etime = clock() - Battle->castTime;
        if(etime<SKILL_DELAY)
            return true;
    }
    Battle->castTime = 0;
    UseDebuffSkill( Enemy, skill );
    Stats->MP -= (skill->mp - (skill->mp * Stats->MPReduction / 100));
    if(Stats->MP<0) Stats->MP=0;
    ClearBattle( Battle );
    GServer->DoSkillScript( this, skill );
    Battle->lastAtkTime = clock( );
    return true;
}

//LMA: A summon does a buff skill
//2do: check if master is debuffed.
bool CCharacter::SummonBuffSkill( CCharacter* Target, CSkills* skill )
{
    Position->destiny = Position->current;
    if(Battle->castTime==0)
    {
        BEGINPACKET( pak, 0x7bb );
        ADDWORD    ( pak, clientid );
        GServer->SendToVisible( &pak, (CCharacter*)this );
        Battle->castTime = clock();
        //Log(MSG_INFO,"0x7bb");
        return true;
    }
    else
    {
        clock_t etime = clock() - Battle->castTime;
        if(etime<SKILL_DELAY)
            return true;
    }

    //LMA: For now, it seems the summon buff only one time :(
    //Log(MSG_INFO,"Real buff skill");
    Battle->castTime = 0;
    UseBuffSkill( Target, skill );
    ClearBattle( Battle );
    GServer->DoSkillScript( this, skill );
    Battle->lastAtkTime = clock( );
    return true;
}

// do AoE skill
bool CCharacter::AoeSkill( CSkills* skill, CCharacter* Enemy )
{

    //LMA: not here since AOE can have NULL Enemy, will be catched later by the call to the skill.
    //bool is_already_dead=Enemy->IsDead();
    if(GServer->ServerDebug)
        Log(MSG_INFO,"In AOE Skill");
    Position->destiny = Position->current;
    //Log(MSG_INFO,"Position in AOE: %.2f,%.2f",Position->current.x,Position->current.y);

    //LMA: handling case of AOE_SKILLS and AOE_TARGET (the target point is not the same).
    fPoint goodtarget;
    goodtarget=Position->current;
    if (Battle->atktype==AOE_TARGET)
    {
       goodtarget=Position->aoedestiny;
    }

    if(Battle->castTime==0)
    {
        BEGINPACKET( pak, 0x7bb );
        ADDWORD    ( pak, clientid );
        GServer->SendToVisible( &pak, (CCharacter*)this );
        Battle->castTime = clock();
        return true;
    }
    else
    {
        //LMA: No delay for AOE_TARGET skill (zone)
        if (Battle->atktype!=AOE_TARGET)
        {
            clock_t etime = clock() - Battle->castTime;
            if(etime<SKILL_DELAY)
                return true;
        }

    }

    Battle->castTime = 0;
    CMap* map = GServer->MapList.Index[Position->Map];

    //osprose
    //LMA: pvp ?
    UINT save_atktype=Battle->atktype;

    if(IsPlayer() || IsSummon())
    {
        //monsters.
        for(UINT i=0;i<map->MonsterList.size();i++)
        {
            CMonster* monster = map->MonsterList.at(i);
            if(monster->clientid == clientid) continue;
            if(monster->IsSummon( ) && (map->allowpvp == 0 || monster->owner == clientid)) continue;
            if(monster->TD) continue; //TD monsters are immune to all AOE attacks
            if(GServer->IsMonInCircle( goodtarget,monster->Position->current,(float)skill->aoeradius+1))
            {
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"AOE Attack (1) player attacks monster %i (clientID %u) radius %.2f",monster->montype,monster->clientid,(float)skill->aoeradius+1);

                //LMA: we have to since AOE doesn't have a specific target and atkskil resets some values.
                Battle->skilltarget=monster->clientid;
                Battle->skillid=skill->id;
                Battle->atktype = save_atktype;
                UseAtkSkill( (CCharacter*) monster, skill );

                //LMA: TEST is there a "buff/debuff" skill with it too?
                /*
                if (skill->status[0]!=0&&skill->buff[0]!=0&&monster->Stats->HP>0)
                {
                    UseBuffSkill( (CCharacter*)monster, skill );
                    Log(MSG_INFO,"AOE Attack (1) player buffs monster %i",monster->montype);
                }
                */

                //LMA: AIP time.
                if(!monster->TD) //PY no AI stuff for TD monsters
                    monster->DoAi(monster->thisnpc->AI, 3);
                //END OF TEST.

            }

        }

        //Pvp, group Vs group, here all.
        if(map->allowpvp!=0&&IsPlayer())
        {
            CPlayer* plattacker=(CPlayer*) this;
            for (UINT i=0;i<map->PlayerList.size();i++)
            {
                CPlayer* thisplayer=map->PlayerList.at(i);
                if(thisplayer==NULL)
                {
                    continue;
                }

                //not attacking allies.
                if(thisplayer->pvp_id==plattacker->pvp_id)
                {
                    continue;
                }

                if(GServer->IsMonInCircle( goodtarget,thisplayer->Position->current,(float)skill->aoeradius+1))
                {
                    //LMA: we have to since AOE doesn't have a specific target and atkskil resets some values.
                    Battle->skilltarget=thisplayer->clientid;
                    Battle->skillid=skill->id;
                    Battle->atktype = save_atktype;
                    UseAtkSkill( (CCharacter*) thisplayer, skill );
                    if(GServer->ServerDebug)
                        Log(MSG_WARNING,"AOE Skill pvp player %s",thisplayer->CharInfo->charname);
                }

            }

        }

    }

    if(IsMonster() && !IsSummon())
    {
        for(UINT i=0;i<map->PlayerList.size();i++)
        {
            CPlayer* player = map->PlayerList.at(i);
            if(player->clientid == clientid) continue;
            if(GServer->IsMonInCircle( goodtarget,player->Position->current,(float)skill->aoeradius+1))
            {
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"AOE Attack (2) monster attacks player %s radius %.2f",player->CharInfo->charname,(float)skill->aoeradius+1);
                UseAtkSkill( (CCharacter*) player, skill );
            }

        }
    }
    //osprose end

    if(Enemy!=NULL)
    {
        if(!Enemy->IsDead( ))
        {
            Battle->atktarget = Battle->target;
            Battle->atktype = NORMAL_ATTACK;
            Battle->skilltarget = 0;
            Battle->skillid = 0;
        }
        else
        {
            ClearBattle( Battle );
        }

    }
    else
    {
        ResumeNormalAttack(NULL,true);
        //ClearBattle( Battle );
    }

    if(skill->costtype[0] == 17)
    Stats->MP -= (skill->mp - (skill->mp * Stats->MPReduction / 100));
    //Some skills uses zulie
    if(skill->costtype[0] == 40)
    {
        CPlayer* thisclient = GServer->GetClientByID(clientid,Position->Map);
        thisclient->CharInfo->Zulies -= skill->costamount[0];
        if(thisclient->CharInfo->Zulies < 0) thisclient->CharInfo->Zulies = 0;
        BEGINPACKET( pak, 0x71d );
        ADDQWORD( pak, thisclient->CharInfo->Zulies );
        thisclient->client->SendPacket( &pak );
    }
    if(skill->costtype[1] == 40)
    {
        CPlayer* thisclient = GServer->GetClientByID(clientid,Position->Map);
        thisclient->CharInfo->Zulies -= skill->costamount[1];
        if(thisclient->CharInfo->Zulies < 0) thisclient->CharInfo->Zulies = 0;
        BEGINPACKET( pak, 0x71d );
        ADDQWORD( pak, thisclient->CharInfo->Zulies );
        thisclient->client->SendPacket( &pak );

    }else if(skill->costtype[1] == 17)
    {
        Stats->MP -= (skill->costamount[1] - (skill->mp * Stats->MPReduction / 100));
    }
    if(Stats->MP<0) Stats->MP=0;
    Battle->lastAtkTime = clock( );
    return true;
}

bool CCharacter::AoeBuff( CSkills* skill )
{
    Position->destiny = Position->current;

    if(Battle->castTime==0)
    {
        BEGINPACKET( pak, 0x7bb );
        ADDWORD    ( pak, clientid );
        GServer->SendToVisible( &pak, (CCharacter*)this );
        Battle->castTime = clock();
        return true;
    }
    else
    {
        clock_t etime = clock() - Battle->castTime;
        if(etime<SKILL_DELAY)
            return true;
    }

    Battle->castTime = 0;
    CMap* map = GServer->MapList.Index[Position->Map];

    //checking if buffing party and no party so himself :)
    if(skill->target==1 && GetParty( )==NULL)
    {
        Stats->MP -= (skill->mp - (skill->mp * Stats->MPReduction / 100));
        if(Stats->MP<0) Stats->MP=0;

        if(GServer->ServerDebug)
            Log(MSG_INFO,"AOEBuffs, buffing only myself");
        UseBuffSkill( this, skill );

        //LMA: do we have to resume a Normal Attack?
        ResumeNormalAttack(NULL,false);
        //ClearBattle( Battle );
        //Battle->lastAtkTime = clock( );

        return true;
    }

    //LMA: Summon don't go there...
    //if(IsMonster())
    if(IsMonster()&&!IsSummon())
    {
        if(GServer->ServerDebug)
            Log(MSG_INFO,"Monster in AOE Buff");
        for(UINT i=0;i<map->MonsterList.size();i++)
        {
            CMonster* monster = map->MonsterList.at(i);
            switch(skill->target)
            {
                case tPartyMember: // party
                break;
                case 2: //tClanMember
                break;
                case 3: //tAlly
                case 7: //tAllCharacters
                case 8: //tAllMembers. all characters
                {
                     //Log(MSG_INFO,"Applying AOE buff as long as I can find a close enough monster");
                     if(GServer->IsMonInCircle( Position->current,monster->Position->current,(float)skill->aoeradius + 1))
                     {
                         UseBuffSkill( (CCharacter*)monster, skill );
                     }
                }
                break;
                case 5: //hostile.
                break;
            }
        }

    }
    else
    {
        //players / summons
        for(UINT i=0;i<map->PlayerList.size();i++)
        {
            CPlayer* player = map->PlayerList.at(i);
            switch(skill->target)
            {
                case 1: // party
                {
                    if(player->Party->party==GetParty( ))
                    {
                        if(GServer->IsMonInCircle( Position->current,player->Position->current,(float)skill->aoeradius+1))
                        {
                            UseBuffSkill( (CCharacter*)player, skill );
                        }

                    }
                }
                break;
                case 2: //clan member
                {
                    if (player->Clan == GetClan( ))
                    {
                        //Log(MSG_INFO,"Applying AOE buff as long as I can find a close enough clan member");
                        if(GServer->IsMonInCircle( Position->current, player->Position->current, (float)skill->aoeradius + 1 ) )
                        {
                            UseBuffSkill( (CCharacter*)player, skill );
                        }

                    }
                }
                break;
                case 3:  //ally
                case 7: //everyone
                case 8: //all characters
                {
                    if(GServer->IsMonInCircle( Position->current,player->Position->current,(float)skill->aoeradius + 1))
                    {
                        UseBuffSkill( (CCharacter*)player, skill );
                    }

                }
                break;
                case 5: //hostile
                break;
            }
        }

    }


    Stats->MP -= (skill->mp - (skill->mp * Stats->MPReduction / 100));
    if(Stats->MP<0) Stats->MP=0;

    //ClearBattle( Battle );
    //osprose
    Battle->bufftarget = 0;
    Battle->skilltarget = 0;
    Battle->skillid = 0;

    //LMA: no because in weird cases (party+first target buff+aoe buff)
    //a party buffer could attack a buffed one.
    //Battle->atktype = NORMAL_ATTACK;
    Battle->atktype = 0;
    //ClearBattle( Battle );
    //osprose end

    Battle->lastAtkTime = clock( );

    //LMA: do we have to resume a Normal Attack?
    ResumeNormalAttack(NULL,false);


    return true;
}

// Aoe Debuff [netwolf]
bool CCharacter::AoeDebuff( CSkills* skill, CCharacter* Enemy )
{
    Position->destiny = Position->current;

    if(Battle->castTime==0)
    {
        BEGINPACKET( pak, 0x7bb );
        ADDWORD    ( pak, clientid );
        GServer->SendToVisible( &pak, (CCharacter*)this );
        Battle->castTime = clock();
        return true;
    }
    else
    {
        clock_t etime = clock() - Battle->castTime;
        if(etime<SKILL_DELAY)
            return true;
    }
    Battle->castTime = 0;

    CMap* map = GServer->MapList.Index[Position->Map];
    for(UINT i=0;i<map->MonsterList.size();i++)
    {
        CMonster* monster = map->MonsterList.at(i);
        if(monster->clientid==clientid) continue;
        if(IsSummon( ) || IsPlayer( ))
        {
            if(monster->IsSummon( ) && (map->allowpvp==0 || monster->owner==clientid)) continue;
        }
        else
        {
            if(!monster->IsSummon( )) continue;
        }
        if(GServer->IsMonInCircle( Position->current,monster->Position->current,(float)skill->aoeradius+1))
        {
            if(GServer->ServerDebug)
                Log(MSG_INFO,"AOE Debuff (1) monster %i",monster->montype);
            UseDebuffSkill( (CCharacter*) monster, skill );
        }

    }

    if(map->allowpvp!=0 || (IsMonster( ) && !IsSummon( )))
    {
        for(UINT i=0;i<map->PlayerList.size();i++)
        {
            CPlayer* player = map->PlayerList.at(i);
            if(player->clientid==clientid) continue;
            if(GServer->IsMonInCircle( Position->current,player->Position->current,(float)skill->aoeradius+1))
            {
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"AOE Debuff (2) player %s",player->CharInfo->charname);
                //UseDebuffSkillTest( (CCharacter*) player, skill );
                UseDebuffSkill( (CCharacter*) player, skill );
            }

        }
    }

    Stats->MP -= (skill->mp - (skill->mp * Stats->MPReduction / 100));
     if(Stats->MP<0) Stats->MP=0;
    //ClearBattle( Battle );
     Battle->lastAtkTime = clock( );

    //LMA: do we have to resume a Normal Attack?
    ResumeNormalAttack(NULL,false);


     return true;
 }

// use skill attack
void CCharacter::UseAtkSkill( CCharacter* Enemy, CSkills* skill, bool deBuff )
{
    Log(MSG_INFO,"UseAtkSkill, Performing skill %i",skill->skilltype);
    //LMA: Sometimes it's fired several times, no need to kill several times ;)
    bool is_already_dead = Enemy->IsDead();

    //We Check if The Skill need a Bow, Gun, Launcher or Crossbow and reduce the number of Arrow, Bullet or Canon the player have by 1 (Client is Buged)
    bool need_arrows=false;
    if(skill->weapon[0]==BOW || skill->weapon[0] == GUN || skill->weapon[0] == LAUNCHER || skill->weapon[0] == CROSSBOW)
    {
        need_arrows=true;
    }
    if(skill->weapon[1]==BOW || skill->weapon[1] == GUN || skill->weapon[1] == LAUNCHER || skill->weapon[1] == CROSSBOW)
    {
        need_arrows=true;
    }
    if (need_arrows)
    {
        ReduceABC(1,false); //We Reduce by 1 because Client is Buged, and we set the flag do_packet to false
    }

    reduceItemsLifeSpan( false );
    Enemy->reduceItemsLifeSpan(true);

    //Skill power calculations LMA/Tomiz : New Way
    long int skillpower = skill->atkpower + Stats->Attack_Power;
    if(GServer->ServerDebug)
        Log( MSG_INFO, "Skillpower = %i (%i + %i)",skillpower,skill->atkpower,Stats->Attack_Power);
    float levelmult = (float) Stats->Level / Enemy->Stats->Level;

    if(levelmult > 2)levelmult = 2; //cap level multiplier at 2
    if(GServer->ServerDebug)
        Log( MSG_INFO, "Level Multiplier = %f",levelmult );
    float atkdefmult = 0;
    float attack = 0;
    float constant = 2.5;

    unsigned int skill_hitvalue = (unsigned int)floor(Stats->Accuracy * 50 / Enemy->Stats->Dodge);
    if(IsMonster( ) && !IsSummon())
        skill_hitvalue = (unsigned int)floor(Stats->Accuracy * 60 / Enemy->Stats->Dodge);
    if(skill_hitvalue>100) skill_hitvalue = 99;
    switch(skill->formula)//Magical Or Weapon Type Skill?
    {
		case 0://Skill Mental Storm
        {
            atkdefmult = 0;
        }
		break;
        case 1://Weapon type dmg
        {
            atkdefmult = (float) Stats->Attack_Power / (Enemy->Stats->Defense / 1.3);
            skillpower += GetSen();
            if(GServer->RandNumber( 0, 100 )>skill_hitvalue) atkdefmult /= 2; // Miss results in damage cut in half.
        }
        break;
        case 2://Magical type dmg
        {
            atkdefmult = (float) Stats->Attack_Power / (Enemy->Stats->Magic_Defense / 1.3);
            skillpower += GetInt();
            if(GServer->RandNumber( 0, 100 )>skill_hitvalue) atkdefmult /= 2; // Miss results in damage cut in half.
        }
        break;
        case 3://Weapon type dmg
        {
            atkdefmult = (float) Stats->Attack_Power / (Enemy->Stats->Defense / 1.3);
            skillpower += GetSen();
            if(GServer->RandNumber( 0, 100 )>skill_hitvalue) atkdefmult = 0; // Complete Miss.
        }
        break;
        case 4://Magical type dmg
        {
            atkdefmult = (float) Stats->Attack_Power / (Enemy->Stats->Magic_Defense / 1.3);
            skillpower += GetInt();
            if(GServer->RandNumber( 0, 100 )>skill_hitvalue) atkdefmult = 0; // Complete Miss.
        }
        break;
        case 6://Dual Scratch
        {
            atkdefmult = 1;
        }
        break;
        default:
        {
            atkdefmult = 5;
        }
        break;
    }
    //END Skill power calculations LMA/Tomiz : New Way

    //Tell enemy he's attacked & add damage & send the dmg packet
    if(GServer->ServerDebug)
        Log( MSG_INFO, "Attack / Def multiplier = %f",atkdefmult );
    attack = (skillpower * levelmult * atkdefmult) / constant;
    if(GServer->ServerDebug)
        Log( MSG_INFO, "Attack before random mod = %f",attack );
    if(attack < 0) attack = 0;
    float d_attack = attack / 100;
    float mod = GServer->RandNumber( 0, 10 ) * d_attack;
    attack += mod;
    if(GServer->ServerDebug)
        Log( MSG_INFO, "Attack after random mod = %f",attack );
    long int hitpower = (long int)floor(attack);

    skillpower = (int)attack;
    if(GServer->ServerDebug)
        Log( MSG_INFO, "skillpower = %i",skillpower );
    bool bflag = false;
    Enemy->OnBeAttacked( this );
    if(skillpower<=0) skillpower = 0;
    if(IsPlayer())//Add Dmg
    {
        if( Stats->ExtraDamage_add != 0)
        {
            //LMA: ED, Devilking / Arnold
            long int hitsave=skillpower;
            skillpower += ((skillpower * Stats->ExtraDamage_add) / 100);
            if(GServer->ServerDebug)
                Log(MSG_INFO,"ExtraDmg Skill atk: before %i, after %i (ED: %i)",hitsave,skillpower,Stats->ExtraDamage_add);
        }
    }
    if(!Enemy->IsSummon( ) && Enemy->IsMonster( ))
    {
        Enemy->AddDamage( this, (long long) skillpower );
        Enemy->damagecounter+= (long long) skillpower;// is for AI
    }

	//Damage reflect
    if(Enemy->Status->ShieldDamage_up != 0xff && ((Stats->HP) > ((long long) (skillpower*(Enemy->shield_reflect) / 100))))
    {
        Stats->HP -= (long long) (skillpower*(Enemy->shield_reflect) / 100);
        if(GServer->ServerDebug)
            Log(MSG_INFO,"%i percent Damage Reflected", Enemy->shield_reflect);

		if(Stats->HP < 0)
		{
			Stats->HP = 0;
		}
        //Log(MSG_HACK,"Value = %i",Enemy->shield_reflect);
    }

    Enemy->Stats->HP -=  (long long) skillpower;
	if (hitpower > 0) {
		Enemy->Battle->hitby = clientid; //Enemy was just hit so make sure that we set up the hitby record
	}
    if(GServer->ServerDebug)
        Log(MSG_INFO,"Atk Skill damage %li, Enemy HP after %li",skillpower,Enemy->Stats->HP);

    // actually the target was hit, if it was sleeping, set duration of
    // sleep to 0. map process will remove sleep then at next player-update
    if (Enemy->Status->Sleep != 0xff) {
        Enemy->MagicStatus[Enemy->Status->Sleep].Duration = 0;
    }

    //LMA: patch by sickb0y
    //Problems with some mage skills.
    /*
    BEGINPACKET( pak, 0x7b6 );
    ADDWORD    ( pak, Enemy->clientid );
    ADDWORD    ( pak, clientid );
    ADDDWORD   ( pak, 0x000007f8 );
    ADDBYTE    ( pak, 0x00 );
    ADDDWORD   ( pak, skillpower );
    */

    //LMA: forcing the HP amount value, only when it's supposed to be dead side.
    if(Enemy->IsMonster()&&Enemy->IsDead())
    {
        BEGINPACKET( pak, 0x79f );
        ADDWORD    ( pak, Enemy->clientid );
        ADDDWORD   ( pak, 1);
        GServer->SendToVisible( &pak, Enemy );
        if(GServer->ServerDebug)
            Log(MSG_INFO,"death skill atk for %i, amount: %i",Enemy->clientid,skillpower);
    }

    unsigned short command = (skill->skilltype == 6 || skill->skilltype == 9) ? 0x799 : 0x7b6;
    BEGINPACKET( pak, command );

    if(command == 0x799)
    {
        ADDWORD    ( pak, clientid );
        ADDWORD    ( pak, Enemy->clientid );
        //LMA: Skillpower elsewhere.
        //ADDDWORD   ( pak, skillpower );
    }
    else
    {
        ADDWORD    ( pak, Enemy->clientid );
        ADDWORD    ( pak, clientid );
        ADDDWORD   ( pak, 0x000007f8 );
        ADDBYTE    ( pak, 0x00 );
        //LMA: Skillpower elsewhere.
        //ADDDWORD   ( pak, skillpower );
    }

    //end of patch

    //If Enemy is killed
    if(Enemy->IsDead())
    {
        //Log(MSG_INFO,"Enemy is dead");
        //LMA: Skillpower elsewhere.
        //ADDDWORD   ( pak, Enemy->Stats->MaxHP);
        ADDDWORD   ( pak, skillpower);

        //LMA: UW handling, adding support for other quests.
        //if (!is_already_dead&&(GServer->MapList.Index[Position->Map]->QSDkilling>0||GServer->MapList.Index[Position->Map]->QSDDeath>0)&&IsPlayer()&&Enemy->IsPlayer())
        if (!is_already_dead&&((GServer->MapList.Index[Position->Map]->QSDkilling>0&&IsPlayer()&&Enemy->IsPlayer())||(GServer->MapList.Index[Position->Map]->QSDDeath&&Enemy->IsPlayer())))
        {

            if(GServer->ServerDebug)
                Log(MSG_INFO,"UWKILL begin skill atk");
            UWKill(Enemy);
        }

        //LMA: test for quest hack (stackable).
        #ifdef QHACK
        if(!is_already_dead&&Enemy->die_quest!=0&&Enemy->IsMonster()&&(IsPlayer()||IsSummon()))
        {
            QuestKill(Enemy->die_quest);
        }
        #endif
        //LMA END

        if(GServer->ServerDebug)
            Log(MSG_INFO,"Enemy is dead");
        CDrop* thisdrop = NULL;
        ADDDWORD   ( pak, 16 );

        if(!Enemy->IsSummon( ) && !Enemy->IsPlayer( ))
        {
            //LMA: looping the drops (double drop medal for example).
            int nb_drops = GServer->Config.ItemDropRate;
            if (IsPlayer())
            {
                CPlayer* plkiller=(CPlayer*) this;
                nb_drops += plkiller->bonusddrop + Stats->itemdroprate;
                if(GServer->ServerDebug)
                    Log(MSG_INFO,"Drop time, there should be %i drops",nb_drops);  // default = 1 with no medal
            }

            //No drop if already dead and drop done.
            if(Enemy->drop_dead)
            {
                if(GServer->ServerDebug)
                    Log(MSG_WARNING,"Trying to make a monster (CID %u, type %u) drop again but already did.",Enemy->clientid,Enemy->char_montype);
                nb_drops=0;
            }

            for (int k=0;k<nb_drops;k++)
            {
                thisdrop = Enemy->GetDrop( );
                if(thisdrop!=NULL)
                {
                    if(GServer->ServerDebug)
                        Log(MSG_INFO,"Dropping Nb %i",k);
                    CMap* map = GServer->MapList.Index[thisdrop->posMap];
                    map->AddDrop( thisdrop );
                }

            }

        }

        GServer->SendToVisible( &pak, Enemy );
        //LMA: end of test.
        //Log(MSG_INFO,"Sent to visible");
        //LMA: test
        //OnEnemyDie( Enemy );
        ClearBattle(Battle);
        if(!is_already_dead && !Enemy->IsPlayer())
        {
            //Log(MSG_INFO,"Enemy NOT already dead");
            CMonster* monster = GServer->GetMonsterByID(Enemy->clientid, Enemy->Position->Map);
            if(monster->TD)
            {
                GServer->TDMonCount --;
                Log(MSG_WARNING,"TD monster died. Current TD Monster Count is %i",GServer->TDMonCount);
            }
            TakeExp(Enemy);

        }
        //end of test.

        //LMA: Special Packet to tell if the player is dead?
        if(Enemy->IsPlayer())
        {
            //Log(MSG_INFO,"Enemy is a player");
            CPlayer* plkilled=(CPlayer*) Enemy;
            if(plkilled!=NULL)
            {
                //Log(MSG_WARNING,"Player %s killed",plkilled->CharInfo->charname);
                BEGINPACKET( pak, 0x820 );
                ADDWORD    ( pak, 1 );
                ADDWORD    ( pak, 0 );
                plkilled->client->SendPacket(&pak);
                if(GServer->ServerDebug)
                    Log(MSG_WARNING,"Packet 820 sent to %s",plkilled->CharInfo->charname);

				for(int i = 0; i < 30; i++)
				{
					plkilled->MagicStatus[i].Duration = 0;
					plkilled->MagicStatus[i].BuffTime = 0;
				}
				plkilled->RefreshBuff();
            }
            else
            {
                Log(MSG_INFO,"plkilled did not initialize properly");
            }

        }
        else if (Enemy->IsMonster())
        {
            //LMA let's send a suicide packet so monster should be killed twice (not anymore).
        }

    }
    else
    {
        //LMA: Skillpower elsewhere.
        ADDDWORD   ( pak, skillpower );

        //If enemy is still alive
        if(GServer->ServerDebug)
            Log(MSG_INFO,"The enemy is still alive");
        //ADDDWORD   ( pak, 4 );
        ADDDWORD   ( pak, 0x00 );
        GServer->SendToVisible( &pak, Enemy );

        //osprose
        //if (deBuff) return;

        //GOTO debuffing section
        //bflag = GServer->AddDeBuffs( skill, Enemy, GetInt( ) );
        bflag = GServer->AddBuffs( skill, Enemy, GetInt( ) ); // send to AddBuffs instead.

        //Send (de)buff information to the whole world
        if(skill->nbuffs>0 && bflag)
        {
            if(GServer->ServerDebug)
                Log(MSG_INFO,"The enemy cliendID %u is being buffed with %u",Battle->skilltarget,Battle->skillid);
            BEGINPACKET( pak, 0x7b5 );
            ADDWORD    ( pak, Battle->skilltarget );
            ADDWORD    ( pak, clientid );
            ADDWORD    ( pak, Battle->skillid );
            ADDWORD    ( pak, GetInt( ) );
            ADDBYTE    ( pak, skill->nbuffs );
            GServer->SendToVisible( &pak, Enemy );
        }

        //LMA: test, trying to send back the monster HP amount.
        /*if(Enemy->IsMonster())
        {
            //LMA: Trying to update real HP amount.
            Log(MSG_INFO,"Sending back monster HP amount");
            BEGINPACKET( pak, 0x79f );
            ADDWORD    ( pak, Enemy->clientid );
            ADDDWORD   ( pak, Enemy->Stats->HP );
            GServer->SendToVisible( &pak, Enemy );
        }*/

    }
    if (deBuff) return;
    //Send skill animation to the world
    RESETPACKET( pak, 0x7b9);
    ADDWORD    ( pak, clientid);
    ADDWORD    ( pak, Battle->skillid);

    //LMA: Is it really used?
    if(command != 0x799)
    {
        ADDWORD    ( pak, 1);
    }

    GServer->SendToVisible( &pak, this );

    //osprose
    /*Battle->bufftarget = 0;
    Battle->skilltarget = 0;
    Battle->skillid = 0;
    Battle->atktype = NORMAL_ATTACK;*/
    //osprose end
    Log(MSG_INFO,"UseAtkSkill function completed and returning");
    return;
}

// use buff skill
void CCharacter::UseBuffSkill( CCharacter* Target, CSkills* skill )
{
    bool bflag = false;
    bflag = GServer->AddBuffs( skill, Target, GetInt( ) );
    if(GServer->ServerDebug)
        Log(MSG_INFO,"In UseBuffSkills, skill %i, nbuffs %i, bflag %i to %u",skill->id,skill->nbuffs,bflag,Target->clientid);
    if(skill->nbuffs>0 && bflag == true)
    {
        BEGINPACKET( pak, 0x7b5 );
        ADDWORD    ( pak, Target->clientid );
        ADDWORD    ( pak, clientid );
        ADDWORD    ( pak, Battle->skillid );
        ADDWORD    ( pak, GetInt( ) );
        ADDBYTE    ( pak, skill->nbuffs );
        GServer->SendToVisible( &pak, Target );
    }
    BEGINPACKET( pak, 0x7b9);
    ADDWORD    ( pak, clientid);
    ADDWORD    ( pak, Battle->skillid);
    ADDWORD    ( pak, 1);
	GServer->SendToVisible( &pak, (CCharacter*)this );

	//LMA: Patch for Revive stuff...
	if(skill->skilltype==20&&Target->IsPlayer())
	{
	    CPlayer* thisclient = GServer->GetClientByID(Target->clientid,Position->Map);
	    if (thisclient==NULL)
	    {
	        CPlayer* thisclient = GServer->GetClientByID(Target->clientid,Target->Position->Map);
	    }

	    if (thisclient==NULL)
	    {
	        if(GServer->ServerDebug)
                Log(MSG_WARNING,"Can't find clientID %i for revive...",Target->clientid);
	        return;
	    }

        //not exact but should be fair enough...
        if(GServer->ServerDebug)
            Log(MSG_INFO,"before Revive, HP %I64i, Xp %I64i, skill %i",thisclient->Stats->HP,thisclient->CharInfo->Exp,skill->atkpower);
        unsigned long long new_exp=(unsigned long long) (thisclient->CharInfo->Exp*3*skill->atkpower/(100*100));
		CMap* map = GServer->MapList.Index[Position->Map];
        if(map->allowpvp!= 1)
        {
            thisclient->CharInfo->Exp+=new_exp;
        }
        thisclient->Stats->HP=thisclient->Stats->MaxHP*30/100;
	    if(GServer->ServerDebug)
            Log(MSG_INFO,"Revive, new HP %I64i, %I64i, Xp added %I64i, new %I64i",thisclient->Stats->HP,Target->Stats->HP,new_exp,thisclient->CharInfo->Exp);
        BEGINPACKET( pak, 0x79b );
        ADDDWORD   ( pak, thisclient->CharInfo->Exp );
        ADDWORD    ( pak, thisclient->CharInfo->stamina );
        ADDWORD    ( pak, 0 );
        thisclient->client->SendPacket( &pak );
	}
	//End of Patch.


	return;
}

// use Debuff skill
void CCharacter::UseDebuffSkill( CCharacter* Enemy, CSkills* skill )
{
    bool bflag = false;
    bflag = GServer->AddBuffs( skill, Enemy, GetInt( ) );
    if(GServer->ServerDebug)
        Log(MSG_INFO,"debufskill %i, %i",clientid,Enemy->clientid);
    if(skill->nbuffs>0 && bflag == true)
    {
        //Log(MSG_INFO,"debufskill %i, %i",clientid,Enemy->clientid);
        BEGINPACKET( pak, 0x7b5 );
        ADDWORD    ( pak, Enemy->clientid );
        ADDWORD    ( pak, clientid );
        ADDWORD    ( pak, Battle->skillid );
        ADDWORD    ( pak, GetInt( ) );
        ADDBYTE    ( pak, skill->nbuffs );
        GServer->SendToVisible( &pak, Enemy );
        //GServer->SendToAllInMap(&pak,this->Position->Map);
    }
    BEGINPACKET( pak, 0x7b9);
    ADDWORD    ( pak, clientid);
    ADDWORD    ( pak, Battle->skillid);
    //ADDWORD    ( pak, 1);
    GServer->SendToVisible( &pak, (CCharacter*)this );
}

// Use a skill (gm command)
bool CCharacter::UseSkill( CSkills* skill, CCharacter *Target )
{
  if (skill->atkpower > 0)
  {
    Log(MSG_INFO, "Need to do %i%s %s in range %i to target %i",
        ( skill->atkpower ),
        ( (skill->range > 0) ? " AOE" : "" ),
        ( (GServer->isSkillTargetFriendly(skill)) ? "healing" : "damage" ),
        ( skill->range ),
        ( skill->target ) );
  }
  for (int i = 0; i < 2; i++) {
    Log(MSG_INFO, "Status ID: %i", skill->status[i]);
    if (skill->status[i] == 0) continue;
    CStatus* status = GServer->GetStatusByID(skill->status[i]);
    if (status == NULL) continue;
    /************************
    We'll probably need to use status->decrease (Figure out whether buf is up/down
    status->repeat will tell us whether it's a one-time (Stat Boost [2]), repeat
    (Recovery, continueing damage [1]) or Special (Status Effect [3]).
    ************************/
    if (status->repeat == 1) // Continuous
    {
      Log(MSG_INFO, "Need to take stat %i and %s it by %i%s over %i seconds", skill->buff[i],
          ((status->decrease) ? "decrease" : "increase"),
          ((skill->value1[i] != 0) ? skill->value1[i] : skill->value2[i]),
          ((skill->value1[i] != 0) ? "" : "%"), skill->duration);

    } else if (status->repeat == 2) // Once (Stat Boost)
    {
      Log(MSG_INFO, "Need to take stat %i and %s it by %i%s for %i seconds", skill->buff[i],
          ((status->decrease) ? "decrease" : "increase"),
          ((skill->value1[i] != 0) ? skill->value1[i] : skill->value2[i]),
          ((skill->value1[i] != 0) ? "" : "%"), skill->duration);

    } else if (status->repeat == 3) // Status Effect (Poison,etc)
    {
      Log(MSG_INFO, "Need to give user status effect %i for %i seconds", skill->buff[i],
          ((skill->value1[i] != 0) ? skill->value1[i] : skill->value2[i]),
          ( skill->duration ) );

    }
  }
  return true;
}

//LMA: We take exp from a dead player.
bool CCharacter::TakeExp( CCharacter *Target )
{
    //LMA: We don't take exp from a player if the killer is a Player.
    if(!Target->IsPlayer()||(Target->IsPlayer()&&IsPlayer()))
    {
        return true;
    }

    //no need to take exp from UW.
    if(Position->Map==9)
    {
        return true;
    }

    //We take 3% of his xp.
    CPlayer* thisclient = GServer->GetClientByID(Target->clientid,Target->Position->Map);
    if (thisclient==NULL)
    {
        CPlayer* thisclient = GServer->GetClientByID(Target->clientid,Position->Map);
    }

    if (thisclient==NULL)
    {
        if(GServer->ServerDebug)
            Log(MSG_WARNING,"Can't find clientID %i to take his Exp...",Target->clientid);
        return true;
    }

    if(GServer->ServerDebug)
        Log(MSG_INFO,"Player %i died, Xp before %I64i",Target->clientid,thisclient->CharInfo->Exp);
    unsigned long long new_exp=(unsigned long long) (thisclient->CharInfo->Exp*3/100);
    thisclient->CharInfo->Exp-=new_exp;
    BEGINPACKET( pak, 0x79b );
    ADDDWORD   ( pak, thisclient->CharInfo->Exp );
    ADDWORD    ( pak, thisclient->CharInfo->stamina );
    ADDWORD    ( pak, 0 );
    thisclient->client->SendPacket( &pak );
    if(GServer->ServerDebug)
        Log(MSG_INFO,"Player %i died, Xp taken %I64i, new %I64i",Target->clientid,new_exp,thisclient->CharInfo->Exp);


    return true;
}

//LMA: Union war kill (and more generally qsd killing / death trigger).
void CCharacter::UWKill(CCharacter* Enemy)
{
    if(!IsPlayer()&&!Enemy->IsPlayer())
    {
        if(GServer->ServerDebug)
            Log(MSG_WARNING,"No player for UWKill");
        return;
    }

    CPlayer* plkiller=NULL;
    CPlayer* plkilled=NULL;

    if(IsPlayer())
    {
        plkiller=(CPlayer*) this;
    }

    if(Enemy->IsPlayer())
    {
        plkilled=(CPlayer*) Enemy;
    }

    int killer_level=0;
    int killed_level=0;

    dword hashkilling = GServer->MapList.Index[Position->Map]->QSDkilling;
    dword hashdeath = GServer->MapList.Index[Position->Map]->QSDDeath;

    //The killer.
    if(hashkilling>0&&plkiller!=NULL)
    {
        if(GServer->ServerDebug)
            Log(MSG_DEBUG,"Executing Quest Trigger for killer from UWKill");
        plkiller->ExecuteQuestTrigger(hashkilling,true);
        //Log(MSG_WARNING,"quest hash %u to the killer %s",hashkilling,plkiller->CharInfo->charname);
        killer_level=plkiller->Stats->Level;
    }

    //The killed one.
    if(hashkilling>0&&plkilled!=NULL)
    {
        if(GServer->ServerDebug)
            Log(MSG_DEBUG,"Executing Quest Trigger for killed from UWKill");
        plkilled->ExecuteQuestTrigger(hashdeath,true);
        //Log(MSG_WARNING,"quest hash %u to the killed %s",hashdeath,plkilled->CharInfo->charname);
        killed_level=plkilled->Stats->Level;
    }

    //Let's give some exp points to the killer...
    if(!IsPlayer()||!Enemy->IsPlayer())
    {
        //only if both are players.
        return;
    }

    if(plkiller==NULL)
    {
        return;
    }

    if(plkilled==NULL)
    {
        killed_level=killer_level;
    }

    UINT bonus_exp=GServer->GetColorExp(killer_level,killed_level,7000);
    plkiller->CharInfo->Exp += bonus_exp;

    //LMA: Only if not level up
    if(plkiller->CharInfo->Exp<plkiller->GetLevelEXP())
    {
        BEGINPACKET( pak, 0x79b );
        ADDDWORD   ( pak, plkiller->CharInfo->Exp );
        ADDWORD    ( pak, plkiller->CharInfo->stamina );

        if(plkilled!=NULL)
        {
            ADDWORD    ( pak, plkilled->clientid );
        }
        else
        {
            ADDWORD    ( pak, 0 );
        }

        plkiller->client->SendPacket( &pak );
        //Log(MSG_WARNING,"Giving %u exp to %s",bonus_exp,plkiller->CharInfo->charname);
    }


  return;
}


//LMA: test for quest hack (stackable).
#ifdef QHACK
//LMA: Adding a kill to the special quest kill list for a player.
void CCharacter::QuestKill(dword die_quest)
{
    CPlayer* plkiller;
    if(!IsPlayer()||die_quest==0)
    {
        if(GServer->ServerDebug)
        {
            if(die_quest == 0)
            {
                Log(MSG_INFO,"No Die Quest for this monster");
            }
        }
        if(IsSummon()&&die_quest!=0)
        {
            CMonster* questMonster = (CMonster*) this;
            plkiller= questMonster->GetOwner();
			if(GServer->ServerDebug)
                Log(MSG_INFO,"Summon got Quest");
        }
        else
        {
            return;
        }
    }
    else
    {
        plkiller = (CPlayer*) this;
        if(GServer->ServerDebug)
        {
            Log(MSG_INFO,"Die Quest for this monster is %i",die_quest);
        }
    }

    //looking for a "free" slot.
    clock_t mytime=clock();
    clock_t oldest_time=mytime;
    int oldest=0;

    for(int k=0;k<10;k++)
    {
        if(plkiller->arr_questid[k].questid==0)
        {
            oldest=k;
            break;
        }

        //looking for time (is it too old?)
        if(mytime>plkiller->arr_questid[k].die_time)
        {
            oldest=k;
            break;
        }

        //is it the oldest? if they are all too recent, it'll be set to 0.
        if(plkiller->arr_questid[k].die_time<oldest_time)
        {
            oldest_time=plkiller->arr_questid[k].die_time;
            oldest=k;
        }

    }

    plkiller->arr_questid[oldest].questid=die_quest;
    plkiller->arr_questid[oldest].die_time=mytime+(CLOCKS_PER_SEC/2);
    if(GServer->ServerDebug)
        Log(MSG_WARNING,"Killing time, offset %i time %u questid %u",oldest,plkiller->arr_questid[oldest].die_time,die_quest);


    return;
}
#endif



