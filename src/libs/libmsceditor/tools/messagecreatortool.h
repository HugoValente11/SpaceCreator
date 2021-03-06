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

#pragma once

#include "basecreatortool.h"
#include "messageitem.h"

namespace msc {

class MessageCreatorTool : public BaseCreatorTool
{
    Q_OBJECT
public:
    MessageCreatorTool(
            MscMessage::MessageType msgType, ChartLayoutManager *model, QGraphicsView *view, QObject *parent = nullptr);
    virtual BaseTool::ToolType toolType() const override;

    void activate();

    bool validateCreate(MscMessage *message) const;

private:
    void createPreviewItem() override;
    void commitPreviewItem() override;

    bool onMousePress(QMouseEvent *e) override;
    bool onMouseRelease(QMouseEvent *e) override;
    bool onMouseMove(QMouseEvent *e) override;

    QVariantList prepareMessage();

    void movePreviewItemTo(const QPointF &newScenePos);

    void processMousePressDrag(QMouseEvent *e);
    void processMouseReleaseDrag(QMouseEvent *e);
    void processMouseMoveDrag(QMouseEvent *e);

    void processMousePressClick(QMouseEvent *e);
    void processMouseReleaseClick(QMouseEvent *e);
    void processMouseMoveClick(QMouseEvent *e);

    bool processKeyPress(QKeyEvent *e) override;

    bool validateUserPoints(msc::MscMessage *message);

private:
    enum class Step
    {
        ChooseSource,
        ChooseTarget,
    };
    Step m_currStep = Step::ChooseSource;

    enum class InteractionMode
    {
        None,
        Drag,
        Click
    };
    InteractionMode m_currMode = InteractionMode::None;

    QPointer<MscMessage> m_message = nullptr;
    QPointer<MessageItem> m_messageItem = nullptr;
    QPointF m_mouseDown;
    MscMessage::MessageType m_messageType;

    void finishArrowCreation(const QPointF &scenePos);

    void updateMessageHead(const QPointF &to);
};

} // ns msc
