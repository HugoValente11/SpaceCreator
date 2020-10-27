/*
  Copyright (C) 2020 European Space Agency - <maxime.perrotin@esa.int>

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

#include <QObject>
#include <QUndoCommand>

namespace aadlinterface {
namespace cmd {

class UndoCommand : public QObject, public QUndoCommand
{
    Q_OBJECT
public:
    explicit UndoCommand(QObject *parent = nullptr);
    explicit UndoCommand(QUndoCommand *parent);
    explicit UndoCommand(const UndoCommand &other) { }

    void setSystemCheck(bool check);
    bool checkSystem() const;

    bool isFirstChange() const;

protected:
    bool m_systemCheck = false;
    bool m_firstRedo = true;
};

} // namespace cmd
} // namespace aadlinterface

Q_DECLARE_METATYPE(aadlinterface::cmd::UndoCommand)
