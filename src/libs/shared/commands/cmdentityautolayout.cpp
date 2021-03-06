/*
  Copyright (C) 2019-2021 European Space Agency - <maxime.perrotin@esa.int>

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

#include "cmdentityautolayout.h"

#include "commandids.h"

namespace shared {
namespace cmd {

CmdEntityAutoLayout::CmdEntityAutoLayout(const QList<QPair<shared::VEObject *, QVector<QPointF>>> &objectsData)
    : CmdEntityGeometryChange(objectsData, QObject::tr("Auto layout items"))
{
}

int CmdEntityAutoLayout::id() const
{
    return AutoLayoutEntity;
}

}
}
