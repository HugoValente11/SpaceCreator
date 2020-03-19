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

#include "aadlconnectiongraphicsitem.h"

#include "aadlcommentgraphicsitem.h"
#include "aadlfunctiongraphicsitem.h"
#include "aadlfunctiontypegraphicsitem.h"
#include "aadlinterfacegraphicsitem.h"
#include "baseitems/common/utils.h"
#include "baseitems/grippoint.h"
#include "baseitems/grippointshandler.h"
#include "colors/colormanager.h"
#include "commands/cmdentitygeometrychange.h"
#include "commands/commandids.h"
#include "commands/commandsfactory.h"
#include "tab_aadl/aadlobjectconnection.h"
#include "tab_aadl/aadlobjectfunction.h"
#include "tab_aadl/aadlobjectiface.h"

#include <QGuiApplication>
#include <QPainter>
#include <QPen>
#include <QtDebug>
#include <QtMath>
#include <app/commandsstack.h>

static const qreal kSelectionOffset = 10;
static const qreal kMinLineLength = 20;
static const qreal kConnectionMargin = 20;

namespace taste3 {
namespace aadl {

static inline QList<QVector<QPointF>> generateConnection(const QPointF &startPoint, const QPointF &endPoint)
{
    if (startPoint == endPoint)
        return {};

    if (qFuzzyCompare(startPoint.x(), endPoint.x()) || qFuzzyCompare(startPoint.y(), endPoint.y()))
        return { { startPoint, endPoint } };

    return { { startPoint, { startPoint.x(), endPoint.y() }, endPoint },
             { startPoint, { endPoint.x(), startPoint.y() }, endPoint } };
}

static inline QVector<QPointF> generateConnection(const QLineF &startDirection, const QLineF &endDirection)
{
    QVector<QPointF> connectionPoints { startDirection.p2(), endDirection.p2() };
    const qreal angle = startDirection.angleTo(endDirection);
    static const qreal tolerance = 0.1;
    if (qAbs(qSin(qDegreesToRadians(angle))) <= tolerance) { // ||
        const QPointF mid = utils::lineCenter(QLineF(connectionPoints.first(), connectionPoints.last()));
        QLineF midLine { mid, QPointF(0, 0) };
        midLine.setAngle(startDirection.angle() - 90);

        QPointF startLastPoint;
        midLine.intersect(startDirection, &startLastPoint);
        connectionPoints.insert(connectionPoints.size() - 1, startLastPoint);

        QPointF endLastPoint;
        midLine.intersect(endDirection, &endLastPoint);
        connectionPoints.insert(connectionPoints.size() - 1, endLastPoint);
    } else if (qAbs(qCos(qDegreesToRadians(angle))) <= tolerance) { // |_
        QPointF mid;
        startDirection.intersect(endDirection, &mid);
        connectionPoints.insert(connectionPoints.size() - 1, mid);
    } else {
        qCritical() << startDirection << endDirection << angle;
    }
    return connectionPoints;
}

static inline QLineF getDirection(const QRectF &sceneRect, const QPointF &point)
{
    switch (utils::getNearestSide(sceneRect, point)) {
    case Qt::AlignTop:
        return QLineF(sceneRect.topLeft(), sceneRect.topRight()).normalVector();
    case Qt::AlignBottom:
        return QLineF(sceneRect.bottomRight(), sceneRect.bottomLeft()).normalVector();
    case Qt::AlignLeft:
        return QLineF(sceneRect.bottomLeft(), sceneRect.topLeft()).normalVector();
    case Qt::AlignRight:
        return QLineF(sceneRect.topRight(), sceneRect.bottomRight()).normalVector();
    default:
        break;
    }
    return QLineF();
};

static inline QList<QVector<QPointF>> findSubPath(const QRectF &itemRect, const QVector<QPointF> &prevPoints,
                                                  const QVector<QPointF> &nextPoints)
{
    Q_ASSERT(itemRect.isValid());
    Q_ASSERT(!prevPoints.isEmpty());
    Q_ASSERT(!nextPoints.isEmpty());

    if (!itemRect.isValid() || prevPoints.isEmpty() || nextPoints.isEmpty())
        return {};

    const QPointF startPoint = prevPoints.last();
    const QPointF endPoint = nextPoints.last();
    Q_ASSERT(startPoint != endPoint);
    if (startPoint == endPoint)
        return {};

    const QRectF itemRectMargins =
            itemRect.adjusted(-kConnectionMargin, -kConnectionMargin, kConnectionMargin, kConnectionMargin);
    const QList<QPointF> rectCorners = utils::sortedCorners(itemRectMargins, startPoint, endPoint);
    QList<QVector<QPointF>> allPaths;
    for (const QPointF &p : rectCorners) {
        for (auto polygon : generateConnection(startPoint, p)) {
            if (!utils::intersects(itemRect, polygon) && !utils::intersects(itemRect, QLineF(p, endPoint))) {
                QVector<QPointF> previousPoints(prevPoints);
                previousPoints.removeLast();
                previousPoints << polygon;
                allPaths << previousPoints;
            }
        }
    }
    return allPaths;
}

static inline QVector<QPointF> findPath(QGraphicsScene *scene, const QLineF &startDirection, const QLineF &endDirection,
                                        QRectF *intersectedRect)
{
    static const QList<int> types = { AADLFunctionGraphicsItem::Type, AADLFunctionTypeGraphicsItem::Type };
    QVector<QPointF> points = generateConnection(startDirection, endDirection);
    const QList<QGraphicsItem *> intersectedItems = utils::sceneItems(scene, points);

    qreal distance = std::numeric_limits<qreal>::max();
    QGraphicsItem *intersectionItem = nullptr;
    for (auto it = intersectedItems.constBegin(); it != intersectedItems.constEnd(); ++it) {
        if (!types.contains((*it)->type()))
            continue;

        QPointF intersectionPoint;
        if (utils::intersects((*it)->sceneBoundingRect(), points, &intersectionPoint)
            && QLineF(startDirection.p2(), intersectionPoint).length() < distance) {
            intersectionItem = *it;
        }
    }
    if (!intersectionItem)
        return points;

    *intersectedRect = intersectionItem->sceneBoundingRect();
    return {};
}

static inline QVector<QPointF> path(QGraphicsScene *scene, const QLineF &startDirection, const QLineF &endDirection)
{
    QRectF intersectedRect;
    QList<QVector<QPointF>> paths { { startDirection.p1(), startDirection.p2() } };
    while (true) {
        QList<QVector<QPointF>> deeper;
        QList<QVector<QPointF>> results;
        for (auto path : paths) {
            Q_ASSERT(path.size() >= 2);
            if (path.size() < 2)
                return {};

            const QLineF prevDirection { path.value(path.size() - 2), path.value(path.size() - 1) };
            auto shortPath = findPath(scene, prevDirection, endDirection, &intersectedRect);
            if (!shortPath.isEmpty()) {
                QVector<QPointF> result;
                result.append(startDirection.p1());
                result.append(path);
                result.append(shortPath);
                result.append(endDirection.p1());
                results.append(result);
                continue;
            }
            const auto subPaths = findSubPath(intersectedRect, path, { endDirection.p1(), endDirection.p2() });
            static const QList<int> types = { AADLFunctionGraphicsItem::Type, AADLFunctionTypeGraphicsItem::Type };
            for (auto subPath : subPaths) {
                if (subPath.isEmpty())
                    continue;

                const QList<QGraphicsItem *> intersectedItems = utils::sceneItems(scene, subPath);
                auto it = std::find_if(
                        intersectedItems.constBegin(), intersectedItems.constEnd(), [subPath](QGraphicsItem *item) {
                            if (!types.contains(item->type()))
                                return false;
                            return utils::intersectionPoints(item->sceneBoundingRect(), subPath).size() > 1;
                        });
                if (it != intersectedItems.constEnd())
                    continue;
                else if (subPath.last() == endDirection.p2())
                    results.append(subPath);
                else
                    deeper.append(subPath);
            }
        }
        if (!results.isEmpty()) {
            std::sort(results.begin(), results.end(), [](const QVector<QPointF> &v1, const QVector<QPointF> &v2) {
                if (v1.size() == v2.size())
                    return utils::distancePolygon(v1) < utils::distancePolygon(v2);
                return v1.size() < v2.size();
            });

            return results.first();
        }

        if (paths.size() != deeper.size() || !std::equal(paths.constBegin(), paths.constEnd(), deeper.constBegin()))
            paths = deeper;
        else if (!deeper.isEmpty())
            return deeper.front();
        else
            break;
    }
    return {};
}

QVector<QPointF> AADLConnectionGraphicsItem::connectionPath(AADLInterfaceGraphicsItem *startItem,
                                                            AADLInterfaceGraphicsItem *endItem)
{
    if (!startItem || !endItem)
        return {};

    QGraphicsScene *scene = startItem->scene();
    Q_ASSERT(startItem->scene() == endItem->scene() && scene);

    return AADLConnectionGraphicsItem::connectionPath(scene, startItem->scenePos(),
                                                      startItem->targetItem()->sceneBoundingRect(), endItem->scenePos(),
                                                      endItem->targetItem()->sceneBoundingRect());
}

QVector<QPointF> AADLConnectionGraphicsItem::connectionPath(QGraphicsScene *scene, const QPointF &startIfacePos,
                                                            const QRectF &sourceRect, const QPointF &endIfacePos,
                                                            const QRectF &targetRect)
{
    Q_ASSERT(scene);

    QLineF startDirection = getDirection(sourceRect, startIfacePos);
    if (startDirection.isNull())
        return {};

    startDirection.translate(startIfacePos - startDirection.p1());
    startDirection.setLength(kConnectionMargin);

    QLineF endDirection = getDirection(targetRect, endIfacePos);
    if (endDirection.isNull())
        return {};

    endDirection.translate(endIfacePos - endDirection.p1());
    endDirection.setLength(kConnectionMargin);

    if (sourceRect.contains(endIfacePos))
        startDirection.setAngle(180 + startDirection.angle());
    if (targetRect.contains(startIfacePos))
        endDirection.setAngle(180 + endDirection.angle());

    const auto points = path(scene, startDirection, endDirection);
    return AADLConnectionGraphicsItem::simplify(points);
}

AADLConnectionGraphicsItem::GraphicsPathItem::GraphicsPathItem(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
}

QPainterPath AADLConnectionGraphicsItem::GraphicsPathItem::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(kSelectionOffset);
    stroker.setJoinStyle(Qt::MiterJoin);

    return stroker.createStroke(path()).simplified();
}

AADLConnectionGraphicsItem::AADLConnectionGraphicsItem(AADLObjectConnection *connection,
                                                       AADLInterfaceGraphicsItem *startIface,
                                                       AADLInterfaceGraphicsItem *endIface, QGraphicsItem *parentItem)
    : InteractiveObject(connection, parentItem)
    , m_startItem(startIface)
    , m_endItem(endIface)
    , m_item(new GraphicsPathItem(this))
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemHasNoContents | QGraphicsItem::ItemClipsToShape
             | QGraphicsItem::ItemContainsChildrenInShape);
    setZValue(2);

    colorSchemeUpdated();

    updateInterfaceConnectionsReference(IfaceConnectionReference::Set);
}

AADLConnectionGraphicsItem::~AADLConnectionGraphicsItem()
{
    m_points.clear();

    updateInterfaceConnectionsReference(IfaceConnectionReference::Remove);
}

void AADLConnectionGraphicsItem::updateInterfaceConnectionsReference(IfaceConnectionReference action)
{
    for (AADLInterfaceGraphicsItem *iface : { startItem(), endItem() }) {
        if (iface) {
            if (action == IfaceConnectionReference::Remove)
                iface->removeConnection(this);
            else
                iface->addConnection(this);
        }
    }
}

void AADLConnectionGraphicsItem::updateFromEntity()
{
    aadl::AADLObjectConnection *obj = entity();
    Q_ASSERT(obj);
    if (!obj)
        return;

    setPoints(utils::polygon(obj->coordinates()));
}

void AADLConnectionGraphicsItem::setPoints(const QVector<QPointF> &points)
{
    if (points.isEmpty()) {
        if (m_startItem && m_endItem)
            instantLayoutUpdate();
        else
            m_points.clear();
        return;
    }

    m_points = points;

    const auto startItem = qgraphicsitem_cast<AADLInterfaceGraphicsItem *>(
            utils::nearestItem(scene(), points.first(), QList<int> { AADLInterfaceGraphicsItem::Type }));

    const auto endItem = qgraphicsitem_cast<AADLInterfaceGraphicsItem *>(
            utils::nearestItem(scene(), points.last(), QList<int> { AADLInterfaceGraphicsItem::Type }));

    if (!startItem || !endItem)
        instantLayoutUpdate();

    updateBoundingRect();
}

QVector<QPointF> AADLConnectionGraphicsItem::points() const
{
    return m_points;
}

QVector<QPointF> AADLConnectionGraphicsItem::graphicsPoints() const
{
    QPolygonF polygon = m_item->path().toFillPolygon();
    if (polygon.isClosed())
        polygon.removeLast();
    return mapToScene(polygon);
}

AADLObjectConnection *AADLConnectionGraphicsItem::entity() const
{
    return qobject_cast<aadl::AADLObjectConnection *>(aadlObject());
}

QPainterPath AADLConnectionGraphicsItem::shape() const
{
    return m_item->shape();
}

QRectF AADLConnectionGraphicsItem::boundingRect() const
{
    return m_item->boundingRect();
}

void AADLConnectionGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void AADLConnectionGraphicsItem::rebuildLayout()
{
    if (!m_startItem || !m_startItem->isVisible() || !m_endItem || !m_endItem->isVisible()) {
        setVisible(false);
        return;
    } else {
        if (utils::pos(m_startItem->entity()->coordinates()).isNull())
            m_startItem->instantLayoutUpdate();
        if (utils::pos(m_endItem->entity()->coordinates()).isNull())
            m_endItem->instantLayoutUpdate();
        if (utils::polygon(entity()->coordinates()).isEmpty()) {
            entity()->setCoordinates(utils::coordinates(connectionPath(m_startItem, m_endItem)));
        }
    }

    setVisible(true);
    m_points = connectionPath(m_startItem, m_endItem);
    updateBoundingRect();
}

void AADLConnectionGraphicsItem::onSelectionChanged(bool isSelected)
{
    const ColorHandler h = colorHandler();
    QPen pen = h.pen();
    pen.setWidthF(isSelected ? 1.5 * pen.widthF() : pen.widthF());
    pen.setStyle(isSelected ? Qt::DotLine : Qt::SolidLine);
    m_item->setPen(pen);

    InteractiveObject::onSelectionChanged(isSelected);
}

void AADLConnectionGraphicsItem::updateBoundingRect()
{
    prepareGeometryChange();

    QPainterPath pp;
    pp.addPolygon(mapFromScene(QPolygonF(m_points)));
    m_item->setPath(pp);
    m_boundingRect = pp.boundingRect();
    updateGripPoints();
}

AADLInterfaceGraphicsItem *AADLConnectionGraphicsItem::endItem() const
{
    return m_endItem;
}

AADLInterfaceGraphicsItem *AADLConnectionGraphicsItem::startItem() const
{
    return m_startItem;
}

AADLFunctionGraphicsItem *AADLConnectionGraphicsItem::sourceItem() const
{
    return qgraphicsitem_cast<AADLFunctionGraphicsItem *>(m_startItem ? m_startItem->targetItem() : nullptr);
}

AADLFunctionGraphicsItem *AADLConnectionGraphicsItem::targetItem() const
{
    return qgraphicsitem_cast<AADLFunctionGraphicsItem *>(m_endItem ? m_endItem->targetItem() : nullptr);
}

QList<QVariantList> AADLConnectionGraphicsItem::prepareChangeCoordinatesCommandParams() const
{
    // item->prepareChangeCoordinatesCommandParams() - will be fixed during work on Undo/Redo issues
    auto prepareParams = [](AADLInterfaceGraphicsItem *item) -> QVariantList {
        return { QVariant::fromValue(item->entity()), QVariant::fromValue<QVector<QPointF>>({ item->scenePos() }) };
    };

    QList<QVariantList> params;
    params.append({ QVariant::fromValue(entity()), QVariant::fromValue(graphicsPoints()) });
    params.append(prepareParams(m_startItem));
    params.append(prepareParams(m_endItem));
    return params;
}

void AADLConnectionGraphicsItem::onManualMoveStart(GripPoint *gp, const QPointF &at)
{
    if (!m_gripPointsHandler || !gp)
        return;

    auto grips = m_gripPointsHandler->gripPoints();
    qDebug() << grips;
    const int idx = grips.indexOf(gp);
    if (idx == -1)
        return;

    if (qApp->keyboardModifiers().testFlag(Qt::ControlModifier)) {
        m_points.insert(idx, at);
        auto grip = m_gripPointsHandler->createGripPoint(gp->location(), idx);
        m_gripPointsHandler->setGripPointPos(grip, at);
        grip->stackBefore(gp);
        updateBoundingRect();
    }
}

void AADLConnectionGraphicsItem::updateGripPoints()
{
    if (!m_gripPointsHandler)
        return;

    const QVector<QPointF> points = graphicsPoints();
    if (points.isEmpty())
        return;

    auto grips = m_gripPointsHandler->gripPoints();
    for (int idx = 0; idx < qMax(points.size(), grips.size()); ++idx) {
        if (idx >= points.size()) {
            m_gripPointsHandler->removeGripPoint(grips.value(idx));
            continue;
        }
        if (idx >= grips.size())
            m_gripPointsHandler->createGripPoint(GripPoint::Absolute);
        m_gripPointsHandler->setGripPointPos(grips.value(idx), points.value(idx));
    }
    InteractiveObject::updateGripPoints();
}

void AADLConnectionGraphicsItem::onManualMoveProgress(GripPoint *gp, const QPointF &from, const QPointF &to)
{
    Q_UNUSED(from)

    if (!m_gripPointsHandler || !gp)
        return;

    auto grips = m_gripPointsHandler->gripPoints();
    int idx = grips.indexOf(gp);
    if (idx == -1)
        return;

    m_points[idx] = to;

    if (!qApp->keyboardModifiers().testFlag(Qt::ControlModifier)) {
        if (removeCollidedGrips(gp)) {
            idx = grips.indexOf(gp);
            if (idx == -1)
                return;
        }

        auto updateEdgeItem = [&](InteractiveObject *item) { item->setPos(item->parentItem()->mapFromScene(to)); };
        if (idx == 0)
            updateEdgeItem(m_startItem);
        else if (idx == grips.size() - 1)
            updateEdgeItem(m_endItem);
    }

    updateBoundingRect();
}

void AADLConnectionGraphicsItem::onManualMoveFinish(GripPoint *gp, const QPointF &pressedAt, const QPointF &releasedAt)
{
    if (pressedAt == releasedAt || !gp)
        return;

    auto grips = m_gripPointsHandler->gripPoints();
    int idx = grips.indexOf(gp);
    if (idx == -1)
        return;

    QPointF intersectionPoint { releasedAt };
    if (idx > 0 && idx < m_points.size() - 1) {
        QLineF prevLine = { m_points.value(idx - 1), intersectionPoint };
        if (utils::alignedLine(prevLine))
            intersectionPoint = prevLine.p2();

        QLineF nextLine = { m_points.value(idx + 1), intersectionPoint };
        if (utils::alignedLine(nextLine))
            intersectionPoint = nextLine.p2();
    }
    m_points[idx] = intersectionPoint;

    if (idx == 0) {
        m_startItem->instantLayoutUpdate();
        m_points[idx] = m_startItem->scenePos();
    } else if (idx == grips.size() - 1) {
        m_endItem->instantLayoutUpdate();
        m_points[idx] = m_endItem->scenePos();
    }

    for (auto item : utils::sceneItems(scene(), m_points)) {
        if (auto nestedItem = qobject_cast<aadl::AADLRectGraphicsItem *>(item->toGraphicsObject())) {
            if (utils::intersectionPoints(item->sceneBoundingRect(), QPolygonF(m_points)).size() > 1) {
                rebuildLayout();
                updateEntity();
                return;
            }
        }
    }

    updateBoundingRect();
    updateEntity();
}

void AADLConnectionGraphicsItem::simplify()
{
    m_points = simplify(m_points);
}

QVector<QPointF> AADLConnectionGraphicsItem::simplify(const QVector<QPointF> &points)
{
    if (points.size() <= 3)
        return points;

    /// TODO: optimize flow
    QVector<QPointF> simplifiedPoints(points);

    for (int idx = 0; idx < simplifiedPoints.size() - 1;) {
        const QLineF currentLine { simplifiedPoints.value(idx), simplifiedPoints.value(idx + 1) };
        if (qFuzzyIsNull(currentLine.length())) {
            simplifiedPoints.removeAt(idx + 1);
            continue;
        }
        if (idx + 2 < simplifiedPoints.size()) {
            const QLineF nextLine { simplifiedPoints.value(idx + 1), simplifiedPoints.value(idx + 2) };
            if (qFuzzyIsNull(nextLine.length()) || qFuzzyCompare(currentLine.angle(), nextLine.angle())
                || int(currentLine.angleTo(nextLine)) % 180 == 0) {
                simplifiedPoints.removeAt(idx + 1);
                continue;
            }
        }

        ++idx;
    }
    for (int idx = 1; idx < simplifiedPoints.size() - 2; ++idx) {
        const QLineF currentLine { simplifiedPoints.value(idx), simplifiedPoints.value(idx + 1) };
        const QLineF prevLine { simplifiedPoints.value(idx - 1), simplifiedPoints.value(idx) };
        const QLineF nextLine { simplifiedPoints.value(idx + 1), simplifiedPoints.value(idx + 2) };

        if (qFuzzyCompare(prevLine.angle(), nextLine.angle()) && currentLine.length() < kMinLineLength) {
            const QPointF midPoint = utils::lineCenter(currentLine);
            const QPointF prevOffset = midPoint - currentLine.p1();
            simplifiedPoints[idx - 1] = prevLine.p1() + prevOffset;
            const QPointF nextOffset = midPoint - currentLine.p2();
            simplifiedPoints[idx + 2] = nextLine.p2() + nextOffset;
            simplifiedPoints.removeAt(idx + 1);
            simplifiedPoints.removeAt(idx);
        }
    }
    if (simplifiedPoints.size() == 2)
        return { points.first(), points.last() };
    return simplifiedPoints;
}

void AADLConnectionGraphicsItem::initGripPoints()
{
    InteractiveObject::initGripPoints();
    for (int idx = 0; idx < m_points.size(); ++idx)
        m_gripPointsHandler->createGripPoint(GripPoint::Absolute);
}

ColorManager::HandledColors AADLConnectionGraphicsItem::handledColorType() const
{
    return ColorManager::HandledColors::Connection;
}

void AADLConnectionGraphicsItem::colorSchemeUpdated()
{
    const ColorHandler &h = colorHandler();
    m_item->setPen(h.pen());
    update();
}

bool AADLConnectionGraphicsItem::removeCollidedGrips(GripPoint *gp)
{
    auto grips = m_gripPointsHandler->gripPoints();
    const int idx = grips.indexOf(gp);
    if (idx == -1)
        return false;

    auto it = std::find_if(grips.cbegin(), grips.cend(), [gp](const GripPoint *const grip) {
        return grip != gp && grip->sceneBoundingRect().intersects(gp->sceneBoundingRect());
    });
    if (it != grips.cend()) {
        const int intersectionIdx = std::distance(grips.cbegin(), it);
        const int startIdx = intersectionIdx < idx ? intersectionIdx : idx + 1;
        const int endIdx = intersectionIdx < idx ? idx : intersectionIdx + 1;
        for (int i = endIdx - 1; i >= startIdx; --i) {
            auto item = grips.takeAt(i);
            m_gripPointsHandler->removeGripPoint(item);
            m_points.removeAt(i);
        }
        return true;
    }
    return false;
};

QString AADLConnectionGraphicsItem::prepareTooltip() const
{
    const QString sign = entity()->sourceInterface()->isRequired() ? "->" : "<-";
    const QString tooltip = QString("%1.%2 %3 %4.%5")
                                    .arg(entity()->sourceName(), entity()->sourceInterfaceName(), sign,
                                         entity()->targetName(), entity()->targetInterfaceName());
    return tooltip;
}

} // namespace aadl
} // namespace taste3
