#include "Define.h"
#include "GossipDef.h"
#include "Item.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Object.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ItemEnchantment.h"
#include "Chat.h"
#include "World.h"
#include "Group.h"

static const char* GetSlotName(uint8 slot, WorldSession* /*session*/)
{
    switch (slot)
    {
    case EQUIPMENT_SLOT_HEAD: return "Head";
    case EQUIPMENT_SLOT_SHOULDERS: return "Shoulders";
    case EQUIPMENT_SLOT_BODY: return "Shirt";
    case EQUIPMENT_SLOT_CHEST: return "Chest";
    case EQUIPMENT_SLOT_WAIST: return "Waist";
    case EQUIPMENT_SLOT_LEGS: return "Legs";
    case EQUIPMENT_SLOT_FEET: return "Feet";
    case EQUIPMENT_SLOT_WRISTS: return "Wrists";
    case EQUIPMENT_SLOT_HANDS: return "Hands";
    case EQUIPMENT_SLOT_FINGER1: return "Right finger";
    case EQUIPMENT_SLOT_FINGER2: return "Left finger";
    case EQUIPMENT_SLOT_TRINKET1: return "Right trinket";
    case EQUIPMENT_SLOT_TRINKET2: return "Left trinket";
    case EQUIPMENT_SLOT_BACK: return "Back";
    case EQUIPMENT_SLOT_MAINHAND: return "Main hand";
    case EQUIPMENT_SLOT_OFFHAND: return "Off hand";
    case EQUIPMENT_SLOT_RANGED: return "Ranged";
    default: return NULL;
    }
}


class ItemEnchantmentPlayer : public PlayerScript {

public:
    ItemEnchantmentPlayer() : PlayerScript("ItemEnchantmentPlayer") { }

    void OnLogin(Player* player, bool firstLogin) {
        ItemEnchantment::ApplySpellOnLogin(player);

    }

};


bool canApplyEnchantment(uint32 entry) {
    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(entry);
    return pProto->Spells[1].SpellId > -1;
}

class ItemEnchantmentScript : public ItemScript
{
public:
    ItemEnchantmentScript() : ItemScript("ItemEnchantmentScript") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override // Any hook here
    {
        ClearGossipMenuFor(player); // Clears old options
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (Item* invItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                if(invItem->IsEquipped() && canApplyEnchantment(invItem->GetEntry()))
                    if (const char* slotname = GetSlotName(slot, player->GetSession()))
                        AddGossipItemFor(player, GOSSIP_ICON_TRAINER, slotname, 0, slot);
        }
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, item->GetGUID());
        return true; // Cast the spell on use normally
    }

    void OnGossipSelect(Player* player, Item* item, uint32 /*sender*/, uint32 slot) override
    {
        Item* invItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (invItem) {
            if (ItemEnchantment::SaveItemSpell(player, invItem, ItemEnchantment::GetRandomEnchantmentByClassId(player))) {
                player->DestroyItemCount(item->GetEntry(), 1, true);
                player->GetSession()->SendAreaTriggerMessage("Item successfuly enchanted");
            }
        }
    }
};

void AddSC_ItemEnchantmentScript() // Add to scriptloader normally
{
    new ItemEnchantmentScript();
    new ItemEnchantmentPlayer();
}
