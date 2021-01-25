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

#include "aadlobject.h"
#include "aadlcomment.h"
#include "aadlconnection.h"
#include "aadlconnectiongroup.h"
#include "aadlfunction.h"
#include "aadlfunctiontype.h"
#include "aadliface.h"
#include "aadlifacegroup.h"
#include "baseitems/common/aadlutils.h"
#include "baseitems/interactiveobject.h"
#include "interface/aadlcommentgraphicsitem.h"
#include "interface/aadlconnectiongraphicsitem.h"
#include "interface/aadlconnectiongroupgraphicsitem.h"
#include "interface/aadlfunctiongraphicsitem.h"
#include "interface/aadlfunctiontypegraphicsitem.h"
#include "interface/aadlinterfacegraphicsitem.h"
#include "interface/aadlinterfacegroupgraphicsitem.h"

#include <QMetaEnum>
#include <QObject>
#include <QtTest>
#include <memory>

class tst_ItemZOrder : public QObject
{
    Q_OBJECT
private:
    void checkItem(ive::InteractiveObject *item, const qreal expectedZ);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testItem_FunctionType();
    void testItem_Function();
    void testItem_Comment();
    void testItem_InterfaceGroup();
    void testItem_RequiredInterface();
    void testItem_ProvidedInterface();
    void testItem_Connection();
    void testItem_ConnectionGroup();

    void testItem_CheckCoverage();

private:
    int m_itemTypesTested { 0 };
};

void tst_ItemZOrder::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_itemTypesTested = 0;
}

void tst_ItemZOrder::cleanupTestCase()
{
    QStandardPaths::setTestModeEnabled(false);
}

void tst_ItemZOrder::checkItem(ive::InteractiveObject *item, const qreal expectedZ)
{
    if (!item || !item->aadlObject() || item->aadlObject()->aadlType() == ivm::AADLObject::Type::Unknown)
        return;

    ++m_itemTypesTested;

    item->setSelected(false);
    QVERIFY(item->isSelected() == false);
    QCOMPARE(ive::itemLevel(item->aadlObject(), item->isSelected()), expectedZ);

    item->setSelected(true);
    QVERIFY(item->isSelected() == true);
    QCOMPARE(ive::itemLevel(item->aadlObject(), item->isSelected()), ive::ZOrder.Selected);
}

void tst_ItemZOrder::testItem_FunctionType()
{
    ivm::AADLFunctionType aadlObject(QStringLiteral("TestFunctionType"));
    ive::AADLFunctionTypeGraphicsItem item(&aadlObject);

    checkItem(&item, ive::ZOrder.Function);
}

void tst_ItemZOrder::testItem_Function()
{
    ivm::AADLFunction aadlObject(QStringLiteral("TestFunction"));
    ive::AADLFunctionGraphicsItem item(&aadlObject);

    checkItem(&item, ive::ZOrder.Function);
}

void tst_ItemZOrder::testItem_Comment()
{
    ivm::AADLComment aadlObject(QStringLiteral("TestComment"));
    ive::AADLCommentGraphicsItem item(&aadlObject);

    checkItem(&item, ive::ZOrder.Comment);
}

void tst_ItemZOrder::testItem_InterfaceGroup()
{
    ivm::AADLFunction aadlFunction(QStringLiteral("TestFunction"));
    ivm::AADLIface::CreationInfo ci;
    ci.function = &aadlFunction;
    ci.type = ivm::AADLIface::IfaceType::Grouped;
    std::unique_ptr<ivm::AADLIfaceGroup> pIface =
            std::unique_ptr<ivm::AADLIfaceGroup>(new ivm::AADLIfaceGroup(ci));
    ive::AADLInterfaceGroupGraphicsItem item(pIface.get());

    checkItem(&item, ive::ZOrder.Interface);
}

void tst_ItemZOrder::testItem_RequiredInterface()
{
    ivm::AADLFunction aadlFunction(QStringLiteral("TestFunction"));
    ivm::AADLIface::CreationInfo ci;
    ci.function = &aadlFunction;
    ci.type = ivm::AADLIface::IfaceType::Required;
    std::unique_ptr<ivm::AADLIface> pIface =
            std::unique_ptr<ivm::AADLIface>(ivm::AADLIface::createIface(ci));
    ive::AADLInterfaceGraphicsItem item(pIface.get());

    checkItem(&item, ive::ZOrder.Interface);
}

void tst_ItemZOrder::testItem_ProvidedInterface()
{
    ivm::AADLFunction aadlFunction(QStringLiteral("TestFunction"));
    ivm::AADLIface::CreationInfo ci;
    ci.function = &aadlFunction;
    ci.type = ivm::AADLIface::IfaceType::Provided;
    std::unique_ptr<ivm::AADLIface> pIface =
            std::unique_ptr<ivm::AADLIface>(ivm::AADLIface::createIface(ci));
    ive::AADLInterfaceGraphicsItem item(pIface.get());

    checkItem(&item, ive::ZOrder.Interface);
}

void tst_ItemZOrder::testItem_Connection()
{
    ivm::AADLFunction aadlFunctionA(QStringLiteral("TestFunctionA"));
    ivm::AADLFunction aadlFunctionB(QStringLiteral("TestFunctionB"));

    ivm::AADLIface::CreationInfo ciA;
    ciA.function = &aadlFunctionA;
    ciA.type = ivm::AADLIface::IfaceType::Provided;
    std::unique_ptr<ivm::AADLIface> pIfaceA =
            std::unique_ptr<ivm::AADLIface>(ivm::AADLIface::createIface(ciA));
    ive::AADLInterfaceGraphicsItem itemA(pIfaceA.get());

    ivm::AADLIface::CreationInfo ciB;
    ciB.function = &aadlFunctionA;
    ciB.type = ivm::AADLIface::IfaceType::Provided;
    std::unique_ptr<ivm::AADLIface> pIfaceB =
            std::unique_ptr<ivm::AADLIface>(ivm::AADLIface::createIface(ciB));
    ive::AADLInterfaceGraphicsItem itemB(pIfaceB.get());

    ivm::AADLConnection aadlConnection(pIfaceA.get(), pIfaceB.get());
    ive::AADLConnectionGraphicsItem connectionItem(&aadlConnection, &itemA, &itemB);

    checkItem(&connectionItem, ive::ZOrder.Connection);
}

void tst_ItemZOrder::testItem_ConnectionGroup()
{
    ivm::AADLFunction aadlFunctionA(QStringLiteral("TestFunctionA"));
    ivm::AADLFunction aadlFunctionB(QStringLiteral("TestFunctionB"));

    ivm::AADLIface::CreationInfo ciA;
    ciA.function = &aadlFunctionA;
    ciA.type = ivm::AADLIface::IfaceType::Provided;
    std::unique_ptr<ivm::AADLIfaceGroup> pIfaceA = std::make_unique<ivm::AADLIfaceGroup>(ciA);
    ive::AADLInterfaceGroupGraphicsItem itemA(pIfaceA.get());

    ivm::AADLIface::CreationInfo ciB;
    ciB.function = &aadlFunctionA;
    ciB.type = ivm::AADLIface::IfaceType::Provided;
    std::unique_ptr<ivm::AADLIfaceGroup> pIfaceB = std::make_unique<ivm::AADLIfaceGroup>(ciB);
    ive::AADLInterfaceGroupGraphicsItem itemB(pIfaceB.get());

    std::unique_ptr<ivm::AADLConnection> aadlConnection { new ivm::AADLConnection(
            pIfaceA.get(), pIfaceB.get()) };

    ivm::AADLConnectionGroup aadlConnectionGroup(
            QStringLiteral("TestConnectionGroup"), pIfaceA.get(), pIfaceB.get(), { aadlConnection.get() });

    ive::AADLConnectionGroupGraphicsItem connectionItem(&aadlConnectionGroup, &itemA, &itemB);

    checkItem(&connectionItem, ive::ZOrder.Connection);
    aadlConnectionGroup.removeConnection(aadlConnection.get());
    QVERIFY(aadlConnectionGroup.groupedConnections().isEmpty());
}

void tst_ItemZOrder::testItem_CheckCoverage()
{
    static const QMetaEnum me = QMetaEnum::fromType<ivm::AADLObject::Type>();
    static const int totalTypesCount = me.keyCount() - 1; // exculde AADLObject::Type::Unknown

    QCOMPARE(m_itemTypesTested, totalTypesCount);
}

QTEST_MAIN(tst_ItemZOrder)

#include "tst_itemzorder.moc"
