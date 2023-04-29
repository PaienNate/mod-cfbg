/*
 * Copyright (С) since 2019+ AzerothCore <www.azerothcore.org>
 * Licence MIT https://opensource.org/MIT
 */

#include "Chat.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "CFBG.h"

using namespace Acore::ChatCommands;

class cfbg_commandscript : public CommandScript
{
public:
    cfbg_commandscript() : CommandScript("cfbg_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable cfbgCommands =
        {
            { "race", HandleCFBGChooseRace, SEC_PLAYER, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "cfbg",  cfbgCommands },
        };

        return commandTable;
    }

    static bool HandleCFBGChooseRace(ChatHandler* handler, std::string raceInput)
    {
        Player* player = handler->GetPlayer();

        uint8 raceId = 0;

        if (sCFBG->RandomizeRaces())
        {
            handler->SendSysMessage("种族选择目前被禁用。");
            handler->SetSentErrorMessage(true);
            return true;
        }

        for (auto const& raceVariable : *sCFBG->GetRaceInfo())
        {
            if (raceInput == raceVariable.RaceName)
            {
                if (player->GetTeamId() == raceVariable.TeamId)
                {
                    raceId = raceVariable.RaceId;
                }
                else
                {
                    handler->SendSysMessage("种族不适用于您的阵营。");
                    handler->SetSentErrorMessage(true);
                    return true;
                }
                
                if (!IsRaceValidForClass(player, raceId))
                {
                    handler->SendSysMessage("种族不适用于您的职业。");
                    handler->SetSentErrorMessage(true);
                    return true;
                }

                if (raceId == RACE_NIGHTELF)
                {
                    handler->SendSysMessage("暗夜精灵模型不可用，因为女性模型丢失，男性模型导致客户端崩溃。");
                    handler->SetSentErrorMessage(true);
                    return true;
                }

                if (player->getGender() == GENDER_FEMALE && (raceId == RACE_TROLL || raceId == RACE_DWARF))
                {
                    handler->SendSysMessage("女性模型不适用于以下种族：巨魔、矮人。");
                    handler->SetSentErrorMessage(true);
                    return true;
                }
            }
        }

        player->UpdatePlayerSetting("mod-cfbg", SETTING_CFBG_RACE, raceId);

        if (!raceId)
        {
            handler->SendSysMessage("种族不可用. 混排战场种族选择设置为随机. 当你进入对方队伍的战场时，将随机为你选择一个种族。");
        }
        else
        {
            handler->PSendSysMessage("混排战场随机选择的种族为： %s", raceInput);
        }

        return true;
    }

    static bool IsRaceValidForClass(Player* player, uint8 fakeRace)
    {
        auto raceData{ *sCFBG->GetRaceData() };

        std::vector<uint8> availableRacesForClass = player->GetTeamId() == TEAM_HORDE ?
            raceData[player->getClass()].availableRacesA : raceData[player->getClass()].availableRacesH;

        for (auto const& races : availableRacesForClass)
        {
            if (races == fakeRace)
            {
                return true;
            }
        }

        return false;
    }
};

void AddSC_cfbg_commandscript()
{
    new cfbg_commandscript();
}
