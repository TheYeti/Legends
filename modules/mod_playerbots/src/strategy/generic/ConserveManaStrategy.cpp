/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ConserveManaStrategy.h"

#include "GenericSpellActions.h"
#include "LastSpellCastValue.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

float HealerAutoSaveManaMultiplier::GetValue(Action* action)
{
    uint8 mana = bot->GetPowerPct(Powers::POWER_MANA);
    if (mana > sPlayerbotAIConfig->saveManaThreshold)
        return 1.0f;
    CastHealingSpellAction* healingAction = dynamic_cast<CastHealingSpellAction*>(action);

    if (!healingAction)
        return 1.0f;

    Unit* target = healingAction->GetTarget();
    if (!target)
        return 1.0f;
    bool isTank = target->ToPlayer() ? PlayerBotSpec::IsTank(target->ToPlayer()) : false;
    uint8 health = target->GetHealthPct();
    HealingManaEfficiency manaEfficiency = healingAction->manaEfficiency;
    uint8 estAmount = healingAction->estAmount;
    uint8 lossAmount = 100 - health;
    if (isTank)
    {
        estAmount /= 1.5;  // tanks have more health
        if (health >= sPlayerbotAIConfig->mediumHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::MEDIUM))
            return 0.0f;
        if (health >= sPlayerbotAIConfig->lowHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::LOW))
            return 0.0f;
    }
    else
    {
        if (health >= sPlayerbotAIConfig->mediumHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::MEDIUM))
            return 0.0f;
        if (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::LOW)
            return 0.0f;
    }
    return 1.0f;
}

void HealerAutoSaveManaStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HealerAutoSaveManaMultiplier(botAI));
}