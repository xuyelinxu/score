// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <score/document/DocumentInterface.hpp>

#include "InspectorWidgetBase.hpp"
#include "InspectorWidgetList.hpp"
#include <Inspector/InspectorWidgetFactoryInterface.hpp>
#include <score/model/IdentifiedObjectAbstract.hpp>

class QWidget;

namespace Inspector
{
QList<QWidget*> InspectorWidgetList::make(
    const score::DocumentContext& doc,
    QList<const IdentifiedObjectAbstract*>
        models,
    QWidget* parent) const
{
  QList<QWidget*> widgs;
  for (const InspectorWidgetFactory& factory : *this)
  {
    // TODO QVector or better, array_view
    QList<const QObject*> objects;
    objects.reserve(models.size());
    for (auto elt : models)
    {
      objects.push_back(elt);
    }

    if (factory.matches(objects))
    {
      auto widg = factory.make(objects, doc, parent);
      if (widg)
        widgs.push_back(widg);
    }
  }

  return widgs;
}
}
