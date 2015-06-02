#include "CurvePointView.hpp"
#include <QPainter>
#include <iscore/selection/Selectable.hpp>
#include "CurvePointModel.hpp"
CurvePointView::CurvePointView(
        CurvePointModel* model,
        QGraphicsItem* parent):
    QGraphicsObject{parent},
    m_model{model}
{
    this->setZValue(2);
    connect(&m_model->selection, &Selectable::changed,
            this, &CurvePointView::setSelected);

}

CurvePointModel &CurvePointView::model() const
{
    return *m_model;
}

int CurvePointView::type() const
{
    return QGraphicsItem::UserType + 10;
}

QRectF CurvePointView::boundingRect() const
{
    return {-3, -3, 6, 6};
}

void CurvePointView::paint(
        QPainter *painter,
        const QStyleOptionGraphicsItem *option,
        QWidget *widget)
{
    QPen pen;
    QColor c = m_selected? Qt::yellow : Qt::green;
    pen.setColor(c);
    pen.setWidth(3);
    painter->setPen(pen);
    painter->setBrush(c);

    painter->drawEllipse(QPointF{0., 0.}, 3, 3);
}

void CurvePointView::setSelected(bool selected)
{
    m_selected = selected;
    update();
}

#include <QGraphicsSceneMouseEvent>
void CurvePointView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    emit pressed(event->scenePos());
}

void CurvePointView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    emit moved(event->scenePos());
}

void CurvePointView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    emit released(event->scenePos());
}
