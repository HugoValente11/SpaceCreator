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
#include "commandsfactory.h"
#include "commandsstack.h"
#include "chartviewmodel.h"

#include "commands/cmdconditionitemmove.h"
#include "commands/cmdconditionitemresize.h"

#include "commands/cmdinstanceitemcreate.h"
#include "commands/cmdinstanceitemmove.h"
#include "commands/cmdinstanceitemresize.h"

#include "commands/cmdmessageitemcreate.h"
#include "commands/cmdmessageitemmove.h"
#include "commands/cmdmessageitemresize.h"

#include "conditionitem.h"
#include "instanceitem.h"
#include "messageitem.h"

#include <QGraphicsItem>
#include <QPointF>
#include <QRectF>

namespace msc {
namespace cmd {

QUndoCommand *CommandsFactory::createMessageItemMove(const QVariantList &params)
{
    Q_ASSERT(params.size() == 2);

    if (MessageItem *item = params.first().value<MessageItem *>()) {
        const QPointF &destination = params.last().toPointF();
        if (item->pos() != destination)
            return new CmdMessageItemMove(item, destination);
    }

    return nullptr;
}

QUndoCommand *CommandsFactory::createMessageItemResize(const QVariantList &params)
{
    Q_ASSERT(params.size() == 3);

    if (MessageItem *pItem = params.at(0).value<MessageItem *>()) {
        const QPointF &head(params.at(1).toPointF());
        const QPointF &tail(params.at(2).toPointF());
        return new CmdMessageItemResize(pItem, head, tail);
    }

    return nullptr;
}

QUndoCommand *CommandsFactory::createMessageItemCreate(const QVariantList &params)
{
    Q_ASSERT(params.size() == 3);

    if (QGraphicsScene *scene = params.at(0).value<QGraphicsScene *>())
        if (ChartViewModel *model = params.at(1).value<ChartViewModel *>()) {
            const QPointF &pos(params.at(2).toPointF());
            return new CmdMessageItemCreate(scene, model, pos);
        }

    return nullptr;
}

QUndoCommand *CommandsFactory::createInstanceItemMove(const QVariantList &params)
{
    Q_ASSERT(params.size() == 2);

    if (InstanceItem *item = params.first().value<InstanceItem *>()) {
        const QPointF &destination = params.last().toPointF();
        if (item->pos() != destination)
            return new CmdInstanceItemMove(item, destination);
    }

    return nullptr;
}

QUndoCommand *CommandsFactory::createInstanceItemResize(const QVariantList &params)
{
    Q_ASSERT(params.size() == 2);

    if (InstanceItem *item = params.first().value<InstanceItem *>()) {
        const QRectF &newGeometry = params.last().toRectF();
        if (item->boundingRect() != newGeometry)
            return new CmdInstanceItemResize(item, newGeometry);
    }

    return nullptr;
}

QUndoCommand *CommandsFactory::createInstanceItemCreate(const QVariantList &params)
{
    Q_ASSERT(params.size() == 3);

    if (QGraphicsScene *scene = params.at(0).value<QGraphicsScene *>())
        if (ChartViewModel *model = params.at(1).value<ChartViewModel *>()) {
            const QPointF &pos(params.at(2).toPointF());
            return new CmdInstanceItemCreate(scene, model, pos);
        }

    return nullptr;
}

QUndoCommand *CommandsFactory::createConditionItemMove(const QVariantList &params)
{
    Q_ASSERT(params.size() == 2);

    if (ConditionItem *item = params.first().value<ConditionItem *>()) {
        const QPointF &destination = params.last().toPointF();
        if (item->pos() != destination)
            return new CmdConditionItemMove(item, destination);
    }

    return nullptr;
}

QUndoCommand *CommandsFactory::createConditionItemResize(const QVariantList &params)
{
    Q_ASSERT(params.size() == 2);

    if (ConditionItem *item = params.first().value<ConditionItem *>()) {
        const QRectF &newGeometry = params.last().toRectF();
        if (item->boundingRect() != newGeometry)
            return new CmdConditionItemResize(item, newGeometry);
    }

    return nullptr;
}

} // ns cmd
} // ns msc
