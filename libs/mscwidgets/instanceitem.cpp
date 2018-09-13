#include "instanceitem.h"

#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>

namespace msc {

static const double SYMBOLS_WIDTH = 60.0;
static const double START_SYMBOL_HEIGHT = 20.0;
static const double END_SYMBOL_HEIGHT = 15.0;

InstanceItem::InstanceItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_headSymbol(new QGraphicsRectItem(this))
    , m_nameItem(new QGraphicsTextItem(this))
    , m_kindItem(new QGraphicsTextItem(this))
    , m_axisSymbol(new QGraphicsLineItem(this))
    , m_endSymbol(new QGraphicsRectItem(this))
{
    m_endSymbol->setBrush(QBrush(Qt::black));
}

QRectF InstanceItem::boundingRect() const
{
    QRectF rect = childrenBoundingRect();
    rect.moveTo(pos());
    return rect;
}

void InstanceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

double InstanceItem::horizontalCenter() const
{
    return x() + m_axisSymbol->line().x1();
}

void InstanceItem::setName(const QString &name)
{
    m_nameItem->setPlainText(name);
    buildLayout();
}

void InstanceItem::setKind(const QString &kind)
{
    m_kindItem->setPlainText(kind);
    buildLayout();
}

QVariant InstanceItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        emit horizontalCenterChanged();
    }

    return QGraphicsObject::itemChange(change, value);
}

void InstanceItem::buildLayout()
{
    QRectF nameRect = m_nameItem->boundingRect();

    const double centerX = std::max(nameRect.width(), SYMBOLS_WIDTH) / 2.0;

    const double nameX = centerX - nameRect.center().x();
    m_nameItem->setX(nameX);

    const double kindY = nameRect.bottom();
    QRectF kindRect = m_kindItem->boundingRect();
    const double kindX = centerX - kindRect.center().x();
    m_kindItem->setX(kindX);
    m_kindItem->setY(kindY);

    QRectF headSymbolRect = nameRect;
    headSymbolRect.moveTop(nameRect.bottom());
    const double headSymbolHeight = std::max(START_SYMBOL_HEIGHT, kindRect.height());
    headSymbolRect.setHeight(headSymbolHeight);
    headSymbolRect.setWidth(SYMBOLS_WIDTH);
    const double symbolX = centerX - headSymbolRect.center().x();
    headSymbolRect.moveLeft(symbolX);
    m_headSymbol->setRect(headSymbolRect);

    double y = headSymbolRect.bottom();
    m_axisSymbol->setLine(centerX, y, centerX, y + 150.0);

    y = m_axisSymbol->boundingRect().bottom();
    m_endSymbol->setRect(symbolX, y, headSymbolRect.width(), END_SYMBOL_HEIGHT);

    emit horizontalCenterChanged();
    prepareGeometryChange();
    update();
}

} // namespace msc
