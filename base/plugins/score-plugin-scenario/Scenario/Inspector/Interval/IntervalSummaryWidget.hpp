#pragma once
#include <score/selection/SelectionDispatcher.hpp>
#include <score/widgets/MarginLess.hpp>
#include <QWidget>
#include <QGridLayout>

namespace score
{
class SelectionDispatcher;
struct DocumentContext;
}

namespace Scenario
{
class IntervalModel;
class IntervalSummaryWidget : public QWidget
{
public:
  explicit IntervalSummaryWidget(
      const IntervalModel& object, const score::DocumentContext& doc,
      QWidget* parent = nullptr);

private:
  score::SelectionDispatcher m_selectionDispatcher;
  score::MarginLess<QGridLayout> m_lay;
};
}
