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
#include "GameObject.h"
#include "GameObjectAI.h"
#include "ObjectMgr.h"

void sendGossipMenuStats(Player* player, Item* item) {

    AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/inv_misc_book_11:30:30:-20:0|tShow me bases stats", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/inv_sword_27:30:30:-20:0|tShow me melee stats", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/inv_weapon_bow_07:30:30:-20:0|tShow me ranged stats", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_fire_flamebolt:30:30:-20:0|tShow me spell stats", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
    AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/inv_shield_04:30:30:-20:0|tShow me defense stats", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
    AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/achievement_general:30:30:-20:0|tBuy 1 talent point (100 points of Knowledge)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
    AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/inv_enchant_disenchant:30:30:-20:0|t|cff003939Reset stats allocation", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
    SendGossipMenuFor(player, 90000, item->GetGUID());
}

void referral(Player* player) {
    QueryResult result = LoginDatabase.PQuery("SELECT COUNT(*) FROM account WHERE last_ip = %u", player->GetSession()->GetAccountId());
    Field* field = result->Fetch();
    bool doubleIP = field[0].GetInt64() == 1;
    if (!doubleIP) {
        QueryResult result2 = LoginDatabase.PQuery("SELECT email_verified, referral_id FROM account WHERE id = %u", player->GetSession()->GetAccountId());
        Field* field2 = result2->Fetch();
        bool emailVerified = field2[0].GetUInt64() == 1;
        bool referral = field2[1].GetUInt64() != 0;
        player->GetSession()->SendAreaTriggerMessage("%u", field2[1].GetUInt64());
        if (emailVerified && referral) {
            QueryResult result3 = LoginDatabase.PQuery("SELECT * FROM referrals WHERE id = %u AND idReferral = %u", player->GetSession()->GetAccountId(), field2[1].GetUInt64() /*referralId*/);
            if (!result3) {
                LoginDatabase.PQuery("UPDATE account SET ptr = ptr + 1 WHERE id = %u", field2[1].GetUInt64() /*referralId*/);
                LoginDatabase.PQuery("INSERT INTO referrals VALUES (%u, %u, NOW())", player->GetSession()->GetAccountId(), field2[1].GetUInt64());
            }
        }
    }
}

class StatsBoostSystem : PlayerScript {


   
    public:

        std::map<ObjectGuid, ObjectGuid> mapLastKillPlayers;
        std::map<ObjectGuid, uint32> mapLastKillRarePlayer;

        StatsBoostSystem() : PlayerScript("StatsBoostSystem") { }


        void killRare(Player* killer, Creature* killed) {
            auto it = mapLastKillRarePlayer.find(killer->GetGUID());

            if (it != mapLastKillRarePlayer.end()) {
                if ((*it).second != killed->GetEntry()) {
                    mapLastKillRarePlayer[killer->GetGUID()] = killed->GetEntry();
                    StatsBoost::GiveStatsPointsToPlayer(killer, 10);
                }
            }
            else {
                mapLastKillRarePlayer[killer->GetGUID()] = killed->GetEntry();
                StatsBoost::GiveStatsPointsToPlayer(killer, 10);
            }
        }

        void OnLevelChanged(Player* player, uint8 oldlevel) override {           // We notice the player he as a new points

            if (player->getLevel() == 20) {
                referral(player);
            }
            int mupltiplicator = player->getLevel() - oldlevel;
            StatsBoost::GiveStatsPointsToPlayer(player, (StatsBoost::REWARD_ON_LEVELUP * mupltiplicator));
        }

        void OnCreatureKill(Player* killer, Creature* killed) {
            if (killed->GetCreatureTemplate()->rank == CREATURE_ELITE_RARE)
                killRare(killer, killed);

            StatsBoost::RewardStatsPointsOnKillBoss(killer, killed);
        }

        void OnPlayerReputationChange(Player* player, uint32 factionID, int32& standing, bool incremental) {
        }

        void OnPVPKill(Player* killer, Player* killed) {

            if (killer->GetGUID() == killed->GetGUID())
                return;

            auto it = mapLastKillPlayers.find(killer->GetGUID());

            if (it != mapLastKillPlayers.end())
                if ((*it).second != killed->GetGUID()) {
                    mapLastKillPlayers[killer->GetGUID()] = killed->GetGUID();
                    StatsBoost::GiveStatsPointsToPlayer(killer, 10);
               }
            else {
                mapLastKillPlayers[killer->GetGUID()] = killed->GetGUID();
                StatsBoost::GiveStatsPointsToPlayer(killer, 10);
            }   
        }

       void OnLogout(Player* player) {
           StatsBoost::onLogoutSaveStats(player);
       }

       void OnSave(Player* player) {
           StatsBoost::onLogoutSaveStats(player);
       }

        void OnChat(Player* player, uint32 /*type*/, uint32 /*lang*/, std::string& msg) {
            StatsBoost::ShowRankByTotalUpgrade(player, msg);
        }

        void OnChat(Player* player, uint32 /*type*/, uint32 /*lang*/, std::string& msg, Player* /*receiver*/) {
            StatsBoost::ShowRankByTotalUpgrade(player, msg);
        }

        void OnChat(Player* player, uint32 /*type*/, uint32 /*lang*/, std::string& msg, Group* /*group*/) {
            StatsBoost::ShowRankByTotalUpgrade(player, msg);
        }

        void OnChat(Player* player, uint32 /*type*/, uint32 /*lang*/, std::string& msg, Guild* /*guild*/) {
            StatsBoost::ShowRankByTotalUpgrade(player, msg);
        }

        void OnChat(Player* player, uint32 /*type*/, uint32 /*lang*/, std::string& msg, Channel* /*channel*/) {
            StatsBoost::ShowRankByTotalUpgrade(player, msg);
        }


        void OnLogin(Player* player, bool firstLogin) {
            switch (player->getRace())
            {
            case RACE_HUMAN:
                player->LearnSpell(458, false);

                break;
            case RACE_BLOODELF:
                player->LearnSpell(34795, false);

                break;
            case RACE_DRAENEI:
                player->LearnSpell(35710, false);

                break;
            case RACE_GNOME:
                player->LearnSpell(10873, false);

                break;
            case RACE_NIGHTELF:
                player->LearnSpell(10789, false);

                break;
            case RACE_ORC:
                player->LearnSpell(6653, false);

                break;
            case RACE_TAUREN:
                player->LearnSpell(18989, false);
                break;
            case RACE_TROLL:
                player->LearnSpell(10796, false);

                break;
            case RACE_DWARF:
                player->LearnSpell(6777, false);

                break;
            case RACE_UNDEAD_PLAYER:
                player->LearnSpell(17464, false);
                break;
            default:
                break;
            }
        }
};

void sendMenuGossip(Player* player, Item* item, uint32& action) {

    ClearGossipMenuFor(player); // Clears old options
    uint64 totalUpgrade = StatsBoost::GetTotalUpgradePlayer(player);
    uint64 requiredNextRank = StatsBoost::GetRequiredUpgradeToReachNextRank(player);
    AddGossipItemFor(player, GOSSIP_ICON_DOT, StatsBoost::GetRankImage(player, "30", "-20") + "Next rank : " + std::to_string(totalUpgrade) + " / " + std::to_string(requiredNextRank), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_arcane_arcane04:30:30:-20:0|tYou have " + std::to_string(StatsBoost::GetStatsPoints(player)) + " point(s) of knowledge", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    switch (action)
    {
    case GOSSIP_ACTION_INFO_DEF:
        sendGossipMenuStats(player, item);
        break;
    case GOSSIP_ACTION_INFO_DEF + 1:
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/inv_enchant_shardbrilliantsmall:30:30:-20:0|t Upgrade Spirit", GOSSIP_SENDER_MAIN, 1, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_strength:30:30:-20:0|t Upgrade Strength", GOSSIP_SENDER_MAIN, 2, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_unyeildingstamina:30:30:-20:0|t Upgrade Stamina", GOSSIP_SENDER_MAIN, 3, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_holy_blessingofagility:30:30:-20:0|t Upgrade Agility", GOSSIP_SENDER_MAIN, 4, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_holy_magicalsentry:30:30:-20:0|t Upgrade Intellect", GOSSIP_SENDER_MAIN, 5, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "<- Back", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, 90001, item->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF + 2:
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_warrior_battleshout:30:30:-20:0|t Upgrade melee attack power", GOSSIP_SENDER_MAIN, 6, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_criticalstrike:30:30:-20:0|tUpgrade melee critical hit", GOSSIP_SENDER_MAIN, 7, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_marksmanship:30:30:-20:0|tUpgrade melee hit rating", GOSSIP_SENDER_MAIN, 8, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_gouge:30:30:-20:0|tUpgrade expertise rating", GOSSIP_SENDER_MAIN, 9, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_warrior_sunder:30:30:-20:0|tUpgrade armor penetration rating", GOSSIP_SENDER_MAIN, 10, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_invisibilty:30:30:-20:0|tUpgrade haste melee", GOSSIP_SENDER_MAIN, 11, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "<- Back", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, 90001, item->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF + 3:
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_trueshot:30:30:-20:0|t Upgrade Ranged attack power", GOSSIP_SENDER_MAIN, 12, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_criticalstrike:30:30:-20:0|tUpgrade Ranged critical hit", GOSSIP_SENDER_MAIN, 13, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_marksmanship:30:30:-20:0|tUpgrade Ranged hit rating", GOSSIP_SENDER_MAIN, 14, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/ability_hunter_runningshot:30:30:-20:0|tUpgrade Haste ranged", GOSSIP_SENDER_MAIN, 15, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "<- Back", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, 90001, item->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF + 4:
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_lightning:30:30:-20:0|tUpgrade Spells power", GOSSIP_SENDER_MAIN, 16, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_strength:30:30:-20:0|tUpgrade Spells critical hit", GOSSIP_SENDER_MAIN, 17, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_drowsy:30:30:-20:0|tUpgrade Spells hit rating", GOSSIP_SENDER_MAIN, 18, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_slowingtotem:30:30:-20:0|tUpgrade Haste spell", GOSSIP_SENDER_MAIN, 19, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_holy_arcaneintellect:30:30:-20:0|tUpgrade Spell penetration rating", GOSSIP_SENDER_MAIN, 20, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "<- Back", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, 90001, item->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF + 5:
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_lightning:30:30:-20:0|tUpgrade Dodge rating", GOSSIP_SENDER_MAIN, 21, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_strength:30:30:-20:0|tUpgrade Parry rating", GOSSIP_SENDER_MAIN, 22, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_nature_drowsy:30:30:-20:0|tUpgrade Block", GOSSIP_SENDER_MAIN, 23, "Amount of upgrade", 0, true);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "<- Back", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, 90001, item->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF + 10:
        if (StatsBoost::GetStatsPoints(player) <= 100) {
            player->GetSession()->SendAreaTriggerMessage("You don't have enough of Points of Knowledge!");
            CloseGossipMenuFor(player);
        }
        else {
            player->SetFreeTalentPoints(1);
            StatsBoost::RemoveStatsPointsToPlayer(player, 100);
            CloseGossipMenuFor(player);
        }
        break;
    case GOSSIP_ACTION_INFO_DEF + 6:
        if (player->getLevel() >= 20)
          AddGossipItemFor(player, GOSSIP_ICON_DOT, "|cff390000Confirm reset stats allocation (cost : 5 golds)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
        else
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|cff390000Confirm reset stats allocation (Free before level 20)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "<- Back", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, 90000, item->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF + 7:
        if (player->getLevel() >= 20) {
            uint32 gold = player->GetMoney();
            if (gold >= 50000) {
                StatsBoost::ResetStatsAllocation(player);
                player->ModifyMoney(-50000);
            }
            else {
                player->GetSession()->SendAreaTriggerMessage("You don't have enough of gold!");
            }
        }
        else 
            StatsBoost::ResetStatsAllocation(player);
        CloseGossipMenuFor(player);
        break;
    }
}
class StatsBoostItem : ItemScript {

public:
    StatsBoostItem() : ItemScript("StatsBoostItem") { }

    std::map<ObjectGuid, uint32> MaplastSavedAction;
    uint32 statsToUpgrade;

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override // Any hook here
    {

        ClearGossipMenuFor(player);
        uint64 totalUpgrade = StatsBoost::GetTotalUpgradePlayer(player);
        uint64 requiredNextRank = StatsBoost::GetRequiredUpgradeToReachNextRank(player);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, StatsBoost::GetRankImage(player, "30", "-20") + "Next rank : " + std::to_string(totalUpgrade) + " / " + std::to_string(requiredNextRank), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "|TInterface/ICONS/spell_arcane_arcane04:30:30:-20:0|tYou have " + std::to_string(StatsBoost::GetStatsPoints(player)) + " point(s) of knowledge", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        sendGossipMenuStats(player, item);
        return false; // Cast the spell on use normally
    }

    void OnGossipSelect(Player* player, Item* item, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player); // Clears old options

        if (action >= 1000)
            MaplastSavedAction[player->GetGUID()] = action;
        else
            MaplastSavedAction[player->GetGUID()] = 1000;


        sendMenuGossip(player, item, MaplastSavedAction[player->GetGUID()]);
    }


    void OnGossipSelectCode(Player* player, Item* item, uint32 sender, uint32 action, const char* code) {
       if (!code) {
            player->GetSession()->SendAreaTriggerMessage("Please enter a number!");
            CloseGossipMenuFor(player);
        }
        float amount = atof(code);
        if (!amount) {
            player->GetSession()->SendAreaTriggerMessage("Please enter a number!");
            CloseGossipMenuFor(player);
        }
        StatsBoost::AddStatToPlayer(player, amount, action);
        sendMenuGossip(player, item, MaplastSavedAction[player->GetGUID()]);
    }

};

class StatsBoostGobject : public GameObjectScript
{
public:
    StatsBoostGobject() : GameObjectScript("StatsBoostGobject") { }
    struct gobject_stats_boost : public GameObjectAI
    {


        gobject_stats_boost(GameObject* go) : GameObjectAI(go) {

        }

        bool GossipHello(Player* player) override
        {
            QueryResult result = CharacterDatabase.PQuery("SELECT * FROM gameobject_statsboost WHERE guid = %u AND position_x = %f AND position_y = %f AND position_z = %f", player->GetGUID(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
            if (!result) {
                StatsBoost::GiveStatsPointsToPlayer(player, 3);
                CharacterDatabase.PQuery("INSERT INTO gameobject_statsboost VALUES (%u, %f, %f, %f)", player->GetGUID(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                me->SendObjectDeSpawnAnim(me->GetGUID());
                CloseGossipMenuFor(player);
            }
            else {
                player->GetSession()->SendAreaTriggerMessage("You've already discovered this artefact!");
                CloseGossipMenuFor(player);
            }

            return true;
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new gobject_stats_boost(go);
    }
};


void AddSC_BoostSystemScript() // Add to scriptloader normally
{
    new StatsBoostItem();
    new StatsBoostSystem();
    new StatsBoostGobject();
}
