#pragma once
#include "Player.h"

class StatsBoost
{
public:
	StatsBoost();
	~StatsBoost();

	static uint64 GetStatsPoints(Player* player);
	static void UpdateStatsPlayer(Player* player, uint32 stats, float amount, bool increase);
	static void UpdateStatsPlayerOnLogin(Player* player);
    static void AddStatToPlayer(Player* player, uint32 amount, uint32 stat);
    static void RemoveStatsPointsToPlayer(Player* player, uint64 amount);
    static void GiveStatsPointsToPlayer(Player* player, uint64 amount);
    static void RewardStatsPointsOnKillBoss(Player* killer, Creature* killed);
    static void ShowRankByTotalUpgrade(Player* player, std::string &msg);
    static uint32 GetRequiredUpgradeToReachNextRank(Player* player);
    static std::vector<int> CalculateUpgrade(Player* player, uint64 totalUpgrade);
    static uint64 GetTotalUpgradePlayer(Player* player);
    static void RewardStatsPointsOnUpgradeReputation(Player * player, uint32 factionID);
    static std::string GetRankImage(Player* player, std::string size, std::string move);
    static std::map<const int, const uint64> MAX_UPDATE_STAT;
    static std::map<const int, std::pair<uint32 /*min*/, uint32 /*max*/>> RanksRequiredUpgrade;
    static std::map<ObjectGuid, uint64> MapTotalUpgradePlayers;
    static std::map<ObjectGuid, uint64> MapTotalStatsPointsPlayer;
    static void onLogoutSaveStats(Player* player);
    static void ResetStatsAllocation(Player* player);
    static const int REWARD_ON_KILL_BOSS = 25;
    static const int REWARD_ON_LEVELUP = 5;

private:
    static const int TOTAL_UPGRADABLE_STAT = 20;
    static void insertTotalUpgradePlayer(Player* player);
    static void insertTotalStatsPointsPlayer(Player* player);



};
 
