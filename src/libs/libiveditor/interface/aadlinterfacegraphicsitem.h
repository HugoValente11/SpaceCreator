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
  along with this program. If not, see
  <https://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#pragma once

#include "ivobject.h"
#include "ivinterface.h"
#include "baseitems/interactiveobject.h"

#include <QPointer>

class tst_PositionLookupHelper;

namespace ive {
class AADLConnectionGraphicsItem;
class AADLFunctionGraphicsItem;
class AADLFunctionTypeGraphicsItem;

class AADLInterfaceGraphicsItem : public InteractiveObject
{
    Q_OBJECT
public:
    explicit AADLInterfaceGraphicsItem(ivm::IVInterface *entity, QGraphicsItem *parent = nullptr);
    enum
    {
        Type = UserType + static_cast<int>(ivm::IVObject::Type::RequiredInterface) // provided?
    };

    ivm::IVInterface *entity() const override;

    void init() override;
    int type() const override { return Type; }
    void addConnection(AADLConnectionGraphicsItem *item);
    void removeConnection(AADLConnectionGraphicsItem *item);
    QList<QPointer<AADLConnectionGraphicsItem>> connectionItems() const;

    ive::AADLFunctionTypeGraphicsItem *targetItem() const;
    void setTargetItem(QGraphicsItem *item, const QPointF &globalPos);

    void setInterfaceName(const QString &name);

    QPointF connectionEndPoint(const bool nestedConnection) const;
    QPointF connectionEndPoint(AADLConnectionGraphicsItem *connection = nullptr) const;
    QPainterPath ifaceShape() const;

    QPainterPath shape() const override;

    void updateFromEntity() override;
    QList<QPair<ivm::IVObject *, QVector<QPointF>>> prepareChangeCoordinatesCommandParams() const override;

    QString prepareTooltip() const override;
    void layout();

    qreal maxWidth() const;

public Q_SLOTS:
    void updateLabel();

protected:
    void rebuildLayout() override;
    void onSelectionChanged(bool isSelected) override;

    void onManualMoveProgress(shared::ui::GripPoint *grip, const QPointF &from, const QPointF &to) override;
    void onManualMoveFinish(shared::ui::GripPoint *grip, const QPointF &from, const QPointF &to) override;

    void updateInternalItems(Qt::Alignment alignment);
    virtual shared::ColorManager::HandledColors handledColorType() const override;
    void adjustItem();
    qreal typeIconHeight() const;
    qreal baseLength() const;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

protected Q_SLOTS:
    virtual void applyColorScheme() override;
    void updateKind();
    void updateIface();
    void onAttrOrPropChanged(ivm::meta::Props::Token t);

    virtual QPainterPath ifacePath() const;
    virtual QPainterPath typePath() const;

private:
    QTransform typeTransform(Qt::Alignment alignment) const;
    QTransform ifaceTransform(Qt::Alignment alignment) const;
    QPainterPath itemPath(Qt::Alignment alignment) const;

private:
    friend tst_PositionLookupHelper;

    QGraphicsPathItem *m_type = nullptr;
    QGraphicsPathItem *m_iface = nullptr;
    QGraphicsTextItem *m_text = nullptr;
    QList<QPointer<AADLConnectionGraphicsItem>> m_connections;
    QPainterPath m_shape;

    QString ifaceLabel() const;

    QPainterPath composeShape() const;
};

}
