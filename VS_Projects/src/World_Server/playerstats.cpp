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

#include "player.h"
#include "worldserver.h"

//CREDIT TO Kirk, lmame, Tomiz and the Other's :)

// Return Dodge with and without PAT                  //A_DODGE(22) / DODGE(101)
unsigned int CPlayer::GetDodge( )
{
    UINT Dodge = 0;
    UINT pDodge = 0;//Passive Skill % Value
    UINT vDodge = 0;//Passive Skill Value
    int itemDodge = 0;

    //Dodge = (UINT)floor((((Stats->Level * 0.3) + ((Attr->Dex + Attr->Edex) * 1.9)) + 10 ) * 0.4);//Tomiz: Old Formula
    Dodge = (UINT)floor((Stats->Level * 0.3) + ((Attr->Dex + Attr->Edex + (int)Attr->MEdex) * 1.25));//Tomiz:Base Stats FIXED

    for(UINT i=1;i<9;i++)               //Dodge from Item Refine
    {
        if(i==7 || i==1)//Weapon, Mask
        {
            continue;
        }
        if(items[i].count>0)
        {
            if(items[i].itemtype>9)
            {
                Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum);
                continue;
            }

            if(items[i].itemtype== MASK || items[i].itemtype==JEWEL || items[i].itemtype==WEAPON || items[i].count<1)
            {
                Log(MSG_WARNING, "Char %s have equipped some item who shouldn't add dodge in slot %i", CharInfo->charname, i);
                continue;
            }
            if(items[i].itemtype!=9){
                itemDodge = GServer->STB_ITEM[items[i].itemtype-1].rows[items[i].itemnum][30];
                Dodge += (UINT)floor(items[i].durability * 0.3 + itemDodge);
            }

            if(items[i].refine>0)
            {
                UINT refine = (UINT)floor(items[i].refine/16);

                //LMA: using the refine STB now.
                //Dr From Refine (value from LIST_GRADE.stb)
                if(refine>0 && refine<GServer->maxGrades && items[i].itemtype!=9)
                {
                    //UINT extra_refine_drv[10] = {0, 1, 2, 3, 5, 7, 9, 12, 15, 18};//Value
                    //UINT extra_refine_drp[10] = {0, 6, 12, 18, 27, 36, 45, 57, 70, 85};//%
                    //Dodge += (UINT)floor(extra_refine_drv[refine]);
                    //Dodge += (UINT)floor(extra_refine_drp[refine] * 0.01 * (items[i].durability * 0.3) );

                    Dodge += GServer->GradeList[refine]->dodge_addbonus;
                    Dodge += (UINT)floor(GServer->GradeList[refine]->dodge_percent * 0.01 * (items[i].durability * 0.3 + itemDodge) );
                }

            }
        }
    }

    if(Status->Stance != DRIVING)       //Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum);
                    continue;
                }

                //LMA: Adding gem support
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_DODGE || GServer->JemList.Index[items[i].gem]->stat1[0] == DODGE)
                        {
                            Dodge += GServer->JemList.Index[items[i].gem]->stat1[1];
                            Dodge += (UINT)floor(Dodge * GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01);
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_DODGE || GServer->JemList.Index[items[i].gem]->stat2[0] == DODGE)
                        {
                            Dodge += GServer->JemList.Index[items[i].gem]->stat2[1];
                            Dodge += (UINT)floor(Dodge * GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01);
                        }
                    }
                }
                //End gem

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_DODGE || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == DODGE)
                {
                    Dodge += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_DODGE || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == DODGE)
                {
                    Dodge += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats && items[i].gem==0)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_DODGE || GServer->StatsList[items[i].stats]->stat[j] == DODGE)
                        {
                            Dodge += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    if(Status->Stance == DRIVING)       //Driving PAT Stats
    {
        for(UINT i=135;i<140;i++)//PAT Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype != 14)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid PAT item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum);
                    continue;
                }

                for (int j=0;j<2;j++)
                {
                    if(GServer->PatList.Index[items[i].itemnum]->options[j] == A_DODGE || GServer->PatList.Index[items[i].itemnum]->options[j] == DODGE)
                    {
                        Dodge += GServer->PatList.Index[items[i].itemnum]->val_options[j];
                        //Log(MSG_INFO, "Char %s have equipped  PAT item: %i,%i in slot %i who give %i Dodge", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, GServer->PatList.Index[items[i].itemnum]->val_options[j]);
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)   //Dodge From Pasive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Passive SKill
        {
            for(UINT j=0;j<3;j++)
            {
                if(cskills[i].thisskill->buff[j] == DODGE || cskills[i].thisskill->buff[j] == A_DODGE)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pDodge += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vDodge += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    Dodge += Dodge * pDodge / 100;      //Apply Passive Skill % Value
    Dodge += vDodge;                    //Apply Passive Skill Value

    if(Status->Dodge_up!=0xff)          //Dodge UP from Buff
    {
        Dodge += MagicStatus[Status->Dodge_up].Value;
    }
    if(Status->Dodge_down!=0xff)        //Dodge DOWN from Buff
    {
        Dodge -= MagicStatus[Status->Dodge_down].Value;
    }

    if(Dodge<30)
    {
        Dodge=30;
    }

    return Dodge;
}

// Return Accuracy with and without PAT                //A_ACCUR(20) / ATK_ACCURACY(99)
unsigned int CPlayer::GetAccuracy( )
{
    UINT Accuracy = 0;
    UINT pAccuracy = 0;//Passive Skill % Value
    UINT vAccuracy = 0;//Passive Skill Value

    //UINT extra_refine[10] = { 0, 4, 7, 10, 14, 20, 26, 33, 40, 50}; //old Way Value Add (list_grade.stb)
    //UINT extra_refine[10] = { 0, 7, 14, 21, 31, 41, 51, 65, 80, 100}; //New Way % Add (list_grade.stb)

    //LMA: refine 15 is the max now! Bogus values for now (old way anyway, we use STb now).
    //UINT extra_refine[10] = { 0, 7, 14, 21, 31, 41, 51, 65, 80, 100}; //New Way % Add (list_grade.stb)
    //UINT extra_refine[16] = { 0, 7, 14, 21, 31, 41, 51, 65, 80, 100,120,140,160,180,190,197};

    if(Status->Stance != DRIVING)       //Walking, Running, Fighting, ...  Stats
    {
        if(items[7].count == 0)
        {
          //Accuracy = (UINT)floor((((Attr->Con + Attr->Econ )+10)*0.5) + 15 );//Tomiz: Old Formula
            Accuracy = (UINT)floor(Stats->Level + (Attr->Con + Attr->Econ) + ((int)Attr->MEcon));//Tomiz:Naked Stats FIXED  TODO : Fix acc With weapon
        }
        else if(items[7].count !=0)
        {
            //LMA:
            //TODO:  basic formula seems to be wrong now (?) for the weapon.
            //8::18 with 100% dura is acc 218, 0% dura is 188, depends on grade too?
            UINT weapon_acc=0;

            int itemAcc = GServer->EquipList[WEAPON].Index[items[7].itemnum]->acc;

            //weapon_acc=(UINT)floor(((items[7].durability*0.3)) + itemAcc + Stats->Level*0.3); Sousie: Not used, the decimal needs to be taken into account.
            Accuracy = (UINT)floor(((Attr->Con+Attr->Econ+(int)(Attr->MEcon))*1.25) + (((items[7].durability*0.3)) + itemAcc + Stats->Level*0.3));

            if(items[7].refine>0)
            {
                UINT refine = (UINT)floor(items[7].refine/16);

                //LMA: new way (using STB)
                if(refine>0 && refine<GServer->maxGrades)
                {
                    Accuracy += GServer->GradeList[refine]->acc_addbonus + (UINT)floor(GServer->GradeList[refine]->acc_percent * 0.01 * ((items[7].durability*0.3) + itemAcc));
                }

                //LMA: old way.
                /*
                if(refine>0 && refine<16)
                {
                    //Accuracy += extra_refine[refine]; //old Way Value Add (list_grade.stb)
                    Accuracy += (UINT)floor(extra_refine[refine] * 0.01 * (GServer->EquipList[WEAPON].Index[items[7].itemnum]->quality * 0.6)); //New Way % Add (list_grade.stb)
                }*/

            }
        }

        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                //LMA: Adding gem support
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_ACCUR || GServer->JemList.Index[items[i].gem]->stat1[0] == ATK_ACCURACY)
                        {
                            Accuracy += GServer->JemList.Index[items[i].gem]->stat1[1];
                            Accuracy += (UINT)floor(Accuracy * GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01);
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_ACCUR || GServer->JemList.Index[items[i].gem]->stat2[0] == ATK_ACCURACY)
                        {
                            Accuracy += GServer->JemList.Index[items[i].gem]->stat2[1];
                            Accuracy += (UINT)floor(Accuracy * GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01);
                        }
                    }
                }
                //End gem

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_ACCUR || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == ATK_ACCURACY)
                {
                    Accuracy += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_ACCUR || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == ATK_ACCURACY)
                {
                    Accuracy += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats && items[i].gem==0)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_ACCUR || GServer->StatsList[items[i].stats]->stat[j] == ATK_ACCURACY)
                        {
                            Accuracy += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    if(Status->Stance == DRIVING)       //Driving PAT Stats
    {
        for(UINT i=135;i<140;i++)
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype != 14)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid PAT item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum);
                        continue;
                }

                if(GServer->PatList.Index[items[i].itemnum]->attackpower > 0)//If Weapon PAT Attackpower > 0 -> PAT weapon, if PAT weapon -> acc
                {
                    UINT bonusPAT = 0;

                    switch (GServer->PatList.Index[items[i].itemnum]->type)
                    {
                        case 611://Punch Arms 331
                        {
                            bonusPAT -= 26;
                        }
                        break;
                        case 612://Drill Arms 332
                        {
                            bonusPAT -= 20;
                        }
                        break;
                        case 613://Sword Whell Arms 333
                        {
                            bonusPAT -= 11;
                        }
                        break;
                        case 614://Cannon Arms 334
                        {
                            bonusPAT -= 14;
                        }
                        break;
                        case 615://Spear Arms 335
                        {
                            bonusPAT -= 17;
                        }
                        break;
                        case 616://Reinforced Push Arms 336
                        {
                            bonusPAT -= 21;
                        }
                        break;
                        case 571://Stone Hammer, Doom Hammer
                        {
                            switch(GServer->PatList.Index[items[i].itemnum]->id)
                            {
                                case 700://Stone Hammer/ok
                                case 800:
                                {
                                    bonusPAT -= 47;
                                }
                                break;
                                case 701://Doom Hammer/ok
                                case 801:
                                {
                                    bonusPAT -= 44;
                                }
                                break;
                                default://Good Type, New Item?
                                {
                                    Log(MSG_INFO, "Char %s have equipped PAT Weapon type(%i) id(%i), ACC is not coded yet", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type,GServer->PatList.Index[items[i].itemnum]->id);
                                }
                                break;
                            }

                            //Log(MSG_INFO, "Char %s have equipped Stone Hammer or Doom Hammer PAT Weapon type(%i)", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type);
                        }
                        break;
                        case 572://Battle Mirror, Battle Crystal
                        {
                            switch(GServer->PatList.Index[items[i].itemnum]->id)
                            {
                                case 706://Battle Mirror/ok
                                case 806:
                                {
                                    bonusPAT -= 49;
                                }
                                break;
                                case 707://Battle Crystal/ok
                                case 807:
                                {
                                    bonusPAT -= 44;
                                }
                                break;
                                default://Good Type, New Item?
                                {
                                    Log(MSG_INFO, "Char %s have equipped PAT Weapon type(%i) id(%i), ACC is not coded yet", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type,GServer->PatList.Index[items[i].itemnum]->id);
                                }
                                break;
                            }

                            //Log(MSG_INFO, "Char %s have equipped Battle Mirror or Battle Crystal PAT Weapon type(%i)", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type);
                        }
                        break;
                        case 573://Battle Bow, Tarkion Bow
                        {
                            switch(GServer->PatList.Index[items[i].itemnum]->id)
                            {
                                case 702://Battle Bow/ok
                                case 802:
                                {
                                    bonusPAT -= 45;
                                }
                                break;
                                case 703://Tarkion Bow/ok
                                case 803:
                                {
                                    bonusPAT -= 40;
                                }
                                break;
                                default://Good Type, New Item?
                                {
                                    Log(MSG_INFO, "Char %s have equipped PAT Weapon type(%i) id(%i), ACC is not coded yet", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type,GServer->PatList.Index[items[i].itemnum]->id);
                                }
                                break;
                            }

                            //Log(MSG_INFO, "Char %s have equipped Battle Bow or Tarkion Bow PAT Weapon type(%i)", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type);
                        }
                        break;
                        case 574://Battle Katar, Battle Spear
                        {
                            switch(GServer->PatList.Index[items[i].itemnum]->id)
                            {
                                case 704://Battle Katar/ok
                                case 804:
                                {
                                    bonusPAT -= 50;
                                }
                                break;
                                case 705://Battle Spear/ok
                                case 805:
                                {
                                    bonusPAT -= 45;
                                }
                                break;
                                default://Good Type, New Item?
                                {
                                    Log(MSG_INFO, "Char %s have equipped PAT Weapon type(%i) id(%i), ACC is not coded yet", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type,GServer->PatList.Index[items[i].itemnum]->id);
                                }
                                break;
                            }

                            //Log(MSG_INFO, "Char %s have equipped Battle Katar or Battle Spear PAT Weapon type(%i)", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type);
                        }
                        break;
                        case 575://Battle Machine Gun, Battle Launcher
                        {
                            switch(GServer->PatList.Index[items[i].itemnum]->id)
                            {
                                case 708://Battle Machine Gun/ok
                                case 808:
                                {
                                    bonusPAT -= 44;
                                }
                                break;
                                case 709://Battle Launcher/ok
                                case 809:
                                {
                                    bonusPAT -= 39;
                                }
                                break;
                                default://Good Type, New Item?
                                {
                                    Log(MSG_INFO, "Char %s have equipped PAT Weapon type(%i) id(%i), ACC is not coded yet", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type,GServer->PatList.Index[items[i].itemnum]->id);
                                }
                                break;
                            }

                            //Log(MSG_INFO, "Char %s have equipped Battle Machine Gun or Battle Launcher PAT Weapon type(%i)", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type);
                        }
                        break;
                        default://Other PAT Weapon
                        {
                            Log(MSG_INFO, "Char %s have equipped a Cart Weapon type %i ACC is not coded yet", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type);
                        }
                        break;
                    }

                    Accuracy = (UINT)floor(((Attr->Con+10)*0.8) + ((GServer->PatList.Index[items[i].itemnum]->quality*0.6) + 80));
                    Accuracy += bonusPAT + (Stats->Level/2);
                    //Log(MSG_INFO, "Char %s have equipped Weapon PAT item: %i,%i in slot %i who give %i Accuracy", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, Accuracy);
                }

                for (int j=0;j<2;j++)
                {
                    if(GServer->PatList.Index[items[i].itemnum]->options[j] == A_ACCUR || GServer->PatList.Index[items[i].itemnum]->options[j] == ATK_ACCURACY)
                    {
                        Accuracy += GServer->PatList.Index[items[i].itemnum]->val_options[j];
                        //Log(MSG_INFO, "Char %s have equipped  PAT item: %i,%i in slot %i who give Accuracy", CharInfo->charname, items[i].itemtype, items[i].itemnum, i );
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)   //Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++)
            {
                if(cskills[i].thisskill->buff[j] == ATK_ACCURACY || cskills[i].thisskill->buff[j] ==  A_ACCUR)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pAccuracy += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vAccuracy += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    Accuracy += vAccuracy;                  //Apply Passive Skill Value
    Accuracy += Accuracy * pAccuracy / 100;   //Apply Passive Skill % Value

    if(Status->Accuracy_up!=0xff)         //ACC UP from Buff
    {
        Accuracy += MagicStatus[Status->Accuracy_up].Value;
    }
    if(Status->Accuracy_down!=0xff)       //ACC DOWN from Buff
    {
        Accuracy -= MagicStatus[Status->Accuracy_down].Value;
    }

    if(Accuracy<30)
    {
        Accuracy=30;
    }

    return Accuracy;
}

// Return Critical with and without PAT              //A_CRITICAL(26) / CRITICAL(100)
unsigned int CPlayer::GetCritical( )
{
    UINT Critical = 0;
    UINT pCritical = 0;//Passive Skill % Value
    UINT vCritical = 0;//Passive Skill Value

    Critical = (UINT)floor(((Attr->Con + Attr->Econ + (int)Attr->MEcon + 20)*0.2) + Attr->Sen + Attr->Esen + (int)Attr->MEsen);

    if(Status->Stance != DRIVING)       //Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                //LMA: Adding Gem support
                if(items[i].gem!=0)
                {
                        if(GServer->JemList.Index[items[i].gem]!=NULL)
                        {
                            if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_CRITICAL || GServer->JemList.Index[items[i].gem]->stat1[0] == CRITICAL)
                            {
                                Critical += GServer->JemList.Index[items[i].gem]->stat1[1];
                                Critical += (UINT)floor(Critical * GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01);
                            }
                            if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_CRITICAL || GServer->JemList.Index[items[i].gem]->stat2[0] == CRITICAL)
                            {
                                Critical += GServer->JemList.Index[items[i].gem]->stat2[1];
                                Critical += (UINT)floor(Critical * GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01);
                            }
                        }
                    }
                //End Gem

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_CRITICAL || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == CRITICAL)
                {
                    Critical += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_CRITICAL || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == CRITICAL)
                {
                    Critical += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats && items[i].gem==0)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_CRITICAL || GServer->StatsList[items[i].stats]->stat[j] == CRITICAL)
                        {
                            Critical += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

    	if(cskills[i].thisskill->type == 15)//Passive Skill
        {
            for(UINT j=0;j<3;j++)
            {
                if(cskills[i].thisskill->buff[j] == A_CRITICAL || cskills[i].thisskill->buff[j] == CRITICAL)
                {
                    if(cskills[i].thisskill->value2[j] > 0 )
                    {
                        pCritical += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0 )
                    {
                        vCritical += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    Critical += vCritical;//Apply Passive Skill Value
    Critical += Critical * pCritical / 100;//Apply Passive Skill % Value

    }

    if(Status->Stance == DRIVING)       //Driving PAT Stats
    {
        for(UINT i=135;i<140;i++)//PAT Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype != 14)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid PAT item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                for (int j=0;j<2;j++)
                {
                    if(GServer->PatList.Index[items[i].itemnum]->options[j] == A_CRITICAL || GServer->PatList.Index[items[i].itemnum]->options[j] == CRITICAL)
                    {
                        Critical += GServer->PatList.Index[items[i].itemnum]->val_options[j];
                        //Log(MSG_INFO, "Char %s have equipped PAT item: %i,%i in slot %i who give %i Critical", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, GServer->PatList.Index[items[i].itemnum]->val_options[j] );
                    }
                }
            }
        }

        for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
        {
            //Some skills are not worthy to search in
            if(cskills[i].id==0&&i<60)
            {
                i=89;
                continue;
            }
            if(cskills[i].id==0&&(i>60&&i<120))
            {
                i=119;
                continue;
            }
            if(cskills[i].id==0&&i>=120)
            {
                break;
            }
            if(cskills[i].thisskill==NULL)
            {
                continue;
            }

            if(cskills[i].thisskill->type == 15)//Passive Skill
            {
                for(UINT j=0;j<3;j++)
                {
                    if(cskills[i].thisskill->buff[j] == A_CRITICAL || cskills[i].thisskill->buff[j] == CRITICAL)
                    {
                        if(cskills[i].thisskill->value2[j] > 0)
                        {
                            pCritical += cskills[i].thisskill->value2[j];
                        }
                        if(cskills[i].thisskill->value1[j] > 0)
                        {
                            vCritical += cskills[i].thisskill->value1[j];
                        }
                    }
                }
            }
        }

        Critical += vCritical;//Apply Passive Skill Value
        Critical += Critical * pCritical / 100;//Apply Passive Skill % Value
    }

    if(Fairy)
    {
        Critical = (unsigned int)floor(Critical*1.2);
    }

    if(Status->Critical_up!=0xff)       //Crit UP from Buff
    {
        Critical += MagicStatus[Status->Critical_up].Value;
    }
    if(Status->Critical_down!=0xff)     //Crit DOWN from Buff
    {
        Critical -= MagicStatus[Status->Critical_down].Value;
    }

    if(Critical<10)
    {
        Critical=10;
    }

    return Critical;
}


// Return Magic Attack based on the weapon currently equipped
unsigned int CPlayer::GetMagicAttack( )
{
    if(GServer->EquipList[WEAPON].Index[items[7].itemnum]->magicattack == 1)
        return 1;
    else
        return 0;
}

// Return Magic Defense with and without PAT         //A_MRESIST(21) / MAGIC_RESISTENCE_2(98) ok
unsigned int CPlayer::GetMagicDefense( )
{
    UINT MagicDefense = 0;
    UINT pMagicDefense = 0;//Passive Skill % Value
    UINT vMagicDefense = 0;//Passive Skill Value

    MagicDefense += (UINT)floor(((Attr->Int + Attr->Eint + (int)Attr->MEint+5)*0.6) + ((Stats->Level+15)*0.8));

    for(UINT i=1;i<9;i++)//Refine
    {
        if(i==7 || i==1)//Weapon, Mask
        {
            continue;
        }
        if(items[i].count>0)
        {
            if(items[i].itemtype>9)
            {
                Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                continue;
            }

            if(items[i].itemtype== MASK || items[i].itemtype==JEWEL || items[i].itemtype==WEAPON || items[i].count<1 )
            {
                Log(MSG_WARNING, "Char %s have equipped some item who shouldn't add mdeff in slot %i", CharInfo->charname, i );
                continue;
            }

            MagicDefense += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->magicresistence;

            if(items[i].refine>0)//MDeff From Refine
            {
                UINT refine = (UINT)floor(items[i].refine/16);

                //LMA: using STB now.
                if(refine>0 && refine<GServer->maxGrades)
                {
                    /*UINT extra_refine_mdefv[10] = {0, 1, 2, 3, 5, 7, 9, 12, 15, 18};//Value : For Match with Client Value you need this line
                    //UINT extra_refine_mdefp[10] = {0, 6, 12, 18, 27, 36, 45, 57, 70, 85};//% : don't match with client but should be good value, comment this line to match client

                    MagicDefense += (UINT)floor(extra_refine_mdefv[refine]);//Value : For Match with Client Value you need this line
                    //MagicDefense += (UINT)floor(extra_refine_mdefp[refine] * 0.01 * GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->magicresistence );//% : don't match with client but should be good value, comment this line to match client*/

                    MagicDefense +=GServer->GradeList[refine]->mdef_addbonus;
                    MagicDefense += (UINT)floor(GServer->GradeList[refine]->mdef_percent * 0.01 * GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->magicresistence );
                }
            }
        }
    }

    if(Status->Stance != DRIVING)       //Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                //LMA: Adding Gem support
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if( GServer->JemList.Index[items[i].gem]->stat1[0] == A_MRESIST || GServer->JemList.Index[items[i].gem]->stat1[0] == MAGIC_RESISTENCE_2)
                        {
                            MagicDefense += GServer->JemList.Index[items[i].gem]->stat1[1];
                            MagicDefense += (UINT)floor(MagicDefense * GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01);
                        }
                        if( GServer->JemList.Index[items[i].gem]->stat2[0] == A_MRESIST || GServer->JemList.Index[items[i].gem]->stat2[0] == MAGIC_RESISTENCE_2)
                        {
                            MagicDefense += GServer->JemList.Index[items[i].gem]->stat2[1];
                            MagicDefense += (UINT)floor(MagicDefense * GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01);
                        }
                    }
                }
                //End Gem

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_MRESIST || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == MAGIC_RESISTENCE_2)
                {
                    MagicDefense += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_MRESIST || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == MAGIC_RESISTENCE_2)
                {
                    MagicDefense += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats && items[i].gem==0)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_MRESIST || GServer->StatsList[items[i].stats]->stat[j] == MAGIC_RESISTENCE_2)
                        {
                            MagicDefense += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    if(Status->Stance == DRIVING)        //Driving PAT Stats
    {
        for(UINT i=135;i<140;i++)//PAT Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype != 14)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid PAT item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                for (int j=0;j<2;j++)
                {
                    if(GServer->PatList.Index[items[i].itemnum]->options[j] == A_MRESIST || GServer->PatList.Index[items[i].itemnum]->options[j] == MAGIC_RESISTENCE_2)
                    {
                        MagicDefense += GServer->PatList.Index[items[i].itemnum]->val_options[j];
                        //Log(MSG_INFO, "Char %s have equipped PAT item: %i,%i in slot %i who give %i M_Deff", CharInfo->charname, items[i].itemtype, items[i].itemnum, i,GServer->PatList.Index[items[i].itemnum]->val_options[j] );
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)   //Passive Skill
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)    //Passive Skill
        {
            for(UINT j=0;j<3;j++)
            {
                if(cskills[i].thisskill->buff[j] == A_MRESIST || cskills[i].thisskill->buff[j] == MAGIC_RESISTENCE_2)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                         pMagicDefense += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                         vMagicDefense += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    MagicDefense += MagicDefense * pMagicDefense / 100; //Apply Passive Skill % Value
    MagicDefense += vMagicDefense;                      //Apply Passive Skill Value

    switch(CharInfo->Job)                   //Bonus From 2nd job
    {
        case 121:case 122:
        case 221:case 222:
        case 321:case 322:
        case 421:case 422:
        {
            MagicDefense += 20;
        }
        break;
    }

    if(Status->Magic_Defense_up!=0xff)      //MDeff UP from Buff
    {
        MagicDefense += MagicStatus[Status->Magic_Defense_up].Value;
    }
    if(Status->Magic_Defense_down!=0xff)    //Mdeff DOWN from Buff
    {
        MagicDefense -= MagicStatus[Status->Magic_Defense_down].Value;
    }

    return MagicDefense;
}

// Return Attack with and without PAT [TODO] : Cart Weapon       //A_ATTACK(18) and the other for weapon type (104-117) (42-48)
unsigned int CPlayer::GetAttackPower( )
{
    UINT attack = 0;
    UINT weapontype = 0;
    UINT weaponatk = 0;
    UINT pattack = 0;//Passive Skill % Value
    UINT vattack = 0;//Passive Skill Value

    float levelfactor = ((float)Stats->Level/600) + 1; // This formula will work for all Class and weapons.

    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        if(items[7].itemnum!=0 && items[7].count>0 && items[7].durability>0)
        {
            //LMA:
            //TODO: refine 15 is possible now! bogus values for refines after 9.
            //UINT extra_refine[10] = {0, 7, 14, 21, 31, 41, 51, 65, 80 , 100};
            //LMA: using STB now.
            //UINT extra_refine[16] = {0, 7, 14, 21, 31, 41, 51, 65, 80 , 100,120,140,160,180,190,197};

            weaponatk = GServer->EquipList[WEAPON].Index[items[7].itemnum]->attackpower;
            weapontype = GServer->EquipList[WEAPON].Index[items[7].itemnum]->type;

            if(items[7].refine>0)
            {
                UINT refine = (UINT)floor(items[7].refine/16);

                if(refine>0 && refine<GServer->maxGrades)
                {
                    weaponatk += (UINT)floor(GServer->GradeList[refine]->atk_percent * 0.01 * weaponatk)+GServer->GradeList[refine]->atk_addbonus;
                }

                //LMA: old way
                /*if(refine>0 && refine<16)
                {
                    weaponatk += (UINT)floor(extra_refine[refine] * 0.01 * weaponatk);
                }*/

            }

        }
        else
        {
			// FIX : This fixes the Attack Power when a weapon is not equipped. Check with "/mystat2 ap" - Terr0risT
            attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.50 + (Attr->Dex + Attr->Edex)*0.40 + (Attr->Sen + Attr->Esen)*0.10 + Stats->Level*0.05 );
        }

        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_ATTACK)
                        {
                            attack += GServer->JemList.Index[items[i].gem]->stat1[1];
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_ATTACK)
                        {
                            attack += GServer->JemList.Index[items[i].gem]->stat2[1];
                        }
                    }
                }

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_ATTACK)
                {
                    attack += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_ATTACK)
                {
                    attack += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_ATTACK)
                        {
                            attack += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }

        switch( weapontype )//AP By Weapon Type And Passive Skills
        {
            case ONE_HAND_SWORD://one hand sword
            {
                attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.65 + (Attr->Dex + Attr->Edex)*0.25 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Passive Skill
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if( cskills[i].thisskill->buff[j] == ONE_HAND_WEAP_AP || cskills[i].thisskill->buff[j] == ONE_HAND_SWORD_AP )
                            {
                                if( cskills[i].thisskill->value2[j] > 0 )
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if( cskills[i].thisskill->value1[j] > 0 )
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case ONE_HAND_BUNT_SWORD://one hand blunt sword
            {
            attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.65 + (Attr->Dex + Attr->Edex)*0.25 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
            // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for( UINT j=0;j<3;j++ )
                        {
                            if(cskills[i].thisskill->buff[j] == ONE_HAND_WEAP_AP || cskills[i].thisskill->buff[j] == ONE_HAND_MELEE_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case TWO_HAND_SWORD://two hand sword
            {
            attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.65 + (Attr->Dex + Attr->Edex)*0.25 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
            // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == TWO_HAND_WEAP_AP || cskills[i].thisskill->buff[j] == TWO_HAND_SWORD_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case SPEAR://spear
            {
            attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.65 + (Attr->Dex + Attr->Edex)*0.25 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
            // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".


                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == TWO_HAND_WEAP_AP || cskills[i].thisskill->buff[j] == TWO_HAND_SPEAR_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case TWO_HAND_AXE://two hand axe
            {
            attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.65 + (Attr->Dex + Attr->Edex)*0.25 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
            // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == TWO_HAND_WEAP_AP || cskills[i].thisskill->buff[j] == TWO_HAND_AXE_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case BOW://bow
            {
                float arrows = 0;

                if(items[132].count>0)
                {
                    arrows = GServer->NaturalList.Index[items[132].itemnum]->quality*0.5;
                }
                attack += (UINT)floor( (Attr->Dex + Attr->Edex)*0.60 + (Attr->Con + Attr->Econ)*0.30 + (Attr->Sen + Attr->Esen)*0.10 + arrows + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {

                            if(cskills[i].thisskill->buff[j] == BOW_AP || cskills[i].thisskill->buff[j] == BOW_WEAP_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case GUN://gun
            case DUAL_GUN://dual guns
            {
                float bullets = 0;

                if(items[133].count>1)
                {
                    bullets = GServer->NaturalList.Index[items[133].itemnum]->quality * 0.8;
                }
                attack += (UINT)floor( (Attr->Dex + Attr->Edex)*0.40 + (Attr->Con + Attr->Econ)*0.50 + (Attr->Sen + Attr->Esen)*0.10 + bullets + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == GUN_AP || cskills[i].thisskill->buff[j] == GUN_WEAP_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case LAUNCHER://launcher
            {
                float cannons = 0;
                if(items[134].count>0)
                {
                    cannons = GServer->NaturalList.Index[items[134].itemnum]->quality * 0.8;
                }
                attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.35 + (Attr->Con + Attr->Econ)*0.55 + (Attr->Sen + Attr->Esen)*0.10 + cannons + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == LAUNCHER_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case STAFF://staff
            {
                attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.30 + (Attr->Int + Attr->Eint)*0.60 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".
                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == STAFF_POWER || cskills[i].thisskill->buff[j] == MAGIC_WEAP_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case WAND://wand
            {
                attack += (UINT)floor( (Attr->Dex + Attr->Edex)*0.35 + (Attr->Int + Attr->Eint)*0.55 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == WAND_POWER || cskills[i].thisskill->buff[j] == MAGIC_WEAP_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }

                    }
                }
            }
            break;
            case KATAR://katar
            {
                attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.35 + (Attr->Dex + Attr->Edex)*0.55 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == KATAR_AP || cskills[i].thisskill->buff[j] == COMBAT_WEAP_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case DOUBLE_SWORD://double sword
            {
                attack += (UINT)floor( (Attr->Str + Attr->Estr)*0.55 + (Attr->Dex + Attr->Edex)*0.35 + (Attr->Sen + Attr->Esen)*0.10 + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    //Some skills are not worthy to search in
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == DUALSWORD_AP || cskills[i].thisskill->buff[j] == COMBAT_WEAP_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
            case CROSSBOW://crossbow
            {
                float arrows = 0;

                if(items[132].count>0)
                {
                    arrows = GServer->NaturalList.Index[items[132].itemnum]->quality*0.5;
                }
                attack += (UINT)floor( (Attr->Dex + Attr->Edex)*0.40 + (Attr->Str + Attr->Estr)*0.50 + (Attr->Sen + Attr->Esen)*0.10 + arrows + weaponatk * levelfactor );
                // Terr0risT - This should fix the attack power for this weapon. Check with "/mystat 2".

                for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
                {
                    if(cskills[i].id==0&&i<60)
                    {
                        i=89;
                        continue;
                    }
                    if(cskills[i].id==0&&(i>60&&i<120))
                    {
                        i=119;
                        continue;
                    }
                    if(cskills[i].id==0&&i>=120)
                    {
                        break;
                    }
                    if(cskills[i].thisskill==NULL)
                    {
                        continue;
                    }

                    if(cskills[i].thisskill->type == 15)//Pasive
                    {
                        for(UINT j=0;j<3;j++)
                        {
                            if(cskills[i].thisskill->buff[j] == CBOW_AP || cskills[i].thisskill->buff[j] == CBOW_WEAP_AP)
                            {
                                if(cskills[i].thisskill->value2[j] > 0)
                                {
                                    pattack += cskills[i].thisskill->value2[j];
                                }
                                if(cskills[i].thisskill->value1[j] > 0)
                                {
                                    vattack += cskills[i].thisskill->value1[j];
                                }
                            }
                        }
                    }
                }
            }
            break;
        }

        attack += attack * pattack / 100;//Apply Passive Skill % Value
        attack += vattack;//Apply Passive Skill Value
    }

    if(Status->Stance == DRIVING)//Riding PAT Stats
    {
        for(UINT i=135;i<140;i++)//PAT Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype != 14)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid PAT item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                //If equipped without Weapon AND Weapon PAT OR If equipped with Weapon Grade 1 like short sword AND Weapon PAT
                if(  GServer->PatList.Index[items[i].itemnum]->attackpower != 0 && items[7].count == 0 ||
                    (GServer->PatList.Index[items[i].itemnum]->attackpower != 0 && items[7].count > 0 && GServer->EquipList[items[7].itemtype].Index[items[7].itemnum]->id == 1))
                {
                    attack += (UINT)floor( Stats->Level*0.2 + Attr->Str*0.5 + Attr->Dex*0.3 );

                    //CG
                    if( GServer->PatList.Index[items[i].itemnum]->type == 611 || GServer->PatList.Index[items[i].itemnum]->type == 612 || GServer->PatList.Index[items[i].itemnum]->type == 613 ||
                        GServer->PatList.Index[items[i].itemnum]->type == 614 || GServer->PatList.Index[items[i].itemnum]->type == 615 || GServer->PatList.Index[items[i].itemnum]->type == 616 )
                    {
                        attack += GServer->PatList.Index[items[i].itemnum]->attackpower / 2;
                        Log(MSG_INFO, "Char %s have equipped a CG Weapon PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, GServer->PatList.Index[items[i].itemnum]->attackpower / 2);
                    }

                    //Cart
                    if( GServer->PatList.Index[items[i].itemnum]->type == 571 || GServer->PatList.Index[items[i].itemnum]->type == 572 || GServer->PatList.Index[items[i].itemnum]->type == 573 ||
                        GServer->PatList.Index[items[i].itemnum]->type == 574 || GServer->PatList.Index[items[i].itemnum]->type == 575 )
                    {
                        attack += GServer->PatList.Index[items[i].itemnum]->attackpower;
                        Log(MSG_INFO, "Char %s have equipped a Cart Weapon PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, GServer->PatList.Index[items[i].itemnum]->attackpower);
                    }
                }
                //If equipped with Weapon Grade > 1 AND Weapon PAT
                else if(GServer->PatList.Index[items[i].itemnum]->attackpower != 0 && items[7].count > 0 && GServer->EquipList[items[7].itemtype].Index[items[7].itemnum]->id > 1)
                {
                    UINT bonusjob = 0;
                    double bonuslvl = 0;
                    double bonusstr = 0;
                    double bonusdex = 0;

                    switch(CharInfo->Job)//bonuslvl, bonusstr and bonusdex for each class for CG Weapon AP Formula
                    {
                        case KNIGHT:
                        {
                            bonuslvl += 0.48;
                            bonusstr += 1.2;
                            bonusdex += 0.72;
                        }
                        break;
                        case CHAMPION:
                        {
                            bonuslvl += 0.52;
                            bonusstr += 1.3;
                            bonusdex += 0.78;
                        }
                        break;
                        case CLERIC:
                        {
                            bonuslvl += 0.48;
                            bonusstr += 1.2;
                            bonusdex += 0.72;
                        }
                        break;
                        case MAGE:
                        {
                            bonuslvl += 0.48;
                            bonusstr += 1.2;
                            bonusdex += 0.72;
                        }
                        break;
                        case RAIDER:
                        {
                            bonuslvl += 0.44;
                            bonusstr += 1.1;
                            bonusdex += 0.66;
                        }
                        break;
                        case SCOUT:
                        {
                            bonuslvl += 0.48;
                            bonusstr += 1.2;
                            bonusdex += 0.72;
                        }
                        break;
                        case BOURGEOIS:
                        {
                            bonuslvl += 0.48;
                            bonusstr += 1.2;
                            bonusdex += 0.72;
                        }
                        break;
                        case ARTISAN:
                        {
                            bonuslvl += 0.32;
                            bonusstr += 0.8;
                            bonusdex += 0.48;
                        }
                        break;
                        default://TODO: other class and visitor // for now, they got the same value of artisan
                        {
                            bonuslvl += 0.32;
                            bonusstr += 0.8;
                            bonusdex += 0.48;
                        }
                        break;
                    }

                    switch (GServer->PatList.Index[items[i].itemnum]->type)//We check the type of the PAT Weapon because some Weapon can be painted and we will not need to add each id for each color
                    {
                        case 611://Punch Arms// normal one : id.331
                        {
                            switch(CharInfo->Job)
                            {
                                case KNIGHT:
                                {
                                    bonusjob += 390;
                                }
                                break;
                                case CHAMPION:
                                {
                                    bonusjob += 605;
                                }
                                break;
                                case CLERIC:
                                {
                                    bonusjob += 440;
                                }
                                break;
                                case MAGE:
                                {
                                    bonusjob += 400;
                                }
                                break;
                                case RAIDER:
                                {
                                    bonusjob += 215;
                                }
                                break;
                                case SCOUT:
                                {
                                    bonusjob += 370;
                                }
                                break;
                                case BOURGEOIS:
                                {
                                    bonusjob += 400;
                                }
                                break;
                                case ARTISAN:
                                {
                                    bonusjob -= 340;
                                }
                                break;
                                default://TODO: other class and visitor // for now, they got the same value of artisan
                                {
                                    bonusjob -= 340;
                                }
                                break;

                            }

                            attack += (UINT)floor( bonusjob + (Stats->Level * bonuslvl) + (Attr->Str * bonusstr) + (Attr->Dex * bonusdex) + GServer->PatList.Index[items[i].itemnum]->attackpower);
                            //Log(MSG_INFO, "Char %s have equipped Punch Arms PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, attack);

                        }
                        break;
                        case 612://Drill Arms// normal one : id.332
                        {
                            switch(CharInfo->Job)
                            {
                                case KNIGHT:
                                {
                                    bonusjob += 510;
                                }
                                break;
                                case CHAMPION:
                                {
                                    bonusjob += 785;
                                }
                                break;
                                case CLERIC:
                                {
                                    bonusjob += 560;
                                }
                                break;
                                case MAGE:
                                {
                                    bonusjob += 520;
                                }
                                break;
                                case RAIDER:
                                {
                                    bonusjob += 275;
                                }
                                break;
                                case SCOUT:
                                {
                                    bonusjob += 490;
                                }
                                break;
                                case BOURGEOIS:
                                {
                                    bonusjob += 520;
                                }
                                break;
                                case ARTISAN:
                                {
                                    bonusjob -= 460;
                                }
                                break;
                                default://TODO: other class and visitor // for now, they got the same value of artisan
                                {
                                    bonusjob -= 460;
                                }
                                break;

                            }

                            attack += (UINT)floor( bonusjob + (Stats->Level * bonuslvl) + (Attr->Str * bonusstr) + (Attr->Dex * bonusdex) + GServer->PatList.Index[items[i].itemnum]->attackpower);
                            //Log(MSG_INFO, "Char %s have equipped Drill Arms PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, attack);

                        }
                        break;
                        case 613://Sword Whell Arms// normal one : id.333
                        {
                            switch(CharInfo->Job)
                            {
                                case KNIGHT:
                                {
                                    bonusjob += 540;
                                }
                                break;
                                case CHAMPION:
                                {
                                    bonusjob += 830;
                                }
                                break;
                                case CLERIC:
                                {
                                    bonusjob += 590;
                                }
                                break;
                                case MAGE:
                                {
                                    bonusjob += 550;
                                }
                                break;
                                case RAIDER:
                                {
                                    bonusjob += 290;
                                }
                                break;
                                case SCOUT:
                                {
                                    bonusjob += 520;
                                }
                                break;
                                case BOURGEOIS:
                                {
                                    bonusjob += 550;
                                }
                                break;
                                case ARTISAN:
                                {
                                    bonusjob -= 490;
                                }
                                break;
                                default://TODO: other class and visitor // for now, they got the same value of artisan
                                {
                                    bonusjob -= 490;
                                }
                                break;

                            }

                            attack += (UINT)floor( bonusjob + (Stats->Level * bonuslvl) + (Attr->Str * bonusstr) + (Attr->Dex * bonusdex) + GServer->PatList.Index[items[i].itemnum]->attackpower);
                            //Log(MSG_INFO, "Char %s have equipped Sword Whell Arms PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, attack);

                        }
                        break;
                        case 614://Cannon Arms// normal one : id.334 should be ok
                        {
                            switch(CharInfo->Job)
                            {
                                case KNIGHT:
                                {
                                    bonusjob += 640;
                                }
                                break;
                                case CHAMPION:
                                {
                                    bonusjob += 980;
                                }
                                break;
                                case CLERIC:
                                {
                                    bonusjob += 690;
                                }
                                break;
                                case MAGE:
                                {
                                    bonusjob += 650;
                                }
                                break;
                                case RAIDER:
                                {
                                    bonusjob += 340;
                                }
                                break;
                                case SCOUT:
                                {
                                    bonusjob += 620;
                                }
                                break;
                                case BOURGEOIS:
                                {
                                    bonusjob += 650;
                                }
                                break;
                                case ARTISAN:
                                {
                                    bonusjob -= 590;
                                }
                                break;
                                default://TODO: other class and visitor // for now, they got the same value of artisan
                                {
                                    bonusjob -= 590;
                                }
                                break;

                            }

                            attack += (UINT)floor( bonusjob + (Stats->Level * bonuslvl) + (Attr->Str * bonusstr) + (Attr->Dex * bonusdex) + GServer->PatList.Index[items[i].itemnum]->attackpower);
                            //Log(MSG_INFO, "Char %s have equipped Cannon Arms PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, attack);

                        }
                        break;
                        case 615://Spear Arms// normal one : id.335
                        {
                            switch(CharInfo->Job)
                            {
                                case KNIGHT:
                                {
                                    bonusjob += 563;
                                }
                                break;
                                case CHAMPION:
                                {
                                    bonusjob += 865;
                                }
                                break;
                                case CLERIC:
                                {
                                    bonusjob += 613;
                                }
                                break;
                                case MAGE:
                                {
                                    bonusjob += 573;
                                }
                                break;
                                case RAIDER:
                                {
                                    bonusjob += 301;
                                }
                                break;
                                case SCOUT:
                                {
                                    bonusjob += 543;
                                }
                                break;
                                case BOURGEOIS:
                                {
                                    bonusjob += 573;
                                }
                                break;
                                case ARTISAN:
                                {
                                    bonusjob -= 516;
                                }
                                break;
                                default://TODO: other class and visitor // for now, they got the same value of artisan
                                {
                                    bonusjob -= 516;
                                }
                                break;

                            }

                            attack += (UINT)floor( bonusjob + (Stats->Level * bonuslvl) + (Attr->Str * bonusstr) + (Attr->Dex * bonusdex) + GServer->PatList.Index[items[i].itemnum]->attackpower);
                            //Log(MSG_INFO, "Char %s have equipped Spear Arms PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, attack);

                        }
                        break;
                        case 616://Reinforced Push Arms// normal one : id.336
                        {
                            switch(CharInfo->Job)
                            {
                                case KNIGHT:
                                {
                                    bonusjob += 490;
                                }
                                break;
                                case CHAMPION:
                                {
                                    bonusjob += 755;
                                }
                                break;
                                case CLERIC:
                                {
                                    bonusjob += 540;
                                }
                                break;
                                case MAGE:
                                {
                                    bonusjob += 500;
                                }
                                break;
                                case RAIDER:
                                {
                                    bonusjob += 265;
                                }
                                break;
                                case SCOUT:
                                {
                                    bonusjob += 470;
                                }
                                break;
                                case BOURGEOIS:
                                {
                                    bonusjob += 500;
                                }
                                break;
                                case ARTISAN:
                                {
                                    bonusjob -= 440;
                                }
                                break;
                                default://TODO: other class and visitor // for now, they got the same value of artisan
                                {
                                    bonusjob -= 440;
                                }
                                break;

                            }

                            attack += (UINT)floor( bonusjob + (Stats->Level * bonuslvl) + (Attr->Str * bonusstr) + (Attr->Dex * bonusdex) + GServer->PatList.Index[items[i].itemnum]->attackpower);
                            //Log(MSG_INFO, "Char %s have equipped Reinforced Push Arms PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, attack);

                        }
                        break;
                        case 571: case 572: case 573: case 574: case 575://[TODO] :all the cart Weapon
                        {
                            attack += (UINT)floor( Stats->Level*0.2 + Attr->Str*0.5 + Attr->Dex*0.3 );
                            attack += GServer->PatList.Index[items[i].itemnum]->attackpower;
                            Log(MSG_INFO, "Char %s have equipped Cart PAT Weapon type(%i), ATK is not coded yet", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type);
                        }
                        break;
                        default://Other PAT Weapon
                        {
                            attack += (UINT)floor( Stats->Level*0.2 + Attr->Str*0.5 + Attr->Dex*0.3 );
                            attack += GServer->PatList.Index[items[i].itemnum]->attackpower;
                            Log(MSG_INFO, "Char %s have equipped a PAT Weapon type %i, ATK is not coded yet for this type", CharInfo->charname, GServer->PatList.Index[items[i].itemnum]->type);
                        }
                        break;

                    }
                }

                for(int j=0;j<2;j++)
                {
                    if(GServer->PatList.Index[items[i].itemnum]->options[j] == A_ATTACK)
                    {
                        attack += GServer->PatList.Index[items[i].itemnum]->val_options[j];
                        Log(MSG_INFO, "Char %s have equipped PAT item: %i,%i in slot %i who give %i AP", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, GServer->PatList.Index[items[i].itemnum]->val_options[j] );
                    }
                }
            }
        }
    }

    switch(CharInfo->Job)
    {
        case 121:case 122:
        case 221:case 222:
        case 321:case 322:
        case 421:case 422:
            attack += 30;
        break;
    }

    if(Fairy)
    {
        attack = (unsigned int)floor(attack*1.2);
    }

    if(Status->Attack_up!=0xff)
    {
        attack += MagicStatus[Status->Attack_up].Value;
    }
    if(Status->Attack_down!=0xff)
    {
        attack -= MagicStatus[Status->Attack_down].Value;
    }

    if(attack < 0) attack = (UINT)floor(14 + weaponatk);

	return attack;
}

// Return Defense with and without PAT                            //A_DEFENSE(19) / DEFENSE(53) / SHIELD_DEFENSE(102)
unsigned int CPlayer::GetDefense( )
{
    UINT defense = 0;
    UINT statdefense = 0;
    UINT pdefense = 0;//Passive Skill % Value
    UINT vdefense = 0;//Passive Skill Value
    UINT spdefense = 0;//Passive Skill % Value Shield
    UINT svdefense = 0;//Passive Skill Value Shield
    UINT itemdef = 0; //Item defense
    UINT pgemdef = 0;
    UINT subweapdef = 0;

    statdefense += (UINT)floor(((Attr->Str + Attr->Estr + (int)Attr->MEstr)+5) * 0.35 );
    defense += statdefense;

    switch(CharInfo->Job)
    {
        case 121:case 122:
        case 221:case 222:
        case 321:case 322:
        case 421:case 422:
            defense += 25;
        break;
    }

    for(UINT i=1;i<9;i++)//Refine
    {
        if(i==7 || i==1)//Weapon, Mask
        {
            continue;
        }
        if(items[i].count>0)
        {
            if(items[i].itemtype>9)
            {
                Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum);
                continue;
            }

            if(items[i].itemtype== MASK || items[i].itemtype==JEWEL || items[i].itemtype==WEAPON || items[i].count<1)
            {
                Log(MSG_WARNING, "Char %s have equipped some item who shouldn't add deff in slot %i", CharInfo->charname, i );
                continue;
            }

            if(items[i].itemtype==SUBWEAPON)
            {
                subweapdef += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->defense;
            }

            itemdef += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->defense;

            if(items[i].refine>0)//Deff From Refine
            {
                UINT refine = (UINT)floor(items[i].refine/16);

                //LMA: using STB now.
                if(refine>0 && refine<GServer->maxGrades)
                {
                    /*UINT extra_refinev[10] = {0, 1, 2, 3, 5, 7, 9, 12, 15, 18};//Value
                    UINT extra_refinep[10] = {0, 6, 12, 18, 27, 36, 45, 57, 70, 85};//%
                    defense += (UINT)floor(extra_refinev[refine]);
                    defense += (UINT)floor(extra_refinep[refine] * 0.01 * GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->defense );*/

                    itemdef += GServer->GradeList[refine]->def_addbonus;
                    itemdef += (UINT)floor(GServer->GradeList[refine]->def_percent * 0.01 * GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->defense );
                    if(items[i].itemtype==SUBWEAPON)
                    {
                        subweapdef += GServer->GradeList[refine]->def_addbonus;
                        subweapdef += (UINT)floor(GServer->GradeList[refine]->def_percent * 0.01 * GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->defense );
                    }
                }
            }
        }
    }

    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_DEFENSE || GServer->JemList.Index[items[i].gem]->stat1[0] == DEFENSE)
                        {
                            itemdef += GServer->JemList.Index[items[i].gem]->stat1[1];
                            pgemdef += GServer->JemList.Index[items[i].gem]->stat1[2];
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_DEFENSE || GServer->JemList.Index[items[i].gem]->stat2[0] == DEFENSE)
                        {
                            itemdef += GServer->JemList.Index[items[i].gem]->stat2[1];
                            pgemdef += GServer->JemList.Index[items[i].gem]->stat2[2];
                        }
                    }
                }

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_DEFENSE || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == DEFENSE)
                {
                    itemdef += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_DEFENSE || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == DEFENSE)
                {
                    itemdef += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats && items[i].gem==0)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_DEFENSE || GServer->StatsList[items[i].stats]->stat[j] == DEFENSE)
                        {
                            itemdef += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    if(Status->Stance == DRIVING)//PAT Stats
    {
        for(UINT i=135;i<140;i++)//PAT Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype != 14)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid PAT item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                for (int j=0;j<2;j++)
                {
                    if(GServer->PatList.Index[items[i].itemnum]->options[j] == A_DEFENSE || GServer->PatList.Index[items[i].itemnum]->options[j] == DEFENSE || GServer->PatList.Index[items[i].itemnum]->options[j] == SHIELD_DEFENSE)
                    {
                        defense += GServer->PatList.Index[items[i].itemnum]->val_options[j];
                        //Log(MSG_INFO, "Char %s have equipped  PAT item: %i,%i in slot %i who give %i Deff", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, GServer->PatList.Index[items[i].itemnum]->val_options[j] );
                        defense -= (defense * 1 / 100) + 10;//it's for try to match client value, Comment this line if you need exact deff from PAT Add.
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Defense From Pasive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++)
            {
                if(cskills[i].thisskill->buff[j] == DEFENSE || cskills[i].thisskill->buff[j] == A_DEFENSE)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pdefense += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vdefense += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    //Log(MSG_HACK, "defense: %i, pdefense: %i, statdefense:%i, vdefense: %i", defense, pdefense, statdefense,vdefense);
    defense += itemdef * pdefense / 100;//Apply Passive Skill % Value
    defense += (UINT)floor(statdefense * pdefense / 100);
    defense += (UINT)floor(statdefense * pgemdef /100);
    defense += (UINT)ceil(itemdef * pgemdef / 100);
    defense += itemdef;
    defense += vdefense;//Apply Passive Skill Value

    if(items[8].count>0)//Defense From Shield Pasive Skill
    {
        for(UINT i=0;i<MAX_ALL_SKILL;i++)
        {
            //Some skills are not worthy to search in
            if(cskills[i].id==0&&i<60)
            {
                i=89;
                continue;
            }
            if(cskills[i].id==0&&(i>60&&i<120))
            {
                i=119;
                continue;
            }
            if(cskills[i].id==0&&i>=120)
            {
                break;
            }
            if(cskills[i].thisskill==NULL)
            {
                continue;
            }

            if(cskills[i].thisskill->type == 15)//Pasive
            {
                for(UINT j=0;j<3;j++)
                {
                    if(cskills[i].thisskill->buff[j] == SHIELD_DEFENSE)
                    {
                        if(cskills[i].thisskill->value2[j] > 0)
                        {
                            spdefense += cskills[i].thisskill->value2[j];
                        }
                        if(cskills[i].thisskill->value1[j] > 0)
                        {
                            svdefense += cskills[i].thisskill->value1[j];
                        }
                    }
                }
            }
        }
    }

    //Log(MSG_HACK,"subweapdef: %i, spdefense: %i, svdefense: %i", subweapdef, spdefense, svdefense);
    defense += subweapdef * spdefense / 100;//Apply Passive Skill Shield % Value
    defense += svdefense;//Apply Passive Skill Shield Value

    if(Fairy)
    {
        defense = (unsigned int)floor(defense*1.2);
    }

    if(Status->Defense_up!=0xff)
    {
        defense += MagicStatus[Status->Defense_up].Value;
    }
    if(Status->Defense_down!=0xff)
    {
        defense -= MagicStatus[Status->Defense_down].Value;
    }



    return defense;
}

// Return Attack Speed with and without PAT                                     //A_HASTE(24) and the other for weapon type (118-131) (49-51) ok
UINT CPlayer::GetAttackSpeed( )
{
    int aspeed = 0;

    //LMA: new formula:
    int ind_0=17;   //For naRose BEFORE client 259, it's 12. It's the Fast + 0 (normal) offset.
    if(items[7].itemnum!=0 && items[7].count > 0)
    {
        UINT wpnspd = 0;
        wpnspd = GServer->EquipList[WEAPON].Index[items[7].itemnum]->attackspeed;

        UINT temp_speed = 88 + ((ind_0 - wpnspd) * 5 );
        if (temp_speed <= 0 || wpnspd == 0)
        {
            Log(MSG_WARNING,"Weird Aspeed value for weapon value=%i (STB ASpeed=%i)",items[7].itemnum,wpnspd);
            temp_speed=88;
        }

        aspeed=temp_speed;
    }
    else
    {
        //bare fists.
        aspeed = 128;
    }
    Log(MSG_DEBUG, "ASPD: %i", aspeed);
    return aspeed;

    //old formula.
    /*
    if(items[7].itemnum!=0 && items[7].count > 0)
    {
        UINT wpnspd = 0;
        wpnspd = GServer->EquipList[WEAPON].Index[items[7].itemnum]->attackspeed;

        switch(wpnspd)
        {
            case 20:
                aspeed=53;  //slow -8
            break;
            case 18:
                aspeed=63;  //slow -6
            break;
            case 13:
                aspeed=83;  //LMA: slow -1
            break;
            case 12: //normal
                aspeed = 88;
            break;
            case 16:
                //fast +1, same as 11
                aspeed = 93;
            break;
            case 11: //+1
                aspeed = 93;
            break;
            case 10: //+2
                aspeed = 100;
            break;
            case 9: //+3
                aspeed = 107;
            break;
            case 8: //+4
                aspeed = 115;
            break;
            case 7: //+5
                aspeed = 125;
            break;
            case 6: //+6
                aspeed = 136;
            break;
            default:
            {
                aspeed=88;  //LMA: normal, else there won't be any attacks.
                Log(MSG_WARNING,"Unknow aspeed for weapon %i (%i)",items[7].itemnum,wpnspd);
            }
            break;
        }
    }
    else
    {
        aspeed = 115;
    }
    */
}

float CPlayer::GetAttackSpeedPercent()
{
    float paspeed = 0;//Passive Skill % Value
    UINT weapontype = 0;
    weapontype = GServer->EquipList[WEAPON].Index[items[7].itemnum]->type;

    switch(weapontype)//Aspeed By Weapon Type And Passive Skills
    {
        case ONE_HAND_SWORD:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == ONE_HAND_SWORD_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case ONE_HAND_BUNT_SWORD:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == ONE_HAND_MELEE_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case TWO_HAND_SWORD:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == TWO_HAND_SWORD_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case SPEAR:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == TWO_HAND_SPEAR_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case TWO_HAND_AXE:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == TWO_HAND_AXE_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case BOW:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                //Some skills are not worthy to search in
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if( cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == BOW_ATK_SPD || cskills[i].thisskill->buff[j] == BOW_WEAP_ASPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case GUN:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                //Some skills are not worthy to search in
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == GUN_ATK_SPD || cskills[i].thisskill->buff[j] == GUN_WEAP_ASPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case DUAL_GUN:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                //Some skills are not worthy to search in
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == GUN_ATK_SPD || cskills[i].thisskill->buff[j] == DUAL_GUN_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case LAUNCHER:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                //Some skills are not worthy to search in
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == LAUNCHER_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case STAFF:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == STAFF_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case WAND:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == WAND_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case KATAR:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                //Some skills are not worthy to search in
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == KATAR_ATK_SPD || cskills[i].thisskill->buff[j] == COMBAT_WEAP_ASPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case DOUBLE_SWORD:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                //Some skills are not worthy to search in
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == DUALSWORD_ATK_SPD || cskills[i].thisskill->buff[j] == COMBAT_WEAP_ASPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
        case CROSSBOW:
            for(UINT i=0;i<MAX_ALL_SKILL;i++)
            {
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == CBOW_ATK_SPD || cskills[i].thisskill->buff[j] == A_HASTE)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                paspeed += cskills[i].thisskill->value2[j];
                            }
                        }
                    }
                }
            }
            break;
    }

    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
	{
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                        continue;
                }

                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_HASTE)
                        {
                            paspeed += ((int)(GServer->JemList.Index[items[i].gem]->stat1[1])*0.3);
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_HASTE)
                        {
                            paspeed += ((int)(GServer->JemList.Index[items[i].gem]->stat2[1])*0.3);
                        }
                    }
                }

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_HASTE)
                {
                    paspeed += ((GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1])*0.3);
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_HASTE)
                {
                    paspeed += ((GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1])*0.3);
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats && items[i].gem==0)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_HASTE)
                        {
                            paspeed += ((GServer->StatsList[items[i].stats]->value[j])*0.3);
                        }
                    }
                }
            }
        }
	}

    if(Status->Stance == DRIVING)//PAT Use Stats
    {
        for(UINT i=135;i<140;i++)//PAT Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype != 14)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid PAT item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                for (int j=0;j<2;j++)
                {
                    if(GServer->PatList.Index[items[i].itemnum]->options[j] == A_HASTE)
                    {
                        paspeed += ((int)GServer->PatList.Index[items[i].itemnum]->val_options[j]);
                        //Log(MSG_INFO, "Char %s have equipped PAT item: %i,%i in slot %i who give %i Aspeed", CharInfo->charname, items[i].itemtype, items[i].itemnum, i, GServer->PatList.Index[items[i].itemnum]->val_options[j] );
                    }
                }
            }
        }
    }

    if(Status->Haste_up!=0xff)
    {
        paspeed += MagicStatus[Status->Haste_up].Value;
    }
    if(Status->Haste_down!=0xff)
    {
        paspeed -= MagicStatus[Status->Haste_down].Value;
    }

    Log(MSG_DEBUG, "ASPD: %f", paspeed);
    return paspeed;
}

//LMA: Special Move Speed calcul for PAT change (only multiplicator)
unsigned int CPlayer::GetCartSpeed( )
{
       unsigned int mspeed=0;
        UINT porc = 1;
        UINT nb_parts=0;
        float lma_speed;

        //returns a value only if cart is complete.
        if (items[135].itemnum==0||items[136].itemnum==0||items[137].itemnum==0)
        {
           return 0;
        }
        if(items[138].itemnum==0&&items[139].itemnum==0)
        {
           return 0;
        }

        for(int k=135;k<138;k++)
        {
          mspeed+=GServer->PatList.Index[items[k].itemnum]->speed;
        }

    return mspeed;
}

// Return Movement Speed with and without PAT   //MOV_SPEED(52) / A_DASH(23)
unsigned int CPlayer::GetMoveSpeed( )
{
	UINT mspeed = 425;
    UINT pmspeed = 0;//Passive Skill % Value
    UINT vmspeed = 0;//Passive Skill Value

    //LMA: Adding a special case since we can drive beeing overweight.
    if(!Status->CanRun&&Status->Stance!=DRIVING)
    {
        Status->Stance = WALKING;
    }

    switch(Status->Stance)
    {
        case WALKING: //walking
        {
            mspeed = 200;
            Stats->Base_Speed=mspeed;   //LMA: Changing the base speed too ^_^
        }
        break;
        case 1:
        case RUNNING: // running
        {
            if(items[6].count != 0)//Foot
            {
                UINT tspeed = GServer->EquipList[SHOE].Index[items[6].itemnum]->movespeed;
        		mspeed += ( tspeed - 65 ) * 5;
            }

            for(UINT i=0;i<12;i++)//Cloth Stats
            {
                if(items[i].count>0)
                {
                    if(items[i].itemtype>9)
                    {
                        Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum);
                        continue;
                    }

                    //LMA: Adding gem support
                    if(items[i].gem!=0)
                    {
                        if(GServer->JemList.Index[items[i].gem]!=NULL)
                        {
                            if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_DASH || GServer->JemList.Index[items[i].gem]->stat1[0] == MOV_SPEED)
                            {
                                mspeed += GServer->JemList.Index[items[i].gem]->stat1[1];
                            }
                            if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_DASH || GServer->JemList.Index[items[i].gem]->stat2[0] == MOV_SPEED)
                            {
                                mspeed += GServer->JemList.Index[items[i].gem]->stat2[1];
                            }
                        }
                    }
                    //End gem

                    if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_DASH || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == MOV_SPEED)
                    {
                        mspeed += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                    }
                    if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_DASH || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == MOV_SPEED)
                    {
                       mspeed += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                    }

                    if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                    {
                        for(UINT j=0;j<2;j++)
                        {
                            if(GServer->StatsList[items[i].stats]->stat[j] == A_DASH || GServer->StatsList[items[i].stats]->stat[j] == MOV_SPEED)
                            {
                                mspeed += GServer->StatsList[items[i].stats]->value[j];
                            }
                        }
                    }
                }
            }

            for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
            {
                //Some skills are not worthy to search in
                if(cskills[i].id==0&&i<60)
                {
                    i=89;
                    continue;
                }
                if(cskills[i].id==0&&(i>60&&i<120))
                {
                    i=119;
                    continue;
                }
                if(cskills[i].id==0&&i>=120)
                {
                    break;
                }
                if(cskills[i].thisskill==NULL)
                {
                    continue;
                }

                if(cskills[i].thisskill->type == 15)//Pasive
                {
                    for(UINT j=0;j<3;j++)
                    {
                        if(cskills[i].thisskill->buff[j] == MOV_SPEED || cskills[i].thisskill->buff[j] == A_DASH)
                        {
                            if(cskills[i].thisskill->value2[j] > 0)
                            {
                                pmspeed += cskills[i].thisskill->value2[j];
                            }
                            if(cskills[i].thisskill->value1[j] > 0)
                            {
                                vmspeed += cskills[i].thisskill->value1[j];
                            }
                        }
                    }
                }
            }

            mspeed += mspeed * pmspeed / 100;//Apply Passive Skill % Value
            mspeed += vmspeed;//Apply Passive Skill Value

            //LMA: Base Speed.
            Stats->Base_Speed=mspeed;

            if(Fairy)
            {
                mspeed = (unsigned int)floor(mspeed*1.2);
            }

            if(Status->Dash_up!=0xff)
            {
                mspeed += MagicStatus[Status->Dash_up].Value;
            }
            if(Status->Dash_down!=0xff)
            {
                mspeed -= MagicStatus[Status->Dash_down].Value;
            }
        }
        break;
        case DRIVING: //cart
        {
            if(!Status->CanRun)
            {
                mspeed=300;
                Stats->Base_Speed=mspeed;   //LMA: Changing the base speed too ^_^

                if(Fairy)
                {
                    mspeed = (unsigned int)floor(mspeed*1.2);
                }


                return mspeed;
            }

            UINT porc = 1;
            UINT nb_parts=0;
            float lma_speed;

            //LMA: new algo for mspeed calculation...
            for (int k=135;k<140;k++)
            {
                 if(items[k].itemnum!=0&&GServer->PatList.Index[items[k].itemnum]->speed>0)
                 {
                    nb_parts++;
                    porc = porc * GServer->PatList.Index[items[k].itemnum]->speed;
                 }

            }

            if (nb_parts>0)
            {
               lma_speed=floor(porc*1000/(pow(100,nb_parts)));
            }

            //LMA: new way:
            /*
            if(items[138].itemnum!=0)
            {
                lma_speed+=GServer->PatList.Index[items[138].itemnum]->modifier;
            }
            */

            //LMA: adding DASH if needed.
            for (int k=135;k<140;k++)
            {
                if(items[k].itemnum==0)
                    continue;

                for(int j=0;j<2;j++)
                {
                    if(GServer->PatList.Index[items[k].itemnum]->options[j]!=A_DASH)
                        continue;
                    lma_speed+=GServer->PatList.Index[items[k].itemnum]->val_options[j];
                }

            }

            mspeed= (UINT) lma_speed;

            //LMA: Base Speed.
            Stats->Base_Speed=mspeed;
            /*if(Fairy)  mspeed = (unsigned int)floor(mspeed*1.2);*/

            //LMA: new way :)
            if(Fairy)
            {
                if(GServer->PatList.Index[items[135].itemnum]->parttype!=31)
                {
                    //cart
                    //mspeed+=148;
                    mspeed = (unsigned int)floor(mspeed*1.2);
                }
                else
                {
                    //CG
                    //mspeed+=101;
                    mspeed = (unsigned int)floor(mspeed*1.2);
                }

            }

            if(Status->Dash_up!=0xff)
            {
                mspeed += MagicStatus[Status->Dash_up].Value;
            }
            if(Status->Dash_down!=0xff)
            {
                mspeed -= MagicStatus[Status->Dash_down].Value;
            }
        }
        break;
    }

    return mspeed;
}

// Return Max HP with and without PAT           //A_MAX_HP(38) / A_HP(16) / MAX_HP(54)
unsigned long long CPlayer::GetMaxHP( )
{
	if(Stats->Level<1)
	{
        Stats->Level=1;
	}

	unsigned int hpmax = (unsigned int)floor(((sqrt(Stats->Level + 20) * (Stats->Level + 5 )) * 3.5) + ((Attr->Str + Attr->Estr + (int)Attr->MEstr) << 1));
	unsigned int hpmaxTemp = hpmax;

    UINT phpmax = 0;//Passive Skill % Value
    UINT vhpmax = 0;//Passive Skill Value


    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                //LMA: Adding Amethyst support (HP gems)
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_MAX_HP)
                        {
                            hpmax += (unsigned int)floor(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * hpmaxTemp);
                            hpmax += GServer->JemList.Index[items[i].gem]->stat1[1];
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_MAX_HP)
                        {
                            hpmax += (unsigned int)floor(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * hpmaxTemp);
                            hpmax += GServer->JemList.Index[items[i].gem]->stat2[1];
                        }
                    }
                }
                //End Amethyst

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_MAX_HP || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_HP ||
                    GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == MAX_HP)
                {
                    hpmax += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_MAX_HP || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_HP ||
                    GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == MAX_HP)
                {
                    hpmax += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats && items[i].gem==0)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if( GServer->StatsList[items[i].stats]->stat[j] == A_MAX_HP || GServer->StatsList[items[i].stats]->stat[j] == A_HP ||
                            GServer->StatsList[items[i].stats]->stat[j] == MAX_HP)
                        {
                            hpmax += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++ )
            {
                if(cskills[i].thisskill->buff[j] == A_MAX_HP || cskills[i].thisskill->buff[j] == A_HP || cskills[i].thisskill->buff[j] == MAX_HP)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        phpmax += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vhpmax += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    hpmax += hpmax * phpmax / 100;//Apply Passive Skill % Value
    hpmax += vhpmax;//Apply Passive Skill Value

    switch(CharInfo->Job)
    {
        case 121:case 122:
        case 221:case 222:
        case 321:case 322:
        case 421:case 422:
            hpmax += 300;
        break;
    }

    if(Fairy)
    {
        hpmax = (unsigned int)floor(hpmax*1.2);
    }

    if(Status->HP_up!=0xff)
    {
        hpmax += MagicStatus[Status->HP_up].Value;
    }
    if(Status->HP_down!=0xff)
    {
        hpmax -= MagicStatus[Status->HP_down].Value;
    }

    return hpmax;
}

// Return Max MP with and without PAT           //A_MAX_MP(39) / A_MP(17) / MAX_MP(55)
unsigned int CPlayer::GetMaxMP( )
{

    if(Stats->Level < 0)
    {
        Stats->Level = 1;
    }

    UINT maxmp = 0;
    UINT pmaxmp = 0;//Passive Skill % Value
    UINT vmaxmp = 0;//Passive Skill Value

    float Mult = 0;
    int JobValue = (int)CharInfo->Job;

    switch(JobValue)
    {
        case 111:
        case 411:
        case 421:
            Mult = 4;
            JobValue = 3;
        break;
        case 122:
            Mult = 5;
            JobValue = -6;
        break;
        case 211:
            Mult = 6;
            JobValue = 0;
        break;
        case 221:
            Mult = 7;
            JobValue = -7;
        break;
        case 222:
            Mult = 6.5;
            JobValue = -4;
        break;
        case 311:
        case 321:
            Mult = 4;
            JobValue = 4; //LMA
        break;
        case 121:
        case 322:
        case 422:
            Mult = 4.5;
            JobValue = 0;
        break;
        default:
            Mult = 3;
            JobValue = 4;
        break;
    }

    maxmp = (UINT)floor((Stats->Level + JobValue) * Mult + ((Attr->Int + Attr->Eint) << 2));

    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                //LMA: Adding Turquoise support (MP gems)
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_MAX_MP || GServer->JemList.Index[items[i].gem]->stat1[0] == A_MP ||
                            GServer->JemList.Index[items[i].gem]->stat1[0] == MAX_MP)
                        {
                            maxmp += GServer->JemList.Index[items[i].gem]->stat1[1];
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_MAX_MP || GServer->JemList.Index[items[i].gem]->stat2[0] == A_MP ||
                            GServer->JemList.Index[items[i].gem]->stat2[0] == MAX_MP)
                        {
                            maxmp += GServer->JemList.Index[items[i].gem]->stat2[1];
                        }
                    }
                }
                //End Turquoise

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_MAX_MP || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_MP ||
                    GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == MAX_MP)
                {
                    maxmp += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_MAX_MP || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_MP ||
                    GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == MAX_MP)
                {
                    maxmp += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if( GServer->StatsList[items[i].stats]->stat[j] == A_MAX_MP || GServer->StatsList[items[i].stats]->stat[j] == A_MP ||
                            GServer->StatsList[items[i].stats]->stat[j] == MAX_MP)
                        {
                            maxmp += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15) //Pasive
        {
            for(UINT j=0;j<3;j++)
            {
                if(cskills[i].thisskill->buff[j] == A_MAX_MP || cskills[i].thisskill->buff[j] == A_MP || cskills[i].thisskill->buff[j] == MAX_MP)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pmaxmp += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vmaxmp += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    maxmp += maxmp * pmaxmp / 100;//Apply Passive Skill % Value
    maxmp += vmaxmp;//Apply Passive Skill Value

    if(Fairy)
    {
        maxmp = (unsigned int)floor(maxmp*1.2);
    }

    if(Status->MP_up!=0xff)
    {
        maxmp += MagicStatus[Status->MP_up].Value;
    }
    if(Status->MP_down!=0xff)
    {
        maxmp -= MagicStatus[Status->MP_down].Value;
    }

	return maxmp;
}

// Get HP Regeneration Amount                   //A_HP_REC_RATE(27) / HP_REC_AMONT5(56)
unsigned int CPlayer::GetHPRegenAmount( )
{
    UINT amount = (UINT)ceil ( Stats->MaxHP * 0.022 );
    UINT pamount = 0;//Passive Skill % Value
    UINT vamount = 0;//Passive Skill Value

    /*LMA: handled elsewhere.
    if( Status->Stance == 1 )
        amount *= 4;*/

    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)              //Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                //LMA: Adding gem support
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_HP_REC_RATE || GServer->JemList.Index[items[i].gem]->stat1[0] == HP_REC_AMONT)
                        {
                            amount += GServer->JemList.Index[items[i].gem]->stat1[1];
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_HP_REC_RATE || GServer->JemList.Index[items[i].gem]->stat2[0] == HP_REC_AMONT)
                        {
                            amount += GServer->JemList.Index[items[i].gem]->stat2[1];
                        }
                    }
                }
                //End gem

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_HP_REC_RATE || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == HP_REC_AMONT)
                {
                    amount += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_HP_REC_RATE || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == HP_REC_AMONT)
                {
                    amount += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_HP_REC_RATE || GServer->StatsList[items[i].stats]->stat[j] == HP_REC_AMONT)
                        {
                            amount += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++)
            {
                if(cskills[i].thisskill->buff[j] == A_HP_REC_RATE || cskills[i].thisskill->buff[j] == HP_REC_AMONT)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pamount += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vamount += cskills[i].thisskill->value2[j];
                    }
                }
            }
        }
    }

    amount += vamount;//Apply Passive Skill Value
    amount += amount * pamount / 100;//Apply Passive Skill % Value

    //Log(MSG_INFO,"HPRegenAmount : %i", amount);

    return amount;
}

// Get MP Regeneration Amount                   //A_MP_REC_RATE(28) / MP_REC_RATE(57)
unsigned int CPlayer::GetMPRegenAmount( )
{
    UINT amount = (UINT)ceil ( Stats->MaxMP * 0.03 );
    UINT pamount = 0;//Passive Skill % Value
    UINT vamount = 0;//Passive Skill Value

    /*LMA: Taken elsewhere
    if( Status->Stance == 1 )
        amount *= 4;
    */

    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                    continue;
                }

                //LMA: Adding gem support
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_MP_REC_RATE || GServer->JemList.Index[items[i].gem]->stat1[0] == MP_REC_RATE)
                        {
                            amount += GServer->JemList.Index[items[i].gem]->stat1[1];
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_MP_REC_RATE || GServer->JemList.Index[items[i].gem]->stat2[0] == MP_REC_RATE)
                        {
                            amount += GServer->JemList.Index[items[i].gem]->stat2[1];
                        }
                    }
                }
                //End gem

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_MP_REC_RATE || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == MP_REC_RATE)
                {
                    amount += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_MP_REC_RATE || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == MP_REC_RATE)
                {
                    amount += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_MP_REC_RATE || GServer->StatsList[items[i].stats]->stat[j] == MP_REC_RATE)
                        {
                            amount += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)   //Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++ )
            {
                if(cskills[i].thisskill->buff[j] == A_MP_REC_RATE || cskills[i].thisskill->buff[j] == MP_REC_RATE)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pamount += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vamount += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    amount += vamount;//Apply Passive Skill Value
    amount += amount * pamount / 100;//Apply Passive Skill % Value

    //Log(MSG_INFO,"MPRegenAmount : %i", amount);

    return amount;
}

// max weight supported                         //A_INVENTORY_CAPACITY(25) / BAGPACK_CAPACITY(58)
unsigned int CPlayer::GetMaxWeight( )
{
    UINT weight = 0;
    UINT pweight = 0;//Passive Skill % Value
    UINT vweight = 0;//Passive Skill Value

    weight = ((Attr->Str + Attr->Estr) + (Attr->Str + Attr->Estr)*2) * 2 + Stats->Level+220 + (Stats->Level+220)*4;

    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                        continue;
                }

                //LMA: Adding gem support
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == A_INVENTORY_CAPACITY || GServer->JemList.Index[items[i].gem]->stat1[0] == BAGPACK_CAPACITY)
                        {
                            weight += GServer->JemList.Index[items[i].gem]->stat1[1];
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == A_INVENTORY_CAPACITY || GServer->JemList.Index[items[i].gem]->stat2[0] == BAGPACK_CAPACITY)
                        {
                            weight += GServer->JemList.Index[items[i].gem]->stat2[1];
                        }
                    }
                }
                //End gem

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == A_INVENTORY_CAPACITY || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == BAGPACK_CAPACITY)
                {
                    weight += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == A_INVENTORY_CAPACITY || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == BAGPACK_CAPACITY)
                {
                    weight += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == A_INVENTORY_CAPACITY || GServer->StatsList[items[i].stats]->stat[j] == BAGPACK_CAPACITY)
                        {
                            weight += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++ )
            {
                if(cskills[i].thisskill->buff[j] == A_INVENTORY_CAPACITY || cskills[i].thisskill->buff[j] == BAGPACK_CAPACITY)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pweight += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vweight += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    weight += vweight;//Apply Passive Skill Value
    weight += weight * pweight / 100;//Apply Passive Skill % Value

    //Log(MSG_INFO,"Max Weight Supported: %i", weight);

    return weight;
}

// Current weight
unsigned int CPlayer::GetCurrentWeight( )
{
    UINT weight = 0;

    for(UINT i=0;i<MAX_INVENTORY;i++)
    {
        if(items[i].itemtype==0||items[i].itemnum==0)
        {
            continue;
        }
        if(items[i].itemtype<10)
        {
            weight += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->weight;
        }
        else
        {
            switch(items[i].itemtype)
            {
                case CONSUMIBLE:
                    weight += GServer->UseList.Index[items[i].itemnum]->weight*items[i].count;
                break;
                case JEM:
                    weight += GServer->JemList.Index[items[i].itemnum]->weight*items[i].count;
                break;
                case NATURAL:
                    weight += GServer->NaturalList.Index[items[i].itemnum]->weight*items[i].count;
                break;
                case PAT:
                    weight += GServer->PatList.Index[items[i].itemnum]->weight;
                break;
            }
        }
    }

    return weight;
}

// MP Reduction                                 //MP_COST_RED(61) / MP_CONSUME(29)
unsigned int CPlayer::GetMPReduction( )
{
    UINT mpreduction = 0;
    UINT pmpreduction = 0;//Passive Skill % Value
    UINT vmpreduction = 0;//Passive Skill Value

    if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                        continue;
                }

                //LMA: Adding gem support
                if(items[i].gem!=0)
                {
                    if(GServer->JemList.Index[items[i].gem]!=NULL)
                    {
                        if(GServer->JemList.Index[items[i].gem]->stat1[0] == MP_COST_RED || GServer->JemList.Index[items[i].gem]->stat1[0] == MP_CONSUME)
                        {
                            mpreduction += GServer->JemList.Index[items[i].gem]->stat1[1];
                        }
                        if(GServer->JemList.Index[items[i].gem]->stat2[0] == MP_COST_RED || GServer->JemList.Index[items[i].gem]->stat2[0] == MP_CONSUME)
                        {
                            mpreduction += GServer->JemList.Index[items[i].gem]->stat2[1];
                        }
                    }
                }
                //End gem

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == MP_COST_RED || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == MP_CONSUME)
                {
                    mpreduction += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == MP_COST_RED || GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == MP_CONSUME)
                {
                    mpreduction += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == MP_COST_RED || GServer->StatsList[items[i].stats]->stat[j] == MP_CONSUME)
                        {
                            mpreduction += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++ )
            {
                if(cskills[i].thisskill->buff[j] == MP_COST_RED || cskills[i].thisskill->buff[j] == MP_CONSUME)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pmpreduction += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0)
                    {
                        vmpreduction += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    mpreduction += vmpreduction;//Apply Passive Skill Value
    mpreduction += mpreduction * pmpreduction / 100;//Apply Passive Skill % Value

    //Log(MSG_INFO,"MPReduction : %i", mpreduction);

    return mpreduction;
}

// Max summon capacity                          //SUMMON_GAUGE(62)
unsigned int CPlayer::GetMaxSummonGauge( )
{
    UINT gauge = 50;
    UINT pgauge = 0;//Passive Skill % Value
    UINT vgauge = 0;//Passive Skill Value

    if(Status->Stance != DRIVING)
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                        continue;
                }

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == SUMMON_GAUGE)
                {
                    gauge += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
                }
                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == SUMMON_GAUGE)
                {
                    gauge += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                {
                    for(UINT j=0;j<2;j++)
                    {
                        if(GServer->StatsList[items[i].stats]->stat[j] == SUMMON_GAUGE)
                        {
                            gauge += GServer->StatsList[items[i].stats]->value[j];
                        }
                    }
                }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++ )
            {
                if(cskills[i].thisskill->buff[j] == SUMMON_GAUGE )
                {
                    if(cskills[i].thisskill->value2[j] > 0 )
                    {
                        pgauge += cskills[i].thisskill->value2[j];
                    }
                    if(cskills[i].thisskill->value1[j] > 0 )
                    {
                        vgauge += cskills[i].thisskill->value1[j];
                    }
                }
            }
        }
    }

    gauge += vgauge;//Apply Passive Skill Value
    gauge += gauge * pgauge / 100;//Apply Passive Skill % Value

    //Log(MSG_INFO,"Summon Gauge Capacity : %i", gauge);

    return gauge;
}

// Extra attributes [str/dex/con/cha/sen/int] From Cloths, Weapon, Gem
void CPlayer::GetExtraStats( )
{
    Attr->Estr = 0;
    Attr->Edex = 0;
    Attr->Eint = 0;
    Attr->Econ = 0;
    Attr->Esen = 0;
    Attr->Echa = 0;

    Attr->MEstr = 0;
    Attr->MEdex = 0;
    Attr->MEint = 0;
    Attr->MEcon = 0;
    Attr->MEsen = 0;
    Attr->MEcha = 0;

    if (Status->Stance != DRIVING)//When Driving Extra Attributes is not added
    {
        for(UINT i=1;i<12;i++)//Cloth Stats
        {
            if(items[i].count>0)
            {
                UINT Stat1 = GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0];
                UINT Stat2 = GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0];

                switch(Stat1)
                {
                    case A_STR: Attr->Estr += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1]; break;
                    case A_DEX: Attr->Edex += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1]; break;
                    case A_INT: Attr->Eint += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1]; break;
                    case A_CON: Attr->Econ += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1]; break;
                    case A_CHA: Attr->Echa += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1]; break;
                    case A_SEN: Attr->Esen += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1]; break;
                }
                switch(Stat2)
                {
                    case A_STR: Attr->Estr += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1]; break;
                    case A_DEX: Attr->Edex += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1]; break;
                    case A_INT: Attr->Eint += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1]; break;
                    case A_CON: Attr->Econ += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1]; break;
                    case A_CHA: Attr->Echa += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1]; break;
                    case A_SEN: Attr->Esen += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1]; break;
                }

                if(items[i].gem!=0) // jem stats
                {
                    UINT Stat1 = GServer->JemList.Index[items[i].gem]!=NULL?GServer->JemList.Index[items[i].gem]->stat1[0]:0;
                    UINT Stat2 = GServer->JemList.Index[items[i].gem]!=NULL?GServer->JemList.Index[items[i].gem]->stat2[0]:0;
                    switch(Stat1)
                    {
                        case A_STR: Attr->Estr += GServer->JemList.Index[items[i].gem]->stat1[1]; Attr->Estr += (UINT)floor(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Str); Attr->MEstr += (GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Str)-(int)(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Str); break;
                        case A_DEX: Attr->Edex += GServer->JemList.Index[items[i].gem]->stat1[1]; Attr->Edex += (UINT)floor(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Dex); Attr->MEdex += (GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Dex)-(int)(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Dex); break;
                        case A_INT: Attr->Eint += GServer->JemList.Index[items[i].gem]->stat1[1]; Attr->Eint += (UINT)floor(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Int); Attr->MEint += (GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Int)-(int)(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Int); break;
                        case A_CON: Attr->Econ += GServer->JemList.Index[items[i].gem]->stat1[1]; Attr->Econ += (UINT)floor(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Con); Attr->MEcon += (GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Con)-(int)(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Con); break;
                        case A_CHA: Attr->Echa += GServer->JemList.Index[items[i].gem]->stat1[1]; Attr->Echa += (UINT)floor(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Cha); Attr->MEcha += (GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Cha)-(int)(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Cha); break;
                        case A_SEN: Attr->Esen += GServer->JemList.Index[items[i].gem]->stat1[1]; Attr->Esen += (UINT)floor(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Sen); Attr->MEsen += (GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Sen)-(int)(GServer->JemList.Index[items[i].gem]->stat1[2] * 0.01 * Attr->Sen); break;
                    }
                    switch(Stat2)
                    {
                        case A_STR: Attr->Estr += GServer->JemList.Index[items[i].gem]->stat2[1];Attr->Estr +=  (UINT)floor(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Str); Attr->MEstr += (GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Str)-(int)(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Str); break;
                        case A_DEX: Attr->Edex += GServer->JemList.Index[items[i].gem]->stat2[1];Attr->Edex +=  (UINT)floor(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Dex); Attr->MEdex += (GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Dex)-(int)(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Dex); break;
                        case A_INT: Attr->Eint += GServer->JemList.Index[items[i].gem]->stat2[1];Attr->Eint +=  (UINT)floor(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Int); Attr->MEint += (GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Int)-(int)(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Int); break;
                        case A_CON: Attr->Econ += GServer->JemList.Index[items[i].gem]->stat2[1];Attr->Econ +=  (UINT)floor(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Con); Attr->MEcon += (GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Con)-(int)(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Con); break;
                        case A_CHA: Attr->Echa += GServer->JemList.Index[items[i].gem]->stat2[1];Attr->Echa +=  (UINT)floor(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Cha); Attr->MEcha += (GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Cha)-(int)(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Cha); break;
                        case A_SEN: Attr->Esen += GServer->JemList.Index[items[i].gem]->stat2[1];Attr->Esen +=  (UINT)floor(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Sen); Attr->MEsen += (GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Sen)-(int)(GServer->JemList.Index[items[i].gem]->stat2[2] * 0.01 * Attr->Sen); break;
                    }
                }

                if(items[i].stats!=0 && items[i].stats<GServer->maxStats)
                {
                    UINT Stat1 = GServer->StatsList[items[i].stats]->stat[0];
                    UINT Stat2 = GServer->StatsList[items[i].stats]->stat[1];
                    switch(Stat1)
                    {
                        case A_STR: Attr->Estr += GServer->StatsList[items[i].stats]->value[0]; break;
                        case A_DEX: Attr->Edex += GServer->StatsList[items[i].stats]->value[0]; break;
                        case A_INT: Attr->Eint += GServer->StatsList[items[i].stats]->value[0]; break;
                        case A_CON: Attr->Econ += GServer->StatsList[items[i].stats]->value[0]; break;
                        case A_CHA: Attr->Echa += GServer->StatsList[items[i].stats]->value[0]; break;
                        case A_SEN: Attr->Esen += GServer->StatsList[items[i].stats]->value[0]; break;
                    }
                    switch(Stat2)
                    {
                        case A_STR: Attr->Estr += GServer->StatsList[items[i].stats]->value[1]; break;
                        case A_DEX: Attr->Edex += GServer->StatsList[items[i].stats]->value[1]; break;
                        case A_INT: Attr->Eint += GServer->StatsList[items[i].stats]->value[1]; break;
                        case A_CON: Attr->Econ += GServer->StatsList[items[i].stats]->value[1]; break;
                        case A_CHA: Attr->Echa += GServer->StatsList[items[i].stats]->value[1]; break;
                        case A_SEN: Attr->Esen += GServer->StatsList[items[i].stats]->value[1]; break;
                    }
                }
            }
        }
    }
}

// Extra attributes [str/dex/con/cha/sen/int] From Passive Skill
void CPlayer::GetExtraStatsSkills( )
{
	UINT Extra[6] = {0,0,0,0,0,0};

    if (Status->Stance != DRIVING)//When Driving Extra Attributes is not added
    {
        for(UINT i=0;i<MAX_ALL_SKILL;i++)
        {
            //Some skills are not worthy to search in
            if(cskills[i].id==0&&i<60)
            {
                i=89;
                continue;
            }
            if(cskills[i].id==0&&(i>60&&i<120))
            {
                i=119;
                continue;
            }
            if(cskills[i].id==0&&i>=120)
                break;

            if(cskills[i].thisskill==NULL)
                continue;

            if(cskills[i].thisskill->type == 15)//Pasive
            {
                for(UINT j=0;j<3;j++)
                {
                    switch(cskills[i].thisskill->buff[j])
		            {
		                case A_STR:
		                	{
				                if(cskills[i].thisskill->value2[j] > 0)
				                {
				                	 //Estr or STR?
				                   Extra[0] += Attr->Estr * cskills[i].thisskill->value2[j] / 100;
				                }
				                if(cskills[i].thisskill->value1[j] > 0)
				                {
				                  	Extra[0] += cskills[i].thisskill->value1[j];
				                }

		                	}
		                	break;
		                case A_DEX:
		                	{
				                if(cskills[i].thisskill->value2[j] > 0)
				                {
				                	 //Edex or DEX?
				                   Extra[1] += Attr->Edex * cskills[i].thisskill->value2[j] / 100;
				                }
				                if(cskills[i].thisskill->value1[j] > 0)
				                {
				                  	Extra[1] += cskills[i].thisskill->value1[j];
				                }

		                	}
		                	break;
		                case A_INT:
		                	{
				                if(cskills[i].thisskill->value2[j] > 0)
				                {
				                	 //Eint or INT?
				                   Extra[2] += Attr->Eint * cskills[i].thisskill->value2[j] / 100;
				                }
				                if(cskills[i].thisskill->value1[j] > 0)
				                {
				                  	Extra[2] += cskills[i].thisskill->value1[j];
				                }

		                	}
		                	break;
		                case A_CON:
		                	{
				                if(cskills[i].thisskill->value2[j] > 0)
				                {
				                	 //Econ or CON?
				                   Extra[3] += Attr->Econ * cskills[i].thisskill->value2[j] / 100;
				                }
				                if(cskills[i].thisskill->value1[j] > 0)
				                {
				                  	Extra[3] += cskills[i].thisskill->value1[j];
				                }

		                	}
		                	break;
		                case A_CHA:
		                	{
				                if(cskills[i].thisskill->value2[j] > 0)
				                {
				                	 //Echa or CHA?
				                   Extra[4] += Attr->Echa * cskills[i].thisskill->value2[j] / 100;
				                }
				                if(cskills[i].thisskill->value1[j] > 0)
				                {
				                  	Extra[4] += cskills[i].thisskill->value1[j];
				                }

		                	}
		                	break;
		                case A_SEN:
		                	{
				                if(cskills[i].thisskill->value2[j] > 0)
				                {
				                	 //Esen or SEN?
				                   Extra[5] += Attr->Esen * cskills[i].thisskill->value2[j] / 100;
				                }
				                if(cskills[i].thisskill->value1[j] > 0)
				                {
				                  	Extra[5] += cskills[i].thisskill->value1[j];
				                }

		                	}
		                	break;
		            }
                }
            }
        }
    }

    //setting final stats.
    Attr->Estr += Extra[0];
    Attr->Edex += Extra[1];
    Attr->Eint += Extra[2];
    Attr->Econ += Extra[3];
    Attr->Echa += Extra[4];
    Attr->Esen += Extra[5];

    return;
}

// Return Attack Distance Weapon and PAT Weapon
float CPlayer::GetAttackDistance( )
{
    float AttackDistance = 0;

    if(Status->Stance == DRIVING)//PAT Stats
	{
        for(UINT i=135;i<140;i++)//PAT Stats
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype != 14)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid PAT item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum);
                    continue;
                }

                if(GServer->PatList.Index[items[i].itemnum]->attackdistance != 0)
                {
                    AttackDistance += GServer->PatList.Index[items[i].itemnum]->attackdistance / 100;
                    //Log(MSG_INFO, "Char %s have equipped PAT Weapon item and should have: %i Atk Distance", CharInfo->charname, (GServer->PatList.Index[items[i].itemnum]->attackdistance / 100));
                }
            }
        }
    }

    else if(Status->Stance != DRIVING)//Walking, Running, Fighting, ...  Stats
    {
        if(items[7].count == 0)//No Weapon
        {
            return (float)1;
        }

        AttackDistance = GServer->EquipList[WEAPON].Index[items[7].itemnum]->attackdistance / 100;
    }

    return AttackDistance;
}

// Return XPRate
unsigned int CPlayer::GetXPRate( )
{
    UINT XPRate = 0;
    if(Stats->ItemXPRate == -1)
        return 0; // sends back a rate of 0

    for(UINT i=1;i<12;i++)
    {
        if( items[i].count != 0 )
        {
            if(items[i].itemtype>9)
            {
                Log(MSG_WARNING, "Char %s have equip invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                continue;
            }
            if( GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == 30) //30 = XP Rate
                XPRate += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
            if( GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == 30)
                XPRate += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
            if(items[i].stats>0 && items[i].stats<500)
            {
                if(GServer->StatsList[items[i].stats]->stat[0] == 30)
                    XPRate += GServer->StatsList[items[i].stats]->value[0];
                if(GServer->StatsList[items[i].stats]->stat[1] == 30)
                    XPRate += GServer->StatsList[items[i].stats]->value[1];
            }
        }
    }
    if(Stats->ItemXPRate != 0 && Stats->ItemXPRate != 1 && Stats->ItemXPRate != 2)
        Stats->ItemXPRate = 0;
    XPRate += Stats->ItemXPRate; // add timed XPRate buffs
    UINT Extra = 0;
    for(UINT i=0;i<MAX_ALL_SKILL;i++)
    {
        if (cskills[i].id == 0)
		    continue;
        CSkills* skill = GServer->GetSkillByID( (cskills[i].id)+(cskills[i].level));
        if( skill == NULL )
            continue;
        for(UINT j = 0; j < 2; j++ )
        {
            if( skill->buff[j] == sEXPRate )
            {
                if( skill->value2[j] > 0 )
                    Extra += XPRate * skill->value2[j] / 100;
                if( skill->value1[j] > 0 )
                    Extra += skill->value1[j];
            }
        }
    }
    XPRate += Extra;

    if(XPRate < 1)
        XPRate = 1;
    Log( MSG_INFO, "XP rate rate calculated as = %i", XPRate);
    return XPRate;
}

// Return Item Drop Rate
unsigned int CPlayer::GetItemDropRate( )
{
    int itemdroprate = 0;
    for(UINT i=1;i<12;i++)
    {
        if( items[i].count != 0 )
        {
            if(items[i].itemtype > 9)
            {
                Log(MSG_WARNING, "Char %s have equip invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                continue;
            }
            if( GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == 134 ) //ITEM_DROP_RATE
                itemdroprate += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
            if( GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == 134 ) //ITEM_DROP_RATE
                itemdroprate += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
            if(items[i].stats > 0 && items[i].stats < 500)
            {
                if(GServer->StatsList[items[i].stats]->stat[0] == 134 ) //ITEM_DROP_RATE
                    itemdroprate += GServer->StatsList[items[i].stats]->value[0];
                if(GServer->StatsList[items[i].stats]->stat[1] == 134 ) //ITEM_DROP_RATE
                    itemdroprate += GServer->StatsList[items[i].stats]->value[1];
            }
        }
    }
    UINT Extra = 0;
    for(UINT i=0;i<30;i++)
    {
        if (cskills[i].id == 0)
		    continue;
        CSkills* skill = GServer->GetSkillByID( (cskills[i].id)+(cskills[i].level));
        if( skill == NULL )
            continue;
        for(UINT j = 0; j < 2; j++ )
        {
            if( skill->buff[j] == 134 ) //sItemDropIncrease
            {
                if( skill->value2[j] > 0 )
                    Extra += itemdroprate * skill->value2[j] / 100;
                if( skill->value1[j] > 0 )
                    Extra += skill->value1[j];
            }
        }
    }
    itemdroprate += Extra;
    //Log( MSG_INFO, "item drop rate calculated as = %i", itemdroprate);
    if(itemdroprate < 0)
        itemdroprate = 0;
    //itemdroprate += GServer->Config.DROP_RATE;
    //Log( MSG_INFO, "item drop rate calculated as = %i", itemdroprate);
    //Log( MSG_INFO, "Server item drop rate = %i", GServer->Config.DROP_RATE);
    return itemdroprate;
}

// Return Zuly Drop Rate
unsigned int CPlayer::GetZulyDropRate( )
{
    int zulydroprate = 1;
    for(UINT i=1;i<12;i++)
    {
        if( items[i].count != 0 )
        {
            if(items[i].itemtype > 9)
            {
                Log(MSG_WARNING, "Char %s have equip invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                continue;
            }
            if( GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == 64 ) //ZULY_DROP_RATE
                zulydroprate += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
            if( GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == 64 ) //ZULY_DROP_RATE
                zulydroprate += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
            if(items[i].stats > 0 && items[i].stats < 500)
            {
                if(GServer->StatsList[items[i].stats]->stat[0] == 64 ) //ZULY_DROP_RATE
                    zulydroprate += GServer->StatsList[items[i].stats]->value[0];
                if(GServer->StatsList[items[i].stats]->stat[1] == 64 ) //ZULY_DROP_RATE
                    zulydroprate += GServer->StatsList[items[i].stats]->value[1];
            }
        }
    }
    UINT Extra = 0;
    for(UINT i=0;i<30;i++)
    {
        if (cskills[i].id == 0)
		    continue;
        CSkills* skill = GServer->GetSkillByID( (cskills[i].id)+(cskills[i].level));
        if( skill == NULL )
            continue;
        for(UINT j = 0; j < 2; j++ )
        {
            if( skill->buff[j] == 64 ) //sItemZulyIncrease
            {
                if( skill->value2[j] > 0 )
                    Extra += zulydroprate * skill->value2[j] / 100;
                if( skill->value1[j] > 0 )
                    Extra += skill->value1[j];
            }
        }
    }
    zulydroprate += Extra;
    if(zulydroprate < 1)
        zulydroprate = 1;
    //Log( MSG_INFO, "zuly drop rate calculated as = %i", zulydroprate);
    return zulydroprate;
}

// Return Zuly Drop Rate
unsigned int CPlayer::GetGreyDrops( )
{
    int GreyDropRate = 0;
    // to be completed later
    return GreyDropRate;
}

unsigned int CPlayer::GetBlockRate( )
{
    unsigned int pBlock_Rate = 0;
    unsigned int itemBlock_Rate = 0;

    if(Status->Stance != DRIVING)
    {
        for(UINT i=1;i<12;i++) // Will leave this loop to look for block rate substats on all items if in future it's placed in the game client
        {
            if(items[i].count>0)
            {
                if(items[i].itemtype>9)
                {
                    Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                        continue;
                }

                if(GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->BlockRate){
                    itemBlock_Rate += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->BlockRate/100;
                } else {
                    itemBlock_Rate = 0; // Just added to ensure that there are no NULL values to avoid possible errors
                }

                if(items[i].stats>0 && items[i].stats<GServer->maxStats)
                    {
                        for(UINT j=0;j<2;j++)
                        {
                            if(GServer->StatsList[items[i].stats]->stat[j] == A_BLOCK_RATE || GServer->StatsList[items[i].stats]->stat[j] == BLOCK_RATE)
                            {
                                itemBlock_Rate += GServer->StatsList[items[i].stats]->value[j];
                            }
                        }
                    }
            }
        }
    }

    for(UINT i=0;i<MAX_ALL_SKILL;i++)//Passive Skills
    {
        //Some skills are not worthy to search in
        if(cskills[i].id==0&&i<60)
        {
            i=89;
            continue;
        }
        if(cskills[i].id==0&&(i>60&&i<120))
        {
            i=119;
            continue;
        }
        if(cskills[i].id==0&&i>=120)
        {
            break;
        }
        if(cskills[i].thisskill==NULL)
        {
            continue;
        }

        if(cskills[i].thisskill->type == 15)//Pasive
        {
            for(UINT j=0;j<3;j++ )
            {
                if(cskills[i].thisskill->buff[j] == BLOCK_RATE || cskills[i].thisskill->buff[j] == A_BLOCK_RATE)
                {
                    if(cskills[i].thisskill->value2[j] > 0)
                    {
                        pBlock_Rate += cskills[i].thisskill->value2[j];
                    }
                }
            }
        }
    }

    itemBlock_Rate += itemBlock_Rate * pBlock_Rate/100;
    return itemBlock_Rate;
}

unsigned int CPlayer::GetBlockedDamage( ){
    UINT BlockDmg = 0;
    UINT subweapdef = 0;
    UINT spdefense = 0;

    for(UINT i=1;i<9;i++)
    {
        if(i==7 || i==1)
        {
            continue;
        }
        if(items[i].count>0)
        {
            if(items[i].itemtype>9)
            {
                Log(MSG_WARNING, "Char %s have equipped invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum);
                continue;
            }

            if(items[i].itemtype== MASK || items[i].itemtype==JEWEL || items[i].itemtype==WEAPON || items[i].count<1)
            {
                Log(MSG_WARNING, "Char %s have equipped some item who shouldn't add deff in slot %i", CharInfo->charname, i );
                continue;
            }

            if(items[i].itemtype==SUBWEAPON)
            {
                subweapdef += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->defense;
            }

            if(items[i].refine>0)
            {
                UINT refine = (UINT)floor(items[i].refine/16);

                if(refine>0 && refine<GServer->maxGrades)
                {
                    if(items[i].itemtype==SUBWEAPON)
                    {
                        subweapdef += GServer->GradeList[refine]->def_addbonus;
                        subweapdef += (UINT)floor(GServer->GradeList[refine]->def_percent * 0.01 * GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->defense );
                    }
                }
            }
        }
    }

    if(items[8].count>0)//Defense From Shield Pasive Skill
    {
        for(UINT i=0;i<MAX_ALL_SKILL;i++)
        {
            //Some skills are not worthy to search in
            if(cskills[i].id==0&&i<60)
            {
                i=89;
                continue;
            }
            if(cskills[i].id==0&&(i>60&&i<120))
            {
                i=119;
                continue;
            }
            if(cskills[i].id==0&&i>=120)
            {
                break;
            }
            if(cskills[i].thisskill==NULL)
            {
                continue;
            }

            if(cskills[i].thisskill->type == 15)//Pasive
            {
                for(UINT j=0;j<3;j++)
                {
                    if(cskills[i].thisskill->buff[j] == SHIELD_DEFENSE)
                    {
                        if(cskills[i].thisskill->value2[j] > 0)
                        {
                            spdefense += cskills[i].thisskill->value2[j];
                        }
                    }
                }
            }
        }
    }
    BlockDmg += subweapdef + (subweapdef*spdefense/100);
    return BlockDmg;
}

// Return Drop Amount
unsigned int CPlayer::GetDropAmount( )
{
    int DropAmount = 0;

    if(CharInfo->Job == 421)//Only bourg's have stockpiling skill
    for(int i=0; i<MAX_CLASS_SKILL; i++)
    {
        //Bourg Stockpiling skill
        if(cskills[i].id == 2356)
        {
            DropAmount += cskills[i].thisskill->value1[0];
        }
    }

    for(UINT i=1;i<12;i++)
    {
        if( items[i].count != 0 )
        {
            if(items[i].itemtype > 9)
            {
                Log(MSG_WARNING, "Char %s have equip invalid item: %i,%i", CharInfo->charname, items[i].itemtype, items[i].itemnum );
                continue;
            }
            if( GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[0] == 63 )
                DropAmount += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat1[1];
            if( GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[0] == 63 )
                DropAmount += GServer->EquipList[items[i].itemtype].Index[items[i].itemnum]->stat2[1];
            if(items[i].stats > 0 && items[i].stats < GServer->maxStats && items[i].gem == 0)
            {
                if(GServer->StatsList[items[i].stats]->stat[0] == 63 )
                    DropAmount += GServer->StatsList[items[i].stats]->value[0];
                if(GServer->StatsList[items[i].stats]->stat[1] == 63 )
                    DropAmount += GServer->StatsList[items[i].stats]->value[1];
            }
        }
    }
    Log(MSG_INFO, "Base drop increased by %i for %s",DropAmount,CharInfo->charname);

    return DropAmount;
}

// calculate Player Stats
void CPlayer::SetStats( )
{
    GetExtraStats( );
    GetExtraStatsSkills( );
    Stats->MaxHP = GetMaxHP( );
    Stats->MaxMP = GetMaxMP( );
    //Tomiz : If we change stance, we don't take in consideration the extra stats str and int and we must check if HP and MP is not > MaxHP and MaxMP
    if(Stats->HP > Stats->MaxHP)
    {
        Stats->HP = Stats->MaxHP;
    }
    if(Stats->MP > Stats->MaxMP)
    {
        Stats->MP = Stats->MaxMP;
    }
    //end
	Stats->MagicAttack = GetMagicAttack( );
    Stats->Attack_Power = GetAttackPower( );
    Stats->Defense = GetDefense( );
    Stats->Critical = GetCritical( );
    Stats->Move_Speed = GetMoveSpeed( );
    Stats->Magic_Defense = GetMagicDefense( );
    Stats->Accuracy = GetAccuracy( );
    Stats->Dodge = GetDodge( );
    Stats->Attack_Speed = GetAttackSpeed( );
    Stats->Attack_Speed_Percent = GetAttackSpeedPercent();
    Stats->Attack_Distance = GetAttackDistance( );
    Stats->MaxWeight = GetMaxWeight( );
    Stats->MaxSummonGauge = GetMaxSummonGauge( );
    Stats->MPReduction = GetMPReduction( );
    Stats->xprate = GetXPRate( );
    Stats->itemdroprate = GetItemDropRate( );
    Stats->zulydroprate = GetZulyDropRate( );
    Stats->Block_Rate = GetBlockRate( );
    Stats->Blocked_dmg = GetBlockedDamage( );
    Stats->Drop_Amount = GetDropAmount( );
}


