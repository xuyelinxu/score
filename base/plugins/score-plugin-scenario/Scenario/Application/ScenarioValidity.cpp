// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "ScenarioValidity.hpp"

namespace Scenario
{

ScenarioValidityChecker::~ScenarioValidityChecker()
{
}

bool ScenarioValidityChecker::validate(const score::DocumentContext& ctx)
{
  auto scenars = ctx.document.model().findChildren<Scenario::ProcessModel*>();
  for (auto scenar : scenars)
  {
    checkValidity(*scenar);
  }

  return true;
}

void ScenarioValidityChecker::checkValidity(const ProcessModel& scenar)
{
#if defined(SCORE_DEBUG)
  for (const IntervalModel& interval : scenar.intervals)
  {
    auto ss = scenar.findState(interval.startState());
    SCORE_ASSERT(ss);
    auto es = scenar.findState(interval.endState());
    SCORE_ASSERT(es);

    SCORE_ASSERT(ss->nextInterval() == interval.id());
    SCORE_ASSERT(es->previousInterval() == interval.id());
  }

  for (const StateModel& state : scenar.states)
  {
    auto ev = scenar.findEvent(state.eventId());
    SCORE_ASSERT(ev);
    SCORE_ASSERT(ev->states().contains(state.id()));

    if (state.previousInterval())
    {
      auto cst = scenar.findInterval(*state.previousInterval());
      SCORE_ASSERT(cst);
      SCORE_ASSERT(cst->endState() == state.id());
    }

    if (state.nextInterval())
    {
      auto cst = scenar.findInterval(*state.nextInterval());
      SCORE_ASSERT(cst);
      SCORE_ASSERT(cst->startState() == state.id());
    }

    auto num = ossia::count_if(scenar.events, [&] (auto& ev) {
        return ossia::contains(ev.states(), state.id());
    });
    SCORE_ASSERT(num == 1);
  }

  for (const EventModel& event : scenar.events)
  {
    auto tn = scenar.findTimeSync(event.timeSync());
    SCORE_ASSERT(tn);
    SCORE_ASSERT(tn->events().contains(event.id()));

    // SCORE_ASSERT(!event.states().empty());
    for (auto& state : event.states())
    {
      auto st = scenar.findState(state);
      SCORE_ASSERT(st);
      /*
      if(st->eventId() != event.id())
      {
          st->setEventId(event.id());
      }*/
      SCORE_ASSERT(st->eventId() == event.id());
    }

    auto num = ossia::count_if(scenar.timeSyncs, [&] (auto& ev) {
        return ossia::contains(ev.events(), event.id());
    });
    SCORE_ASSERT(num == 1);
  }

  for (const TimeSyncModel& tn : scenar.timeSyncs)
  {
    SCORE_ASSERT(!tn.events().empty());
    for (auto& event : tn.events())
    {
      auto ev = scenar.findEvent(event);
      SCORE_ASSERT(ev);
      SCORE_ASSERT(ev->timeSync() == tn.id());
    }
  }
#endif
}
}
