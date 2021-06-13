﻿/*
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

#include "creatortool.h"

#include "baseitems/common/ivutils.h"
#include "commands/cmdcommentitemcreate.h"
#include "commands/cmdconnectiongroupitemcreate.h"
#include "commands/cmdconnectionitemcreate.h"
#include "commands/cmdentitiesremove.h"
#include "commands/cmdentitygeometrychange.h"
#include "commands/cmdfunctionitemcreate.h"
#include "commands/cmdfunctiontypeitemcreate.h"
#include "commands/cmdinterfaceitemcreate.h"
#include "commands/cmdmyfunctionitemcreate.h"
#include "commandsstack.h"
#include "connectioncreationvalidator.h"
#include "context/action/actionsmanager.h"
#include "graphicsitemhelpers.h"
#include "graphicsviewutils.h"
#include "interface/createconnectiongroupdialog.h"
#include "interfacedocument.h"
#include "ivcomment.h"
#include "ivcommentgraphicsitem.h"
#include "ivconnection.h"
#include "ivconnectiongraphicsitem.h"
#include "ivconnectiongroup.h"
#include "ivfunction.h"
#include "ivmyfunction.h"
#include "ivfunctiongraphicsitem.h"
#include "ivfunctiontype.h"
#include "ivfunctiontypegraphicsitem.h"
#include "ivmyfunctiongraphicsitem.h"
#include "ivinterface.h"
#include "ivinterfacegraphicsitem.h"
#include "ivitemmodel.h"
#include "ui/grippointshandler.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPointer>
#include <QSet>
#include <QVector>
#include <QtDebug>
#include <QtMath>
#include <limits>

static const qreal kContextMenuItemTolerance = 10.;
static const QList<int> kFunctionTypes = { ive::IVFunctionGraphicsItem::Type, ive::IVFunctionTypeGraphicsItem::Type, ive::IVMyFunctionGraphicsItem::Type };
static const qreal kPreviewItemPenWidth = 2.;

namespace ive {

struct CreatorTool::CreatorToolPrivate {
    CreatorToolPrivate(CreatorTool *tool, InterfaceDocument *doc)
        : thisTool(tool)
        , doc(doc)
        , view(doc->graphicsView())
        , model(doc->itemsModel())
    {
    }

    bool showContextMenu(const QPoint &globalPos);
    void populateContextMenu_commonCreate(QMenu *menu, const QPointF &scenePos);
    void populateContextMenu_commonEdit(QMenu *menu, const QPointF &scenePos);
    void populateContextMenu_propertiesDialog(QMenu *menu);
    void populateContextMenu_user(QMenu *menu, const QPointF &scenePos);

    void handleToolType(CreatorTool::ToolType type, const QPointF &pos);
    void handleComment(QGraphicsScene *scene, const QPointF &pos);
    void handleMyFunction(QGraphicsScene *scene, const QPointF &pos);
    void handleFunctionType(QGraphicsScene *scene, const QPointF &pos);
    void handleFunction(QGraphicsScene *scene, const QPointF &pos);
    void handleInterface(QGraphicsScene *scene, ivm::IVInterface::InterfaceType type, const QPointF &pos);
    bool handleConnectionCreate(const QPointF &pos);
    void handleDirectConnection(const QPointF &pos);
    void handleConnectionReCreate(const QVector<QPointF> &graphicPoints);
    void handleConnection(const QVector<QPointF> &graphicPoints) const;

    bool warnConnectionPreview(const QPointF &pos);

    QUndoCommand *createInterfaceCommand(const ivm::IVInterface::CreationInfo &info) const;

    void clearPreviewItem();

    CreatorTool *thisTool;

    QPointF cursorInScene() const;
    QPointF cursorInScene(const QPoint &screenPos) const;

    QPointer<InterfaceDocument> doc;
    QPointer<QGraphicsView> view;
    QPointer<IVItemModel> model;
    QGraphicsRectItem *previewItem = nullptr;
    QGraphicsPathItem *previewConnectionItem = nullptr;
    QVector<QPointF> connectionPoints;
    QPointF clickScenePos;
    QCursor cursor;
    CreatorTool::ToolType toolType = ToolType::Pointer;
    QSet<shared::ui::VEInteractiveObject *> collidedItems;
};

CreatorTool::CreatorTool(InterfaceDocument *doc)
    : QObject(doc)
    , d(new CreatorToolPrivate(this, doc))
{
    Q_ASSERT(doc != nullptr);
    Q_ASSERT(d->view != nullptr);
    Q_ASSERT(d->model != nullptr);

    if (d->view && d->view->viewport()) {
        d->view->installEventFilter(this);
        d->view->viewport()->installEventFilter(this);
    }
}

CreatorTool::~CreatorTool()
{
    delete d;
    d = nullptr;
}

CreatorTool::ToolType CreatorTool::toolType() const
{
    return d->toolType;
}

void CreatorTool::setCurrentToolType(CreatorTool::ToolType type)
{
    if (d->toolType == type)
        return;

    d->toolType = type;

    d->clearPreviewItem();

    if (d->view)
        d->view->setFocus();
}

void CreatorTool::removeSelectedItems()
{
    if (!d->view)
        return;

    if (auto scene = d->view->scene()) {
        QStringList clonedIfaces;
        QList<QPointer<ivm::IVObject>> entities;
        d->clearPreviewItem();
        while (!scene->selectedItems().isEmpty()) {
            QGraphicsItem *item = scene->selectedItems().first();
            item->setSelected(false);

            if (auto iObj = qobject_cast<shared::ui::VEInteractiveObject *>(item->toGraphicsObject())) {
                if (auto entity = iObj->entity() ? iObj->entity()->as<ivm::IVObject *>() : nullptr) {
                    if (entity->isRootObject()) {
                        continue;
                    }
                    if (entity->isInterface()) {
                        if (auto iface = entity->as<const ivm::IVInterface *>()) {
                            if (auto srcIface = iface->cloneOf()) {
                                clonedIfaces.append(QStringLiteral("%1's %2 is from %3")
                                                            .arg(iface->parentObject()->title(), iface->title(),
                                                                    srcIface->parentObject()->title()));
                                continue;
                            }
                        }
                    }
                    entities.append(entity);
                }
            }
        }
        auto cmdRm = new cmd::CmdEntitiesRemove(entities, d->model->objectsModel());
        cmdRm->setText(tr("Remove selected item(s)"));
        d->doc->commandsStack()->push(cmdRm);

        if (!clonedIfaces.isEmpty()) {
            const QString names = clonedIfaces.join(QStringLiteral("<br>"));
            const QString msg = tr("The following interfaces can not be removed directly:<br><br>"
                                   "<b>%1</b><br><br>"
                                   "Please edit the related FunctionType.")
                                        .arg(names);
            Q_EMIT informUser(tr("Interface removal"), msg);
        }
    }
}

void CreatorTool::groupSelectedItems()
{
    QList<ivm::IVConnectionGroup::CreationInfo> groupCreationDataList;

    auto processConnection = [&](ivm::IVConnection *connection) {
        auto it = std::find_if(groupCreationDataList.begin(), groupCreationDataList.end(),
                [connection](const ivm::IVConnectionGroup::CreationInfo &data) {
                    const bool functionMatch = data.sourceObject->id() == connection->source()->id()
                            && data.targetObject->id() == connection->target()->id();
                    const bool functionMatchReversed = data.targetObject->id() == connection->source()->id()
                            && data.sourceObject->id() == connection->target()->id();

                    return functionMatch || functionMatchReversed;
                });

        if (it != groupCreationDataList.end()) {
            it->connections.append(connection);
        } else {
            QVector<QPointF> points = shared::graphicsviewutils::polygon(connection->coordinates());
            if (points.isEmpty()) {
                const QGraphicsItem *sourceItem = d->model->getItem(connection->source()->id());
                const QGraphicsItem *targetItem = d->model->getItem(connection->target()->id());
                if (!sourceItem || !targetItem) {
                    return;
                }

                QPointF startPoint = sourceItem->sceneBoundingRect().center();
                QPointF endPoint = targetItem->sceneBoundingRect().center();
                const bool startAdjusted = shared::graphicsviewutils::intersects(
                        sourceItem->sceneBoundingRect(), { startPoint, endPoint }, &startPoint);
                const bool endAdjusted = shared::graphicsviewutils::intersects(
                        targetItem->sceneBoundingRect(), { startPoint, endPoint }, &endPoint);
                if (!startAdjusted || !endAdjusted) {
                    return;
                }
                points << startPoint << endPoint;
            }
            groupCreationDataList.append({ d->model->objectsModel(), connection->parentObject(), connection->source(),
                    connection->target(), {}, { connection }, points });
        }
    };

    for (const auto item : d->view->scene()->selectedItems()) {
        if (item->type() == IVConnectionGraphicsItem::Type) {
            if (auto connectionItem = qgraphicsitem_cast<IVConnectionGraphicsItem *>(item)) {
                if (ivm::IVConnection *connectionObj = connectionItem->entity()) {
                    processConnection(connectionObj);
                }
            }
        }
    }

    CreateConnectionGroupDialog *dialog = new CreateConnectionGroupDialog(groupCreationDataList, d->view->window());
    if (dialog->exec() == QDialog::Accepted) {
        for (const ivm::IVConnectionGroup::CreationInfo &data : dialog->info()) {
            auto cmd = new cmd::CmdConnectionGroupItemCreate(data);
            d->doc->commandsStack()->push(cmd);
        }
    }
    dialog->deleteLater();
}

bool CreatorTool::eventFilter(QObject *watched, QEvent *event)
{
    if (d->view && watched == d->view->viewport()) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            return onMousePress(static_cast<QMouseEvent *>(event));
        case QEvent::MouseButtonRelease:
            return onMouseRelease(static_cast<QMouseEvent *>(event));
        case QEvent::MouseMove:
            return onMouseMove(static_cast<QMouseEvent *>(event));
        case QEvent::MouseButtonDblClick: {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);
            if (e->button() & Qt::RightButton) // block double click for right click and handle it
                return onMousePress(e);
            return false;
        }
        default:
            break;
        }
    }

    if (d->view == watched) {
        if (event->type() == QEvent::ContextMenu)
            return onContextMenu(static_cast<QContextMenuEvent *>(event));

        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
            case Qt::Key_Escape: {
                if (toolType() == ToolType::Pointer) {
                    if (auto scene = d->view->scene())
                        scene->clearSelection();
                } else {
                    d->clearPreviewItem();
                    Q_EMIT created();
                }
            } break;
            }
        }
    }

    return false;
}

// Right mouse (ctrl or not) = get the context menu with the creation choices
// Left mouse + ctrl = shortcut to creating a connection + the PI and RI between two boxes
// Left mouse without ctrl: If a tool is selected, perform the tool. Otherwise, select the items
bool CreatorTool::onMousePress(QMouseEvent *e)
{
    if (d->view.isNull()) {
        return false;
    }

    auto scene = d->view->scene();
    if (scene == nullptr) {
        return false;
    }

    const QPointF scenePos = d->cursorInScene(e->globalPos());
    if ((d->toolType == ToolType::ReCreateConnection || e->modifiers() & Qt::ShiftModifier)
            && e->button() != Qt::RightButton) {
        if (!d->previewConnectionItem) {
            QGraphicsItem *item = shared::graphicsviewutils::nearestItem(
                    scene, scenePos, shared::graphicsviewutils::kInterfaceTolerance, { IVInterfaceGraphicsItem::Type });
            if (!item || item->type() != IVInterfaceGraphicsItem::Type)
                return false;

            auto interfaceItem = qgraphicsitem_cast<const IVInterfaceGraphicsItem *>(item);
            if (!interfaceItem)
                return false;

            if (interfaceItem->connectionItems().size() != 1 || !interfaceItem->entity()->isProvided())
                return false;

            auto connectionItem = interfaceItem->connectionItems().front();
            Q_ASSERT(connectionItem);

            auto reqIface = connectionItem->startItem();
            Q_ASSERT(reqIface);

            d->toolType = ToolType::ReCreateConnection;
            d->previewConnectionItem = new QGraphicsPathItem;
            d->previewConnectionItem->setPen(QPen(Qt::black, 2, Qt::DotLine));
            d->previewConnectionItem->setZValue(ZOrder.Preview);
            d->previewConnectionItem->setData(Qt::UserRole, connectionItem->entity()->id());
            scene->addItem(d->previewConnectionItem);
            d->connectionPoints.append(reqIface->connectionEndPoint(connectionItem));
            connectionItem->setVisible(false);
        }
        return true;
    } else if (e->modifiers() & Qt::ControlModifier) {
        auto itemAtCursor = d->view->itemAt(e->pos());
        if ((e->button() & Qt::MouseButton::LeftButton)
                && (!itemAtCursor || itemAtCursor->type() != shared::ui::GripPoint::Type))
            d->toolType = ToolType::DirectConnection;
        else
            return false;
    } else if (!(e->button() & Qt::RightButton) && d->toolType == ToolType::Pointer) {
        return false;
    }

    if (d->toolType == ToolType::DirectConnection && e->button() != Qt::RightButton) {
        if (!shared::graphicsviewutils::nearestItem(scene, scenePos, QList<int> { IVFunctionGraphicsItem::Type, IVMyFunctionGraphicsItem::Type })) {
            if (!shared::graphicsviewutils::nearestItem(scene, scenePos, shared::graphicsviewutils::kInterfaceTolerance,
                        { IVInterfaceGraphicsItem::Type }))
                return false;
        }

        if (d->previewConnectionItem) {
            d->connectionPoints.clear();
        } else {
            d->previewConnectionItem = new QGraphicsPathItem;
            d->previewConnectionItem->setPen(QPen(Qt::black, 2, Qt::DotLine));
            d->previewConnectionItem->setZValue(ZOrder.Preview);
            scene->addItem(d->previewConnectionItem);
        }
        d->connectionPoints.append(scenePos);
        return true;
    } else if (d->toolType == ToolType::MultiPointConnection && e->button() != Qt::RightButton) {
        if (!d->previewConnectionItem) {
            QGraphicsItem *item = shared::graphicsviewutils::nearestItem(
                    scene, scenePos, shared::graphicsviewutils::kInterfaceTolerance, { IVInterfaceGraphicsItem::Type });
            if (!item)
                return false;

            const QPointF startPoint = item->mapToScene(QPointF(0, 0));
            d->previewConnectionItem = new QGraphicsPathItem;
            d->previewConnectionItem->setPen(QPen(Qt::black, 2, Qt::DotLine));
            d->previewConnectionItem->setZValue(ZOrder.Preview);
            scene->addItem(d->previewConnectionItem);
            d->connectionPoints.append(startPoint);
            return true;
        }
        return !d->connectionPoints.contains(scenePos);
    } else if (e->button() == Qt::RightButton
            || (d->toolType != ToolType::RequiredInterface && d->toolType != ToolType::ProvidedInterface)) {
        if (!d->previewItem) {
            QGraphicsItem *parentItem = d->view->itemAt(e->pos());
            while (parentItem != nullptr && parentItem->type() != IVFunctionGraphicsItem::Type
                    && parentItem->type() != IVFunctionTypeGraphicsItem::Type) {
                parentItem = parentItem->parentItem();
            }
            d->previewItem = new QGraphicsRectItem(parentItem);
            d->previewItem->setPen(QPen(Qt::blue, kPreviewItemPenWidth, Qt::SolidLine));
            d->previewItem->setBrush(QBrush(QColor(30, 144, 255, 90)));
            d->previewItem->setZValue(ZOrder.Preview);
            d->clickScenePos = scenePos;

            if (!parentItem)
                scene->addItem(d->previewItem);

            if (!e->buttons().testFlag(Qt::MaxMouseButton)) {
                auto items = d->view->items(e->pos());
                for (auto item : items) {
                    if (item->type() > QGraphicsItem::UserType) {
                        if (!item->isSelected()) {
                            scene->clearSelection();
                            item->setSelected(true);
                        }
                        break;
                    }
                }
            }
        }
        const QPointF mappedScenePos = d->previewItem->mapFromScene(scenePos);
        d->previewItem->setRect({ mappedScenePos, mappedScenePos });
        return true;
    }

    return true;
}

bool CreatorTool::onMouseRelease(QMouseEvent *e)
{
    if (!d->view)
        return false;

    if ((e->button() & Qt::RightButton) && d->previewItem) {
        return d->showContextMenu(e->globalPos());
    } else if (d->toolType != ToolType::Pointer) {
        const bool hasPreview = d->previewItem || d->previewConnectionItem;
        const bool isIface = d->toolType == ToolType::ProvidedInterface || d->toolType == ToolType::RequiredInterface;
        if (hasPreview || isIface) {
            const QPointF &scenePos = d->cursorInScene(e->globalPos());
            d->handleToolType(d->toolType, scenePos);
            return true;
        }
    }
    return false;
}

bool CreatorTool::onMouseMove(QMouseEvent *e)
{
    if (!d->view || !d->view->scene())
        return false;

    const QPointF &scenePos = d->cursorInScene(e->globalPos());
    if (d->previewItem && d->previewItem->isVisible()) {
        const QRectF newGeometry = QRectF(d->clickScenePos, scenePos).normalized();
        if (!newGeometry.isValid())
            return true;

        QSet<shared::ui::VEInteractiveObject *> items;
        const QRectF expandedGeometry { newGeometry.marginsAdded(shared::graphicsviewutils::kContentMargins) };
        QList<QGraphicsItem *> newCollidedItems = d->view->scene()->items(expandedGeometry);
        std::for_each(newCollidedItems.begin(), newCollidedItems.end(),
                [this, &items, expandedGeometry](QGraphicsItem *item) {
                    if (item->type() == IVInterfaceGraphicsItem::Type || item->type() == d->previewItem->type())
                        return;

                    auto iObjItem = qobject_cast<shared::ui::VEInteractiveObject *>(item->toGraphicsObject());
                    if (!iObjItem)
                        return;

                    if (item->parentItem() == d->previewItem->parentItem()
                            || (d->previewItem->parentItem() == item
                                    && !item->sceneBoundingRect().contains(expandedGeometry))) {
                        items.insert(iObjItem);
                    }
                });
        QSet<shared::ui::VEInteractiveObject *> newItems(items);
        newItems.subtract(d->collidedItems);
        for (auto item : newItems)
            item->doHighlighting(Qt::red, true);

        QSet<shared::ui::VEInteractiveObject *> oldItems(d->collidedItems);
        oldItems.subtract(items);

        for (auto item : oldItems)
            item->doHighlighting(Qt::green, false);

        d->collidedItems = items;
        d->previewItem->setRect(d->previewItem->mapRectFromScene(newGeometry));
        return true;
    } else if (d->previewConnectionItem && d->previewConnectionItem->isVisible() && !d->connectionPoints.isEmpty()) {
        if (d->view->scene()) {
            QPainterPath pp;
            pp.addPolygon(d->connectionPoints);
            pp.lineTo(scenePos);
            d->previewConnectionItem->setPath(pp);
            d->warnConnectionPreview(scenePos);
            return true;
        }
    }

    return false;
}

bool CreatorTool::onContextMenu(QContextMenuEvent *e)
{
    if (!d->view || e->reason() == QContextMenuEvent::Mouse)
        return false;

    QGraphicsScene *scene = d->view->scene();
    if (!scene)
        return false;

    QPoint viewPos, globalPos;
    if (!scene->selectedItems().isEmpty()) {
        QGraphicsItem *selectedItem = scene->selectedItems().first();
        const QPointF &scenePos = selectedItem->mapToScene(selectedItem->boundingRect().bottomRight());
        viewPos = d->view->mapFromScene(scenePos);
        globalPos = d->view->mapToGlobal(viewPos);
    } else {
        globalPos = QCursor::pos();
        viewPos = d->view->mapFromGlobal(globalPos);
    }

    // onMousePress is needed to set an appropriate d->previewItem
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, viewPos, Qt::RightButton, Qt::RightButton | Qt::MaxMouseButton,
            nullptr); // Qt::MaxMouseButton is a fake button
                      // to distinguish this mouse event
                      // and thus avoid selecting of another object
    onMousePress(&mouseEvent);

    return d->showContextMenu(globalPos);
}

template<typename ItemType>
ItemType *itemAt(const QGraphicsScene *scene, const QPointF &point)
{
    QList<QGraphicsItem *> items = scene->items(point);
    if (items.isEmpty())
        return nullptr;
    auto it = std::find_if(items.constBegin(), items.constEnd(),
            [](const QGraphicsItem *item) { return item->type() == ItemType::Type; });
    if (it == items.constEnd())
        return nullptr;

    return qgraphicsitem_cast<ItemType *>(*it);
}

static inline QRectF adjustToSize(const QRectF &rect, const QSizeF &minSize)
{
    QRectF itemRect = rect;
    if (itemRect.width() < minSize.width())
        itemRect.setWidth(minSize.width());
    if (itemRect.height() < minSize.height())
        itemRect.setHeight(minSize.height());
    return itemRect;
};

bool CreatorTool::CreatorToolPrivate::showContextMenu(const QPoint &globalPos)
{
    QMenu *menu = new QMenu(view);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    auto scenePos = cursorInScene(globalPos);
    populateContextMenu_commonCreate(menu, scenePos);
    populateContextMenu_commonEdit(menu, scenePos);
    populateContextMenu_propertiesDialog(menu);
    populateContextMenu_user(menu, scenePos);

    if (menu->isEmpty()) {
        delete menu;
        menu = nullptr;
        return false;
    }

    connect(menu, &QMenu::aboutToHide, [this]() {
        if (previewItem) {
            previewItem->setVisible(false);
        }
    });
    menu->exec(globalPos);
    clearPreviewItem();
    return true;
}

void CreatorTool::CreatorToolPrivate::populateContextMenu_commonCreate(QMenu *menu, const QPointF &scenePos)
{
    if (this->previewItem) {
        static const QSizeF emptyPreviewItemSize = QSizeF(kPreviewItemPenWidth, kPreviewItemPenWidth);
        const bool isRect = this->previewItem->boundingRect().size() != emptyPreviewItemSize;
        // TODO: use a Fn/FnType/Comment's min size to disable related actions if the creation is impossible?

        auto action = menu->addAction(QIcon(QLatin1String(":/tab_interface/toolbar/icns/function_type.svg")),
                thisTool->tr("Function Type"), thisTool,
                [this, scenePos]() { handleToolType(ToolType::FunctionType, scenePos); });

        action = menu->addAction(QIcon(QLatin1String(":/tab_interface/toolbar/icns/function.svg")),
                thisTool->tr("Function"), thisTool,
                [this, scenePos]() { handleToolType(ToolType::Function, scenePos); });

        action = menu->addAction(QIcon(QLatin1String(":/tab_interface/toolbar/icns/comment.svg")),
                thisTool->tr("Comment"), thisTool, [this, scenePos]() { handleToolType(ToolType::Comment, scenePos); });

        action = menu->addAction(QIcon(QLatin1String(":/tab_interface/toolbar/icns/comment.svg")),
                thisTool->tr("My Function action"), thisTool, [this, scenePos]() { handleToolType(ToolType::MyFunction, scenePos); });

        action = menu->addAction(QIcon(QLatin1String(":/tab_interface/toolbar/icns/ri.svg")),
                thisTool->tr("Required Interface"), thisTool,
                [this, scenePos]() { handleToolType(ToolType::RequiredInterface, scenePos); });
        action->setEnabled(!isRect && this->previewItem->parentItem());

        action = menu->addAction(QIcon(QLatin1String(":/tab_interface/toolbar/icns/pi.svg")),
                thisTool->tr("Provided Interface"), thisTool,
                [this, scenePos]() { handleToolType(ToolType::ProvidedInterface, scenePos); });
        action->setEnabled(!isRect && this->previewItem->parentItem());

        action = menu->addAction(QIcon(QLatin1String(":/tab_interface/toolbar/icns/connection_group.svg")),
                thisTool->tr("Connection group"), thisTool, [this]() { thisTool->groupSelectedItems(); });
        const auto selectedItems = previewItem->scene()->selectedItems();
        const auto it = std::find_if(selectedItems.cbegin(), selectedItems.cend(),
                [](const QGraphicsItem *item) { return item->type() == IVConnectionGraphicsItem::Type; });
        action->setEnabled(it != selectedItems.cend());
    }
}

void CreatorTool::CreatorToolPrivate::populateContextMenu_commonEdit(QMenu *menu, const QPointF &scenePos)
{
    const QList<QGraphicsItem *> selectedItems = view->scene()->selectedItems();
    static const QList<int> kNestedTypes { ive::IVFunctionGraphicsItem::Type, ive::IVFunctionTypeGraphicsItem::Type,
        ive::IVCommentGraphicsItem::Type };
    const bool copyable = std::any_of(selectedItems.cbegin(), selectedItems.cend(),
            [](const QGraphicsItem *item) { return kNestedTypes.contains(item->type()); });
    auto action = menu->addAction(
            QIcon(QLatin1String(":/tab_interface/toolbar/icns/copy.svg")), thisTool->tr("Copy"), thisTool,
            [this]() { Q_EMIT thisTool->copyActionTriggered(); }, QKeySequence::Copy);
    action->setEnabled(copyable);

    action = menu->addAction(
            QIcon(QLatin1String(":/tab_interface/toolbar/icns/cut.svg")), thisTool->tr("Cut"), thisTool,
            [this]() { Q_EMIT thisTool->cutActionTriggered(); }, QKeySequence::Cut);
    action->setEnabled(copyable);

    action = menu->addAction(
            QIcon(QLatin1String(":/tab_interface/toolbar/icns/paste.svg")), thisTool->tr("Paste"), thisTool,
            [this, scenePos]() { Q_EMIT thisTool->pasteActionTriggered(scenePos); }, QKeySequence::Paste);
    action->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void CreatorTool::CreatorToolPrivate::populateContextMenu_propertiesDialog(QMenu *menu)
{
    QGraphicsScene *scene = view->scene();
    if (!scene) {
        return;
    }

    QGraphicsItem *gi = scene->selectedItems().isEmpty() ? nullptr : scene->selectedItems().first();
    if (ivm::IVObject *ivObj = gi::object(gi)) {
        if (ivObj->type() != ivm::IVObject::Type::Connection) {
            menu->addSeparator();
            QAction *action = menu->addAction(tr("Properties"));
            action->setEnabled(ivObj);

            connect(action, &QAction::triggered,
                    [this, ivObj]() { Q_EMIT thisTool->propertyEditorRequest(ivObj->id()); });
            ActionsManager::registerAction(Q_FUNC_INFO, action, "Properties", "Show IV object properties editor");
        } else {
            menu->addSeparator();
            QAction *action = menu->addAction(tr("Re-create path"));
            action->setEnabled(ivObj);

            connect(action, &QAction::triggered, [gi]() {
                if (auto connectionItem = qgraphicsitem_cast<IVConnectionGraphicsItem *>(gi)) {
                    connectionItem->layout();
                    connectionItem->updateEntity();
                }
            });
            ActionsManager::registerAction(Q_FUNC_INFO, action, "Connection re-creation",
                    "Create a generic connection path instead of existing one");
        }
    }
}

void CreatorTool::CreatorToolPrivate::populateContextMenu_user(QMenu *menu, const QPointF &scenePos)
{
    QGraphicsScene *scene = view->scene();
    if (!scene)
        return;

    static const QList<int> showProps { IVInterfaceGraphicsItem::Type, IVFunctionTypeGraphicsItem::Type,
        IVFunctionGraphicsItem::Type, IVCommentGraphicsItem::Type, IVConnectionGraphicsItem::Type };

    ivm::IVObject *ivObj { nullptr };
    if (QGraphicsItem *gi = scene->selectedItems().size() == 1
                    ? scene->selectedItems().first()
                    : shared::graphicsviewutils::nearestItem(scene, scenePos, kContextMenuItemTolerance, showProps)) {

        switch (gi->type()) {
        case IVFunctionTypeGraphicsItem::Type: {
            ivObj = gi::functionTypeObject(gi);
            break;
        }
        case IVFunctionGraphicsItem::Type: {
            ivObj = gi::functionObject(gi);
            break;
        }
        case IVInterfaceGraphicsItem::Type: {
            ivObj = gi::interfaceObject(gi);
            break;
        }
        case IVCommentGraphicsItem::Type: {
            ivObj = gi::commentObject(gi);
            break;
        }
        case IVConnectionGraphicsItem::Type: {
            ivObj = gi::connectionObject(gi);
            break;
        }
        default:
            return;
        }
    }

    ActionsManager::populateMenu(menu, ivObj, doc);
}

void CreatorTool::CreatorToolPrivate::handleToolType(CreatorTool::ToolType type, const QPointF &pos)
{
    if (!view)
        return;

    if (auto scene = view->scene()) {
        switch (type) {
        case ToolType::Comment:
            handleComment(scene, pos);
            break;
        case ToolType::FunctionType:
            handleFunctionType(scene, pos);
            break;
        case ToolType::Function:
            handleFunction(scene, pos);
            break;
        case ToolType::MyFunction:
            handleMyFunction(scene, pos);
            break;
        case ToolType::ProvidedInterface:
            handleInterface(scene, ivm::IVInterface::InterfaceType::Provided, pos);
            break;
        case ToolType::RequiredInterface:
            handleInterface(scene, ivm::IVInterface::InterfaceType::Required, pos);
            break;
        case ToolType::MultiPointConnection:
            if (!handleConnectionCreate(pos))
                return;
            handleConnection(connectionPoints);
            break;
        case ToolType::DirectConnection:
            handleDirectConnection(pos);
            break;
        case ToolType::ReCreateConnection:
            if (!handleConnectionCreate(pos))
                return;
            handleConnectionReCreate(connectionPoints);
            break;
        default:
            break;
        }
        clearPreviewItem();
    }

    Q_EMIT thisTool->created();
}

void CreatorTool::CreatorToolPrivate::handleComment(QGraphicsScene *scene, const QPointF &pos)
{
    Q_UNUSED(scene)
    Q_UNUSED(pos)

    if (this->previewItem) {
        ivm::IVFunctionType *parentObject = gi::functionObject(this->previewItem->parentItem());
        if (!parentObject)
            parentObject = gi::functionTypeObject(this->previewItem->parentItem());

        QRectF itemSceneRect = adjustToSize(this->previewItem->mapRectToScene(this->previewItem->rect()),
                shared::graphicsviewutils::kDefaultGraphicsItemSize);
        if (auto parentItem = previewItem->parentItem()) {
            if (!parentItem->sceneBoundingRect()
                            .marginsRemoved(shared::graphicsviewutils::kRootMargins)
                            .contains(itemSceneRect)) {
                itemSceneRect = QRectF();
            }
        }
        auto cmd = new cmd::CmdCommentItemCreate(model->objectsModel(), parentObject, itemSceneRect);
        doc->commandsStack()->push(cmd);
    }
}

void CreatorTool::CreatorToolPrivate::handleMyFunction(QGraphicsScene *scene, const QPointF &pos)
{
    Q_UNUSED(scene)
    Q_UNUSED(pos)

    if (this->previewItem) {
        ivm::IVFunctionType *parentObject = gi::functionObject(this->previewItem->parentItem());
        if (!parentObject)
            parentObject = gi::functionTypeObject(this->previewItem->parentItem());

        QRectF itemSceneRect = adjustToSize(this->previewItem->mapRectToScene(this->previewItem->rect()),
                shared::graphicsviewutils::kDefaultGraphicsItemSize);
        if (auto parentItem = previewItem->parentItem()) {
            if (!parentItem->sceneBoundingRect()
                            .marginsRemoved(shared::graphicsviewutils::kRootMargins)
                            .contains(itemSceneRect)) {
                itemSceneRect = QRectF();
            }
        }
        auto cmd = new cmd::CmdMyFunctionItemCreate(model->objectsModel(), parentObject, itemSceneRect);
        doc->commandsStack()->push(cmd);
    }
}

void CreatorTool::CreatorToolPrivate::handleFunctionType(QGraphicsScene *scene, const QPointF &pos)
{
    Q_UNUSED(scene)
    Q_UNUSED(pos)

    if (this->previewItem) {
        QRectF itemSceneRect = adjustToSize(this->previewItem->mapRectToScene(this->previewItem->rect()),
                shared::graphicsviewutils::kDefaultGraphicsItemSize);

        if (!shared::graphicsviewutils::isBounded(this->previewItem, itemSceneRect))
            return;

        if (auto parentItem = previewItem->parentItem()) {
            if (!parentItem->sceneBoundingRect()
                            .marginsRemoved(shared::graphicsviewutils::kRootMargins)
                            .contains(itemSceneRect)) {
                itemSceneRect = QRectF();
            }
        }

        ivm::IVFunction *parentObject = gi::functionObject(this->previewItem->parentItem());
        const shared::Id id = shared::createId();
        auto cmd = new cmd::CmdFunctionTypeItemCreate(model->objectsModel(), parentObject, itemSceneRect, id);
        if (doc->commandsStack()->push(cmd)) {
            Q_EMIT thisTool->functionCreated(id);
        }
    }
}

void CreatorTool::CreatorToolPrivate::handleFunction(QGraphicsScene *scene, const QPointF &pos)
{
    Q_UNUSED(scene)
    Q_UNUSED(pos)

    if (previewItem) {
        QRectF itemSceneRect = adjustToSize(
                previewItem->mapRectToScene(previewItem->rect()), shared::graphicsviewutils::kDefaultGraphicsItemSize);

        if (!shared::graphicsviewutils::isBounded(previewItem, itemSceneRect))
            return;

        if (auto parentItem = previewItem->parentItem()) {
            if (!parentItem->sceneBoundingRect()
                            .marginsRemoved(shared::graphicsviewutils::kRootMargins)
                            .contains(itemSceneRect)) {
                itemSceneRect = QRectF();
            }
        }

        ivm::IVFunction *parentObject = gi::functionObject(previewItem->parentItem());
        const shared::Id id = shared::createId();
        auto cmd = new cmd::CmdFunctionItemCreate(model->objectsModel(), parentObject, itemSceneRect, QString(), id);
        if (doc->commandsStack()->push(cmd)) {
            Q_EMIT thisTool->functionCreated(id);
        }
    }
}

void CreatorTool::CreatorToolPrivate::handleInterface(
        QGraphicsScene *scene, ivm::IVInterface::InterfaceType type, const QPointF &pos)
{
    if (auto parentItem = shared::graphicsviewutils::nearestItem(scene,
                shared::graphicsviewutils::adjustFromPoint(pos, shared::graphicsviewutils::kInterfaceTolerance),
                kFunctionTypes)) {
        ivm::IVFunctionType *parentObject = gi::functionTypeObject(parentItem);
        ivm::IVInterface::CreationInfo ifaceDescr(model->objectsModel(), parentObject, pos, type, shared::createId());
        ifaceDescr.resetKind();

        if (auto cmd = createInterfaceCommand(ifaceDescr))
            doc->commandsStack()->push(cmd);
    }
}

bool CreatorTool::CreatorToolPrivate::handleConnectionCreate(const QPointF &pos)
{
    QGraphicsScene *scene = this->view ? this->view->scene() : nullptr;
    if (!scene)
        return false;

    if (!this->previewConnectionItem)
        return false;

    if (auto itemUnderCursor = qgraphicsitem_cast<IVInterfaceGraphicsItem *>(shared::graphicsviewutils::nearestItem(
                scene, pos, shared::graphicsviewutils::kInterfaceTolerance, { IVInterfaceGraphicsItem::Type }))) {
        const QPointF finishPoint = itemUnderCursor->connectionEndPoint();
        if (!itemUnderCursor->ifaceShape().boundingRect().contains(this->connectionPoints.front())) {
            this->connectionPoints.append(finishPoint);
            return true;
        }
    } else {
        this->connectionPoints.append(pos);
    }

    QPainterPath pp;
    pp.addPolygon(this->connectionPoints);
    pp.lineTo(pos);
    this->previewConnectionItem->setPath(pp);
    return false;
}

void CreatorTool::CreatorToolPrivate::handleDirectConnection(const QPointF &pos)
{
    if (this->connectionPoints.size() < 1)
        return;

    this->connectionPoints.append(pos);

    handleConnection(this->connectionPoints);
}

void CreatorTool::CreatorToolPrivate::handleConnection(const QVector<QPointF> &graphicPoints) const
{
    const auto info = ive::gi::validateConnectionCreate(this->view ? this->view->scene() : nullptr, graphicPoints);
    if (info.failed())
        return;

    const IVFunctionGraphicsItem *parentForConnection = nullptr;
    QPointF startInterfacePoint { info.startPointAdjusted };
    QPointF endInterfacePoint { info.endPointAdjusted };
    ivm::IVInterface::CreationInfo ifaceCommons;
    cmd::CommandsStack::Macro cmdMacro(doc->undoStack(), tr("Create connection"));

    if (info.startIface && !info.endIface) {
        ifaceCommons = ivm::IVInterface::CreationInfo::fromIface(info.startIface);
        ifaceCommons.function = info.endObject;
        ifaceCommons.position = info.endPointAdjusted;
        ifaceCommons.type =
                info.isToOrFromNested ? info.startIface->direction() : ivm::IVInterface::InterfaceType::Provided;
        ifaceCommons.id = info.endIfaceId;
        ifaceCommons.resetKind();

        if (!cmdMacro.push(createInterfaceCommand(ifaceCommons)))
            return;
    } else if (info.endIface && !info.startIface) {
        ifaceCommons = ivm::IVInterface::CreationInfo::fromIface(info.endIface);
        ifaceCommons.function = info.startObject;
        ifaceCommons.position = info.startPointAdjusted;
        ifaceCommons.type =
                info.isToOrFromNested ? info.endIface->direction() : ivm::IVInterface::InterfaceType::Required;
        ifaceCommons.id = info.startIfaceId;
        ifaceCommons.resetKind();

        if (!cmdMacro.push(createInterfaceCommand(ifaceCommons)))
            return;
    } else if (!info.startIface && !info.endIface) {
        ifaceCommons.model = model->objectsModel();

        ifaceCommons.function = info.startObject;
        ifaceCommons.position = info.startPointAdjusted;
        ifaceCommons.id = info.startIfaceId;
        ifaceCommons.type = info.isToOrFromNested && graphicPoints.last() == info.connectionPoints.first()
                ? ivm::IVInterface::InterfaceType::Provided
                : ivm::IVInterface::InterfaceType::Required;
        ifaceCommons.resetKind();

        if (!cmdMacro.push(createInterfaceCommand(ifaceCommons)))
            return;

        ifaceCommons.function = info.endObject;
        ifaceCommons.position = info.endPointAdjusted;
        ifaceCommons.id = info.endIfaceId;
        if (!info.isToOrFromNested) {
            ifaceCommons.type = ifaceCommons.type == ivm::IVInterface::InterfaceType::Provided
                    ? ivm::IVInterface::InterfaceType::Required
                    : ivm::IVInterface::InterfaceType::Provided;
        }
        ifaceCommons.resetKind();

        if (!cmdMacro.push(createInterfaceCommand(ifaceCommons)))
            return;
    } else {
        ivm::IVInterface *pi =
                ivm::IVConnection::selectIface<ivm::IVInterfaceProvided *>(info.startIface, info.endIface);
        if (!pi)
            pi = info.startIface;
        ifaceCommons = ivm::IVInterface::CreationInfo::fromIface(pi);
        ifaceCommons.resetKind();
        ifaceCommons.name.clear();
    }

    auto *prevStartItem = info.functionAtStartPos;
    QPointF firstExcludedPoint = *std::next(info.connectionPoints.constBegin());
    shared::Id prevStartIfaceId = info.startIfaceId;
    while (auto item = qgraphicsitem_cast<ive::IVFunctionGraphicsItem *>(prevStartItem->parentItem())) {
        if (item == info.functionAtEndPos && info.endIface) {
            parentForConnection = item;
            break;
        }
        const QRectF rect = item->sceneBoundingRect();
        const QVector<QPointF> intersectionPoints =
                shared::graphicsviewutils::intersectionPoints(rect, info.connectionPoints);
        if (intersectionPoints.isEmpty() || intersectionPoints.size() % 2 == 0) {
            parentForConnection = item;
            break;
        }

        auto beginIt = std::find(info.connectionPoints.cbegin(), info.connectionPoints.cend(), firstExcludedPoint);
        auto endIt = std::find_if(
                beginIt, info.connectionPoints.cend(), [&rect](const QPointF &point) { return !rect.contains(point); });
        QVector<QPointF> points { startInterfacePoint };
        std::copy(beginIt, endIt, std::back_inserter(points));
        points.append(intersectionPoints.last());

        ifaceCommons.type = info.startIface
                ? info.startIface->direction()
                : (graphicPoints.last() == info.connectionPoints.first() ? ivm::IVInterface::InterfaceType::Provided
                                                                         : ivm::IVInterface::InterfaceType::Required);
        if (item == info.functionAtEndPos) {
            ifaceCommons.id = info.endIfaceId;
        } else {
            ifaceCommons.function = item->entity();
            ifaceCommons.position = intersectionPoints.last();
            ifaceCommons.id = shared::createId();

            if (!cmdMacro.push(createInterfaceCommand(ifaceCommons)))
                return;
        }

        auto cmd = new cmd::CmdConnectionItemCreate(
                model->objectsModel(), item->entity(), prevStartIfaceId, ifaceCommons.id, points);
        if (!cmdMacro.push(cmd))
            return;

        firstExcludedPoint = endIt != points.constEnd() ? *endIt : QPointF();
        startInterfacePoint = intersectionPoints.last();
        prevStartItem = item;
        prevStartIfaceId = ifaceCommons.id;
    }

    QPointF lastExcludedPoint = *std::next(info.connectionPoints.crbegin());
    auto prevEndItem = info.functionAtEndPos;
    shared::Id prevEndIfaceId = info.endIfaceId;
    while (auto item = qgraphicsitem_cast<ive::IVFunctionGraphicsItem *>(prevEndItem->parentItem())) {
        if (item == info.functionAtStartPos && info.startIface) {
            Q_ASSERT(parentForConnection == item || parentForConnection == nullptr);
            parentForConnection = item;
            break;
        }
        const QRectF rect = item->sceneBoundingRect();
        const auto intersectionPoints = shared::graphicsviewutils::intersectionPoints(rect, info.connectionPoints);
        if (intersectionPoints.isEmpty() || intersectionPoints.size() % 2 == 0) {
            Q_ASSERT(parentForConnection == item || parentForConnection == nullptr);
            parentForConnection = item;
            break;
        }
        auto beginIt = std::find(info.connectionPoints.crbegin(), info.connectionPoints.crend(), lastExcludedPoint);
        auto endIt = std::find_if(beginIt, info.connectionPoints.crend(),
                [&rect](const QPointF &point) { return !rect.contains(point); });
        QVector<QPointF> points { endInterfacePoint };
        std::copy(beginIt, endIt, std::back_inserter(points));
        points.append(intersectionPoints.first());

        if (item == info.functionAtStartPos) {
            ifaceCommons.id = info.startIfaceId;
        } else {
            ifaceCommons.id = shared::createId();

            ifaceCommons.function = item->entity();
            ifaceCommons.position = intersectionPoints.first();

            ifaceCommons.type = info.endIface ? info.endIface->direction()
                                              : (graphicPoints.last() == info.connectionPoints.first()
                                                              ? ivm::IVInterface::InterfaceType::Required
                                                              : ivm::IVInterface::InterfaceType::Provided);

            if (!cmdMacro.push(createInterfaceCommand(ifaceCommons)))
                return;
        }

        auto cmd = new cmd::CmdConnectionItemCreate(
                model->objectsModel(), item->entity(), prevEndIfaceId, ifaceCommons.id, points);
        if (!cmdMacro.push(cmd))
            return;

        lastExcludedPoint = endIt != info.connectionPoints.crend() ? *endIt : QPointF();
        endInterfacePoint = intersectionPoints.first();
        prevEndItem = item;
        prevEndIfaceId = ifaceCommons.id;
    }

    auto beginIt = std::find(info.connectionPoints.cbegin(), info.connectionPoints.cend(), firstExcludedPoint);
    auto endIt = std::find(beginIt, info.connectionPoints.cend(), lastExcludedPoint);
    QVector<QPointF> resultPoints { startInterfacePoint };
    if (endIt != info.connectionPoints.cend()) {
        std::advance(endIt, 1);
        std::copy(beginIt, endIt, std::back_inserter(resultPoints));
    }
    resultPoints.append(endInterfacePoint);
    Q_ASSERT(resultPoints.size() >= 2);
    if (resultPoints.first() != resultPoints.last()) {
        auto cmd = new cmd::CmdConnectionItemCreate(model->objectsModel(),
                parentForConnection ? parentForConnection->entity() : nullptr, prevStartIfaceId, prevEndIfaceId,
                resultPoints);
        if (!cmdMacro.push(cmd))
            return;
    }

    cmdMacro.setComplete(true);
}

void CreatorTool::CreatorToolPrivate::handleConnectionReCreate(const QVector<QPointF> &graphicPoints)
{
    toolType = ToolType::Pointer;
    const auto info = ive::gi::validateConnectionCreate(view ? view->scene() : nullptr, graphicPoints);
    if (info.status != ivm::ConnectionCreationValidator::FailReason::MulticastDisabled
            && info.status != ivm::ConnectionCreationValidator::FailReason::NotFail) {
        return;
    }
    const shared::Id id = previewConnectionItem->data(Qt::UserRole).toUuid();
    if (id.isNull()) {
        return;
    }

    if (auto item = model->getItem(id)) {
        if (auto connection = qgraphicsitem_cast<IVConnectionGraphicsItem *>(item)) {
            if (connection->startItem()->entity()->id() != info.startIfaceId
                    || connection->endItem()->entity()->id() != info.endIfaceId) {
                return;
            }

            QList<QPair<shared::VEObject *, QVector<QPointF>>> paramsList { { connection->entity(),
                    info.connectionPoints } };
            auto cmd = new shared::cmd::CmdEntityGeometryChange(paramsList);
            doc->commandsStack()->push(cmd);
        }
    }
}

bool CreatorTool::CreatorToolPrivate::warnConnectionPreview(const QPointF &pos)
{
    QVector<QPointF> connectionPoints = this->connectionPoints;
    if (connectionPoints.size() > 1)
        connectionPoints.replace(connectionPoints.size() - 1, pos);
    else
        connectionPoints.append(pos);

    auto info = ive::gi::validateConnectionCreate(this->view ? this->view->scene() : nullptr, connectionPoints);
    bool warn = true;
    if (toolType == ToolType::ReCreateConnection) {
        if ((info.status != ivm::ConnectionCreationValidator::FailReason::MulticastDisabled
                    && info.status != ivm::ConnectionCreationValidator::FailReason::NotFail)
                || !info.endIface || !info.startIface) {
            warn = true;
        } else {
            auto startItem = qgraphicsitem_cast<IVInterfaceGraphicsItem *>(model->getItem(info.startIfaceId));
            auto endItem = qgraphicsitem_cast<IVInterfaceGraphicsItem *>(model->getItem(info.endIfaceId));
            if (!startItem || !endItem) {
                warn = true;
            } else {
                warn = !startItem->ifaceShape().boundingRect().contains(connectionPoints.first())
                        && endItem->ifaceShape().boundingRect().contains(connectionPoints.last());
            }
        }
    } else {
        warn = info.failed();
    }
    if (this->previewConnectionItem) {
        QPen p = this->previewConnectionItem->pen();
        p.setColor(warn ? Qt::red : Qt::black);
        this->previewConnectionItem->setPen(p);
    }

    return warn;
}

QUndoCommand *CreatorTool::CreatorToolPrivate::createInterfaceCommand(const ivm::IVInterface::CreationInfo &info) const
{
    if (!info.function)
        return nullptr;

    if (info.function->isFunction()) {
        if (auto fn = info.function->as<const ivm::IVFunction *>()) {
            if (const ivm::IVFunctionType *fnType = fn->instanceOf()) {
                const QString message = thisTool->tr("Can't add interface directly in <b>%1</b>.<br>"
                                                     "Please edit the related <b>%2</b> instead.")
                                                .arg(fn->titleUI(), fnType->titleUI());
                Q_EMIT thisTool->informUser(thisTool->tr("Interface adding"), message);
                return nullptr;
            }
        }
    }
    if (info.type == ivm::IVInterface::InterfaceType::Provided) {
        Q_EMIT thisTool->propertyEditorRequest(info.id);
    }

    return new cmd::CmdInterfaceItemCreate(info);
}

void CreatorTool::CreatorToolPrivate::clearPreviewItem()
{
    for (auto iObj : this->collidedItems) {
        iObj->highlightConnected();
    }
    this->collidedItems.clear();

    this->clickScenePos = QPointF();

    this->connectionPoints.clear();
    if (this->previewConnectionItem) {
        const auto id = previewConnectionItem->data(Qt::UserRole).toUuid();
        if (!id.isNull()) {
            if (auto item = model->getItem(id)) {
                item->setVisible(true);
            }
        }

        this->previewConnectionItem->scene()->removeItem(this->previewConnectionItem);
        delete this->previewConnectionItem;
        this->previewConnectionItem = nullptr;
    }

    if (this->previewItem) {
        this->previewItem->scene()->removeItem(this->previewItem);
        delete this->previewItem;
        this->previewItem = nullptr;
    }
}

QPointF CreatorTool::CreatorToolPrivate::cursorInScene() const
{
    return cursorInScene(QCursor::pos()); // TODO: add current screen detection
}

QPointF CreatorTool::CreatorToolPrivate::cursorInScene(const QPoint &globalPos) const
{
    QPointF sceneCoordinates;
    if (this->view) {
        const QPoint viewCoordinates = this->view->viewport()->mapFromGlobal(globalPos);
        sceneCoordinates = this->view->mapToScene(viewCoordinates);
    }
    return sceneCoordinates;
}
}
