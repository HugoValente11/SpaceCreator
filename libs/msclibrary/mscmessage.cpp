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

#include "mscmessage.h"
#include "mscinstance.h"

namespace msc {

MscMessage::MscMessage(QObject *parent)
    : MscEntity(parent)
{
}

MscMessage::MscMessage(const QString &name, QObject *parent)
    : MscEntity(name, parent)
{
}

MscInstance *MscMessage::sourceInstance() const
{
    return m_source;
}

void MscMessage::setSourceInstance(MscInstance *source)
{
    if (source == m_source) {
        return;
    }

    m_source = source;
    Q_EMIT sourceChanged(m_source);
}

MscInstance *MscMessage::targetInstance() const
{
    return m_target;
}

void MscMessage::setTargetInstance(MscInstance *target)
{
    if (target == m_target) {
        return;
    }

    m_target = target;
    Q_EMIT targetChanged(m_target);
}

MscEntity::EntityType MscMessage::elementType() const
{
    return MscEntity::EntityType::Message;
}

} // namespace msc
