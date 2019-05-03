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

#include "baseitems/common/coordinatesconverter.h"
#include "baseitems/common/utils.h"
#include "chartitem.h"
#include "chartviewmodel.h"
#include "instanceitem.h"
#include "messageitem.h"
#include "mscchart.h"
#include "mscfile.h"
#include "mscinstance.h"
#include "mscmessage.h"
#include "mscmodel.h"
#include "msctimer.h"
#include "timeritem.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QScopedPointer>
#include <QVector>
#include <QtTest>
#include <mscdocument.h>

using namespace msc;

class tst_ChartViewModel : public QObject
{
    Q_OBJECT

private:
    void parseMsc(const QString &mscText);

private Q_SLOTS:
    void init();
    void cleanup();
    void testNearestInstanceSimple();
    void testNearestInstanceCreate();

    void testTimerPositionWithCifInstance();
    void testLoadedMessagePosition();
    void testLoadedCifMessagePosition();
    void testDefaultChartSize();
    void testInstanceCifExtendedChartWidth();
    void testCifExtendedChartWidth();

private:
    QGraphicsView m_view;

    QScopedPointer<ChartViewModel> m_chartModel;
    QScopedPointer<MscModel> m_mscModel;
    QPointer<MscChart> m_chart;
    QVector<MscInstance *> m_instances;
    QVector<InstanceItem *> m_instanceItems;
    QVector<QRectF> m_instanceRects;

    const QSizeF defaultSize = { 200.0, 200.0 };
    const qreal m_maxOffset = 1.5; // used for size comparisons
};

void tst_ChartViewModel::parseMsc(const QString &mscText)
{
    MscFile mscFile;
    m_mscModel.reset(mscFile.parseText(mscText));

    if (m_mscModel->charts().isEmpty()) {
        MscDocument *doc = m_mscModel->documents().at(0);
        while (doc->charts().isEmpty()) {
            doc = doc->documents().at(0);
        }
        m_chart = doc->charts().at(0);
    } else {
        m_chart = m_mscModel->charts().at(0);
    }
    m_chartModel->fillView(m_chart);
    QApplication::processEvents();

    for (MscInstance *instance : m_chart->instances()) {
        InstanceItem *instanceItem = m_chartModel->itemForInstance(instance);
        QVERIFY(instanceItem != nullptr);

        m_instances.append(instance);
        m_instanceItems.append(instanceItem);
        m_instanceRects.append(instanceItem->sceneBoundingRect());
    }
}

void tst_ChartViewModel::init()
{
    m_chartModel.reset(new ChartViewModel);
    m_view.setScene(m_chartModel->graphicsScene());
    msc::utils::CoordinatesConverter::instance()->setScene(m_chartModel->graphicsScene());
    msc::utils::CoordinatesConverter::setPhysicalDPI(QPoint(254, 254)); // results in cif <-> pixel as 1:1
    msc::utils::CoordinatesConverter::setLogicalDPI(QPoint(254, 254));
}

void tst_ChartViewModel::cleanup()
{
    m_instances.clear();
    m_instanceItems.clear();
    m_instanceRects.clear();
}

void tst_ChartViewModel::testNearestInstanceSimple()
{
    QString mscText = "MSC msc1; \
                          INSTANCE inst1; \
                              OUT Msg01 TO inst2; \
                          ENDINSTANCE; \
                          INSTANCE inst2; \
                              IN Msg01 FROM inst1; \
                          ENDINSTANCE; \
                       ENDMSC;";
    parseMsc(mscText);

    // point on the left
    MscInstance *inst = m_chartModel->nearestInstance({ m_instanceRects[0].left() - 10., 50. });
    QCOMPARE(inst, m_instances[0]);
    // point on the left
    inst = m_chartModel->nearestInstance({ m_instanceRects[1].left() + 10., 50. });
    QCOMPARE(inst, m_instances[1]);
    // point between itens, but closer to left
    qreal middle = m_instanceRects[0].right() + 20.;
    inst = m_chartModel->nearestInstance({ middle, 50. });
    QCOMPARE(inst, m_instances[0]);

    // point too far on the left
    inst = m_chartModel->nearestInstance({ m_instanceRects[0].left() - 100., 50. });
    QCOMPARE(inst, static_cast<MscInstance *>(nullptr));
    // point too far on the left
    inst = m_chartModel->nearestInstance({ m_instanceRects[1].right() + 100., 50. });
    QCOMPARE(inst, static_cast<MscInstance *>(nullptr));
}

void tst_ChartViewModel::testNearestInstanceCreate()
{
    QString mscText = "MSC msc1; \
                          INSTANCE Instance_A; \
                              ACTION 'Boot'; \
                              CREATE New_Instance1; \
                              IN Msg01 FROM New_Instance1; \
                          ENDINSTANCE; \
                          INSTANCE New_Instance1; \
                              OUT Msg01 TO Instance_A; \
                          ENDINSTANCE; \
                       ENDMSC;";
    parseMsc(mscText);
    QCOMPARE(m_instanceItems.size(), 2);

    // point on the left
    MscInstance *inst = m_chartModel->nearestInstance(m_instanceRects[1].center());
    QCOMPARE(inst, m_instances[1]);

    // point above the create instance (but on height of first instance)
    inst = m_chartModel->nearestInstance({ m_instanceRects[1].center().x(), m_instanceRects[0].top() });
    QCOMPARE(inst, static_cast<MscInstance *>(nullptr));
}

void tst_ChartViewModel::testTimerPositionWithCifInstance()
{
    QString mscText = "MSC alarm; \
            /* CIF TextMode 4 */ \
            /* CIF Modified */ \
            /* CIF INSTANCE (261, 65), (349, 150), (800, 696) */ \
            INSTANCE inst_1_cu_nominal.cu_controller : PROCESS /* MSC AT [206] */ ; \
                /* CIF TIMEOUT (436, 303), (95, 110) */ \
                /* CIF TextMode 3 */ \
                /* CIF Modified */ \
                TIMEOUT watchdog /* MSC AT [195] */ ; \
            ENDINSTANCE; \
            ENDMSC;";
    parseMsc(mscText);
    QCOMPARE(m_instanceItems.size(), 1);
    QCOMPARE(m_chart->instanceEvents().size(), 1);

    MscTimer *watchdogEntity = qobject_cast<MscTimer *>(m_chart->instanceEvents().at(0));
    TimerItem *watchdogItem = m_chartModel->itemForTimer(watchdogEntity);
    QVERIFY(watchdogItem != nullptr);

    // Check that the timer is below the instance head
    const QPointF instanceHeadBottom = m_instanceItems[0]->axis().p1();
    QVERIFY(watchdogItem->scenePos().y() > instanceHeadBottom.y());
}

void tst_ChartViewModel::testLoadedMessagePosition()
{
    QString mscText = "msc Untitled_MSC;\
                            instance Instance_1;\
                                out A to env;\
                            endinstance;\
                        endmsc;";
    parseMsc(mscText);
    QCOMPARE(m_instanceItems.size(), 1);
    QCOMPARE(m_chart->instanceEvents().size(), 1);

    ChartItem *chartItem = m_chartModel->chartItem();
    MscMessage *message = qobject_cast<MscMessage *>(m_chart->instanceEvents().at(0));
    MessageItem *messageItem = m_chartModel->itemForMessage(message);
    QVERIFY(messageItem != nullptr);

    // Check that the message is below the instance head
    const QPointF instanceHeadBottom = m_instanceItems[0]->axis().p1();
    QVERIFY(messageItem->scenePos().y() > instanceHeadBottom.y());
    // Check that the message is above the instance end
    const QPointF instanceEndTop = m_instanceItems[0]->axis().p2();
    QVERIFY(messageItem->sceneBoundingRect().bottom() < instanceEndTop.y());

    // Chart geometry is forced to default minimum (200)
    QVERIFY(qFuzzyCompare(chartItem->contentRect().width(), defaultSize.width()));
    QVERIFY(chartItem->contentRect().height() <= defaultSize.height());
    // message points to the left
    QVector<QPointF> points = messageItem->messagePoints();
    QVERIFY(qFuzzyCompare(points.at(0).x(), m_instanceItems[0]->axis().p1().x()));
    QVERIFY(qFuzzyCompare(points.at(1).x(), chartItem->sceneBoundingRect().left()));
}

void tst_ChartViewModel::testLoadedCifMessagePosition()
{
    QSKIP("Force to 200x200 does not work");

    QString mscText = "msc Untitled_MSC;\
                            instance Instance_1;\
                            /* CIF MESSAGE (37, 208) (3002, 681) */\
                                out A to env;\
                            endinstance;\
                        endmsc;";
    parseMsc(mscText);
    QCOMPARE(m_instanceItems.size(), 1);
    QCOMPARE(m_chart->instanceEvents().size(), 1);

    ChartItem *chartItem = m_chartModel->chartItem();
    MscMessage *message = qobject_cast<MscMessage *>(m_chart->instanceEvents().at(0));
    MessageItem *messageItem = m_chartModel->itemForMessage(message);
    QVERIFY(messageItem != nullptr);

    const QRectF msgRect = messageItem->sceneBoundingRect().normalized();
    const QRectF insRect = m_instanceItems[0]->sceneBoundingRect().normalized();
    QVERIFY(qFuzzyCompare(chartItem->contentRect().height(), (msgRect | insRect).height()));
    QVERIFY(qFuzzyCompare(insRect.height(), chartItem->contentRect().height()));

    // message should be non horizontal (from instance to the right edge)
    const QVector<QPointF> &points = messageItem->messagePoints();
    const QPointF &center = insRect.center();

    QVERIFY(std::abs(points.at(0).x() - center.x()) <= m_maxOffset);
    QVERIFY(qFuzzyCompare(points.at(1).x(), chartItem->sceneBoundingRect().right()));
}

void tst_ChartViewModel::testDefaultChartSize()
{
    QString mscText = "msc Untitled_MSC;\
                            instance Instance_1;\
                            endinstance;\
                        endmsc;";
    parseMsc(mscText);

    QPointer<ChartItem> chartItem = m_chartModel->chartItem();
    // content area is forced to default minimum size
    QVERIFY(qFuzzyCompare(chartItem->contentRect().width(), defaultSize.width()));
    QVERIFY(qFuzzyCompare(chartItem->contentRect().height(), defaultSize.height()));
}

void tst_ChartViewModel::testInstanceCifExtendedChartWidth()
{
    QString mscText = "mscdocument Untitled_Document /* MSC AND */;\
                      mscdocument Untitled_Leaf /* MSC LEAF */;\
                          msc Untitled_MSC;\
                              instance Instance_1;\
                              endinstance;\
                              /* CIF INSTANCE (695, 42) (72, 31) (800, 264) */\
                              instance Instance_2;\
                              endinstance;\
                          endmsc;\
                      endmscdocument;\
                  endmscdocument;";
    parseMsc(mscText);

    QPointer<ChartItem> chartItem = m_chartModel->chartItem();
    // content area width and height are stretched accordingly to the instance1's CIF
    const QRectF &inst2Rect = m_instanceItems[1]->sceneBoundingRect();
    QVERIFY(qFuzzyCompare(chartItem->contentRect().width(), inst2Rect.right()));
    QVERIFY(qFuzzyCompare(chartItem->contentRect().height(), inst2Rect.height() + inst2Rect.top()));
}

void tst_ChartViewModel::testCifExtendedChartWidth()
{
    const QSizeF chartSize(1425, 575);
    const QString mscText = QString("mscdocument Untitled_Document /* MSC AND */;\
                      /* CIF MSCDOCUMENT (0, 0) (%1, %2) */\
                          mscdocument Untitled_Leaf /* MSC LEAF */;\
                              msc Untitled_MSC;\
                                  /* CIF INSTANCE (163, 42) (143, 62) (800, 264) */\
                                  instance Instance_1;\
                                  endinstance;\
                                  instance Instance_2;\
                                  endinstance;\
                              endmsc;\
                          endmscdocument;\
                      endmscdocument;")
                                    .arg(chartSize.width())
                                    .arg(chartSize.height());
    parseMsc(mscText);

    const QSizeF &chartBoxSize = m_chartModel->chartItem()->contentRect().size();
    QVERIFY(std::abs(chartBoxSize.width() - chartSize.width()) < m_maxOffset);
    QVERIFY(std::abs(chartBoxSize.height() - chartSize.height()) < m_maxOffset);
}

QTEST_MAIN(tst_ChartViewModel)

#include "tst_chartviewmodel.moc"
