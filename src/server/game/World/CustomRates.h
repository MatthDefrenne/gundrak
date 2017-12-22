#pragma once
#include "Define.h"
#include "GossipDef.h"
#include "Item.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "WorldSession.h"
#include "DatabaseEnv.h"
#include "StatsBoost.h"
#include "Group.h"
#include "Creature.h"
#include "Chat.h"
#include "World.h"
#include "ReputationMgr.h"
#include "Map.h"
#include "ObjectMgr.h"

class CustomRates {

public:
    static std::map<ObjectGuid, float> m_CustomRates;
    static void setCustomRate(Player* player, float rate);
    static void setCustomCommandeOnLogin(Player* player);
    static float get(Player* player);
};
