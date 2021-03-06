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

#include "exceptions.h"
#include "mscaction.h"
#include "mscchart.h"
#include "msccomment.h"
#include "msccondition.h"
#include "msccoregion.h"
#include "msccreate.h"
#include "mscgate.h"
#include "mscinstance.h"
#include "mscmessage.h"
#include "msctimer.h"

#include <QtTest>

using namespace msc;

class tst_MscChart : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();
    void testDestructor();
    void testAddInstance();
    void testRemoveInstance();
    void testNoDuplicateInstance();
    void testNoNullPtrInstance();
    void testInstanceByName();
    void testAddActions();
    void testAddMessage();
    void testRemoveMessage();
    void testNoDuplicateMessage();
    void testNoNullPtrMessage();
    void testMessageByName();
    void testAddGate();
    void testRemoveGate();
    void testAddCondition();
    void testAddSharedCondition();
    void testAddSharedConditionMix();
    void testRemoveCondition();
    void testTimerRelation();
    void testInvalidTimerRelation();
    void testMaxInstanceNameNumber();
    void testSetInstanceNameNumber();
    void testMoveCoregion();
    void testCreateMessage();
    void testAddCreateAfterMessage();
    void testIndicesOfEvent();
    void testIsCrossingMessage();

private:
    MscChart *m_chart = nullptr;
};

void tst_MscChart::init()
{
    m_chart = new MscChart;
}

void tst_MscChart::cleanup()
{
    delete m_chart;
    m_chart = nullptr;
}

void tst_MscChart::testDestructor()
{
    MscInstance *instance = nullptr;
    QVector<QPointer<MscEntity>> chartEntities;
    MscChart *chart = new MscChart;
    const QMetaEnum &chartEntitiesMeta = QMetaEnum::fromType<MscEntity::EntityType>();
    for (int i = 0; i < chartEntitiesMeta.keyCount(); ++i) {
        const MscEntity::EntityType entityType = static_cast<MscEntity::EntityType>(chartEntitiesMeta.value(i));
        switch (entityType) {
        case MscEntity::EntityType::Chart:
        case MscEntity::EntityType::Document:
            break;
        case MscEntity::EntityType::Instance: {
            instance = new MscInstance();
            chart->addInstance(instance);
            chartEntities.append(instance);
            break;
        }
        case MscEntity::EntityType::Message: {
            auto message = new MscMessage("name", instance, nullptr);
            chart->addInstanceEvent(message, { { instance, -1 } });
            chartEntities.append(message);
            break;
        }
        case MscEntity::EntityType::Timer: {
            auto timer = new MscTimer("name", instance, MscTimer::TimerType::Start);
            chart->addInstanceEvent(timer, { { instance, -1 } });
            chartEntities.append(timer);
            break;
        }
        case MscEntity::EntityType::Gate: {
            auto gate = new MscGate();
            chart->addGate(gate);
            chartEntities.append(chart->gates().first());
            break;
        }
        case MscEntity::EntityType::Condition: {
            auto condition = new MscCondition();
            condition->setInstance(instance);
            chart->addInstanceEvent(condition, { { instance, -1 } });
            chartEntities.append(condition);
            break;
        }
        case MscEntity::EntityType::Action: {
            auto action = new MscAction;
            action->setInstance(instance);
            chart->addInstanceEvent(action, { { instance, -1 } });
            chartEntities.append(action);
            break;
        }
        case MscEntity::EntityType::Coregion: {
            auto coregion = new MscCoregion(instance, MscCoregion::Type::Begin);
            chart->addInstanceEvent(coregion, { { instance, -1 } });
            chartEntities.append(coregion);
            break;
        }
        case MscEntity::EntityType::Create: {
            auto create = new MscCreate("start", instance, nullptr);
            chart->addInstanceEvent(create, { { instance, -1 } });
            chartEntities.append(create);
            break;
        }
        case MscEntity::EntityType::Comment: {
            auto comment = new MscComment;
            Q_ASSERT(instance);
            comment->setCommentString(QLatin1String("Text Comment for the First Instance"));
            comment->attachTo(instance);
            instance->setComment(comment);
            chart->addInstanceEvent(comment, {});
            chartEntities.append(comment);
        } break;
        default:
            QFAIL("It seems a new MscEntity::EntityType has been introduced,\n"
                  "but it's not covered here.\n"
                  "Please add it to process or ignore explicitly.");
            break;
        }
    }

    QCOMPARE(chart->instances().size(), 1);
    QCOMPARE(chart->totalEventNumber(), 7);
    QCOMPARE(chart->gates().size(), 1);

    delete chart;

    for (const QPointer<MscEntity> &chartEntity : chartEntities)
        QCOMPARE(chartEntity, QPointer<MscEntity>(nullptr));
}

void tst_MscChart::testAddInstance()
{
    QCOMPARE(m_chart->instances().size(), 0);

    auto instance1 = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance1);
    auto instance2 = new MscInstance("OUT", m_chart);
    m_chart->addInstance(instance2);
    QCOMPARE(m_chart->instances().size(), 2);
}

void tst_MscChart::testRemoveInstance()
{
    QCOMPARE(m_chart->instances().size(), 0);

    QScopedPointer<MscInstance> instance1(new MscInstance("IN", m_chart));
    QScopedPointer<MscInstance> instance2(new MscInstance("OUT", m_chart));

    m_chart->addInstance(instance1.data());
    m_chart->addInstance(instance2.data());
    QCOMPARE(m_chart->instances().size(), 2);

    m_chart->removeInstance(instance1.data());
    QCOMPARE(m_chart->instances().size(), 1);
    m_chart->removeInstance(instance2.data());
    QCOMPARE(m_chart->instances().size(), 0);
}

void tst_MscChart::testNoDuplicateInstance()
{
    auto instance = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance);
    m_chart->addInstance(instance);
    QCOMPARE(m_chart->instances().size(), 1);
}

void tst_MscChart::testNoNullPtrInstance()
{
    m_chart->addInstance(nullptr);
    QCOMPARE(m_chart->instances().size(), 0);
}

void tst_MscChart::testInstanceByName()
{
    auto instance = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance);
    QCOMPARE(m_chart->instanceByName("IN"), instance);
    QCOMPARE(m_chart->instanceByName("OUT"), static_cast<MscInstance *>(nullptr));
}

void tst_MscChart::testAddActions()
{
    auto instance1 = new MscInstance("Sender", m_chart);
    m_chart->addInstance(instance1);
    auto instance2 = new MscInstance("Receiver", m_chart);
    m_chart->addInstance(instance2);

    auto actionA1 = new MscAction("Start");
    actionA1->setInstance(instance1);
    m_chart->addInstanceEvent(actionA1, { { instance1, 0 } });
    QCOMPARE(m_chart->eventsForInstance(instance1).size(), 1);
    QCOMPARE(m_chart->eventsForInstance(instance2).size(), 0);

    auto action_fail = new MscAction("Ooops");
    action_fail->setInstance(instance1);
    m_chart->addInstanceEvent(action_fail, { { instance2, 0 } }); // ignored as using the wrong instance
    QCOMPARE(m_chart->eventsForInstance(instance1).size(), 1);
    QCOMPARE(m_chart->eventsForInstance(instance2).size(), 0);

    auto actionB1 = new MscAction("Wait");
    actionB1->setInstance(instance2);
    m_chart->addInstanceEvent(actionB1, { { instance2, -1 } });
    QCOMPARE(m_chart->eventsForInstance(instance1).size(), 1);
    QCOMPARE(m_chart->eventsForInstance(instance2).size(), 1);

    auto actionA2 = new MscAction("Stop");
    actionA2->setInstance(instance1);
    m_chart->addInstanceEvent(actionA2, { { instance1, 1 } });
    auto actionA3 = new MscAction("ReStart");
    actionA3->setInstance(instance1);
    m_chart->addInstanceEvent(actionA3, { { instance1, 1 } }); // insert between other actions
    QCOMPARE(m_chart->eventsForInstance(instance1).size(), 3);
    QCOMPARE(m_chart->eventsForInstance(instance2).size(), 1);

    const QVector<MscInstanceEvent *> events1 = m_chart->eventsForInstance(instance1);
    const QVector<MscInstanceEvent *> ev1 = { actionA1, actionA3, actionA2 };
    QCOMPARE(events1, ev1);

    const QVector<MscInstanceEvent *> events2 = m_chart->eventsForInstance(instance2);
    const QVector<MscInstanceEvent *> ev2 = { actionB1 };
    QCOMPARE(events2, ev2);
}

void tst_MscChart::testAddMessage()
{
    auto instanceA = new MscInstance("Sender", m_chart);
    m_chart->addInstance(instanceA);
    auto instanceB = new MscInstance("Receiver", m_chart);
    m_chart->addInstance(instanceB);

    QCOMPARE(m_chart->totalEventNumber(), 0);

    auto message1 = new MscMessage("IN", instanceA, instanceB, m_chart);
    m_chart->addInstanceEvent(message1, { { instanceA, 0 }, { instanceB, 0 } });

    // crossing message
    auto message2 = new MscMessage("OUT", instanceB, instanceA, m_chart);
    m_chart->addInstanceEvent(message2, { { instanceA, 1 }, { instanceB, 0 } });
    QCOMPARE(m_chart->totalEventNumber(), 2);
}

void tst_MscChart::testRemoveMessage()
{
    MscInstance *instance1 = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance1);
    QCOMPARE(m_chart->totalEventNumber(), 0);

    QScopedPointer<MscMessage> message1(new MscMessage("IN", m_chart));
    message1->setSourceInstance(instance1);
    QScopedPointer<MscMessage> message2(new MscMessage("OUT", m_chart));
    message2->setTargetInstance(instance1);

    m_chart->addInstanceEvent(message1.data(), { { instance1, -1 } });
    m_chart->addInstanceEvent(message2.data(), { { instance1, -1 } });
    QCOMPARE(m_chart->totalEventNumber(), 2);

    m_chart->removeInstanceEvent(message1.data());
    QCOMPARE(m_chart->totalEventNumber(), 1);
    m_chart->removeInstanceEvent(message2.data());
    QCOMPARE(m_chart->totalEventNumber(), 0);
}

void tst_MscChart::testNoDuplicateMessage()
{
    MscInstance *instance1 = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance1);
    auto message = new MscMessage("IN", m_chart);
    message->setTargetInstance(instance1);
    m_chart->addInstanceEvent(message, { { instance1, -1 } });
    m_chart->addInstanceEvent(message, { { instance1, -1 } });
    QCOMPARE(m_chart->totalEventNumber(), 1);
}

void tst_MscChart::testNoNullPtrMessage()
{
    m_chart->addInstanceEvent(nullptr, {});
    QCOMPARE(m_chart->totalEventNumber(), 0);
}

void tst_MscChart::testMessageByName()
{
    MscInstance *instance1 = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance1);
    auto message = new MscMessage("IN", m_chart);
    message->setSourceInstance(instance1);
    m_chart->addInstanceEvent(message, { { instance1, -1 } });
    QCOMPARE(m_chart->messageByName("IN"), message);
    QCOMPARE(m_chart->messageByName("OUT"), static_cast<MscMessage *>(nullptr));
}

void tst_MscChart::testAddGate()
{
    auto gate = new MscGate("testGateName", m_chart);
    m_chart->addGate(gate);
    QCOMPARE(m_chart->gates().size(), 1);
    m_chart->addGate(gate);
    QCOMPARE(m_chart->gates().size(), 1);
    m_chart->addGate(nullptr);
    QCOMPARE(m_chart->gates().size(), 1);
}

void tst_MscChart::testRemoveGate()
{
    QScopedPointer<MscGate> gate1(new MscGate("A", m_chart));
    m_chart->addGate(gate1.data());
    MscGate *gate2 = new MscGate("B", m_chart);
    m_chart->addGate(gate2);
    QCOMPARE(m_chart->gates().size(), 2);
    m_chart->removeGate(nullptr);
    QCOMPARE(m_chart->gates().size(), 2);
    auto gateOne = m_chart->gates().first();
    m_chart->removeGate(gateOne);
    QCOMPARE(m_chart->gates().size(), 1);
    m_chart->removeGate(gateOne);
    QCOMPARE(m_chart->gates().size(), 1);
}

void tst_MscChart::testAddCondition()
{
    MscInstance *instance1 = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance1);

    auto condition = new MscCondition("Condition_1", m_chart);
    condition->setInstance(instance1);

    m_chart->addInstanceEvent(condition, { { instance1, -1 } });
    QCOMPARE(m_chart->totalEventNumber(), 1);

    m_chart->addInstanceEvent(condition, { { instance1, -1 } });
    QCOMPARE(m_chart->totalEventNumber(), 1);

    m_chart->addInstanceEvent(nullptr, {});
    QCOMPARE(m_chart->totalEventNumber(), 1);
}

void tst_MscChart::testAddSharedCondition()
{
    auto instanceA = new MscInstance("IA");
    m_chart->addInstance(instanceA);
    auto instanceB = new MscInstance("IB");
    m_chart->addInstance(instanceB);

    auto condition1 = new MscCondition("C_1");
    condition1->setInstance(instanceB);
    m_chart->addInstanceEvent(condition1, { { instanceB, -1 } });
    QCOMPARE(m_chart->totalEventNumber(), 1);

    auto condition2 = new MscCondition("C_S1");
    condition2->setInstance(instanceA);
    condition2->setShared(true);
    m_chart->addInstanceEvent(condition2, { { instanceA, 0 }, { instanceB, 1 } });
    QCOMPARE(m_chart->totalEventNumber(), 2);

    QVector<MscInstanceEvent *> events = m_chart->eventsForInstance(instanceA);
    QCOMPARE(events.at(0), condition2);
    events = m_chart->eventsForInstance(instanceB);
    QCOMPARE(events.at(0), condition1);
    QCOMPARE(events.at(1), condition2);
}

void tst_MscChart::testAddSharedConditionMix()
{
    auto instanceA = new MscInstance("IA");
    m_chart->addInstance(instanceA);
    auto instanceB = new MscInstance("IB");
    m_chart->addInstance(instanceB);
    auto instanceC = new MscInstance("IC");
    m_chart->addInstance(instanceC);

    MscTimer *timer1 = new MscTimer("T1", MscTimer::TimerType::Stop);
    timer1->setInstance(instanceC);
    QHash<MscInstance *, int> indexes = { { instanceC, -1 } };
    m_chart->addInstanceEvent(timer1, { { instanceC, -1 } });

    auto condition1 = new MscCondition("C_1");
    condition1->setInstance(instanceB);
    m_chart->addInstanceEvent(condition1, { { instanceB, -1 } });
    QCOMPARE(m_chart->totalEventNumber(), 2);
    QCOMPARE(m_chart->totalEventNumber(), 2);

    auto condition2 = new MscCondition("C_S1");
    condition2->setInstance(instanceB);
    condition2->setShared(true);
    m_chart->addInstanceEvent(condition2, { { instanceA, 0 }, { instanceB, 1 }, { instanceC, 1 } });
    QCOMPARE(m_chart->totalEventNumber(), 3);

    QVector<MscInstanceEvent *> events = m_chart->eventsForInstance(instanceA);
    QCOMPARE(events.at(0), condition2);
    events = m_chart->eventsForInstance(instanceB);
    QCOMPARE(events.at(0), condition1);
    QCOMPARE(events.at(1), condition2);
    events = m_chart->eventsForInstance(instanceC);
    QCOMPARE(events.at(0), timer1);
    QCOMPARE(events.at(1), condition2);
}

void tst_MscChart::testRemoveCondition()
{
    MscInstance *instance1 = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance1);
    QScopedPointer<MscCondition> condition1(new MscCondition("Condition_1", m_chart));
    condition1->setInstance(instance1);
    m_chart->addInstanceEvent(condition1.data(), { { instance1, -1 } });
    MscCondition *condition2 = new MscCondition("Condition_2", m_chart);
    condition2->setInstance(instance1);
    m_chart->addInstanceEvent(condition2, { { instance1, -1 } });

    QCOMPARE(m_chart->totalEventNumber(), 2);

    m_chart->removeInstanceEvent(nullptr);
    QCOMPARE(m_chart->totalEventNumber(), 2);

    m_chart->removeInstanceEvent(condition1.get());
    QCOMPARE(m_chart->totalEventNumber(), 1);

    m_chart->removeInstanceEvent(condition1.get());
    QCOMPARE(m_chart->totalEventNumber(), 1);
}

void tst_MscChart::testTimerRelation()
{
    MscInstance *instance1 = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance1);
    MscTimer *timer1 = new MscTimer("T1", MscTimer::TimerType::Start);
    timer1->setInstance(instance1);
    m_chart->addInstanceEvent(timer1, { { instance1, -1 } });
    MscTimer *timer2 = new MscTimer("T1", MscTimer::TimerType::Stop);
    timer2->setInstance(instance1);
    m_chart->addInstanceEvent(timer2, { { instance1, -1 } });
    QCOMPARE(timer1->followingTimer(), timer2);
    MscTimer *timer3 = new MscTimer("T3", MscTimer::TimerType::Timeout);
    timer3->setInstance(instance1);
    m_chart->addInstanceEvent(timer3, { { instance1, -1 } });

    MscInstance *instance2 = new MscInstance("OUT", m_chart);
    m_chart->addInstance(instance2);
    MscTimer *timer4 = new MscTimer("T1", MscTimer::TimerType::Start);
    timer4->setInstance(instance2);
    m_chart->addInstanceEvent(timer4, { { instance2, -1 } });

    QVERIFY(timer1->precedingTimer() == nullptr);
    QCOMPARE(timer1->followingTimer(), timer2);

    QCOMPARE(timer2->precedingTimer(), timer1);
    QVERIFY(timer2->followingTimer() == nullptr);

    QVERIFY(timer3->precedingTimer() == nullptr);
    QVERIFY(timer3->followingTimer() == nullptr);

    // update on name change
    timer2->setName("T3");
    QVERIFY(timer1->precedingTimer() == nullptr);
    QCOMPARE(timer1->name(), QString("T3"));
    QCOMPARE(timer1->followingTimer(), timer2);
    QCOMPARE(timer2->precedingTimer(), timer1);
    QVERIFY(timer2->followingTimer() == nullptr);
    QVERIFY(timer3->precedingTimer() == nullptr);
    QVERIFY(timer3->followingTimer() == nullptr);
    QVERIFY(timer4->precedingTimer() == nullptr);
    QVERIFY(timer4->followingTimer() == nullptr);

    // update on type change
    timer2->setTimerType(MscTimer::TimerType::Start);
    m_chart->resetTimerRelations(timer2);
    QVERIFY(timer1->precedingTimer() == nullptr);
    QCOMPARE(timer1->followingTimer(), timer2);
    QCOMPARE(timer2->precedingTimer(), timer1);
    QCOMPARE(timer3->precedingTimer(), timer2);
    QVERIFY(timer3->followingTimer() == nullptr);
    QVERIFY(timer4->precedingTimer() == nullptr);

    // update on new timer
    MscTimer *timer5 = new MscTimer("T1", MscTimer::TimerType::Timeout);
    timer5->setInstance(instance2);
    m_chart->addInstanceEvent(timer5, { { instance2, -1 } });
    QVERIFY(timer1->precedingTimer() == nullptr);
    QCOMPARE(timer1->followingTimer(), timer2);
    QCOMPARE(timer2->precedingTimer(), timer1);
    QCOMPARE(timer2->followingTimer(), timer3);
    QCOMPARE(timer3->precedingTimer(), timer2);
    QVERIFY(timer3->followingTimer() == nullptr);
    QVERIFY(timer4->precedingTimer() == nullptr);
    QCOMPARE(timer4->followingTimer(), timer5);
    QCOMPARE(timer5->precedingTimer(), timer4);
    QVERIFY(timer5->followingTimer() == nullptr);

    // update on deleted timer
    m_chart->removeInstanceEvent(timer5);
    delete timer5;
    QVERIFY(timer1->precedingTimer() == nullptr);
    QCOMPARE(timer1->followingTimer(), timer2);
    QCOMPARE(timer2->precedingTimer(), timer1);
    QCOMPARE(timer2->followingTimer(), timer3);
    QCOMPARE(timer3->precedingTimer(), timer2);
    QVERIFY(timer3->followingTimer() == nullptr);
    QVERIFY(timer4->precedingTimer() == nullptr);
    QVERIFY(timer4->followingTimer() == nullptr);

    // update on instance moved timer
    timer1->setInstance(instance2);
    QVERIFY(timer1->precedingTimer() == nullptr);
    QVERIFY(timer1->followingTimer() == nullptr);
    QVERIFY(timer2->precedingTimer() == nullptr);
    QCOMPARE(timer2->followingTimer(), timer3);
    QCOMPARE(timer3->precedingTimer(), timer2);
    QVERIFY(timer3->followingTimer() == nullptr);
    QVERIFY(timer4->precedingTimer() == nullptr);
    QVERIFY(timer4->followingTimer() == nullptr);
}

void tst_MscChart::testInvalidTimerRelation()
{
    MscInstance *instance1 = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance1);
    MscTimer *timer1 = new MscTimer("T1", MscTimer::TimerType::Stop);
    timer1->setInstance(instance1);
    m_chart->addInstanceEvent(timer1, { { instance1, -1 } });
    MscTimer *timer2 = new MscTimer("T1", MscTimer::TimerType::Timeout);
    timer2->setInstance(instance1);
    m_chart->addInstanceEvent(timer2, { { instance1, -1 } });

    QVERIFY(timer1->precedingTimer() == nullptr);
    QVERIFY(timer1->followingTimer() == nullptr);

    QVERIFY(timer2->precedingTimer() == nullptr);
    QVERIFY(timer2->followingTimer() == nullptr);
}

void tst_MscChart::testMaxInstanceNameNumber()
{
    auto instance = new MscInstance("IN", m_chart);
    m_chart->addInstance(instance);

    int num = m_chart->maxInstanceNameNumber();
    QCOMPARE(num, 1);

    auto message1 = new MscMessage("Msg1", m_chart);
    message1->setSourceInstance(instance);
    m_chart->addInstanceEvent(message1, { { instance, -1 } });

    num = m_chart->maxInstanceNameNumber();
    QCOMPARE(num, 1);

    message1->setMessageInstanceName("3");
    num = m_chart->maxInstanceNameNumber();
    QCOMPARE(num, 3);

    auto timer1 = new MscTimer(m_chart);
    timer1->setName("Timer1");
    timer1->setInstance(instance);
    timer1->setTimerInstanceName("5");
    m_chart->addInstanceEvent(timer1, { { instance, -1 } });
    num = m_chart->maxInstanceNameNumber();
    QCOMPARE(num, 5);
}

void tst_MscChart::testSetInstanceNameNumber()
{
    auto instance1 = new MscInstance("Inst1", m_chart);
    m_chart->addInstance(instance1);
    auto instance2 = new MscInstance("Inst2", m_chart);
    m_chart->addInstance(instance2);

    auto message1 = new MscMessage("Msg", m_chart);
    message1->setSourceInstance(instance1);
    m_chart->addInstanceEvent(message1, { { instance1, -1 } });

    // identical - set number - as with first
    auto message2 = new MscMessage("Msg", m_chart);
    message2->setSourceInstance(instance1);
    m_chart->addInstanceEvent(message2, { { instance1, -1 } });

    // Different name
    auto message3 = new MscMessage("Message", m_chart);
    message3->setSourceInstance(instance1);
    m_chart->addInstanceEvent(message3, { { instance1, -1 } });

    // Different instance
    auto message4 = new MscMessage("Msg", m_chart);
    message4->setTargetInstance(instance1);
    m_chart->addInstanceEvent(message4, { { instance1, -1 } });

    int result = m_chart->setInstanceNameNumbers(5);
    QCOMPARE(result, 7);
    QCOMPARE(message1->messageInstanceName(), QString("5"));
    QCOMPARE(message2->messageInstanceName(), QString("6"));
    QCOMPARE(message3->messageInstanceName(), QString(""));
    QCOMPARE(message4->messageInstanceName(), QString(""));
}

void tst_MscChart::testMoveCoregion()
{
    auto instance1 = new MscInstance("Inst1", m_chart);
    m_chart->addInstance(instance1);
    auto instance2 = new MscInstance("Inst2", m_chart);
    m_chart->addInstance(instance2);

    auto message1 = new MscMessage("Msg1", m_chart);
    message1->setSourceInstance(instance1);
    message1->setTargetInstance(instance2);
    m_chart->addInstanceEvent(message1, { { instance1, -1 }, { instance2, -1 } });

    auto coregionBegin = new MscCoregion(MscCoregion::Type::Begin, m_chart);
    coregionBegin->setInstance(instance1);
    m_chart->addInstanceEvent(coregionBegin, { { instance1, -1 } });

    auto message2 = new MscMessage("Msg2", m_chart);
    message2->setTargetInstance(instance1);
    message2->setSourceInstance(instance2);
    m_chart->addInstanceEvent(message2, { { instance1, -1 }, { instance2, -1 } });

    auto coregionEnd = new MscCoregion(MscCoregion::Type::End, m_chart);
    coregionEnd->setInstance(instance1);
    m_chart->addInstanceEvent(coregionEnd, { { instance1, -1 } });

    auto message3 = new MscMessage("Msg3", m_chart);
    message3->setSourceInstance(instance1);
    message3->setTargetInstance(instance2);
    m_chart->addInstanceEvent(message3, { { instance1, -1 }, { instance2, -1 } });

    QCOMPARE(m_chart->indexofEventAtInstance(coregionBegin, instance1), 1);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionEnd, instance1), 3);

    // Move begin up
    m_chart->updateCoregionPos(coregionBegin, coregionEnd, instance1, 0, 3);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionBegin, instance1), 0);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionEnd, instance1), 3);

    // Move begin down
    m_chart->updateCoregionPos(coregionBegin, coregionEnd, instance1, 1, 3);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionBegin, instance1), 1);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionEnd, instance1), 3);

    // Move end down
    m_chart->updateCoregionPos(coregionBegin, coregionEnd, instance1, 1, 4);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionBegin, instance1), 1);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionEnd, instance1), 4);

    // Move end up
    m_chart->updateCoregionPos(coregionBegin, coregionEnd, instance1, 1, 3);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionBegin, instance1), 1);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionEnd, instance1), 3);

    // end has to be after begin
    m_chart->updateCoregionPos(coregionBegin, coregionEnd, instance1, 4, 0);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionBegin, instance1), 1);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionEnd, instance1), 3);

    // Move to other instance
    m_chart->updateCoregionPos(coregionBegin, coregionEnd, instance2, 1, 3);
    QCOMPARE(coregionBegin->instance(), instance2);
    QCOMPARE(coregionEnd->instance(), instance2);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionBegin, instance1), -1);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionBegin, instance2), 1);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionEnd, instance1), -1);
    QCOMPARE(m_chart->indexofEventAtInstance(coregionEnd, instance2), 3);
}

void tst_MscChart::testCreateMessage()
{
    auto instance1 = new MscInstance("Inst1", m_chart);
    m_chart->addInstance(instance1);
    auto instance2 = new MscInstance("Inst2", m_chart);
    m_chart->addInstance(instance2);

    auto action = new MscAction("DoIt");
    action->setInstance(instance1);
    m_chart->addInstanceEvent(action, { { instance1, -1 } });

    auto create1 = new MscCreate("Create", m_chart);
    create1->setSourceInstance(instance2);
    create1->setTargetInstance(instance1);
    m_chart->addInstanceEvent(create1, { { instance1, -1 }, { instance2, -1 } });

    int idx = m_chart->eventsForInstance(instance1).indexOf(action);
    QCOMPARE(idx, 1);
    idx = m_chart->eventsForInstance(instance1).indexOf(create1);
    QCOMPARE(idx, 0);
    idx = m_chart->eventsForInstance(instance2).indexOf(create1);
    QCOMPARE(idx, 0);
}

void tst_MscChart::testAddCreateAfterMessage()
{
    auto instance1 = new MscInstance("Inst1", m_chart);
    m_chart->addInstance(instance1);
    auto instance2 = new MscInstance("Inst2", m_chart);
    m_chart->addInstance(instance2);

    auto message1 = new MscMessage("Msg", m_chart);
    message1->setTargetInstance(instance1);
    m_chart->addInstanceEvent(message1, { { instance1, -1 } });

    auto message2 = new MscMessage("Msg", m_chart);
    message2->setSourceInstance(instance1);
    message2->setTargetInstance(instance2);
    m_chart->addInstanceEvent(message2, { { instance1, -1 }, { instance2, -1 } });

    auto message3 = new MscMessage("Msg", m_chart);
    message3->setSourceInstance(instance1);
    m_chart->addInstanceEvent(message3, { { instance1, -1 }, { instance2, -1 } });

    auto create1 = new MscCreate("Create", m_chart);
    create1->setSourceInstance(instance1);
    create1->setTargetInstance(instance2);
    m_chart->addInstanceEvent(create1, { { instance1, -1 }, { instance2, -1 } });

    int idx = m_chart->eventsForInstance(instance1).indexOf(create1);
    QCOMPARE(idx, 1);
    idx = m_chart->eventsForInstance(instance2).indexOf(create1);
    QCOMPARE(idx, 0);
}

void tst_MscChart::testIndicesOfEvent()
{
    auto instance1 = new MscInstance("Inst1", m_chart);
    m_chart->addInstance(instance1);
    auto instance2 = new MscInstance("Inst2", m_chart);
    m_chart->addInstance(instance2);

    auto message1 = new MscMessage("Msg", m_chart);
    message1->setTargetInstance(instance1);
    m_chart->addInstanceEvent(message1, { { instance1, -1 } });
    ChartIndexList indices { { instance1, 0 } };
    QCOMPARE(m_chart->indicesOfEvent(message1), indices);

    auto message2 = new MscMessage("Msg", m_chart);
    message2->setSourceInstance(instance1);
    message2->setTargetInstance(instance2);
    m_chart->addInstanceEvent(message2, { { instance1, -1 }, { instance2, -1 } });
    indices = { { instance1, 1 }, { instance2, 0 } };
    QCOMPARE(m_chart->indicesOfEvent(message2), indices);
}

void tst_MscChart::testIsCrossingMessage()
{
    auto instance1 = new MscInstance("Inst1", m_chart);
    m_chart->addInstance(instance1);
    auto instance2 = new MscInstance("Inst2", m_chart);
    m_chart->addInstance(instance2);

    auto message1 = new MscMessage("Msg1", m_chart);
    message1->setTargetInstance(instance1);
    m_chart->addInstanceEvent(message1, { { instance1, -1 } });

    QCOMPARE(m_chart->isCrossingMessage(message1), false);

    auto message2 = new MscMessage("Msg2", m_chart);
    message2->setSourceInstance(instance1);
    message2->setTargetInstance(instance2);
    m_chart->addInstanceEvent(message2, { { instance1, -1 }, { instance2, -1 } });

    QCOMPARE(m_chart->isCrossingMessage(message1), false);
    QCOMPARE(m_chart->isCrossingMessage(message2), false);

    auto message3 = new MscMessage("Msg3", m_chart);
    message3->setSourceInstance(instance2);
    message3->setTargetInstance(instance1);
    m_chart->addInstanceEvent(message3, { { instance1, 1 }, { instance2, -1 } });

    QCOMPARE(m_chart->isCrossingMessage(message1), false);
    QCOMPARE(m_chart->isCrossingMessage(message2), true);
    QCOMPARE(m_chart->isCrossingMessage(message3), true);
}

QTEST_APPLESS_MAIN(tst_MscChart)

#include "tst_mscchart.moc"
