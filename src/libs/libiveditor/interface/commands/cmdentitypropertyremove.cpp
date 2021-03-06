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

#include "cmdentitypropertyremove.h"

#include "commandids.h"

#include <ivmodel.h>

namespace ive {
namespace cmd {

static inline QVariantHash getCurrentProperties(ivm::IVObject *entity, const QStringList &props)
{
    QVariantHash result;
    for (const QString &prop : props)
        result.insert(prop, entity->entityAttributeValue(prop));
    return result;
}

CmdEntityPropertyRemove::CmdEntityPropertyRemove(ivm::IVObject *entity, const QStringList &props)
    : QUndoCommand()
    , m_entity(entity)
    , m_names(props)
    , m_props(getCurrentProperties(entity, props))
{
    setText(QObject::tr("Remove Property"));
}

void CmdEntityPropertyRemove::redo()
{
    EntityAttributes props = m_entity->entityAttributes();
    for (const QString &name : m_names)
        props.remove(name);
    m_entity->setEntityAttributes(props);
}

void CmdEntityPropertyRemove::undo()
{
    EntityAttributes props = m_entity->entityAttributes();
    for (auto it = m_props.constBegin(); it != m_props.constEnd(); ++it)
        props[it.key()].setValue(it.value());
    m_entity->setEntityAttributes(props);
}

bool CmdEntityPropertyRemove::mergeWith(const QUndoCommand *)
{
    return false;
}

int CmdEntityPropertyRemove::id() const
{
    return RemoveEntityProperty;
}

}
}
