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

#include "cmdmyfunctionitemcreate.h"

#include "baseitems/common/ivutils.h"
#include "commandids.h"
#include "ivmyfunction.h"
#include "ivfunctiontype.h"
#include "ivmodel.h"

#include <QRectF>

namespace ive {
namespace cmd {

CmdMyFunctionItemCreate::CmdMyFunctionItemCreate(ivm::IVModel *model, ivm::IVFunctionType *parent, const QRectF &geometry)
    : shared::cmd::CmdEntityGeometryChange({}, QObject::tr("Create My Function"))
    , m_model(model)
    , m_parent(parent)
    , m_entity(new ivm::IVMyFunction(
              QString(), m_parent ? qobject_cast<QObject *>(m_parent) : qobject_cast<QObject *>(m_model)))
{
    prepareData({ qMakePair(m_entity, QVector<QPointF> { geometry.topLeft(), geometry.bottomRight() }) });
}

CmdMyFunctionItemCreate::~CmdMyFunctionItemCreate()
{
    if (m_entity && !m_entity->parent()) {
        delete m_entity;
    }
}

void CmdMyFunctionItemCreate::redo()
{
    shared::cmd::CmdEntityGeometryChange::redo();

    if (m_parent)
        m_parent->addChild(m_entity);
    if (m_model)
        m_model->addObject(m_entity);
}

void CmdMyFunctionItemCreate::undo()
{
    shared::cmd::CmdEntityGeometryChange::undo();

    if (m_model)
        m_model->removeObject(m_entity);
    if (m_parent)
        m_parent->removeChild(m_entity);
}

int CmdMyFunctionItemCreate::id() const
{
    return CreateMyFunctionEntity;
}

}
}
