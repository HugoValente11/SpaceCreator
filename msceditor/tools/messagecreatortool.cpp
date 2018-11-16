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

#include <mscmessage.h>
#include <messageitem.h>
#include <baseitems/arrowitem.h>

#include <QGraphicsScene>
#include <QMouseEvent>

#include <QDebug>

namespace msc {

MessageCreatorTool::MessageCreatorTool(QGraphicsView *view, QObject *parent)
    : BaseTool(view, parent)
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
    qDebug() << Q_FUNC_INFO << title();

    if (!m_scene)
        return;
}

void MessageCreatorTool::commitPreviewItem()
{
    qDebug() << Q_FUNC_INFO << title();

    if (!m_previewItem || !m_scene)
        return;
}

} // ns msc
