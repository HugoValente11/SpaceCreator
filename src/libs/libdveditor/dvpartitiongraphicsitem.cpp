/*
   Copyright (C) 2021 European Space Agency - <maxime.perrotin@esa.int>

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

#include "dvpartitiongraphicsitem.h"

#include "graphicsviewutils.h"

#include <QPainter>

namespace dve {

DVPartitionGraphicsItem::DVPartitionGraphicsItem(dvm::DVPartition *partition, QGraphicsItem *parent)
    : shared::ui::VERectGraphicsItem(partition, parent)
{
}

dvm::DVPartition *DVPartitionGraphicsItem::entity() const
{
    return m_dataObject.isNull() ? nullptr : m_dataObject->as<dvm::DVPartition *>();
}

QSizeF DVPartitionGraphicsItem::minimalSize() const
{
    return shared::graphicsviewutils::kDefaultGraphicsItemSize;
}

void DVPartitionGraphicsItem::applyColorScheme()
{
    const shared::ColorHandler &h = colorHandler();
    QPen pen = h.pen();
    pen.setCapStyle(Qt::FlatCap);
    pen.setStyle(Qt::SolidLine);
    setPen(pen);
    setBrush(h.brush());
    update();
}

void DVPartitionGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (!entity())
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen());
    painter->setBrush(brush());
    const QRectF br = boundingRect();
    painter->drawRect(br);
    painter->setFont(font());
    painter->drawText(br, Qt::AlignCenter, entity()->titleUI());
    painter->restore();
}

shared::ColorManager::HandledColors DVPartitionGraphicsItem::handledColorType() const
{
    return shared::ColorManager::HandledColors::Partition;
}

} // namespace dve
