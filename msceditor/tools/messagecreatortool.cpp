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

#include "messagecreatortool.h"

#include <baseitems/arrowitem.h>
#include "commands/common/commandsstack.h"

#include <QDebug>

namespace msc {

MessageCreatorTool::MessageCreatorTool(ChartViewModel *model, QGraphicsView *view, QObject *parent)
    : BaseCreatorTool(model, view, parent)
{
    m_title = tr("Message");
    m_description = tr("Create new Message item");
}

ToolType MessageCreatorTool::toolType() const
{
    return msc::ToolType::MessageCreator;
}

void MessageCreatorTool::createPreviewItem()
{
    if (!m_scene || (m_message && m_messageItem))
        return;

    m_message = new MscMessage(tr("New message"), this);
    m_messageItem = MessageItem::createDefaultItem(m_message, scenePos());
    m_scene->addItem(m_messageItem);
    m_messageItem->setAutoResizable(false);

    m_previewItem = m_messageItem;
    m_previewItem->setOpacity(0.5);
}

void MessageCreatorTool::commitPreviewItem()
{
    if (!m_previewItem || !m_scene)
        return;

    MessageItem *messageItem = static_cast<MessageItem *>(m_previewItem);
    const QVariantList &cmdParams = { QVariant::fromValue<QGraphicsScene *>(m_scene),
                                      QVariant::fromValue<ChartViewModel *>(m_model),
                                      messageItem->pos() };

    msc::cmd::CommandsStack::push(msc::cmd::Id::CreateMessage, cmdParams);
}

void MessageCreatorTool::removePreviewItem()
{
    BaseCreatorTool::removePreviewItem();
    delete m_message;
}

} // ns msc
