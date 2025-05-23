#include "FleeStrategy.h"

#include "Playerbots.h"

void FleeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("panic", NextAction::array(0, new NextAction("flee", ACTION_EMERGENCY + 9), nullptr)));
    triggers.push_back(
        new TriggerNode("outnumbered", NextAction::array(0, new NextAction("flee", ACTION_EMERGENCY + 9), nullptr)));
    triggers.push_back(
        new TriggerNode("critical health", NextAction::array(0, new NextAction("flee", ACTION_MEDIUM_HEAL), nullptr)));
}

void FleeFromAddsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("has nearest adds", NextAction::array(0, new NextAction("runaway", 50.0f), nullptr)));
}
