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

#pragma once

#include "basecommand.h"

#include <QPointF>

namespace msc {

class MessageItem;

namespace cmd {

class CmdMessageItemResize : public BaseCommand
{
public:
    CmdMessageItemResize(MessageItem *messageItem, const QPointF &head, const QPointF &tail);

    void redo() override;
    void undo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override;

private:
    MessageItem *m_messageItem = nullptr;

    QPointF m_newHead;
    QPointF m_newTail;
    QPointF m_oldHead;
    QPointF m_oldTail;
};

} // ns cmd
} // ns msc
