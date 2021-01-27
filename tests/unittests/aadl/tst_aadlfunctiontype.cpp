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

#include "aadlobject.h"
#include "aadlfunction.h"
#include "aadlfunctiontype.h"
#include "aadliface.h"
#include "aadltestutils.h"

#include <QDebug>
#include <QtTest>

class tst_AADLFunctionType : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAadlType();
    void testRequiredInterfacesManagement();
    void testProvidedInterfacesManagement();
    void testCommonInterfacesManagement();

    void testChildrenManagementFunction();
    void testChildrenManagementContainer();
    void testChildrenManagementMixed();

private:
    void testChildrenManagement(ivm::AADLFunctionType *obj, const QVector<ivm::AADLObject *> &children,
            bool addedOnCreation = false);
};

void tst_AADLFunctionType::testAadlType()
{
    ivm::AADLFunctionType obj;

    QCOMPARE(obj.aadlType(), ivm::AADLObject::Type::FunctionType);
}

void tst_AADLFunctionType::testRequiredInterfacesManagement()
{
    ivm::AADLFunctionType obj;

    QCOMPARE(obj.ris().size(), 0);

    auto ifaceType = ivm::AADLIface::IfaceType::Required;
    const int ifacesCount = 10;
    auto ciRI = ivm::testutils::init(ifaceType, &obj);
    for (int i = 0; i < ifacesCount; ++i) {
        ciRI.name = QString("test required iface #%1").arg(i);
        auto ri = ivm::AADLIface::createIface(ciRI);
        const bool ok = obj.addChild(ri);
        QVERIFY(ok);
        QCOMPARE(obj.ris().size(), i + 1);
    }

    for (int i = 0; i < ifacesCount; ++i) {
        auto ri = obj.ris().first();
        const bool ok = obj.removeChild(ri);
        QVERIFY(ok);
        QCOMPARE(obj.ris().size(), ifacesCount - i - 1);
        delete ri;
    }
}

void tst_AADLFunctionType::testProvidedInterfacesManagement()
{
    ivm::AADLFunctionType obj;

    QCOMPARE(obj.ris().size(), 0);

    auto ifaceType = ivm::AADLIface::IfaceType::Provided;
    const int ifacesCount = 10;
    auto ciPI = ivm::testutils::init(ifaceType, &obj);
    for (int i = 0; i < ifacesCount; ++i) {
        ciPI.name = QString("test provided iface #%1").arg(i);
        auto pi = ivm::AADLIface::createIface(ciPI);
        const bool ok = obj.addChild(pi);
        QVERIFY(ok);
        QCOMPARE(obj.pis().size(), i + 1);
    }

    QCOMPARE(obj.pis().size(), ifacesCount);

    for (int i = 0; i < ifacesCount; ++i) {
        auto pi = obj.pis().first();
        const bool ok = obj.removeChild(pi);
        QVERIFY(ok);
        QCOMPARE(obj.pis().size(), ifacesCount - i - 1);
        delete pi;
    }

    QCOMPARE(obj.pis().size(), 0);
}

void tst_AADLFunctionType::testCommonInterfacesManagement()
{
    ivm::AADLFunction obj;

    QCOMPARE(obj.ris().size(), 0);

    const int ifacesCountHalf = 5;

    QVector<ivm::AADLIface *> ifaces;
    for (int i = 0; i < ifacesCountHalf; ++i) {
        auto ci = ivm::testutils::init(ivm::AADLIface::IfaceType::Provided, &obj);
        ci.name = QString("test provided iface #%1").arg(i);
        ifaces << ivm::AADLIface::createIface(ci);

        ci = ivm::testutils::init(ivm::AADLIface::IfaceType::Required, &obj);
        ci.name = QString("test required iface #%1").arg(i);
        ifaces << ivm::AADLIface::createIface(ci);
    }

    QCOMPARE(ifaces.size(), ifacesCountHalf * 2);

    for (auto iface : ifaces) {
        const bool ok = obj.addChild(iface);
        QVERIFY(ok);
    }

    QCOMPARE(obj.ris().size(), ifacesCountHalf);
    QCOMPARE(obj.pis().size(), ifacesCountHalf);

    while (ifaces.size()) {
        if (auto iface = ifaces.takeLast()) {
            const int prevCount = iface->isProvided() ? obj.pis().size() : obj.ris().size();
            const bool ok = obj.removeChild(iface);
            QVERIFY(ok);
            const int currCount = iface->isProvided() ? obj.pis().size() : obj.ris().size();
            QVERIFY(prevCount != currCount);
            delete iface;
        }
    }

    QCOMPARE(obj.pis().size(), 0);
    QCOMPARE(obj.ris().size(), 0);
}

void tst_AADLFunctionType::testChildrenManagementFunction()
{
    ivm::AADLFunctionType obj;

    auto itProvided = ivm::AADLIface::IfaceType::Provided;
    auto itRequired = ivm::AADLIface::IfaceType::Required;

    ivm::AADLFunction fn0("Fn0", &obj);
    ivm::AADLFunction fn1("Fn1", &obj);
    fn1.addChild(ivm::testutils::createIface(&fn1, itProvided));
    ivm::AADLFunction fn2("Fn2", &obj);
    fn2.addChild(ivm::testutils::createIface(&fn2, itRequired));
    ivm::AADLFunction fn3("Fn3", &obj);
    fn3.addChild(ivm::testutils::createIface(&fn3, itRequired));
    fn3.addChild(ivm::testutils::createIface(&fn3, itProvided));

    const QVector<ivm::AADLObject *> functions { &fn0, &fn1, &fn2, &fn3 };
    testChildrenManagement(&obj, functions);
}

void tst_AADLFunctionType::testChildrenManagement(
        ivm::AADLFunctionType *obj, const QVector<ivm::AADLObject *> &children, bool addedOnCreation)
{
    QCOMPARE(obj->children().size(), addedOnCreation ? children.size() : 0);

    for (auto fn : children) {
        const int prevCount = obj->children().size();
        const bool ok = obj->addChild(fn);
        if (addedOnCreation)
            QVERIFY(!ok);
        else
            QVERIFY(ok);
        const int currCount = obj->children().size();
        if (addedOnCreation)
            QVERIFY(prevCount == currCount);
        else
            QVERIFY(prevCount != currCount);
    }

    QCOMPARE(obj->children().size(), children.size());

    for (auto fn : children) {
        const int prevCount = obj->children().size();
        const bool ok = obj->addChild(fn);
        QVERIFY(!ok);
        const int currCount = obj->children().size();
        QVERIFY(prevCount == currCount);
    }

    QCOMPARE(obj->children().size(), children.size());

    for (auto fn : children) {
        const int prevCount = obj->children().size();
        const bool ok = obj->removeChild(fn);
        QVERIFY(ok);
        const int currCount = obj->children().size();
        QVERIFY(prevCount != currCount);
    }

    QCOMPARE(obj->children().size(), 0);
    const bool added = obj->addChild(nullptr);
    QVERIFY(!added);
    QCOMPARE(obj->children().size(), 0);
    const bool removed = obj->removeChild(nullptr);
    QVERIFY(!removed);
    QCOMPARE(obj->children().size(), 0);
}

void tst_AADLFunctionType::testChildrenManagementContainer()
{
    ivm::AADLFunctionType obj;

    auto itProvided = ivm::AADLIface::IfaceType::Provided;
    auto itRequired = ivm::AADLIface::IfaceType::Required;

    ivm::AADLFunctionType fnType0("FnType0", &obj);
    obj.addChild(&fnType0);

    ivm::AADLFunctionType fnType1("FnType1", &obj);
    obj.addChild(&fnType1);
    fnType1.addChild(ivm::testutils::createIface(&fnType1, itProvided));

    ivm::AADLFunctionType fnType2("FnType2", &obj);
    obj.addChild(&fnType2);
    fnType2.addChild(ivm::testutils::createIface(&fnType2, itRequired));

    ivm::AADLFunctionType fnType3("FnType3", &obj);
    obj.addChild(&fnType3);
    fnType3.addChild(ivm::testutils::createIface(&fnType3, itRequired));
    fnType3.addChild(ivm::testutils::createIface(&fnType3, itProvided));

    const QVector<ivm::AADLObject *> functionTypes { &fnType0, &fnType1, &fnType2, &fnType3 };
    testChildrenManagement(&obj, functionTypes, true);
}

void tst_AADLFunctionType::testChildrenManagementMixed()
{
    auto itProvided = ivm::AADLIface::IfaceType::Provided;
    auto itRequired = ivm::AADLIface::IfaceType::Required;

    ivm::AADLFunctionType obj;

    ivm::AADLFunction fn0("Fn0", &obj);
    ivm::AADLFunction fn1("Fn1", &obj);
    fn1.addChild(ivm::testutils::createIface(&fn1, itProvided));

    ivm::AADLFunction fn2("Fn2", &obj);
    fn1.addChild(ivm::testutils::createIface(&fn2, itRequired));

    ivm::AADLFunction fn3("Fn3", &obj);
    fn3.addChild(ivm::testutils::createIface(&fn3, itRequired));
    fn3.addChild(ivm::testutils::createIface(&fn3, itProvided));

    ivm::AADLFunctionType fnType0("FnType0", &obj);
    ivm::AADLFunctionType fnType1("FnType1", &obj);
    fnType1.addChild(ivm::testutils::createIface(&fnType1, itProvided));

    ivm::AADLFunctionType fnType2("FnType2", &obj);
    fnType2.addChild(ivm::testutils::createIface(&fnType2, itRequired));

    ivm::AADLFunctionType fnType3("FnType3", &obj);
    fnType3.addChild(ivm::testutils::createIface(&fnType3, itRequired));
    fnType3.addChild(ivm::testutils::createIface(&fnType3, itProvided));

    const QVector<ivm::AADLObject *> children { &fnType0, &fn0, &fnType1, &fn1, &fnType2, &fn3, &fnType3, &fn2 };
    testChildrenManagement(&obj, children);
}

QTEST_APPLESS_MAIN(tst_AADLFunctionType)

#include "tst_aadlfunctiontype.moc"