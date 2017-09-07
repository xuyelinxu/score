// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <Scenario/Document/Interval/IntervalModel.hpp>

#include "SetRigidity.hpp"
#include <Process/TimeValue.hpp>
#include <Scenario/Document/Interval/IntervalDurations.hpp>
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/model/path/Path.hpp>
#include <iscore/model/path/PathSerialization.hpp>

namespace Scenario
{
namespace Command
{
// Rigid interval == end TimeSync has a trigger

SetRigidity::SetRigidity(const IntervalModel& interval, bool rigid)
    : m_path{interval}, m_rigidity{rigid}
{
  // TODO make a class that embodies the logic for the relationship between
  // rigidity and min/max.
  // Also, min/max are indicative if rigid, they can still be set but won't be
  // used.
  m_oldMinDuration = interval.duration.minDuration();
  m_oldMaxDuration = interval.duration.maxDuration();
  m_oldIsNull = interval.duration.isMinNull();
  m_oldIsInfinite = interval.duration.isMaxInfinite();
}

void SetRigidity::undo(const iscore::DocumentContext& ctx) const
{
  auto& interval = m_path.find(ctx);
  interval.duration.setRigid(!m_rigidity);

  interval.duration.setMinNull(m_oldIsNull);
  interval.duration.setMaxInfinite(m_oldIsInfinite);
  interval.duration.setMinDuration(m_oldMinDuration);
  interval.duration.setMaxDuration(m_oldMaxDuration);
}

void SetRigidity::redo(const iscore::DocumentContext& ctx) const
{
  auto& interval = m_path.find(ctx);
  interval.duration.setRigid(m_rigidity);
  auto dur = interval.duration.defaultDuration();

  if (m_rigidity)
  {
    interval.duration.setMinNull(false);
    interval.duration.setMaxInfinite(false);
    interval.duration.setMinDuration(dur);
    interval.duration.setMaxDuration(dur);
  }
  else
  {
    interval.duration.setMinNull(true);
    interval.duration.setMaxInfinite(true);
  }
}

void SetRigidity::serializeImpl(DataStreamInput& s) const
{
  s << m_path << m_oldMinDuration << m_oldMaxDuration << m_rigidity
    << m_oldIsNull << m_oldIsInfinite;
}

void SetRigidity::deserializeImpl(DataStreamOutput& s)
{
  s >> m_path >> m_oldMinDuration >> m_oldMaxDuration >> m_rigidity
      >> m_oldIsNull >> m_oldIsInfinite;
}
}
}