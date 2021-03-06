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
   along with this program. If not, see <https://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#include "basecreatortool.h"

#include "baseitems/common/mscutils.h"
#include "baseitems/interactiveobject.h"
#include "mscchart.h"

namespace msc {

BaseCreatorTool::BaseCreatorTool(ChartLayoutManager *model, QGraphicsView *view, QObject *parent)
    : BaseTool(view, parent)
    , m_model(model)
    , m_activeChart(m_model ? m_model->currentChart() : nullptr)

{
    if (model) {
        connect(model, &ChartLayoutManager::currentChartChanged, this, &BaseCreatorTool::onCurrentChartChagend);
    }
}

void BaseCreatorTool::setModel(ChartLayoutManager *model)
{
    m_model = model;
}

void BaseCreatorTool::onCurrentChartChagend(msc::MscChart *chart)
{
    if (chart == m_activeChart)
        return;

    removePreviewItem();

    m_activeChart = chart;
    if (isActive()) {
        createPreviewItem();
    }
}

void BaseCreatorTool::startWaitForModelLayoutComplete(MscEntity *addedEntity)
{
    m_addedEntity = addedEntity;
    dropModelLayoutUpdateConnection();
    m_modelUpdateFinishedListener =
            connect(m_model, &ChartLayoutManager::layoutComplete, this, &BaseCreatorTool::onModelLayoutComplete);
}

void BaseCreatorTool::removePreviewItem()
{
    if (!m_previewItem) {
        return;
    }

    removeSceneItem(m_previewItem);
    delete m_previewItem.data();
    m_previewEntity.reset();
}

void BaseCreatorTool::onModelLayoutComplete()
{
    dropModelLayoutUpdateConnection();

    if (!m_scene || !m_addedEntity) {
        return;
    }

    if (InteractiveObject *item = m_model->itemForEntity(m_addedEntity)) {
        item->postCreatePolishing();
    }
    m_addedEntity.clear();
}

void BaseCreatorTool::dropModelLayoutUpdateConnection()
{
    if (m_modelUpdateFinishedListener)
        QObject::disconnect(m_modelUpdateFinishedListener);
}

} // ns msc
