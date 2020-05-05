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

#include "aadlobjectiface.h"

#include <QCursor>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QVector>

class QMouseEvent;
class QContextMenuEvent;
class QGraphicsView;
class QGraphicsRectItem;
class QGraphicsPathItem;
class QGraphicsScene;
class QMenu;
class QUndoCommand;

namespace aadl {
class AADLObjectsModel;
class AADLObject;
}

namespace aadlinterface {

class InteractiveObject;
class CreatorTool : public QObject
{
    Q_OBJECT
public:
    explicit CreatorTool(QGraphicsView *view, aadl::AADLObjectsModel *model, QObject *parent = nullptr);
    enum class ToolType
    {
        Pointer = 0,
        Function,
        FunctionType,
        Comment,
        ProvidedInterface,
        RequiredInterface,
        MultiPointConnection,
        DirectConnection
    };
    Q_ENUM(ToolType)

    CreatorTool::ToolType toolType() const;
    void setCurrentToolType(CreatorTool::ToolType type);
    void removeSelectedItems();

Q_SIGNALS:
    void created();
    void propertyEditorRequest(aadl::AADLObject *entity) const;
    void informUser(const QString &title, const QString &message) const;

protected:
    QPointer<QGraphicsView> m_view;
    QPointer<aadl::AADLObjectsModel> m_model;
    QGraphicsRectItem *m_previewItem = nullptr;
    QGraphicsPathItem *m_previewConnectionItem = nullptr;
    QVector<QPointF> m_connectionPoints;
    QPointF m_clickScenePos;
    QCursor m_cursor;

    bool eventFilter(QObject *watched, QEvent *event) override;

    virtual bool onMousePress(QMouseEvent *e);
    virtual bool onMouseRelease(QMouseEvent *e);
    virtual bool onMouseMove(QMouseEvent *e);
    virtual bool onContextMenu(QContextMenuEvent *e);

    QPointF cursorInScene() const;
    QPointF cursorInScene(const QPoint &screenPos) const;

private:
    void clearPreviewItem();

    void handleToolType(CreatorTool::ToolType type, const QPointF &pos);

    void handleComment(QGraphicsScene *scene, const QPointF &pos);
    void handleFunctionType(QGraphicsScene *scene, const QPointF &pos);
    void handleFunction(QGraphicsScene *scene, const QPointF &pos);
    void handleInterface(QGraphicsScene *scene, aadl::AADLObjectIface::IfaceType type, const QPointF &pos);
    bool handleConnectionCreate(const QPointF &pos);
    void handleDirectConnection(const QPointF &pos);
    void handleConnection(const QVector<QPointF> &connectionPoints) const;

    QMenu *populateContextMenu(const QPointF &scenePos);
    void populateContextMenu_commonCreate(QMenu *menu, const QPointF &scenePos);
    void populateContextMenu_propertiesDialog(QMenu *menu, const QPointF &scenePos);
    void populateContextMenu_user(QMenu *menu, const QPointF &scenePos);

    bool warnConnectionPreview(const QPointF &pos);

    bool showContextMenu(const QPoint &globalPos);

private:
    CreatorTool::ToolType m_toolType { ToolType::Pointer };
    QSet<InteractiveObject *> m_collidedItems;

    QUndoCommand *createInterfaceCommand(const aadl::AADLObjectIface::CreationInfo &info) const;
};

}