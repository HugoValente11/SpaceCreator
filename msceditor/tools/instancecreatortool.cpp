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

#include "instancecreatortool.h"
#include "commands/common/commandsstack.h"

#include <QDebug>

namespace msc {

InstanceCreatorTool::InstanceCreatorTool(ChartViewModel *model, QGraphicsView *view, QObject *parent)
    : BaseCreatorTool(model, view, parent)
{
    m_title = tr("Instance");
    m_description = tr("Create new Instance item");
}

ToolType InstanceCreatorTool::toolType() const
{
    return msc::ToolType::InstanceCreator;
}

void InstanceCreatorTool::createPreviewItem()
{
    if (!m_scene || (m_instance && m_instanceItem))
        return;

    m_instance = new MscInstance(tr("New message"), this);
    m_instanceItem = InstanceItem::createDefaultItem(m_instance, scenePos());
    m_scene->addItem(m_instanceItem);

    m_previewItem = m_instanceItem;
    m_previewItem->setOpacity(0.5);
}

void InstanceCreatorTool::commitPreviewItem()
{
    if (!m_previewItem || !m_scene)
        return;

    const QVariantList &cmdParams = { QVariant::fromValue<QGraphicsScene *>(m_scene),
                                      QVariant::fromValue<ChartViewModel *>(m_model),
                                      m_previewItem->pos() };

    msc::cmd::CommandsStack::push(msc::cmd::Id::CreateInstance, cmdParams);
}

void InstanceCreatorTool::removePreviewItem()
{
    BaseCreatorTool::removePreviewItem();
    delete m_instance;
}

} // ns msc
