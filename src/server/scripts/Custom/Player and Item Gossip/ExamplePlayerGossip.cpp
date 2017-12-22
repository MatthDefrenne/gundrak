#include "Define.h"
#include "GossipDef.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

#define MENU_ID 123 // Our menuID used to match the sent menu to select hook (playerscript)

class example_PlayerGossip : public PlayerScript
{
public: 
    example_PlayerGossip() : PlayerScript("example_PlayerGossip") {}


};

void AddSC_example_PlayerGossip() // Add to scriptloader normally
{
    new example_PlayerGossip();
}
