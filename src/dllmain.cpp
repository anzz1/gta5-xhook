
#include <windows.h>

extern "C" unsigned __int64 __stdcall GetTickCount64(void);

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "lib/ScriptHookV.lib")

#include "include/main.h"
#include "include/natives.h"
#include "include/types.h"

#define PED_FLAG_CAN_FLY_THRU_WINDSCREEN    32
#define PED_MODEL_FRANKLIN                  0x9B22DBAF
#define VEH_MODEL_CARGOBOB3                 0x53174EEF

static BOOL bRespawn = TRUE;
static ULONGLONG ticks = 0ULL;

void on_update();

void ScriptMain()
{
    bRespawn = TRUE;

    // Enable MP map
    //DLC::ON_ENTER_MP();
    //MISC::SET_INSTANCE_PRIORITY_MODE(1);

    while (TRUE)
    {
        on_update();        
        WAIT(0);
    }
}

extern "C" BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        scriptRegister(hInstance, ScriptMain);
        break;
    case DLL_PROCESS_DETACH:
        scriptUnregister(hInstance);
        break;
    }
    return TRUE;
}

static const char* weaponNames[] = {
    "WEAPON_PISTOL",
    "WEAPON_PISTOL_MK2",
    "WEAPON_COMBATPISTOL",
    "WEAPON_PISTOL50",
    "WEAPON_SNSPISTOL",
    "WEAPON_SNSPISTOL_MK2",
    "WEAPON_HEAVYPISTOL",
    "WEAPON_VINTAGEPISTOL",
    "WEAPON_MARKSMANPISTOL",
    "WEAPON_REVOLVER",
    "WEAPON_REVOLVER_MK2",
    "WEAPON_DOUBLEACTION",
    "WEAPON_APPISTOL",
    "WEAPON_STUNGUN",
    "WEAPON_FLAREGUN",
    "WEAPON_MICROSMG",
    "WEAPON_MACHINEPISTOL",
    "WEAPON_MINISMG",
    "WEAPON_SMG",
    "WEAPON_SMG_MK2",
    "WEAPON_ASSAULTSMG",
    "WEAPON_COMBATPDW",
    "WEAPON_MG",
    "WEAPON_COMBATMG",
    "WEAPON_GUSENBERG",
    "WEAPON_ASSAULTRIFLE",
    "WEAPON_ASSAULTRIFLE_MK2",
    "WEAPON_CARBINERIFLE",
    "WEAPON_CARBINERIFLE_MK2",
    "WEAPON_ADVANCEDRIFLE",
    "WEAPON_SPECIALCARBINE",
    "WEAPON_SPECIALCARBINE_MK2",
    "WEAPON_BULLPUPRIFLE",
    "WEAPON_BULLPUPRIFLE_MK2",
    "WEAPON_COMPACTRIFLE",
    "WEAPON_SNIPERRIFLE",
    "WEAPON_HEAVYSNIPER_MK2",
    "WEAPON_HEAVYSNIPER",
    "WEAPON_MARKSMANRIFLE",
    "WEAPON_MARKSMANRIFLE_MK2",
    "WEAPON_KNIFE",
    "WEAPON_NIGHTSTICK",
    "WEAPON_HAMMER",
    "WEAPON_BAT",
    "WEAPON_CROWBAR",
    "WEAPON_GOLFCLUB",
    "WEAPON_BOTTLE",
    "WEAPON_DAGGER",
    "WEAPON_HATCHET",
    "WEAPON_KNUCKLE",
    "WEAPON_MACHETE",
    "WEAPON_FLASHLIGHT",
    "WEAPON_SWITCHBLADE",
    "WEAPON_BATTLEAXE",
    "WEAPON_POOLCUE",
    "WEAPON_WRENCH",
    "WEAPON_PUMPSHOTGUN",
    "WEAPON_PUMPSHOTGUN_MK2",
    "WEAPON_SAWNOFFSHOTGUN",
    "WEAPON_BULLPUPSHOTGUN",
    "WEAPON_ASSAULTSHOTGUN",
    "WEAPON_HEAVYSHOTGUN",
    "WEAPON_MUSKET",
    "WEAPON_DBSHOTGUN",
    "WEAPON_AUTOSHOTGUN",
    "WEAPON_GRENADELAUNCHER",
    "WEAPON_RPG",
    "WEAPON_MINIGUN",
    "WEAPON_FIREWORK",
    "WEAPON_RAILGUN",
    "WEAPON_HOMINGLAUNCHER",
    "WEAPON_COMPACTLAUNCHER",
    "WEAPON_GRENADE",
    "WEAPON_STICKYBOMB",
    "WEAPON_PROXMINE",
    "WEAPON_PIPEBOMB",
    "WEAPON_SMOKEGRENADE",
    "WEAPON_MOLOTOV",
    "WEAPON_FIREEXTINGUISHER",
    "WEAPON_PETROLCAN",
    "WEAPON_SNOWBALL",
    "WEAPON_FLARE",
    "WEAPON_GADGETPISTOL",
    "WEAPON_MILITARYRIFLE",
    "WEAPON_COMBATSHOTGUN",
    "WEAPON_NAVYREVOLVER",
    "WEAPON_CERAMICPISTOL",
    "WEAPON_STONE_HATCHET",
    "GADGET_PARACHUTE"

    //"WEAPON_RAYPISTOL",
    //"WEAPON_RAYCARBINE",
    //"WEAPON_RAYMINIGUN",
    //"WEAPON_EMPLAUNCHER",
    //"WEAPON_FERTILIZERCAN",

    //"WEAPON_STINGER",          // not working
    //"WEAPON_BZGAS",            // buggy tear gas clone from mission
    //"WEAPON_BALL",             // chops tennisball
    //"WEAPON_TRANQUILIZER",     // not working
    //"WEAPON_DIGISCANNER",      // not working
    //"GADGET_NIGHTVISION",      // not working

    //"WEAPON_COMBATMG_MK2",     // special case (merryweather bug)
};

static char statNames[][30] = {
    "SP0_SPECIAL_ABILITY_UNLOCKED",
    "SP0_STAMINA",
    "SP0_STEALTH_ABILITY",
    "SP0_LUNG_CAPACITY",
    "SP0_FLYING_ABILITY",
    "SP0_SHOOTING_ABILITY",
    "SP0_STRENGTH",
    "SP0_WHEELIE_ABILITY"
};

static char statNames2[][30] = {
    "SP0_SPECIAL_ABILITY_UNLOCKED",
    "SP0_TIME_DRIVING_BICYCLE",      // SP0_STAMINA
    "SP0_KILLS_STEALTH",             // SP0_STEALTH_ABILITY
    "SP0_TIME_UNDERWATER",           // SP0_LUNG_CAPACITY
    "SP0_PLANE_LANDINGS",            // SP0_FLYING_ABILITY
    "SP0_HITS_PEDS_VEHICLES",        // SP0_SHOOTING_ABILITY
    "SP0_UNARMED_HITS",              // SP0_STRENGTH
    "SP0_NUMBER_NEAR_MISS",          // SP0_WHEELIE_ABILITY
};

static int statAmounts[] = {
    100,
    3600000,
    1000,
    3600000,
    100,
    15000,
    10000,
    10000
};

static char cashMoney[] = "SP0_TOTAL_CASH";

__forceinline static void max_stats()
{
    Hash hash;
    int iVar = 0;

    for (int x = 0; x < 3; x++)
    {
        for (int i = 0; i < sizeof(statNames) / sizeof(statNames[0]); i++)
        {
            statNames[i][2] = x + 48;
            hash = GAMEPLAY::GET_HASH_KEY(statNames[i]);
            STATS::STAT_GET_INT(hash, &iVar, -1);
            if (iVar < 100)
            {
                STATS::STAT_SET_INT(hash, 100, TRUE);
                statNames2[i][2] = x + 48;
                hash = GAMEPLAY::GET_HASH_KEY(statNames2[i]);
                STATS::STAT_GET_INT(hash, &iVar, -1);
                if (iVar < statAmounts[i])
                    STATS::STAT_SET_INT(hash, statAmounts[i], TRUE);
            }
        }
    }
}

__forceinline static void set_cash()
{
    Hash hash;
    int cash = 0;
    int mincash = 900000000;

    for (int x = 0; x < 3; x++)
    {
        cashMoney[2] = x + 48;
        hash = GAMEPLAY::GET_HASH_KEY(cashMoney);
        STATS::STAT_GET_INT(hash, &cash, -1);
        if (cash < mincash)
            STATS::STAT_SET_INT(hash, mincash, TRUE);
    }
}

__forceinline static void givemod(Ped playerPed, Hash wpn, char* mod)
{
    Hash comp = GAMEPLAY::GET_HASH_KEY(mod);
    if (!WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(playerPed, wpn, comp))
        WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(playerPed, wpn, comp);
}

__forceinline static void give_weapons(Ped playerPed)
{
    Hash wpn;
    for (int i = 0; i < sizeof(weaponNames) / sizeof(weaponNames[0]); i++)
    {
        wpn = GAMEPLAY::GET_HASH_KEY((char*)weaponNames[i]);
        if (!WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
            WEAPON::GIVE_DELAYED_WEAPON_TO_PED(playerPed, wpn, 1000, 0);
        else if (WEAPON::IS_WEAPON_VALID(wpn))
        {
            int maxAmmo;
            if (WEAPON::GET_MAX_AMMO(playerPed, wpn, &maxAmmo))
            {
                WEAPON::SET_PED_INFINITE_AMMO(playerPed, TRUE, wpn);
            }
        }
    }

    wpn = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL_MK2");
    if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
    {
        givemod(playerPed, wpn, "COMPONENT_AT_PI_SUPP_02");
    }

    wpn = GAMEPLAY::GET_HASH_KEY("WEAPON_SNSPISTOL_MK2");
    if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
    {
        givemod(playerPed, wpn, "COMPONENT_AT_PI_SUPP_02");
    }

    wpn = GAMEPLAY::GET_HASH_KEY("WEAPON_HEAVYSNIPER_MK2");
    if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
    {
        givemod(playerPed, wpn, "COMPONENT_AT_SR_SUPP_03");
    }

    wpn = GAMEPLAY::GET_HASH_KEY("WEAPON_MARKSMANRIFLE_MK2");
    if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
    {
        givemod(playerPed, wpn, "COMPONENT_AT_AR_SUPP");
        givemod(playerPed, wpn, "COMPONENT_AT_AR_AFGRIP_02");
        givemod(playerPed, wpn, "COMPONENT_AT_MRFL_BARREL_02");
    }

    wpn = GAMEPLAY::GET_HASH_KEY("WEAPON_GUSENBERG");
    if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
    {
        givemod(playerPed, wpn, "COMPONENT_GUSENBERG_CLIP_02");
    }

    wpn = GAMEPLAY::GET_HASH_KEY("WEAPON_PUMPSHOTGUN_MK2");
    if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
    {
        givemod(playerPed, wpn, "COMPONENT_AT_AR_FLSH");
    }

    wpn = GAMEPLAY::GET_HASH_KEY("WEAPON_COMBATSHOTGUN");
    if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
    {
        givemod(playerPed, wpn, "COMPONENT_AT_AR_FLSH");
    }

    // merryweather mission fix for combatmg MK2
    Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
    BOOL merryfix = (GAMEPLAY::GET_MISSION_FLAG() && veh != 0 && VEHICLE::IS_VEHICLE_MODEL(veh, VEH_MODEL_CARGOBOB3) && PED::IS_PED_MODEL(playerPed, PED_MODEL_FRANKLIN));
    wpn = GAMEPLAY::GET_HASH_KEY("WEAPON_COMBATMG_MK2");
    if (WEAPON::HAS_PED_GOT_WEAPON(playerPed, wpn, FALSE))
    {
        if (merryfix)
            WEAPON::REMOVE_WEAPON_FROM_PED(playerPed, wpn);
        else
        {
            givemod(playerPed, wpn, "COMPONENT_COMBATMG_MK2_CLIP_02");
            givemod(playerPed, wpn, "COMPONENT_AT_AR_AFGRIP_02");
            givemod(playerPed, wpn, "COMPONENT_AT_MG_BARREL_02");
        }
    }
    else if (!merryfix)
    {
        WEAPON::GIVE_DELAYED_WEAPON_TO_PED(playerPed, wpn, 1000, 0);
    }
}

__forceinline static void returning_dlc_content()
{
    *getGlobalPtr(151761) = 2;

    STATS::STAT_SET_INT(GAMEPLAY::GET_HASH_KEY("MPPLY_UNLOCK_EXCLUS_CONTENT"), -1, TRUE);
    STATS::STAT_SET_INT(GAMEPLAY::GET_HASH_KEY("SP_UNLOCK_EXCLUS_CONTENT"), -1, TRUE);
    STATS::STAT_SET_INT(GAMEPLAY::GET_HASH_KEY("MPPLY_PLAT_UP_LB_CHECK"), -1, TRUE);
    STATS::_UNLOCK_EXCLUS_CONTENT(-1);
}

void on_update()
{    
    Player player = PLAYER::PLAYER_ID();
    Ped playerPed = PLAYER::PLAYER_PED_ID();
    BOOL bPlayerExists = ENTITY::DOES_ENTITY_EXIST(playerPed);

    if (!bPlayerExists)
        return;

    if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
    {
        bRespawn = TRUE;
        return;
    }

    PLAYER::RESET_PLAYER_STAMINA(player);
    PLAYER::SPECIAL_ABILITY_FILL_METER(player, 1);
    PLAYER::SET_SWIM_MULTIPLIER_FOR_PLAYER_DW(player, /*1.49*/ 0x3FC00000);
    PLAYER::SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER_DW(player, /* 1.34 */ 0x3FAAAAAA);

    if (PED::GET_PED_MAX_HEALTH(playerPed) < 300)
        PED::SET_PED_MAX_HEALTH(playerPed, 300);

    if (PED::GET_PED_CONFIG_FLAG(playerPed, PED_FLAG_CAN_FLY_THRU_WINDSCREEN, TRUE))
        PED::SET_PED_CONFIG_FLAG(playerPed, PED_FLAG_CAN_FLY_THRU_WINDSCREEN, FALSE);

    Hash wpn;
    if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &wpn, 1))
    {
        if (WEAPON::IS_WEAPON_VALID(wpn))
        {
            int maxAmmo;
            if (WEAPON::GET_MAX_AMMO(playerPed, wpn, &maxAmmo))
            {
                WEAPON::SET_PED_INFINITE_AMMO(playerPed, TRUE, wpn);
            }
        }
    }

    ULONGLONG t = GetTickCount64();
    if (t >= ticks) {
        ticks = t + 1000;
        DWORD curHealth = ENTITY::GET_ENTITY_HEALTH(playerPed);
        if (curHealth < 300 /* ENTITY::GET_ENTITY_MAX_HEALTH(playerPed) */)
            ENTITY::SET_ENTITY_HEALTH(playerPed, curHealth + 1);

        PLAYER::SET_AUTO_GIVE_PARACHUTE_WHEN_ENTER_PLANE(player, TRUE);

        give_weapons(playerPed);
        set_cash();
        max_stats();
        returning_dlc_content();
    }

    if (bRespawn)
    {
        bRespawn = FALSE;

        PLAYER::SET_PLAYER_MAX_ARMOUR(player, 100);
        PED::SET_PED_ARMOUR(playerPed, /* PLAYER::GET_PLAYER_MAX_ARMOUR(player) */ 100);
        ENTITY::SET_ENTITY_HEALTH(playerPed, /* ENTITY::GET_ENTITY_MAX_HEALTH(playerPed) */ 300);
    }
}

