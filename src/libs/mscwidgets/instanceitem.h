/*
   Copyright (C) 2018-2019 European Space Agency - <maxime.perrotin@esa.int>

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

#pragma once

#include "baseitems/common/utils.h"
#include "baseitems/interactiveobject.h"

#include <QPointer>

class QGraphicsRectItem;
class QGraphicsTextItem;

namespace msc {

class MscInstance;
class MscChart;

class ChartViewModel;
class InstanceHeadItem;
class InstanceEndItem;
class MessageItem;
class TextItem;

class InstanceItem : public InteractiveObject
{
    Q_OBJECT

public:
    explicit InstanceItem(MscInstance *instance, ChartViewModel *chartView = nullptr, MscChart *chart = nullptr,
                          QGraphicsItem *parent = nullptr);

    MscInstance *modelItem() const;

    QString name() const;
    QString kind() const;

    void setAxisHeight(qreal height);

    qreal axisHeight() const;
    QLineF axis() const;

    QPainterPath shape() const override;
    void setBoundingRect(const QRectF &geometry);

    static InstanceItem *createDefaultItem(ChartViewModel *model, MscInstance *instance, MscChart *chart,
                                           const QPointF &pos);

    QPair<QPointF, bool> commentPoint() const override;

    void applyCif() override;

    QRectF boundingRect() const override;
    void updateCif() override;

    QPointF avoidOverlaps(InstanceItem *caller, const QPointF &delta, const QRectF &shiftedRect) const;

    void setInitialLocation(const QPointF &requested, const QRectF &chartRect, qreal horSpan);

    QRectF kindBox() const;

    static qreal defaultAxisHeight();

public Q_SLOTS:
    void setName(const QString &name);
    void setDenominatorAndKind(const QString &kind);
    void setExplicitStop(bool exStop);
    void rebuildLayout() override;

protected:
    void onMoveRequested(GripPoint *gp, const QPointF &from, const QPointF &to) override;
    void onResizeRequested(GripPoint *gp, const QPointF &from, const QPointF &to) override;

    void prepareHoverMark() override;
    cif::CifLine::CifType mainCifType() const override;

#ifdef QT_DEBUG
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
#endif

private:
    ChartViewModel *m_model = nullptr;
    msc::MscInstance *m_instance = nullptr;
    QPointer<msc::MscChart> m_chart;
    QGraphicsLineItem *m_axisSymbol = nullptr;
    InstanceHeadItem *m_headSymbol = nullptr;
    InstanceEndItem *m_endSymbol = nullptr;
    qreal m_axisHeight = defaultAxisHeight();

    Q_INVOKABLE void reflectTextLayoutChange();

    void updatePropertyString(const QLatin1String &property, const QString &value);

    QVariantList prepareChangePositionCommand() const;

private Q_SLOTS:
    void onNameEdited(const QString &newName);
    void onKindEdited(const QString &newKind);
    void onManualGeometryChangeFinished(GripPoint::Location pos, const QPointF &from, const QPointF &to);
};

} // namespace msc