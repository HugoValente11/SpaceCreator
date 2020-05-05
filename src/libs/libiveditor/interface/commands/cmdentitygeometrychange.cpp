/*
  Copyright (C) 2019 European Space Agency - <maxime.perrotin@esa.int>

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

#include "cmdentitygeometrychange.h"

#include "commandsstack.h"
#include "commandids.h"

#include <QtDebug>
#include <baseitems/common/utils.h>
#include <aadlobjectsmodel.h>

namespace aadlinterface {
namespace cmd {

CmdEntityGeometryChange::CmdEntityGeometryChange(const QList<QPair<aadl::AADLObject *, QVector<QPointF>>> &objectsData, const QString &title)
    : QUndoCommand(title.isEmpty() ? QObject::tr("Change item(s) geometry/position") : title)
    , m_internalData(objectsData)
    , m_data(convertData(m_internalData))
{
}

CmdEntityGeometryChange::~CmdEntityGeometryChange()
{
    qDeleteAll(m_mergedCmds);
}

void CmdEntityGeometryChange::redo()
{
    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it) {
        if (!it->entity)
            continue;

        it->entity->setCoordinates(it->newCoordinates);
    }

    for (auto it = m_mergedCmds.cbegin(); it != m_mergedCmds.cend(); ++it)
        (*it)->redo();
}

void CmdEntityGeometryChange::undo()
{
    for (auto it = m_mergedCmds.crbegin(); it != m_mergedCmds.crend(); ++it)
        (*it)->undo();

    for (auto it = m_data.cbegin(); it != m_data.cend(); ++it) {
        if (!it->entity)
            continue;

        it->entity->setCoordinates(it->prevCoordinates);
    }
}

int CmdEntityGeometryChange::id() const
{
    return ChangeEntityGeometry;
}

void CmdEntityGeometryChange::mergeCommand(QUndoCommand *command)
{
    if (command->id() != AutoLayoutEntity)
        return;

    m_mergedCmds.append(command);
}

static inline int parentLevel(aadl::AADLObject *object)
{
    int idx = 0;
    while (auto parentObject = object->parentObject()) {
        ++idx;
        object = parentObject;
    }
    return idx;
}

QList<CmdEntityGeometryChange::ObjectData>
CmdEntityGeometryChange::convertData(const QList<QPair<aadl::AADLObject *, QVector<QPointF>>> &objectsData)
{
    QList<ObjectData> result;
    for (const auto &objectData : objectsData)
        result.append({ objectData.first, objectData.first->coordinates(), coordinates(objectData.second) });

    std::stable_sort(result.begin(), result.end(), [](const ObjectData &data1, const ObjectData &data2) {
        if (data1.entity->aadlType() == data2.entity->aadlType())
            return parentLevel(data1.entity) < parentLevel(data2.entity);

        return data1.entity->aadlType() < data2.entity->aadlType();
    });

    return result;
}

void CmdEntityGeometryChange::prepareData(const QList<QPair<aadl::AADLObject *, QVector<QPointF>>> &objectsData)
{
    m_data = convertData(objectsData);
}

}
}