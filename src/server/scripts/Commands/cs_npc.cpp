/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/* ScriptData
Name: npc_commandscript
%Complete: 100
Comment: All npc related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Transport.h"
#include "CreatureGroups.h"
#include "Language.h"
#include "TargetedMovementGenerator.h"                      // for HandleNpcUnFollowCommand
#include "CreatureAI.h"
#include "Player.h"
#include "Pet.h"

template<typename E, typename T = char const*>
struct EnumName
{
    E Value;
    T Name;
};

#define CREATE_NAMED_ENUM(VALUE) { VALUE, STRINGIZE(VALUE) }

#define NPCFLAG_COUNT   27

EnumName<NPCFlags, int32> const npcFlagTexts[NPCFLAG_COUNT] =
{
    { UNIT_NPC_FLAG_AUCTIONEER,         LANG_NPCINFO_AUCTIONEER         },
    { UNIT_NPC_FLAG_BANKER,             LANG_NPCINFO_BANKER             },
    { UNIT_NPC_FLAG_BATTLEMASTER,       LANG_NPCINFO_BATTLEMASTER       },
    { UNIT_NPC_FLAG_FLIGHTMASTER,       LANG_NPCINFO_FLIGHTMASTER       },
    { UNIT_NPC_FLAG_GOSSIP,             LANG_NPCINFO_GOSSIP             },
    { UNIT_NPC_FLAG_GUILD_BANKER,       LANG_NPCINFO_GUILD_BANKER       },
    { UNIT_NPC_FLAG_INNKEEPER,          LANG_NPCINFO_INNKEEPER          },
    { UNIT_NPC_FLAG_PETITIONER,         LANG_NPCINFO_PETITIONER         },
    { UNIT_NPC_FLAG_PLAYER_VEHICLE,     LANG_NPCINFO_PLAYER_VEHICLE     },
    { UNIT_NPC_FLAG_QUESTGIVER,         LANG_NPCINFO_QUESTGIVER         },
    { UNIT_NPC_FLAG_REFORGER,           LANG_NPCINFO_REFORGER           },
    { UNIT_NPC_FLAG_REPAIR,             LANG_NPCINFO_REPAIR             },
    { UNIT_NPC_FLAG_SPELLCLICK,         LANG_NPCINFO_SPELLCLICK         },
    { UNIT_NPC_FLAG_SPIRITGUIDE,        LANG_NPCINFO_SPIRITGUIDE        },
    { UNIT_NPC_FLAG_SPIRITHEALER,       LANG_NPCINFO_SPIRITHEALER       },
    { UNIT_NPC_FLAG_STABLEMASTER,       LANG_NPCINFO_STABLEMASTER       },
    { UNIT_NPC_FLAG_TABARDDESIGNER,     LANG_NPCINFO_TABARDDESIGNER     },
    { UNIT_NPC_FLAG_TRAINER,            LANG_NPCINFO_TRAINER            },
    { UNIT_NPC_FLAG_TRAINER_CLASS,      LANG_NPCINFO_TRAINER_CLASS      },
    { UNIT_NPC_FLAG_TRAINER_PROFESSION, LANG_NPCINFO_TRAINER_PROFESSION },
    { UNIT_NPC_FLAG_TRANSMOGRIFIER,     LANG_NPCINFO_TRANSMOGRIFIER     },
    { UNIT_NPC_FLAG_VAULTKEEPER,        LANG_NPCINFO_VAULTKEEPER        },
    { UNIT_NPC_FLAG_VENDOR,             LANG_NPCINFO_VENDOR             },
    { UNIT_NPC_FLAG_VENDOR_AMMO,        LANG_NPCINFO_VENDOR_AMMO        },
    { UNIT_NPC_FLAG_VENDOR_FOOD,        LANG_NPCINFO_VENDOR_FOOD        },
    { UNIT_NPC_FLAG_VENDOR_POISON,      LANG_NPCINFO_VENDOR_POISON      },
    { UNIT_NPC_FLAG_VENDOR_REAGENT,     LANG_NPCINFO_VENDOR_REAGENT     }
};

EnumName<Mechanics> const mechanicImmunes[MAX_MECHANIC] =
{
    CREATE_NAMED_ENUM(MECHANIC_NONE),
    CREATE_NAMED_ENUM(MECHANIC_CHARM),
    CREATE_NAMED_ENUM(MECHANIC_DISORIENTED),
    CREATE_NAMED_ENUM(MECHANIC_DISARM),
    CREATE_NAMED_ENUM(MECHANIC_DISTRACT),
    CREATE_NAMED_ENUM(MECHANIC_FEAR),
    CREATE_NAMED_ENUM(MECHANIC_GRIP),
    CREATE_NAMED_ENUM(MECHANIC_ROOT),
    CREATE_NAMED_ENUM(MECHANIC_SLOW_ATTACK),
    CREATE_NAMED_ENUM(MECHANIC_SILENCE),
    CREATE_NAMED_ENUM(MECHANIC_SLEEP),
    CREATE_NAMED_ENUM(MECHANIC_SNARE),
    CREATE_NAMED_ENUM(MECHANIC_STUN),
    CREATE_NAMED_ENUM(MECHANIC_FREEZE),
    CREATE_NAMED_ENUM(MECHANIC_KNOCKOUT),
    CREATE_NAMED_ENUM(MECHANIC_BLEED),
    CREATE_NAMED_ENUM(MECHANIC_BANDAGE),
    CREATE_NAMED_ENUM(MECHANIC_POLYMORPH),
    CREATE_NAMED_ENUM(MECHANIC_BANISH),
    CREATE_NAMED_ENUM(MECHANIC_SHIELD),
    CREATE_NAMED_ENUM(MECHANIC_SHACKLE),
    CREATE_NAMED_ENUM(MECHANIC_MOUNT),
    CREATE_NAMED_ENUM(MECHANIC_INFECTED),
    CREATE_NAMED_ENUM(MECHANIC_TURN),
    CREATE_NAMED_ENUM(MECHANIC_HORROR),
    CREATE_NAMED_ENUM(MECHANIC_INVULNERABILITY),
    CREATE_NAMED_ENUM(MECHANIC_INTERRUPT),
    CREATE_NAMED_ENUM(MECHANIC_DAZE),
    CREATE_NAMED_ENUM(MECHANIC_DISCOVERY),
    CREATE_NAMED_ENUM(MECHANIC_IMMUNE_SHIELD),
    CREATE_NAMED_ENUM(MECHANIC_SAPPED),
    CREATE_NAMED_ENUM(MECHANIC_ENRAGED),
    CREATE_NAMED_ENUM(MECHANIC_WOUNDED)
};

EnumName<UnitFlags> const unitFlags[MAX_UNIT_FLAGS] =
{
    CREATE_NAMED_ENUM(UNIT_FLAG_SERVER_CONTROLLED),
    CREATE_NAMED_ENUM(UNIT_FLAG_NON_ATTACKABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_DISABLE_MOVE),
    CREATE_NAMED_ENUM(UNIT_FLAG_PVP_ATTACKABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_RENAME),
    CREATE_NAMED_ENUM(UNIT_FLAG_PREPARATION),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_6),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_ATTACKABLE_1),
    CREATE_NAMED_ENUM(UNIT_FLAG_IMMUNE_TO_PC),
    CREATE_NAMED_ENUM(UNIT_FLAG_IMMUNE_TO_NPC),
    CREATE_NAMED_ENUM(UNIT_FLAG_LOOTING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PET_IN_COMBAT),
    CREATE_NAMED_ENUM(UNIT_FLAG_PVP),
    CREATE_NAMED_ENUM(UNIT_FLAG_SILENCED),
    CREATE_NAMED_ENUM(UNIT_FLAG_CANNOT_SWIM),
    CREATE_NAMED_ENUM(UNIT_FLAG_CAN_SWIM),
    CREATE_NAMED_ENUM(UNIT_FLAG_NON_ATTACKABLE_2),
    CREATE_NAMED_ENUM(UNIT_FLAG_PACIFIED),
    CREATE_NAMED_ENUM(UNIT_FLAG_STUNNED),
    CREATE_NAMED_ENUM(UNIT_FLAG_IN_COMBAT),
    CREATE_NAMED_ENUM(UNIT_FLAG_TAXI_FLIGHT),
    CREATE_NAMED_ENUM(UNIT_FLAG_DISARMED),
    CREATE_NAMED_ENUM(UNIT_FLAG_CONFUSED),
    CREATE_NAMED_ENUM(UNIT_FLAG_FLEEING),
    CREATE_NAMED_ENUM(UNIT_FLAG_PLAYER_CONTROLLED),
    CREATE_NAMED_ENUM(UNIT_FLAG_NOT_SELECTABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_SKINNABLE),
    CREATE_NAMED_ENUM(UNIT_FLAG_MOUNT),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_28),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_29),
    CREATE_NAMED_ENUM(UNIT_FLAG_SHEATHE),
    CREATE_NAMED_ENUM(UNIT_FLAG_UNK_31)
};

class npc_commandscript : public CommandScript
{
public:
    npc_commandscript() : CommandScript("npc_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> npcAddCommandTable =
        {
            { "formation",       SEC_ADMINISTRATOR,  false,  &HandleNpcAddFormationCommand,      },
            { "item",            SEC_ADMINISTRATOR,  false,  &HandleNpcAddVendorItemCommand,     },
            { "move",            SEC_ADMINISTRATOR,  false,  &HandleNpcAddMoveCommand,           },
            { "temp",            SEC_ADMINISTRATOR,  false,  &HandleNpcAddTempSpawnCommand,      },
            { "",                SEC_ADMINISTRATOR,  false,  &HandleNpcAddCommand,               },
        };
        static std::vector<ChatCommand> npcDeleteCommandTable =
        {
            { "item",            SEC_ADMINISTRATOR,  false,  &HandleNpcDeleteVendorItemCommand,  },
            { "",                SEC_ADMINISTRATOR,  false,  &HandleNpcDeleteCommand,            },
        };
        static std::vector<ChatCommand> npcFollowCommandTable =
        {
            { "stop",            SEC_ADMINISTRATOR,  false,  &HandleNpcUnFollowCommand,          },
            { "",                SEC_ADMINISTRATOR,  false,  &HandleNpcFollowCommand,            },
        };
        static std::vector<ChatCommand> npcSetCommandTable =
        {
            { "allowmove",       SEC_ADMINISTRATOR,  false,  &HandleNpcSetAllowMovementCommand,  },
            { "entry",           SEC_ADMINISTRATOR,  false,  &HandleNpcSetEntryCommand,          },
            { "factionid",       SEC_ADMINISTRATOR,  false,  &HandleNpcSetFactionIdCommand,      },
            { "flag",            SEC_ADMINISTRATOR,  false,  &HandleNpcSetFlagCommand,           },
            { "level",           SEC_ADMINISTRATOR,  false,  &HandleNpcSetLevelCommand,          },
            { "link",            SEC_ADMINISTRATOR,  false,  &HandleNpcSetLinkCommand,           },
            { "model",           SEC_ADMINISTRATOR,  false,  &HandleNpcSetModelCommand,          },
            { "movetype",        SEC_ADMINISTRATOR,  false,  &HandleNpcSetMoveTypeCommand,       },
            { "phase",           SEC_ADMINISTRATOR,  false,  &HandleNpcSetPhaseCommand,          },
            { "phaseid",         SEC_ADMINISTRATOR,  false,  &HandleNpcSetPhaseIDCommand,        },
            { "phasegroup",      SEC_ADMINISTRATOR,  false,  &HandleNpcSetPhaseGroup,            },
            { "wanderdistance",  SEC_ADMINISTRATOR,  false,  &HandleNpcSetWanderDistanceCommand, },
            { "spawntime",       SEC_ADMINISTRATOR,  false,  &HandleNpcSetSpawnTimeCommand,      },
            { "data",            SEC_ADMINISTRATOR,  false,  &HandleNpcSetDataCommand,           },
        };
        static std::vector<ChatCommand> npcCommandTable =
        {
            { "info",            SEC_ADMINISTRATOR,  false,  &HandleNpcInfoCommand,              },
            { "near",            SEC_ADMINISTRATOR,  false,  &HandleNpcNearCommand,              },
            { "move",            SEC_ADMINISTRATOR,  false,  &HandleNpcMoveCommand,              },
            { "playemote",       SEC_ADMINISTRATOR,  false,  &HandleNpcPlayEmoteCommand,         },
            { "say",             SEC_ADMINISTRATOR,  false,  &HandleNpcSayCommand,               },
            { "textemote",       SEC_ADMINISTRATOR,  false,  &HandleNpcTextEmoteCommand,         },
            { "whisper",         SEC_ADMINISTRATOR,  false,  &HandleNpcWhisperCommand,           },
            { "yell",            SEC_ADMINISTRATOR,  false,  &HandleNpcYellCommand,              },
            { "tame",            SEC_ADMINISTRATOR,  false,  &HandleNpcTameCommand,              },
            { "scale",           SEC_ADMINISTRATOR,  false,  &HandleNpcScaleCommand,             },
            { "add",             SEC_ADMINISTRATOR,  false,  npcAddCommandTable                  },
            { "delete",          SEC_ADMINISTRATOR,  false,  npcDeleteCommandTable               },
            { "follow",          SEC_ADMINISTRATOR,  false,  npcFollowCommandTable               },
            { "set",             SEC_ADMINISTRATOR,  false,  npcSetCommandTable                  },
            
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "npc",             SEC_ADMINISTRATOR,  false,  npcCommandTable                     },
        };
        return commandTable;
    }

    //add spawn of creature
    static bool HandleNpcAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* charID = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        if (!charID)
            return false;

        char* team = strtok(NULL, " ");
        int32 teamval = 0;
        if (team)
            teamval = atoi(team);

        if (teamval < 0)
            teamval = 0;

        uint32 id  = atoi(charID);
        CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(id);
        if (!cinfo)
            return false;

        Player* chr = handler->GetSession()->GetPlayer();
        float x = chr->GetPositionX();
        float y = chr->GetPositionY();
        float z = chr->GetPositionZ();
        float o = chr->GetOrientation();
        Map* map = chr->GetMap();

        Transport* trans = chr->GetTransport();
        if (trans && trans->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            QueryResult result = WorldDatabase.Query("SELECT MAX(guid) + 1 FROM creature");
            ASSERT(result);
            uint32 guid = (*result)[0].GetUInt32();
            CreatureData& data = sObjectMgr->NewOrExistCreatureData(guid);
            data.id = id;
            data.phaseMask = chr->GetPhaseMgr().GetPhaseMaskForSpawn();
            data.posX = chr->GetTransOffsetX();
            data.posY = chr->GetTransOffsetY();
            data.posZ = chr->GetTransOffsetZ();
            data.orientation = chr->GetTransOffsetO();
            data.curhealth = 0;
            data.curmana = 0;
            data.currentwaypoint = 0;
            //data.displayid = cinfo->GetFirstValidModelId(); Not sure need this line here
            data.dynamicflags = 0;
            data.equipmentId = 0;
            data.mapId = trans->GetGOInfo()->moTransport.mapID;
            data.movementType = 0;
            data.npcflag = 0;
            data.npcflag2 = 0;
            data.spawntimesecs = 120; // ¯\_(ツ)_/¯
            data.spawntimesecs = 0;

            Creature* creature = trans->CreateNPCPassenger(guid, &data);

            creature->SaveToDB(trans->GetGOInfo()->moTransport.mapID, 1 << map->GetSpawnMode(), chr->GetPhaseMgr().GetPhaseMaskForSpawn());

            sObjectMgr->AddCreatureToGrid(guid, &data);
            return true;
        }

        Creature* creature = new Creature();
        if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, chr->GetPhaseMgr().GetPhaseMaskForSpawn(), id, 0, (uint32)teamval, x, y, z, o))
        {
            delete creature;
            return false;
        }

        for (auto phase : chr->GetPhases())
            creature->SetPhased(phase, false, true);

        creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), chr->GetPhaseMgr().GetPhaseMaskForSpawn());

        uint32 db_guid = creature->GetDBTableGUIDLow();

        // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
        if (!creature->LoadCreatureFromDB(db_guid, map))
        {
            delete creature;
            return false;
        }

        sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
        return true;
    }

    //add item in vendorlist
    static bool HandleNpcAddVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument

        char* pitem  = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        int32 item_int = atol(pitem);
        if (item_int <= 0)
            return false;

        uint32 itemId = item_int;

        char* fmaxcount = strtok(NULL, " ");                    //add maxcount, default: 0
        uint32 maxcount = 0;
        if (fmaxcount)
            maxcount = atol(fmaxcount);

        char* fincrtime = strtok(NULL, " ");                    //add incrtime, default: 0
        uint32 incrtime = 0;
        if (fincrtime)
            incrtime = atol(fincrtime);

        char* fextendedcost = strtok(NULL, " ");                //add ExtendedCost, default: 0
        uint32 extendedcost = fextendedcost ? atol(fextendedcost) : 0;
        Creature* vendor = handler->getSelectedCreature();
        if (!vendor)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* addMulti = strtok(NULL, " ");
        uint32 vendor_entry = addMulti ? handler->GetSession()->GetCurrentVendor() : vendor ? vendor->GetEntry() : 0;

        if (!sObjectMgr->IsVendorItemValid(vendor_entry, itemId, maxcount, incrtime, extendedcost, type, handler->GetSession()->GetPlayer()))
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        sObjectMgr->AddVendorItem(vendor_entry, itemId, maxcount, incrtime, extendedcost, type);

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(LANG_ITEM_ADDED_TO_LIST, itemId, itemTemplate->Name1.c_str(), maxcount, incrtime, extendedcost);
        return true;
    }

    //add move for creature
    static bool HandleNpcAddMoveCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* guidStr = strtok((char*)args, " ");
        char* waitStr = strtok((char*)NULL, " ");

        uint32 lowGuid = atoi((char*)guidStr);

        Creature* creature = NULL;

        /* FIXME: impossible without entry
        if (lowguid)
            creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), MAKE_GUID(lowguid, HighGuid::Unit));
        */

        // attempt check creature existence by DB data
        if (!creature)
        {
            CreatureData const* data = sObjectMgr->GetCreatureData(lowGuid);
            if (!data)
            {
                handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowGuid);
                handler->SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            // obtain real GUID for DB operations
            lowGuid = creature->GetDBTableGUIDLow();
        }

        int wait = waitStr ? atoi(waitStr) : 0;

        if (wait < 0)
            wait = 0;

        // Update movement type
        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_MOVEMENT_TYPE);

        stmt->setUInt8(0, uint8(WAYPOINT_MOTION_TYPE));
        stmt->setUInt32(1, lowGuid);

        WorldDatabase.Execute(stmt);

        if (creature && creature->GetWaypointPath())
        {
            creature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
            creature->GetMotionMaster()->Initialize();
            if (creature->IsAlive())                            // dead creature will reset movement generator at respawn
            {
                creature->setDeathState(JUST_DIED);
                creature->Respawn(true);
            }
            creature->SaveToDB();
        }

        handler->SendSysMessage(LANG_WAYPOINT_ADDED);

        return true;
    }

    static bool HandleNpcSetAllowMovementCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (sWorld->getAllowMovement())
        {
            sWorld->SetAllowMovement(false);
            handler->SendSysMessage(LANG_CREATURE_MOVE_DISABLED);
        }
        else
        {
            sWorld->SetAllowMovement(true);
            handler->SendSysMessage(LANG_CREATURE_MOVE_ENABLED);
        }
        return true;
    }

    static bool HandleNpcSetEntryCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 newEntryNum = atoi(args);
        if (!newEntryNum)
            return false;

        Unit* unit = handler->getSelectedUnit();
        if (!unit || unit->GetTypeId() != TYPEID_UNIT)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }
        Creature* creature = unit->ToCreature();
        if (creature->UpdateEntry(newEntryNum))
            handler->SendSysMessage(LANG_DONE);
        else
            handler->SendSysMessage(LANG_ERROR);
        return true;
    }

    //change level of creature or pet
    static bool HandleNpcSetLevelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint8 lvl = (uint8) atoi((char*)args);
        if (lvl < 1 || lvl > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) + 3)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (creature->IsPet())
        {
            if (((Pet*)creature)->getPetType() == HUNTER_PET)
            {
                creature->SetUInt32Value(UNIT_FIELD_PET_NEXT_LEVEL_EXPERIENCE, sObjectMgr->GetXPForLevel(lvl)/4);
                creature->SetUInt32Value(UNIT_FIELD_PET_EXPERIENCE, 0);
            }
            ((Pet*)creature)->GivePetLevel(lvl);
        }
        else
        {
            creature->SetMaxHealth(100 + 30*lvl);
            creature->SetHealth(100 + 30*lvl);
            creature->SetLevel(lvl);
            creature->SaveToDB();
        }

        return true;
    }

    static bool HandleNpcDeleteCommand(ChatHandler* handler, char const* args)
    {
        Creature* unit = NULL;

        if (*args)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            uint32 lowguid = atoi(cId);
            if (!lowguid)
                return false;

            if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
                unit = handler->GetSession()->GetPlayer()->GetMap()->GetCreature(ObjectGuid(HighGuid::Unit, cr_data->id, lowguid));
        }
        else
            unit = handler->getSelectedCreature();

        if (!unit || unit->IsPet() || unit->IsTotem())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Delete the creature
        unit->CombatStop();
        unit->DeleteFromDB();
        unit->AddObjectToRemoveList();

        handler->SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

        return true;
    }

    //del item from vendor list
    static bool HandleNpcDeleteVendorItemCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* vendor = handler->getSelectedCreature();
        if (!vendor || !vendor->IsVendor())
        {
            handler->SendSysMessage(LANG_COMMAND_VENDORSELECTION);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* pitem  = handler->extractKeyFromLink((char*)args, "Hitem");
        if (!pitem)
        {
            handler->SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
            handler->SetSentErrorMessage(true);
            return false;
        }
        uint32 itemId = atol(pitem);

        const uint8 type = 1; // FIXME: make type (1 item, 2 currency) an argument
        char* addMulti = strtok(NULL, " ");

        if (!sObjectMgr->RemoveVendorItem(addMulti ? handler->GetSession()->GetCurrentVendor() : vendor->GetEntry(), itemId, type))
        {
            handler->PSendSysMessage(LANG_ITEM_NOT_IN_LIST, itemId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);

        handler->PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST, itemId, itemTemplate->Name1.c_str());
        return true;
    }

    //set faction of creature
    static bool HandleNpcSetFactionIdCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 factionId = (uint32) atoi((char*)args);

        if (!sFactionTemplateStore.LookupEntry(factionId))
        {
            handler->PSendSysMessage(LANG_WRONG_FACTION, factionId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetFaction(factionId);

        // Faction is set in creature_template - not inside creature

        // Update in memory..
        if (CreatureTemplate const* cinfo = creature->GetCreatureTemplate())
        {
            const_cast<CreatureTemplate*>(cinfo)->faction = factionId;
        }

        // ..and DB
        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_FACTION);

        stmt->setUInt16(0, uint16(factionId));
        stmt->setUInt32(1, creature->GetEntry());

        WorldDatabase.Execute(stmt);

        return true;
    }

    //set npcflag of creature
    static bool HandleNpcSetFlagCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 npcFlags = (uint32) atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, npcFlags);

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_NPCFLAG);

        stmt->setUInt32(0, npcFlags);
        stmt->setUInt32(1, creature->GetEntry());

        WorldDatabase.Execute(stmt);

        handler->SendSysMessage(LANG_VALUE_SAVED_REJOIN);

        return true;
    }

    //set data of creature for testing scripting
    static bool HandleNpcSetDataCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* arg1 = strtok((char*)args, " ");
        char* arg2 = strtok((char*)NULL, "");

        if (!arg1 || !arg2)
            return false;

        uint32 data_1 = (uint32)atoi(arg1);
        uint32 data_2 = (uint32)atoi(arg2);

        if (!data_1 || !data_2)
            return false;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->AI()->SetData(data_1, data_2);
        std::string AIorScript = creature->GetAIName() != "" ? "AI type: " + creature->GetAIName() : (creature->GetScriptName() != "" ? "Script Name: " + creature->GetScriptName() : "No AI or Script Name Set");
        handler->PSendSysMessage(LANG_NPC_SETDATA, creature->GetGUID().GetCounter(), creature->GetEntry(), creature->GetName().c_str(), data_1, data_2, AIorScript.c_str());
        return true;
    }

    //npc follow handling
    static bool HandleNpcFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Follow player - Using pet's default dist and angle
        creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, creature->GetFollowAngle());

        handler->PSendSysMessage(LANG_CREATURE_FOLLOW_YOU_NOW, creature->GetName().c_str());
        return true;
    }

    static bool HandleNpcInfoCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* target = handler->getSelectedCreature();

        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* cInfo = target->GetCreatureTemplate();

        uint32 faction = target->GetFaction();
        uint32 npcflags = target->GetUInt32Value(UNIT_FIELD_NPC_FLAGS);
        uint32 mechanicImmuneMask = cInfo->MechanicImmuneMask;
        uint32 displayid = target->GetDisplayId();
        uint32 nativeid = target->GetNativeDisplayId();
        uint32 Entry = target->GetEntry();

        int64 curRespawnDelay = target->GetRespawnTimeEx()-time(NULL);
        if (curRespawnDelay < 0)
            curRespawnDelay = 0;
        std::string curRespawnDelayStr = secsToTimeString(uint64(curRespawnDelay), true);
        std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);
        std::string defRespawnDelayMaxStr = target->GetRespawnDelayMax() ? secsToTimeString(target->GetRespawnDelayMax(), true) : defRespawnDelayStr;

        handler->PSendSysMessage(LANG_NPCINFO_CHAR,  target->GetDBTableGUIDLow(), target->GetGUID().GetCounter(), faction, npcflags, Entry, displayid, nativeid);
        handler->PSendSysMessage(LANG_NPCINFO_LEVEL, target->GetLevel());
        handler->PSendSysMessage(LANG_NPCINFO_EQUIPMENT, target->GetCurrentEquipmentId(), target->GetOriginalEquipmentId());
        handler->PSendSysMessage(LANG_NPCINFO_HEALTH, target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());

        handler->PSendSysMessage("Gossip Menu ID: %u", cInfo->GossipMenuId);
        handler->PSendSysMessage(LANG_NPCINFO_MOVEMENT_DATA, target->GetMovementTemplate().ToString().c_str());

        handler->PSendSysMessage(LANG_NPCINFO_UNIT_FIELD_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS));
        for (uint8 i = 0; i < MAX_UNIT_FLAGS; ++i)
            if (target->GetUInt32Value(UNIT_FIELD_FLAGS) & unitFlags[i].Value)
                handler->PSendSysMessage("%s (0x%X)", unitFlags[i].Name, unitFlags[i].Value);

        handler->PSendSysMessage(LANG_NPCINFO_FLAGS2, target->GetUInt32Value(UNIT_FIELD_FLAGS_2), target->GetUInt32Value(OBJECT_FIELD_DYNAMIC_FLAGS), target->GetFaction());

        handler->PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), defRespawnDelayMaxStr.c_str(),  curRespawnDelayStr.c_str());
        handler->PSendSysMessage(LANG_NPCINFO_LOOT,  cInfo->lootid, cInfo->pickpocketLootId, cInfo->SkinLootId);
        handler->PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetInstanceId(), target->GetMap()->GetDifficulty());
        handler->PSendSysMessage(LANG_NPCINFO_PHASEMASK, target->GetPhaseMask());

        if (CreatureData const* data = sObjectMgr->GetCreatureData(target->GetDBTableGUIDLow()))
        {
            handler->PSendSysMessage(LANG_NPCINFO_PHASES, data->phaseid, data->phaseGroup);
            if (data->phaseGroup)
            {
                std::set<uint32> _phases = target->GetPhases();

                if (!_phases.empty())
                {
                    handler->PSendSysMessage(LANG_NPCINFO_PHASE_IDS);
                    for (uint32 phaseId : _phases)
                        handler->PSendSysMessage("%u", phaseId);
                }
            }
        }

        handler->PSendSysMessage(LANG_NPCINFO_ARMOR, target->GetArmor());
        handler->PSendSysMessage(LANG_NPCINFO_POSITION, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());

        if (target->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE ||
            target->GetMotionMaster()->GetCurrentMovementGeneratorType() == RANDOM_MOTION_TYPE ||
            target->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
            handler->PSendSysMessage("Motion Type: |CFFFF0000%u|R", target->GetMotionMaster()->GetCurrentMovementGeneratorType());
        else
            handler->PSendSysMessage("Motion Type: |CFFFF0000%u|R", target->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE);

        handler->PSendSysMessage("Wander Distance: |CFFFF0000%g|R", target->GetWanderDistance());

        handler->PSendSysMessage(LANG_NPCINFO_AIINFO, target->GetAIName().c_str(), target->GetScriptName().c_str());

        for (uint8 i = 0; i < NPCFLAG_COUNT; i++)
            if (npcflags & npcFlagTexts[i].Value)
                handler->PSendSysMessage(npcFlagTexts[i].Name);

        handler->PSendSysMessage(LANG_NPCINFO_MECHANIC_IMMUNE, mechanicImmuneMask);
        for (uint8 i = 0; i < MAX_MECHANIC; ++i)
            if ((mechanicImmuneMask << 1) & mechanicImmunes[i].Value)
                handler->PSendSysMessage("%s (0x%X)", mechanicImmunes[i].Name, mechanicImmunes[i].Value);

        return true;
    }

    static bool HandleNpcNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : float((atof(args)));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();
        bool moTransport = player->GetTransport() && player->GetTransport()->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT;
        uint32  mapId =  moTransport ? player->GetTransport()->GetGOInfo()->moTransport.mapID : player->GetMapId();
        Position pos = moTransport ? player->m_movementInfo.transport.pos : *player;

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_CREATURE_NEAREST);
        stmt->setFloat(0, pos.GetPositionX());
        stmt->setFloat(1, pos.GetPositionY());
        stmt->setFloat(2, pos.GetPositionZ());
        stmt->setUInt32(3, mapId);
        stmt->setFloat(4, pos.GetPositionX());
        stmt->setFloat(5, pos.GetPositionY());
        stmt->setFloat(6, pos.GetPositionZ());
        stmt->setFloat(7, distance * distance);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 guid = fields[0].GetUInt32();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                uint16 mapId = fields[5].GetUInt16();

                CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(entry);
                if (!creatureTemplate)
                    continue;

                handler->PSendSysMessage(LANG_CREATURE_LIST_CHAT, guid, entry, guid, creatureTemplate->Name.c_str(), x, y, z, mapId);

                ++count;
            }
            while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_NEAR_NPC_MESSAGE, distance, count);

        return true;
    }

    //move selected creature
    static bool HandleNpcMoveCommand(ChatHandler* handler, char const* args)
    {
        uint32 lowguid = ObjectGuid::Empty;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
            char* cId = handler->extractKeyFromLink((char*)args, "Hcreature");
            if (!cId)
                return false;

            lowguid = atoi(cId);

            /* FIXME: impossible without entry
            if (lowguid)
                creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), MAKE_GUID(lowguid, HighGuid::Unit));
            */

            // Attempting creature load from DB data
            if (!creature)
            {
                CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
                if (!data)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }

                uint32 map_id = data->mapId;

                if (handler->GetSession()->GetPlayer()->GetMapId() != map_id)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATUREATSAMEMAP, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                lowguid = creature->GetDBTableGUIDLow();
            }
        }
        else
        {
            lowguid = creature->GetDBTableGUIDLow();
        }

        float x = handler->GetSession()->GetPlayer()->GetPositionX();
        float y = handler->GetSession()->GetPlayer()->GetPositionY();
        float z = handler->GetSession()->GetPlayer()->GetPositionZ();
        float o = handler->GetSession()->GetPlayer()->GetOrientation();

        if (creature)
        {
            if (CreatureData const* data = sObjectMgr->GetCreatureData(creature->GetDBTableGUIDLow()))
            {
                const_cast<CreatureData*>(data)->posX = x;
                const_cast<CreatureData*>(data)->posY = y;
                const_cast<CreatureData*>(data)->posZ = z;
                const_cast<CreatureData*>(data)->orientation = o;
            }
            creature->SetPosition(x, y, z, o);
            creature->GetMotionMaster()->Initialize();
            if (creature->IsAlive())                            // dead creature will reset movement generator at respawn
            {
                creature->setDeathState(JUST_DIED);
                creature->Respawn();
            }
        }

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_POSITION);

        stmt->setFloat(0, x);
        stmt->setFloat(1, y);
        stmt->setFloat(2, z);
        stmt->setFloat(3, o);
        stmt->setUInt32(4, lowguid);

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(LANG_COMMAND_CREATUREMOVED);
        return true;
    }

    //play npc emote
    static bool HandleNpcPlayEmoteCommand(ChatHandler* handler, char const* args)
    {
        uint32 emote = atoi((char*)args);

        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!target->HandleSwitchEmoteCommand(emote))
        {
            handler->SetSentErrorMessage(true);
            return false;
        }

        return true;
    }

    //set model of creature
    static bool HandleNpcSetModelCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 displayId = (uint32) atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();

        if (!creature || creature->IsPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetDisplayId(displayId);
        creature->SetNativeDisplayId(displayId);

        creature->SaveToDB();

        return true;
    }

    /**HandleNpcSetMoveTypeCommand
    * Set the movement type for an NPC.<br/>
    * <br/>
    * Valid movement types are:
    * <ul>
    * <li> stay - NPC wont move </li>
    * <li> random - NPC will move randomly according to the wander_distance </li>
    * <li> way - NPC will move with given waypoints set </li>
    * </ul>
    * additional parameter: NODEL - so no waypoints are deleted, if you
    *                       change the movement type
    */
    static bool HandleNpcSetMoveTypeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // 3 arguments:
        // GUID (optional - you can also select the creature)
        // stay|random|way (determines the kind of movement)
        // NODEL (optional - tells the system NOT to delete any waypoints)
        //        this is very handy if you want to do waypoints, that are
        //        later switched on/off according to special events (like escort
        //        quests, etc)
        char* guid_str = strtok((char*)args, " ");
        char* type_str = strtok((char*)NULL, " ");
        char* dontdel_str = strtok((char*)NULL, " ");

        bool doNotDelete = false;

        if (!guid_str)
            return false;

        uint32 lowguid = ObjectGuid::Empty;
        Creature* creature = NULL;

        if (dontdel_str)
        {
            //TC_LOG_ERROR("misc", "DEBUG: All 3 params are set");

            // All 3 params are set
            // GUID
            // type
            // doNotDEL
            if (stricmp(dontdel_str, "NODEL") == 0)
            {
                //TC_LOG_ERROR("misc", "DEBUG: doNotDelete = true;");
                doNotDelete = true;
            }
        }
        else
        {
            // Only 2 params - but maybe NODEL is set
            if (type_str)
            {
                TC_LOG_ERROR("misc", "DEBUG: Only 2 params ");
                if (stricmp(type_str, "NODEL") == 0)
                {
                    //TC_LOG_ERROR("misc", "DEBUG: type_str, NODEL ");
                    doNotDelete = true;
                    type_str = NULL;
                }
            }
        }

        if (!type_str)                                           // case .setmovetype $move_type (with selected creature)
        {
            type_str = guid_str;
            creature = handler->getSelectedCreature();
            if (!creature || creature->IsPet())
                return false;
            lowguid = creature->GetDBTableGUIDLow();
        }
        else                                                    // case .setmovetype #creature_guid $move_type (with selected creature)
        {
            lowguid = atoi((char*)guid_str);

            /* impossible without entry
            if (lowguid)
                creature = ObjectAccessor::GetCreature(*handler->GetSession()->GetPlayer(), MAKE_GUID(lowguid, HighGuid::Unit));
            */

            // attempt check creature existence by DB data
            if (!creature)
            {
                CreatureData const* data = sObjectMgr->GetCreatureData(lowguid);
                if (!data)
                {
                    handler->PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                    handler->SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                lowguid = creature->GetDBTableGUIDLow();
            }
        }

        // now lowguid is low guid really existed creature
        // and creature point (maybe) to this creature or NULL

        MovementGeneratorType move_type;

        std::string type = type_str;

        if (type == "stay")
            move_type = IDLE_MOTION_TYPE;
        else if (type == "random")
            move_type = RANDOM_MOTION_TYPE;
        else if (type == "way")
            move_type = WAYPOINT_MOTION_TYPE;
        else
            return false;

        // update movement type
        //if (doNotDelete == false)
        //    WaypointMgr.DeletePath(lowguid);

        if (creature)
        {
            // update movement type
            if (doNotDelete == false)
                creature->LoadPath(0);

            creature->SetDefaultMovementType(move_type);
            creature->GetMotionMaster()->Initialize();
            if (creature->IsAlive())                            // dead creature will reset movement generator at respawn
            {
                creature->setDeathState(JUST_DIED);
                creature->Respawn();
            }
            creature->SaveToDB();
        }
        if (doNotDelete == false)
        {
            handler->PSendSysMessage(LANG_MOVE_TYPE_SET, type_str);
        }
        else
        {
            handler->PSendSysMessage(LANG_MOVE_TYPE_SET_NODEL, type_str);
        }

        return true;
    }

    //npc phasemask handling
    //change phasemask of creature or pet
    static bool HandleNpcSetPhaseCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 phasemask = (uint32) atoi((char*)args);
        if (phasemask == 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->SetPhaseMask(phasemask, true);

        if (!creature->IsPet())
            creature->SaveToDB();

        return true;
    }

    //npc phase handling
    //change phase of creature
    static bool HandleNpcSetPhaseGroup(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 phaseGroupId = (uint32)atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();
        if (!creature || creature->IsPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->ClearPhases();

        for (uint32 id : GetPhasesForGroup(phaseGroupId))
            creature->SetPhased(id, false, true); // don't send update here for multiple phases, only send it once after adding all phases

        creature->UpdateObjectVisibility();

        creature->SaveToDB();

        return true;
    }

    //npc phase handling
    //change phase of creature
    static bool HandleNpcSetPhaseIDCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 phase = (uint32)atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();
        if (!creature || creature->IsPet())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->ClearPhases();
        creature->SetPhased(phase, true, true);
        creature->SaveToDB();

        return true;
    }

    //set spawn dist of creature
    static bool HandleNpcSetWanderDistanceCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        float option = (float)(atof((char*)args));
        if (option < 0.0f)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            return false;
        }

        MovementGeneratorType mtype = IDLE_MOTION_TYPE;
        if (option >0.0f)
            mtype = RANDOM_MOTION_TYPE;

        Creature* creature = handler->getSelectedCreature();
        uint32 guidLow = 0;

        if (creature)
            guidLow = creature->GetDBTableGUIDLow();
        else
            return false;

        creature->SetWanderDistance((float)option);
        creature->SetDefaultMovementType(mtype);
        creature->GetMotionMaster()->Initialize();
        if (creature->IsAlive())                                // dead creature will reset movement generator at respawn
        {
            creature->setDeathState(JUST_DIED);
            creature->Respawn();
        }

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_WANDER_DISTANCE);

        stmt->setFloat(0, option);
        stmt->setUInt8(1, uint8(mtype));
        stmt->setUInt32(2, guidLow);

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage(LANG_COMMAND_WANDER_DISTANCE, option);
        return true;
    }

    //spawn time handling
    static bool HandleNpcSetSpawnTimeCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* stime = strtok((char*)args, " ");

        if (!stime)
            return false;

        int spawnTime = atoi((char*)stime);

        if (spawnTime < 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Creature* creature = handler->getSelectedCreature();
        uint32 guidLow = 0;

        if (creature)
            guidLow = creature->GetDBTableGUIDLow();
        else
            return false;

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_UPD_CREATURE_SPAWN_TIME_SECS);

        stmt->setUInt32(0, uint32(spawnTime));
        stmt->setUInt32(1, guidLow);

        WorldDatabase.Execute(stmt);

        creature->SetRespawnDelay((uint32)spawnTime);
        handler->PSendSysMessage(LANG_COMMAND_SPAWNTIME, spawnTime);

        return true;
    }

    static bool HandleNpcSayCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->Say(args, LANG_UNIVERSAL);

        // make some emotes
        char lastchar = args[strlen(args) - 1];
        switch (lastchar)
        {
        case '?':   creature->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);      break;
        case '!':   creature->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);   break;
        default:    creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);          break;
        }

        return true;
    }

    //show text emote by creature in chat
    static bool HandleNpcTextEmoteCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->TextEmote(args, 0);

        return true;
    }

    //npc unfollow handling
    static bool HandleNpcUnFollowCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->PSendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (/*creature->GetMotionMaster()->empty() ||*/
            creature->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        FollowMovementGenerator<Creature> const* mgen = static_cast<FollowMovementGenerator<Creature> const*>((creature->GetMotionMaster()->top()));

        if (mgen->GetTarget() != player)
        {
            handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName().c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }

        // reset movement
        creature->GetMotionMaster()->MovementExpired(true);

        handler->PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU_NOW, creature->GetName().c_str());
        return true;
    }

    // make npc whisper to player
    static bool HandleNpcWhisperCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* receiver_str = strtok((char*)args, " ");
        char* text = strtok(NULL, "");

        Creature* creature = handler->getSelectedCreature();
        if (!creature || !receiver_str || !text)
            return false;

        ObjectGuid receiver_guid = ObjectGuid(HighGuid::Player, uint32(atol(receiver_str)));

        // check online security
        Player* receiver = ObjectAccessor::FindPlayer(receiver_guid);
        if (handler->HasLowerSecurity(receiver, ObjectGuid::Empty))
            return false;

        creature->Whisper(text, LANG_UNIVERSAL, receiver);
        return true;
    }

    static bool HandleNpcYellCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Creature* creature = handler->getSelectedCreature();
        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        creature->Yell(args, LANG_UNIVERSAL);

        // make an emote
        creature->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);

        return true;
    }

    // add creature, temp only
    static bool HandleNpcAddTempSpawnCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* charID = handler->extractKeyFromLink((char*)args, "Hcreature_entry");
        if (!charID)
            return false;

        Player* chr = handler->GetSession()->GetPlayer();

        uint32 id = atoi(charID);
        if (!id)
            return false;

        if (!sObjectMgr->GetCreatureTemplate(id))
            return false;

        chr->SummonCreature(id, *chr, TEMPSUMMON_CORPSE_DESPAWN, 120);

        return true;
    }

    //npc tame handling
    static bool HandleNpcTameCommand(ChatHandler* handler, char const* /*args*/)
    {
        Creature* creatureTarget = handler->getSelectedCreature();
        if (!creatureTarget || creatureTarget->IsPet())
        {
            handler->PSendSysMessage (LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage (true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        if (player->GetPetGUID())
        {
            handler->SendSysMessage (LANG_YOU_ALREADY_HAVE_PET);
            handler->SetSentErrorMessage (true);
            return false;
        }

        CreatureTemplate const* cInfo = creatureTarget->GetCreatureTemplate();

        if (!cInfo->IsTameable (player->CanTameExoticPets()))
        {
            handler->PSendSysMessage (LANG_CREATURE_NON_TAMEABLE, cInfo->Entry);
            handler->SetSentErrorMessage (true);
            return false;
        }

        int8 newPetSlot = player->GetSlotForNewPet();
        if (newPetSlot == -1)
            return false;

        // Everything looks OK, create new pet
        Pet* pet = player->CreateTamedPetFrom(creatureTarget);
        if (!pet)
        {
            handler->PSendSysMessage (LANG_CREATURE_NON_TAMEABLE, cInfo->Entry);
            handler->SetSentErrorMessage (true);
            return false;
        }

        // place pet before player
        float x, y, z;
        player->GetClosePoint (x, y, z, creatureTarget->GetObjectSize(), CONTACT_DISTANCE);
        pet->Relocate(x, y, z, M_PI-player->GetOrientation());

        // set pet to defensive mode by default (some classes can't control controlled pets in fact).
        pet->SetReactState(REACT_DEFENSIVE);

        // calculate proper level
        uint8 level = (creatureTarget->GetLevel() < (player->GetLevel() - 5)) ? (player->GetLevel() - 5) : creatureTarget->GetLevel();

        // prepare visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, level - 1);

        // add to world
        pet->GetMap()->AddToMap(pet->ToCreature());

        // visual effect for levelup
        pet->SetUInt32Value(UNIT_FIELD_LEVEL, level);

        // caster have pet now
        player->SetMinion(pet, true);

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        pet->SavePetToDB(trans);
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHAR_PET_SLOT_BY_ID);
        stmt->setUInt8(0, newPetSlot);
        stmt->setUInt32(1, player->GetGUID().GetCounter());
        stmt->setUInt32(2, pet->GetCharmInfo()->GetPetNumber());
        trans->Append(stmt);
        CharacterDatabase.CommitTransaction(trans);
        player->PetSpellInitialize();

        player->GetSession()->SendPetList(ObjectGuid::Empty, PET_SLOT_ACTIVE_FIRST, PET_SLOT_ACTIVE_LAST);

        return true;
    }

    static bool HandleNpcAddFormationCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 leaderGUID = (uint32) atoi((char*)args);
        Creature* creature = handler->getSelectedCreature();

        if (!creature || !creature->GetDBTableGUIDLow())
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 lowguid = creature->GetDBTableGUIDLow();
        if (creature->GetFormation())
        {
            handler->PSendSysMessage("Selected creature is already member of group %u", creature->GetFormation()->GetId());
            return false;
        }

        if (!lowguid)
            return false;

        Player* chr = handler->GetSession()->GetPlayer();
        FormationInfo group_member;

        group_member.follow_angle   = (creature->GetAngle(chr) - chr->GetOrientation()) * 180 / M_PI;
        group_member.follow_dist    = sqrtf(pow(chr->GetPositionX() - creature->GetPositionX(), int(2))+pow(chr->GetPositionY() - creature->GetPositionY(), int(2)));
        group_member.leaderGUID     = leaderGUID;
        group_member.groupAI        = 0;

        sFormationMgr->CreatureGroupMap[lowguid] = group_member;
        creature->SearchFormation();

        WorldDatabasePreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_CREATURE_FORMATION);

        stmt->setUInt32(0, leaderGUID);
        stmt->setUInt32(1, lowguid);
        stmt->setFloat(2, group_member.follow_dist);
        stmt->setFloat(3, group_member.follow_angle);
        stmt->setUInt32(4, uint32(group_member.groupAI));

        WorldDatabase.Execute(stmt);

        handler->PSendSysMessage("Creature %u added to formation with leader %u", lowguid, leaderGUID);

        return true;
    }

    static bool HandleNpcSetLinkCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        uint32 linkguid = (uint32) atoi((char*)args);

        Creature* creature = handler->getSelectedCreature();

        if (!creature)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!creature->GetDBTableGUIDLow())
        {
            handler->PSendSysMessage("Selected creature %u isn't in creature table", creature->GetGUID().GetCounter());
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!sObjectMgr->SetCreatureLinkedRespawn(creature->GetDBTableGUIDLow(), linkguid))
        {
            handler->PSendSysMessage("Selected creature can't link with guid '%u'", linkguid);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage("LinkGUID '%u' added to creature with DBTableGUID: '%u'", linkguid, creature->GetDBTableGUIDLow());
        return true;
    }

    static bool HandleNpcScaleCommand(ChatHandler* handler, char const* args)
    {
        Creature* target = handler->getSelectedCreature();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Tokenizer tok{ args, ' ' };
        uint32 count = std::strtoul(tok[0], nullptr, 10);
        if (!count || count < 10)
        {
            handler->PSendSysMessage("Incorrect value.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto info = sObjectMgr->GetCreatureScalingData(target->GetEntry(), count);
        if (!info)
        {
            handler->PSendSysMessage("Hasn't data for this creature.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        target->RecalculateDynamicHealth(info->Health);
        return true;
    }
};

void AddSC_npc_commandscript()
{
    new npc_commandscript();
}
