#pragma once
#include "Define.h"
#include "Player.h"
#include "WorldSession.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Bag.h"
#include "ObjectGuid.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <vector>
#include <string>
#include "DatabaseEnv.h"
#include "Chat.h"
class ItemEnchantment {
public:
    static void ApplyVisualEnchantment(Player* player, uint32 entry, uint32 spell);
    static bool SaveItemSpell(Player* player, Item* item, uint32 spell);
    static void ApplySpellOnLogin(Player* player);
    static uint32 GetRandomEnchantmentByClassId(Player* player);
};
