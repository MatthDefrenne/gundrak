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

StatsBoost::StatsBoost() {}

StatsBoost::~StatsBoost()
{

}


std::string StatsBoost::GetStatsPoints(Player * player)
{
	QueryResult result = CharacterDatabase.PQuery("SELECT statsPoints FROM characters WHERE guid = %u", player->GetGUID());
	Field* fields = result->Fetch();
	std::stringstream sp;
	sp << fields[0].GetUInt64();
	return sp.str();
}

void StatsBoost::UpdateStatsPlayer(Player * player, uint32 stat, float amount)
{
	switch (stat)
	{
	case 1: // Spirit
		player->SetStatFlatModifier(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE, player->GetFlatModifierValue(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE) + amount);
		break;
	case 2: // Strength
		player->SetStatFlatModifier(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE, player->GetFlatModifierValue(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE) + amount);
		break;
	case 3: // Stamina
		player->SetStatFlatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, player->GetFlatModifierValue(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE) + amount);
		break;
	case 4: // Agility 
		player->SetStatFlatModifier(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE, player->GetFlatModifierValue(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE) + amount);
		break;
	case 5: // Intellect
		player->SetStatFlatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, player->GetFlatModifierValue(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE) + amount);
		break;
	case 6: // melee attack power
		player->SetStatFlatModifier(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE, player->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER, TOTAL_VALUE) + amount);
		break;
	case 7:
		player->ApplyRatingMod(CR_CRIT_MELEE, amount, true);
		break;
	case 8:
		player->ApplyRatingMod(CR_HIT_MELEE, amount, true);
		break;
	case 9:
		player->ApplyRatingMod(CR_EXPERTISE, amount, true);
		break;
	case 10:
		player->ApplyRatingMod(CR_ARMOR_PENETRATION, amount, true);
		break;
	case 11:
		player->ApplyRatingMod(CR_HASTE_MELEE, amount, true);
		break;
	case 12:
		player->SetStatFlatModifier(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE, player->GetFlatModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, TOTAL_VALUE) + amount);
		break;
	case 13:
		player->ApplyRatingMod(CR_CRIT_RANGED, amount, true);
		break;
	case 14:
		player->ApplyRatingMod(CR_HIT_RANGED, amount, true);
		break;
	case 15:
		player->ApplyRatingMod(CR_HASTE_RANGED, amount, true);
		break;
	case 16:
		player->ApplySpellPowerBonus(amount, true);
		break;
	case 17:
		player->ApplyRatingMod(CR_CRIT_SPELL, amount, true);
		break;
	case 18:
		player->ApplyRatingMod(CR_HIT_SPELL, amount, true);
		break;
	case 19:
		player->ApplyRatingMod(CR_HASTE_SPELL, amount, true);
        break;
	case 20:
		player->ApplySpellPenetrationBonus(amount, true);
		break;
	}
}

void StatsBoost::UpdateStatsPlayerOnLogin(Player * player)
{

    StatsBoost::insertTotalUpgradePlayer(player);

    QueryResult result = CharacterDatabase.PQuery("SELECT * FROM characters_stats_boost WHERE guid = %u", player->GetGUID());
    if (!result)
        return;
    do
    {
      Field* fields = result->Fetch();
      StatsBoost::UpdateStatsPlayer(player, fields[1].GetUInt64(), fields[2].GetFloat());
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

    QueryResult result2 = CharacterDatabase.PQuery("SELECT statsPoints, totalUpgrade FROM characters WHERE guid = %u", player->GetGUID());

    if (!result2)
        return;

    Field* fields2 = result2->Fetch();
    uint64 statsPoints = fields2[0].GetUInt64();
    uint64 totalUpgrade = fields2[1].GetUInt64();

    std::vector<int> required = StatsBoost::CalculateUpgrade(player, totalUpgrade);
    costToUpgradeStat = required[0 /*RANK*/];

    if (statsPoints < costToUpgradeStat) {
        player->GetSession()->SendAreaTriggerMessage("you don't have enough of stats points");
        return;
    }

    CharacterDatabase.PQuery("INSERT INTO characters_stats_boost VALUES (%u, %u, %f) ON DUPLICATE KEY UPDATE amount = amount + %f", player->GetGUID(), stat, 1.0f, 1.0f);

    StatsBoost::RemoveStatsPointsToPlayer(player, costToUpgradeStat);
    StatsBoost::UpdateStatsPlayer(player, stat, 1.f);
}

void StatsBoost::RemoveStatsPointsToPlayer(Player * player, uint64 amount)
{
    auto it = StatsBoost::MapTotalUpgradePlayers.find(player->GetGUID());

    if (it != StatsBoost::MapTotalUpgradePlayers.end())
        StatsBoost::MapTotalUpgradePlayers[player->GetGUID()] = (*it).second + 1;

    CharacterDatabase.PExecute("UPDATE characters SET statsPoints = statsPoints - %u, totalUpgrade = totalUpgrade + %u WHERE guid = %u", amount,amount, player->GetGUID());
}

void StatsBoost::GiveStatsPointsToPlayer(Player * player, uint64 amount)
{
    CharacterDatabase.PExecute("UPDATE characters SET statsPoints = statsPoints + %u WHERE guid = %u", amount, player->GetGUID());
    std::string amountToChar = "Congratulations, you have earned " + std::to_string(amount) + " stats points. You can use your grimoire in your inventory to spend it.";
    char const *pchar = amountToChar.c_str();  //use char const* as target type
    ChatHandler(player->GetSession()).PSendSysMessage(pchar);
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
