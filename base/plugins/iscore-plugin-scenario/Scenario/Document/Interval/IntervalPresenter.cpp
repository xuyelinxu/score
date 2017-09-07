// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <iscore/tools/std/Optional.hpp>
#include <qnamespace.h>

#include "IntervalHeader.hpp"
#include "IntervalPresenter.hpp"
#include "IntervalView.hpp"
#include <Process/TimeValue.hpp>
#include <Process/ZoomHelper.hpp>
#include <Scenario/Document/Interval/IntervalDurations.hpp>
#include <Scenario/Document/Interval/Temporal/Braces/LeftBrace.hpp>
#include <Scenario/Document/ModelConsistency.hpp>
#include <iscore/selection/Selectable.hpp>

#include <iscore/model/EntityMap.hpp>
#include <iscore/model/Identifier.hpp>
#include <iscore/tools/Todo.hpp>

class QObject;
namespace Scenario
{
IntervalPresenter::IntervalPresenter(
    const IntervalModel& model,
    IntervalView* view,
    IntervalHeader* header,
    const Process::ProcessPresenterContext& ctx,
    QObject* parent)
    : QObject{parent}
    , m_model{model}
    , m_view{view}
    , m_header{header}
    , m_context{ctx}
{
  auto& interval = m_model;
  m_header->setParentItem(m_view);
  m_header->setIntervalView(m_view);
  m_header->hide();
  //m_header->setPos(0, -m_header->headerHeight());

  con(interval.duration, &IntervalDurations::minNullChanged, this,
      [&](bool b) { updateBraces(); });
  con(interval.duration, &IntervalDurations::minDurationChanged, this,
      [&](const TimeVal& val) {
        on_minDurationChanged(val);
        updateChildren();
      });
  con(interval.duration, &IntervalDurations::maxDurationChanged, this,
      [&](const TimeVal& val) {
        on_maxDurationChanged(val);
        updateChildren();
      });

  con(interval, &IntervalModel::heightPercentageChanged, this,
      &IntervalPresenter::heightPercentageChanged);
  con(interval, &IntervalModel::executionStarted,
      this, [=] { m_view->setExecuting(true); });
  con(interval, &IntervalModel::executionStopped,
      this, [=] { m_view->setExecuting(false); });

  con(interval.consistency, &ModelConsistency::validChanged, m_view,
      &IntervalView::setValid);
  con(interval.consistency, &ModelConsistency::warningChanged, m_view,
      &IntervalView::setWarning);
}

IntervalPresenter::~IntervalPresenter()
{
}

void IntervalPresenter::updateScaling()
{
  const auto& cm = m_model;

  on_minDurationChanged(cm.duration.minDuration());
  on_maxDurationChanged(cm.duration.maxDuration());
  on_playPercentageChanged(cm.duration.playPercentage());

  updateChildren();
}

void IntervalPresenter::on_zoomRatioChanged(ZoomRatio val)
{
  m_zoomRatio = val;
  updateScaling();
}

const Id<IntervalModel>& IntervalPresenter::id() const
{
  return model().id();
}

void IntervalPresenter::on_minDurationChanged(const TimeVal& min)
{
  auto x = min.toPixels(m_zoomRatio);
  m_view->setMinWidth(x);
  m_view->leftBrace().setX(x);
  updateBraces();
}

void IntervalPresenter::on_maxDurationChanged(const TimeVal& max)
{
  auto x = max.toPixels(m_zoomRatio);
  m_view->setMaxWidth(max.isInfinite(), max.isInfinite() ? -1 : x);
  m_view->rightBrace().setX(x + 2);
  updateBraces();
}

double IntervalPresenter::on_playPercentageChanged(double t)
{
  if (!m_view->infinite())
    return m_view->setPlayWidth(m_view->maxWidth() * t);
  else
    return m_view->setPlayWidth(m_view->defaultWidth() * t);
}

void IntervalPresenter::selectedSlot(int) const {
}

void IntervalPresenter::updateChildren()
{
  emit askUpdate();

  m_view->update();
  m_header->update();
}

bool IntervalPresenter::isSelected() const
{
  return m_model.selection.get();
}

const IntervalModel& IntervalPresenter::model() const
{
  return m_model;
}

IntervalView* IntervalPresenter::view() const
{
  return m_view;
}

void IntervalPresenter::updateBraces()
{
  const auto& dur = m_model.duration;
  auto& lb = m_view->leftBrace();
  auto& rb = m_view->rightBrace();
  const bool rigid = dur.isRigid();

  lb.setVisible(!dur.isMinNull() && !rigid);
  rb.setVisible(!dur.isMaxInfinite() && !rigid);
}

}