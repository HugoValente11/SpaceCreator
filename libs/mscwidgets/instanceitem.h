/*
   Copyright (C) 2018 European Space Agency - <maxime.perrotin@esa.int>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#ifndef INSTANCEITEM_H
#define INSTANCEITEM_H

#include "baseitems/interactiveobject.h"

namespace msc {
class MscInstance;
class MessageItem;
class TextItem;
}

class QGraphicsRectItem;
class QGraphicsTextItem;

namespace msc {

class InstanceItem : public InteractiveObject
{
    Q_OBJECT

public:
    explicit InstanceItem(MscInstance *instance, QGraphicsItem *parent = nullptr);

    MscInstance *modelItem() const;

    QString name() const;
    QString kind() const;

    void setAxisHeight(double height);
    QLineF axis() const;

    void updateLayout();

    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setBoundingRect(const QRectF &geometry);

    static InstanceItem *createDefaultItem(MscInstance *instance, const QPointF &pos);

public Q_SLOTS:
    void setName(const QString &name);
    void setKind(const QString &kind);
    void buildLayout();
    void rebuildLayout();

protected:
    void onMoveRequested(GripPoint *gp, const QPointF &from, const QPointF &to) override;
    void onResizeRequested(GripPoint *gp, const QPointF &from, const QPointF &to) override;

    void prepareHoverMark() override;

private:
    msc::MscInstance *m_instance = nullptr;
    QGraphicsRectItem *m_headSymbol = nullptr;
    TextItem *m_nameItem = nullptr;
    TextItem *m_kindItem = nullptr;
    QGraphicsLineItem *m_axisSymbol = nullptr;
    QGraphicsRectItem *m_endSymbol = nullptr;
    qreal m_axisHeight = 150.0;
    bool m_layoutDirty = false;

    static QLinearGradient createGradientFor(const QGraphicsItem *target);
};

} // namespace msc

#endif // INSTANCEITEM_H
