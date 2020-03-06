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

#pragma once

#include "app/common.h"
#include "cmdentitygeometrychange.h"

#include <QPointer>
#include <QRect>
#include <QVector>

namespace taste3 {
namespace aadl {
class AADLObjectIface;
class AADLObjectConnection;
class AADLObjectFunction;
class AADLObjectsModel;
namespace cmd {

class CmdConnectionItemCreate : public CmdEntityGeometryChange
{
public:
    explicit CmdConnectionItemCreate(taste3::aadl::AADLObjectsModel *model, taste3::aadl::AADLObjectFunction *parent,
                                     const common::Id sourceIfaceId, const common::Id &targetIfaceId,
                                     const QVector<QPointF> &points);

    void redo() override;
    void undo() override;
    int id() const override;

private:
    QPointer<AADLObjectsModel> m_model;
    QPointer<AADLObjectConnection> m_entity;
};

} // namespace cmd
} // namespace aadl
} // namespace taste3
