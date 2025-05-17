#ifndef _PLAYERBOT_ENEMYPLAYERVALUE_H
#define _PLAYERBOT_ENEMYPLAYERVALUE_H

#include "PlayerbotAIConfig.h"
#include "PossibleTargetsValue.h"
#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class NearestEnemyPlayersValue : public PossibleTargetsValue
{
public:
    NearestEnemyPlayersValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->grindDistance)
        : PossibleTargetsValue(botAI, "nearest enemy players", range)
    {
    }

public:
    bool AcceptUnit(Unit* unit) override;
};

class EnemyPlayerValue : public UnitCalculatedValue
{
public:
    EnemyPlayerValue(PlayerbotAI* botAI, std::string const name = "enemy player")
        : UnitCalculatedValue(botAI, name, 1 * 1000)
    {
    }

    Unit* Calculate() override;

private:
    float GetMaxAttackDistance();
};

#endif
