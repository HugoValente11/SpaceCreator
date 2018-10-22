#include "messageitem.h"
#include "instanceitem.h"

#include <mscmessage.h>

#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QPolygonF>

namespace msc {

static const double ARROW_HEIGHT = 10.0;
static const double ARROW_WIDTH = 20.0;
static const double DEFAULT_WIDTH = 80.0;

MessageItem::MessageItem(MscMessage *message, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_message(message)
    , m_line(new QGraphicsLineItem(this))
    , m_leftArrow(new QGraphicsPolygonItem(this))
    , m_rightArrow(new QGraphicsPolygonItem(this))
    , m_nameItem(new QGraphicsTextItem(this))
{
    Q_ASSERT(m_message != nullptr);
    m_leftArrow->setBrush(QBrush(Qt::black));
    QPolygonF polygon;
    polygon.append(QPointF(0.0, ARROW_HEIGHT / 2.0));
    polygon.append(QPointF(ARROW_WIDTH, 0.0));
    polygon.append(QPointF(ARROW_WIDTH, ARROW_HEIGHT));
    m_leftArrow->setPolygon(polygon);

    m_rightArrow->setBrush(QBrush(Qt::black));
    polygon.clear();
    polygon.append(QPointF(0.0, 0.0));
    polygon.append(QPointF(ARROW_WIDTH, ARROW_HEIGHT / 2.0));
    polygon.append(QPointF(0.0, ARROW_HEIGHT));
    m_rightArrow->setPolygon(polygon);

    setName(m_message->name());
    connect(m_message, &msc::MscMessage::nameChanged, this, &msc::MessageItem::setName);

    updateLayout();
}

QRectF MessageItem::boundingRect() const
{
    QRectF rect = childrenBoundingRect();
    rect.moveTo(pos());
    return rect;
}

void MessageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

void MessageItem::setSourceInstanceItem(InstanceItem *sourceInstance)
{
    if (sourceInstance == m_sourceInstance) {
        return;
    }

    m_sourceInstance = sourceInstance;
    if (sourceInstance) {
        connect(m_sourceInstance, &InstanceItem::horizontalCenterChanged, this, &MessageItem::buildLayout);
    }
    updateLayout();
}

void MessageItem::setTargetInstanceItem(InstanceItem *targetInstance)
{
    if (targetInstance == m_targetInstance) {
        return;
    }

    m_targetInstance = targetInstance;
    if (m_targetInstance != nullptr) {
        connect(m_targetInstance, &InstanceItem::horizontalCenterChanged, this, &MessageItem::buildLayout);
    }
    updateLayout();
}

QString MessageItem::name() const
{
    return m_nameItem->toPlainText();
}

void MessageItem::updateLayout()
{
    if (m_layoutDirty) {
        return;
    }

    m_layoutDirty = true;
    QMetaObject::invokeMethod(this, "buildLayout", Qt::QueuedConnection);
}

void MessageItem::setName(const QString &name)
{
    m_nameItem->setPlainText(name);
    updateLayout();
}

void MessageItem::buildLayout()
{
    double arrowY = m_nameItem->boundingRect().height();
    m_leftArrow->setY(arrowY);
    m_rightArrow->setY(arrowY);

    if (m_sourceInstance && m_targetInstance) {
        double x1 = m_sourceInstance->horizontalCenter();
        double x2 = m_targetInstance->horizontalCenter();

        if (x1 > x2) {
            double tmp = x1;
            x1 = x2;
            x2 = tmp;
            m_leftArrow->setVisible(true);
            m_rightArrow->setVisible(false);
        } else {
            m_leftArrow->setVisible(false);
            m_rightArrow->setVisible(true);
        }
        setX(x1);
        setWidth(x2 - x1);
    }
    if (m_sourceInstance && m_targetInstance == nullptr) {
        setX(m_sourceInstance->horizontalCenter());
        setWidth(DEFAULT_WIDTH);
        m_leftArrow->setVisible(false);
        m_rightArrow->setVisible(true);
    }
    if (m_sourceInstance == nullptr && m_targetInstance) {
        setX(m_targetInstance->horizontalCenter() - DEFAULT_WIDTH);
        setWidth(DEFAULT_WIDTH);
        m_leftArrow->setVisible(false);
        m_rightArrow->setVisible(true);
    }

    centerName();

    prepareGeometryChange();
    update();

    m_layoutDirty = false;
}

void MessageItem::setWidth(double width)
{
    m_rightArrow->setX(width - ARROW_WIDTH);
    double y = m_nameItem->boundingRect().height() + ARROW_HEIGHT / 2.0;
    m_line->setLine(0.0, y, width, y);
}

void MessageItem::centerName()
{
    m_nameItem->setX((m_line->boundingRect().width() - m_nameItem->boundingRect().width()) / 2.0);
}

} // namespace mas
