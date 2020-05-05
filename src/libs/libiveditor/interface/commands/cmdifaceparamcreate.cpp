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

#include "cmdifaceparamcreate.h"

#include "commandids.h"

#include <aadlobjectsmodel.h>

namespace aadlinterface {
namespace cmd {

CmdIfaceParamCreate::CmdIfaceParamCreate(aadl::AADLObject *entity, const aadl::IfaceParameter &param)
    : CmdIfaceParamBase(entity ? entity->as<aadl::AADLObjectIface *>() : nullptr)
    , m_targetParams({ param })
    , m_sourceParams(m_iface ? m_iface->params() : QVector<aadl::IfaceParameter>())
{
    setText(QObject::tr("Create Iface Parameter"));
}

void CmdIfaceParamCreate::redo()
{
    if (!m_iface)
        return;

    QVector<aadl::IfaceParameter> currParams = m_iface->params();
    currParams.append(m_targetParams);
    m_iface->setParams(currParams);

    CmdIfaceParamBase::redo();
}

void CmdIfaceParamCreate::undo()
{
    if (!m_iface)
        return;

    m_iface->setParams(m_sourceParams);

    CmdIfaceParamBase::undo();
}

int CmdIfaceParamCreate::id() const
{
    return CreateIfaceParam;
}

}
}