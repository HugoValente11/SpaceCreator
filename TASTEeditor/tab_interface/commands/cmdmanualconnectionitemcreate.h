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

#include <QPointer>
#include <QRect>
#include <QUndoCommand>
#include <QVector>

namespace taste3 {
namespace aadl {
class AADLObjectIfaceRequired;
class AADLObjectIfaceProvided;
class AADLObjectConnection;
class AADLObjectFunction;
class AADLObjectsModel;
namespace cmd {

class CmdManualConnectionItemCreate : public QUndoCommand
{
public:
    explicit CmdManualConnectionItemCreate(AADLObjectsModel *model, taste3::aadl::AADLObjectFunction *startFunction,
                                           taste3::aadl::AADLObjectFunction *endFunction,
                                           AADLObjectIfaceProvided *providedIface,
                                           AADLObjectIfaceRequired *requiredIface, const QVector<QPointF> &points);

    void redo() override;
    void undo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override;

private:
    QPointer<AADLObjectsModel> m_model;
    QVector<qint32> m_coordinates;
    QPointer<AADLObjectFunction> m_startFunction;
    QPointer<AADLObjectFunction> m_endFunction;
    QPointer<AADLObjectIfaceProvided> m_providedIface;
    QPointer<AADLObjectIfaceRequired> m_requiredIface;
    QPointer<AADLObjectConnection> m_entity;
};

} // namespace cmd
} // namespace aadl
} // namespace taste3
