/*
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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
Name: titles_commandscript
%Complete: 100
Comment: All titles related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "DBCStores.h"
#include "Language.h"
#include "Player.h"
#include "RBAC.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"

class store_commandscript : public CommandScript
{
public:
    store_commandscript() : CommandScript("store_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {

        static std::vector<ChatCommand> commandTable =
        {
            { "store", rbac::RBAC_PERM_COMMAND_STORE,     false, &HandleItemCommande,     "" },
        };
        return commandTable;
    }

    static bool HandleItemCommande(ChatHandler* handler, char const* args)
    {
        if (!*args) {
            handler->SendSysMessage("Please enter a correct store Id, you can found them on the website!");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (handler->HasLowerSecurity(player, ObjectGuid::Empty))
            return false;


        uint32 itemId = (uint32)atoi(args);

        if (!itemId) {
            handler->SendSysMessage("Please enter enter a correct store Id, you can found them on the website!");
            return false;
        }

        QueryResult result = LoginDatabase.PQuery("SELECT * FROM store WHERE id = %u", itemId);

        if (!result) {
            handler->SendSysMessage("StoreId not found, please try a different one.");
            return false;
        }

        Field* field = result->Fetch();
        uint32 storeId = field[1].GetInt32();
        uint32 storeAmount = field[2].GetInt32();
        uint32 cost = field[3].GetInt32();
        std::string currency = field[4].GetString();


        QueryResult account = LoginDatabase.PQuery("SELECT " + currency + " FROM account WHERE id = %u", player->GetSession()->GetAccountId());

        Field* field2 = account->Fetch();
        uint32 amountCurrency = field2[0].GetInt32();

         
        if (amountCurrency < cost) {
            handler->SendSysMessage("You don't have enough of point to buy this item!");
            return false;
        }

        player->AddItem(storeId, storeAmount);
        LoginDatabase.PQuery("UPDATE account SET " + currency + " = " + currency + " - %u  WHERE id = %u", cost, player->GetSession()->GetAccountId());
        handler->SendSysMessage("Thank you for buying this item through the shop! Have fun on Gundrak");

        return true;
    }

    
};

void AddSC_store_commandscript()
{
    new store_commandscript();
}
