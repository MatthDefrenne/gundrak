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

class ItemEnchantment {
public:
    static void ApplyVisualEnchantment(Player* player, uint32 entry);
};
