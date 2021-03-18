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

#include "eventitem.h"

#include "mscinstanceevent.h"

namespace msc {

EventItem::EventItem(msc::MscInstanceEvent *entity, msc::ChartLayoutManager *chartLayoutManager, QGraphicsItem *parent)
    : InteractiveObject(entity, chartLayoutManager, parent)
{
}

/*!
   Sets, where the item is horizontally centered
 */
void EventItem::setTargetHCenter(qreal x)
{
    m_targetHCenter = x;
    centerOnTargetH();
}

/*!
   Does set the item to be horizontally centered. The position needs to be set via setTargetHCenter before. \sa
   setTargetHCenter
 */
void EventItem::centerOnTargetH()
{
    if (m_targetHCenter > -9.8e12) {
        setX(m_targetHCenter - boundingRect().width() / 2);
    }
}

} // namespace msc