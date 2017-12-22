#include "CustomRates.h"


std::map<ObjectGuid, float> CustomRates::m_CustomRates = {};

void CustomRates::setCustomRate(Player * player, float rate)
{
    CustomRates::m_CustomRates[player->GetGUID()] = rate;
    CharacterDatabase.PQuery("UPDATE characters SET rate = %f WHERE guid = %u", rate, player->GetGUID());
}

void CustomRates::setCustomCommandeOnLogin(Player* player)
{
    QueryResult result = CharacterDatabase.PQuery("SELECT rate FROM characters WHERE guid = %u", player->GetGUID());
    Field* field = result->Fetch();
    CustomRates::m_CustomRates[player->GetGUID()] = field[0].GetFloat();
}

float CustomRates::get(Player * player)
{
    float rate = 5.f;

    auto it = m_CustomRates.find(player->GetGUID());

    if (it != CustomRates::m_CustomRates.end())
        rate = (*it).second;

    return rate;
}
