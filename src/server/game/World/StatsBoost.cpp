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

std::map<const int, const uint64> StatsBoost::MAX_UPDATE_STAT = {
    { 1 /* AGILITY */, 500 /* MAX UPGRADABLE */ },
    { 2 /* AGILITY */, 500 /* MAX UPGRADABLE */ },
    { 4 /* AGILITY */, 500 /* MAX UPGRADABLE */ },
    { 5 /* AGILITY */, 500 /* MAX UPGRADABLE */ },
    { 19 /* AGILITY */, 500 /* MAX UPGRADABLE */ },
    { 11 /* AGILITY */, 500 /* MAX UPGRADABLE */ },
    { 15 /* AGILITY */, 500 /* MAX UPGRADABLE */ },
    { 20 /* AGILITY */, 200 /* MAX UPGRADABLE */ },
    { 21 /* AGILITY */, 200 /* MAX UPGRADABLE */ },
    { 22 /* AGILITY */, 200 /* MAX UPGRADABLE */ },
    { 23 /* AGILITY */, 200 /* MAX UPGRADABLE */ },
};

std::map<const int /*rank*/, std::pair<uint32 /*min*/, uint32 /*max*/>> StatsBoost::RanksRequiredUpgrade = {
    { 1, { 0, 500 } },
    { 2, { 500, 600 } },
    { 3, { 600, 700 } },
    { 4, { 700, 800 } },
    { 5, { 800, 900 } },
    { 6, { 900, 1000 } },
    { 7, { 1000, 1500 } },
    { 8, { 1500, 2500 } },
    { 9, { 2500, 4500 } },
    { 10, { 4500, 9000 } },
    { 11, { 9000, 18000 } },
    { 12, { 18000, 30000 } },
    { 14, { 30000, 60000 } },
    { 15, { 60000, 100000 } },
};

std::map<ObjectGuid, uint64> StatsBoost::MapTotalUpgradePlayers = {};
std::map<ObjectGuid, uint64> StatsBoost::MapTotalStatsPointsPlayer = {};

StatsBoost::StatsBoost() {}

StatsBoost::~StatsBoost()
{

}


uint64 StatsBoost::GetStatsPoints(Player * player)
{
    uint64 totalStatsPoints = 0;
    auto it = StatsBoost::MapTotalStatsPointsPlayer.find(player->GetGUID());

    if (it != StatsBoost::MapTotalStatsPointsPlayer.end()) {
        totalStatsPoints = (*it).second;
    }

    return totalStatsPoints;
}

void StatsBoost::UpdateStatsPlayer(Player * player, uint32 stat, float amount, bool increase)
{
    uint32 stats = 0;

	switch (stat)
	{
	case 1: // Spirit
        if (increase)
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE) + amount;
        else
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE) - amount;

        player->SetStatFlatModifier(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE, stats);
		break;
	case 2: // Strength
        if (increase)
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE) + amount;
        else
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE) - amount;

		player->SetStatFlatModifier(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE, stats);
		break;
	case 3: // Stamina
        if (increase)
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE) + amount;
        else
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE) - amount;

		player->SetStatFlatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, stats);
		break;
	case 4: // Agility
        if (increase)
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE) + amount;
        else
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE) - amount;

		player->SetStatFlatModifier(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE, stats);
		break;
	case 5: // Intellect
        if (increase)
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE) + amount;
        else
            stats = player->GetFlatModifierValue(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE) - amount;

		player->SetStatFlatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, stats);
		break;
	case 6: // melee attack power
        if (increase)
            stats = player->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE) + amount;
        else
            stats = player->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE) - amount;

		player->SetStatFlatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, stats);
		break;
	case 7:
        if (increase)
		    player->ApplyRatingMod(CR_CRIT_MELEE, amount, true);
        else
            player->ApplyRatingMod(CR_CRIT_MELEE, -amount, true);
		break;
	case 8:
        if(increase)
		    player->ApplyRatingMod(CR_HIT_MELEE, amount, true);
        else
            player->ApplyRatingMod(CR_HIT_MELEE, -amount, true);
		break;
	case 9:
        if(increase)
		player->ApplyRatingMod(CR_EXPERTISE, amount, true);
        else
        player->ApplyRatingMod(CR_EXPERTISE, -amount, true);
		break;
	case 10:
        if (increase)
		player->ApplyRatingMod(CR_ARMOR_PENETRATION, amount, true);
        else
            player->ApplyRatingMod(CR_ARMOR_PENETRATION, -amount, true);
		break;
	case 11:
        if (increase)
		player->ApplyRatingMod(CR_HASTE_MELEE, amount, true);
        else
            player->ApplyRatingMod(CR_HASTE_MELEE, -amount, true);
		break;
	case 12:
        if (increase)
            stats = player->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE) + amount;
        else
            stats = player->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE) - amount;
        player->SetStatFlatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, stats);
		break;
	case 13:
        if (increase)
		player->ApplyRatingMod(CR_CRIT_RANGED, amount, true);
        else
            player->ApplyRatingMod(CR_CRIT_RANGED, -amount, true);
		break;
	case 14:
        if (increase)
		player->ApplyRatingMod(CR_HIT_RANGED, amount, true);
        else
            player->ApplyRatingMod(CR_HIT_RANGED, -amount, true);
		break;
	case 15:
        if (increase)
		player->ApplyRatingMod(CR_HASTE_RANGED, amount, true);
        else
            player->ApplyRatingMod(CR_HASTE_RANGED, -amount, true);
		break;
	case 16:
        if (increase)
            player->ApplySpellPowerBonus(amount, true);
        else
            player->ApplySpellPowerBonus(-amount, true);
		break;
	case 17:
        if (increase)
        player->ApplyRatingMod(CR_CRIT_SPELL, amount, true);
        else
            player->ApplyRatingMod(CR_CRIT_SPELL, -amount, true);
		break;
	case 18:
        if (increase)
		player->ApplyRatingMod(CR_HIT_SPELL, amount, true);
        else
            player->ApplyRatingMod(CR_HIT_SPELL, -amount, true);
		break;
	case 19:
        if (increase)
		player->ApplyRatingMod(CR_HASTE_SPELL, amount, true);
        else
            player->ApplyRatingMod(CR_HASTE_SPELL, -amount, true);
        break;
	case 20:
        if (increase)
		player->ApplySpellPenetrationBonus(amount, true);
        else
            player->ApplySpellPenetrationBonus(-amount, true);
		break;
    case 21:
        if (increase)
        player->ApplyRatingMod(CR_DODGE, amount, true);
        else
            player->ApplyRatingMod(CR_DODGE, -amount, true);
        break;
    case 22:
        if (increase)
        player->ApplyRatingMod(CR_PARRY, amount, true);
        else
            player->ApplyRatingMod(CR_PARRY, -amount, true);
        break;
    case 23:
        if (increase)
        player->ApplyRatingMod(CR_BLOCK, amount, true);
        else
            player->ApplyRatingMod(CR_BLOCK, -amount, true);
        break;
	}
}

void StatsBoost::UpdateStatsPlayerOnLogin(Player * player)
{

    StatsBoost::insertTotalUpgradePlayer(player);
    StatsBoost::insertTotalStatsPointsPlayer(player);

    QueryResult result = CharacterDatabase.PQuery("SELECT * FROM characters_stats_boost WHERE guid = %u", player->GetGUID());
    if (!result)
        return;
    do
    {
      Field* fields = result->Fetch();
      StatsBoost::UpdateStatsPlayer(player, fields[1].GetUInt64(), fields[2].GetFloat(), true);
    } while (result->NextRow());
}

void StatsBoost::AddStatToPlayer(Player * player, uint32 amount, uint32 stat)
{

    uint64 amountUpgrade;
    uint64 MAX_UPDATE = 999999;
    uint64 costToUpgradeStat = 1;

    QueryResult result = CharacterDatabase.PQuery("SELECT amount FROM characters_stats_boost WHERE guid = %u and stat_type = %u", player->GetGUID(), stat);

    if (!result)
        amountUpgrade = 0;
    else {
        Field* fields = result->Fetch();
        amountUpgrade = fields[0].GetUInt64();
    }

    auto it = StatsBoost::MAX_UPDATE_STAT.find(stat);

    if (it != StatsBoost::MAX_UPDATE_STAT.end()) {
        MAX_UPDATE = it->second;
    }

    if (MAX_UPDATE <= amountUpgrade) {
        player->GetSession()->SendAreaTriggerMessage("You cannot upgrade this characteristics anymore");
        return;
    }

    QueryResult result2 = CharacterDatabase.PQuery("SELECT totalUpgrade FROM characters WHERE guid = %u", player->GetGUID());

    if (!result2)
        return;

    Field* fields2 = result2->Fetch();
    uint64 statsPoints = StatsBoost::GetStatsPoints(player);
    uint64 totalUpgrade = fields2[0].GetUInt64();

    std::vector<int> required = StatsBoost::CalculateUpgrade(player, totalUpgrade);
    costToUpgradeStat = required[0 /*RANK*/];

    if (statsPoints < costToUpgradeStat) {
        player->GetSession()->SendAreaTriggerMessage("You don't have enough knowledge points");
        return;
    }

    CharacterDatabase.PQuery("INSERT INTO characters_stats_boost VALUES (%u, %u, %f) ON DUPLICATE KEY UPDATE amount = amount + %f", player->GetGUID(), stat, 1.0f, 1.0f);

    StatsBoost::RemoveStatsPointsToPlayer(player, costToUpgradeStat);
    StatsBoost::UpdateStatsPlayer(player, stat, 1.f, true);
}

void StatsBoost::RemoveStatsPointsToPlayer(Player * player, uint64 amount)
{
    // Upgrade total upgrade -----------------------------------------------
    auto it = StatsBoost::MapTotalUpgradePlayers.find(player->GetGUID());

    if (it != StatsBoost::MapTotalUpgradePlayers.end())
        StatsBoost::MapTotalUpgradePlayers[player->GetGUID()] = (*it).second + amount;

    // Upgrade total Stats points ------------------------------------------
    auto itJ = StatsBoost::MapTotalStatsPointsPlayer.find(player->GetGUID());

    if (itJ != StatsBoost::MapTotalStatsPointsPlayer.end())
        StatsBoost::MapTotalStatsPointsPlayer[player->GetGUID()] = (*itJ).second - amount;
}

void StatsBoost::GiveStatsPointsToPlayer(Player * player, uint64 amount)
{

    // Upgrade total Stats points -------------------------------------------------------
    auto it = StatsBoost::MapTotalStatsPointsPlayer.find(player->GetGUID());

    if (it != StatsBoost::MapTotalStatsPointsPlayer.end())
        StatsBoost::MapTotalStatsPointsPlayer[player->GetGUID()] = (*it).second + amount;
    // ----------------------------------------------------------------------------------

    std::string amountToChar = "Congratulations, you have earned " + std::to_string(amount) + " point(s) of knowledge. You can use your Grimoire of Stats allocation in your inventory to spend it.";
    char const *pchar = amountToChar.c_str();  //use char const* as target type
    ChatHandler(player->GetSession()).PSendSysMessage(pchar);
    player->GetSession()->SendAreaTriggerMessage(pchar);
}

void StatsBoost::RewardStatsPointsOnKillBoss(Player* killer, Creature* killed) {

    uint32 level = killer->getLevel();

    if (level > 60)
        level += 5;

    if (!killed->IsDungeonBoss() || level >= killed->getLevel())
        return;

    Group* group = killer->GetGroup();

    if (!group) {
        StatsBoost::GiveStatsPointsToPlayer(killer, StatsBoost::REWARD_ON_KILL_BOSS);
        return;
    }

    bool canRewardGroup = false;

    for (auto it = group->GetMemberSlots().begin(); it != group->GetMemberSlots().end(); ++it)
    {
        Player* player = ObjectAccessor::FindPlayer((*it).guid);
        if (player) {
            canRewardGroup = player->getLevel() >= killed->getLevel();
        }
    }

    for (auto it2 = group->GetMemberSlots().begin(); it2 != group->GetMemberSlots().end(); ++it2)
    {
        Player* player = ObjectAccessor::FindPlayer((*it2).guid);
        if (player && canRewardGroup) {
            StatsBoost::GiveStatsPointsToPlayer(player, StatsBoost::REWARD_ON_KILL_BOSS);
        }
    }
}

void StatsBoost::ShowRankByTotalUpgrade(Player * player, std::string & msg)
{

    if (player->getLevel() < 60)
        return;

    msg = StatsBoost::GetRankImage(player, "18", "0") + msg;
}

std::string StatsBoost::GetRankImage(Player* player, std::string size, std::string move) {
    uint64 totalUpgrade = StatsBoost::GetTotalUpgradePlayer(player);

    std::string rankImage;
    std::string team = player->GetTeamId() == TEAM_HORDE ? "h" : "a";
    std::vector<int> required = StatsBoost::CalculateUpgrade(player, totalUpgrade);
    uint32 rank = required[0 /*RANK*/];
    if (rank > 9) {
        rankImage = "|TInterface/ICONS/achievement_pvp_" + team + "_" + std::to_string(rank) + ":"+size+":" + size + ":" + move + ":0|t";
    }
    else {
        rankImage = "|TInterface/ICONS/achievement_pvp_" + team + "_0" + std::to_string(rank) + ":" + size + ":" + size + ":" + move + ":0|t";
    }

    return rankImage;
}

void StatsBoost::onLogoutSaveStats(Player * player)
{
    uint64 totalUpgrade = 0;
    uint64 totalStatsPoints = 0;

    auto it = StatsBoost::MapTotalUpgradePlayers.find(player->GetGUID());

    if (it != StatsBoost::MapTotalUpgradePlayers.end())
        totalUpgrade = StatsBoost::MapTotalUpgradePlayers[player->GetGUID()];

    // Upgrade total Stats points ------------------------------------------
    auto itJ = StatsBoost::MapTotalStatsPointsPlayer.find(player->GetGUID());

    if (itJ != StatsBoost::MapTotalStatsPointsPlayer.end())
        totalStatsPoints = StatsBoost::MapTotalStatsPointsPlayer[player->GetGUID()];


    CharacterDatabase.PExecute("UPDATE characters SET statsPoints = %u WHERE guid = %u", totalStatsPoints, player->GetGUID());
    CharacterDatabase.PExecute("UPDATE characters SET totalUpgrade = %u WHERE guid = %u", totalUpgrade, player->GetGUID());

}

uint32 StatsBoost::GetRequiredUpgradeToReachNextRank(Player* player) {


    uint64 totalUpgrade = StatsBoost::GetTotalUpgradePlayer(player);
    std::vector<int> required = StatsBoost::CalculateUpgrade(player, totalUpgrade);

    return required[1 /*NEXT UPGRADE*/];
}

std::vector<int> StatsBoost::CalculateUpgrade(Player* player, uint64 totalUpgrade)
{
    std::vector<int> required;
    uint32 rank;
    uint32 upgrade;
    for (auto it = StatsBoost::RanksRequiredUpgrade.begin(); it != StatsBoost::RanksRequiredUpgrade.end(); ++it)
    {
        if (it->second.first >= totalUpgrade && totalUpgrade <= it->second.second)
            break;

        rank = it->first; // Rank
        upgrade = it->second.second; // Next upgrade
    }

    required.push_back(rank);
    required.push_back(upgrade);
    return required;
}

uint64 StatsBoost::GetTotalUpgradePlayer(Player * player)
{
    uint64 totalUpgrade = 0;
    auto it = StatsBoost::MapTotalUpgradePlayers.find(player->GetGUID());

    if (it != StatsBoost::MapTotalUpgradePlayers.end()) {
        totalUpgrade = (*it).second;
    }
    return totalUpgrade;
}

void StatsBoost::insertTotalUpgradePlayer(Player * player)
{
    uint64 totalUpgrade;
    QueryResult result2 = CharacterDatabase.PQuery("SELECT totalUpgrade FROM characters WHERE guid = %u", player->GetGUID());

    if (!result2)
        totalUpgrade = 0;

    Field* fields2 = result2->Fetch();
    totalUpgrade = fields2[0].GetUInt64();

    StatsBoost::MapTotalUpgradePlayers[player->GetGUID()] = totalUpgrade;

}
void StatsBoost::insertTotalStatsPointsPlayer(Player * player)
{
    uint64 totalStatsPoints;
    QueryResult result = CharacterDatabase.PQuery("SELECT statsPoints FROM characters WHERE guid = %u", player->GetGUID());

    if (!result)
        totalStatsPoints = 0;

    Field* fields = result->Fetch();
    totalStatsPoints = fields[0].GetUInt64();

    StatsBoost::MapTotalStatsPointsPlayer[player->GetGUID()] = totalStatsPoints;
}

void StatsBoost::ResetStatsAllocation(Player* player) {

    auto it = StatsBoost::MapTotalUpgradePlayers.find(player->GetGUID());

    if (it != StatsBoost::MapTotalUpgradePlayers.end()) {
        auto itJ = StatsBoost::MapTotalStatsPointsPlayer.find(player->GetGUID());

        if (itJ != StatsBoost::MapTotalStatsPointsPlayer.end())
            StatsBoost::MapTotalStatsPointsPlayer[player->GetGUID()] = ((*itJ).second += StatsBoost::MapTotalUpgradePlayers[player->GetGUID()]) - 1;

        StatsBoost::MapTotalUpgradePlayers[player->GetGUID()] = 1;
    }

    QueryResult result = CharacterDatabase.PQuery("SELECT * FROM characters_stats_boost WHERE guid = %u", player->GetGUID());

    if (!result)
        return;
    do
    {
        Field* fields = result->Fetch();
        StatsBoost::UpdateStatsPlayer(player, fields[1].GetUInt64(), fields[2].GetFloat(), false);
    } while (result->NextRow());

    CharacterDatabase.PQuery("DELETE FROM characters_stats_boost WHERE guid = %u", player->GetGUID());

}

