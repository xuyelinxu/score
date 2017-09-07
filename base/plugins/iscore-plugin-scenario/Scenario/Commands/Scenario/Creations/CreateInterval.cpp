// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Process/Algorithms/StandardCreationPolicy.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <Scenario/Process/Algorithms/VerticalMovePolicy.hpp>
#include <iscore/tools/RandomNameProvider.hpp>

#include <QDataStream>
#include <QList>
#include <QVector>
#include <QtGlobal>
#include <algorithm>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/multi_index/detail/hash_index_iterator.hpp>
#include <iscore/tools/IdentifierGeneration.hpp>
#include <vector>

#include "CreateInterval.hpp"
#include <Process/Process.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <iscore/model/ModelMetadata.hpp>
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/model/EntityMap.hpp>
#include <iscore/model/path/Path.hpp>
#include <iscore/model/path/PathSerialization.hpp>
#include <iscore/model/path/ObjectPath.hpp>
#include <iscore/model/Identifier.hpp>

namespace Scenario
{
namespace Command
{
CreateInterval::CreateInterval(
    const Scenario::ProcessModel& scenar,
    Id<StateModel> startState,
    Id<StateModel> endState)
    : m_path{scenar}
    , m_createdName{RandomNameProvider::generateRandomName()}
    , m_startStateId{std::move(startState)}
    , m_endStateId{std::move(endState)}
{
    auto ss = scenar.states.find(m_startStateId);
    if(ss != scenar.states.end())
        m_startStatePos = ss->heightPercentage();
    auto es = scenar.states.find(m_startStateId);
    if(es != scenar.states.end())
        m_endStatePos = es->heightPercentage();

  m_createdIntervalId = getStrongId(scenar.intervals);
}

void CreateInterval::undo(const iscore::DocumentContext& ctx) const
{
  auto& scenar = m_path.find(ctx);

  ScenarioCreate<IntervalModel>::undo(m_createdIntervalId, scenar);
  if(m_startStatePos != -1)
  {
      auto& sst = scenar.states.at(m_startStateId);
      if(sst.previousInterval())
          updateIntervalVerticalPos(m_startStatePos, *sst.previousInterval(), scenar);
      else
          sst.setHeightPercentage(m_startStatePos);
  }
}

void CreateInterval::redo(const iscore::DocumentContext& ctx) const
{
  auto& scenar = m_path.find(ctx);
  auto& sst = scenar.states.at(m_startStateId);
  auto& est = scenar.states.at(m_endStateId);

  ScenarioCreate<IntervalModel>::redo(
      m_createdIntervalId,
      sst,
      est,
      sst.heightPercentage(),
      scenar);

  scenar.intervals.at(m_createdIntervalId)
      .metadata()
      .setName(m_createdName);

  updateIntervalVerticalPos(est.heightPercentage(), m_createdIntervalId, scenar);
}

void CreateInterval::serializeImpl(DataStreamInput& s) const
{
  s << m_path << m_createdName << m_createdIntervalId << m_startStateId
    << m_endStateId << m_startStatePos << m_endStatePos;
}

void CreateInterval::deserializeImpl(DataStreamOutput& s)
{
  s >> m_path >> m_createdName >> m_createdIntervalId >> m_startStateId
      >> m_endStateId >> m_startStatePos >> m_endStatePos;
}
}
}