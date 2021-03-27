/*
   Copyright (C) 2021 European Space Agency - <maxime.perrotin@esa.int>

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

#include "aadlcommonprops.h"
#include "aadlfunction.h"
#include "aadliface.h"
#include "aadllibrary.h"
#include "aadlmodel.h"
#include "aadlnamevalidator.h"
#include "propertytemplateconfig.h"

#include <QTest>

class tst_AADLIface : public QObject
{
    Q_OBJECT

private:
    ivm::PropertyTemplateConfig *cfg { nullptr };

private Q_SLOTS:
    void initTestCase();
    void tst_setAttr_Autoname();
    void tst_reqIfaceAutorename();
};

void tst_AADLIface::initTestCase()
{
    ivm::initAadlLibrary();
    cfg = ivm::PropertyTemplateConfig::instance();
    cfg->init(QLatin1String("default_attributes.xml"));
}

void tst_AADLIface::tst_setAttr_Autoname()
{
    static const QString autonamedPropName = ivm::meta::Props::token(ivm::meta::Props::Token::Autonamed);

    ivm::AADLModel model(cfg);

    std::unique_ptr<ivm::AADLFunction> func = std::make_unique<ivm::AADLFunction>();
    model.addObject(func.get());

    ivm::AADLIface::CreationInfo ci;
    ci.id = shared::createId();
    ci.type = ivm::AADLIface::IfaceType::Required;
    ci.function = func.get();
    ci.model = &model;
    std::unique_ptr<ivm::AADLIfaceRequired> reqIface = std::make_unique<ivm::AADLIfaceRequired>(ci);
    func->addChild(reqIface.get());
    model.addObject(reqIface.get());

    QVERIFY(reqIface->isInheritPI());
    QVERIFY(reqIface->hasProperty(autonamedPropName, true));

    reqIface->setTitle(QLatin1String("SomeNameForReqIface"));
    QCOMPARE(reqIface->hasProperty(autonamedPropName, false), true);

    reqIface->setAttr(ivm::meta::Props::token(ivm::meta::Props::Token::name), QString());
    QVERIFY(ivm::AADLNameValidator::isAutogeneratedName(reqIface.get(), reqIface->title()));
    QVERIFY(reqIface->hasProperty(autonamedPropName));

    ci.id = shared::createId();
    ci.type = ivm::AADLIface::IfaceType::Provided;
    std::unique_ptr<ivm::AADLIfaceProvided> provIface = std::make_unique<ivm::AADLIfaceProvided>(ci);
    model.addObject(provIface.get());
    reqIface->setPrototype(provIface.get());
    QVERIFY(!reqIface->hasProperty(autonamedPropName));

    provIface->setTitle(QLatin1String("ProvIfaceName"));
    QCOMPARE(provIface->title(), reqIface->title());

    reqIface->setTitle(QLatin1String("NewNameForReqIface"));
    QVERIFY(provIface->title() != reqIface->title());
    QVERIFY(!reqIface->hasProperty(autonamedPropName));

    reqIface->unsetPrototype(provIface.get());
    QVERIFY(provIface->title() != reqIface->title());
    QVERIFY(reqIface->hasProperty(autonamedPropName, false));
}

void tst_AADLIface::tst_reqIfaceAutorename()
{
    ivm::AADLModel model(cfg);

    ivm::AADLIface::CreationInfo ci1;
    std::unique_ptr<ivm::AADLFunction> func1 = std::make_unique<ivm::AADLFunction>();
    func1->setTitle(QLatin1String("F1"));
    model.addObject(func1.get());
    ci1.model = &model;
    ci1.name = QLatin1String("P1");
    ci1.function = func1.get();
    ci1.type = ivm::AADLIface::IfaceType::Provided;

    std::unique_ptr<ivm::AADLIfaceProvided> provIface1 = std::make_unique<ivm::AADLIfaceProvided>(ci1);
    func1->addChild(provIface1.get());
    model.addObject(provIface1.get());

    ivm::AADLIface::CreationInfo ci2;
    std::unique_ptr<ivm::AADLFunction> func2 = std::make_unique<ivm::AADLFunction>();
    func2->setTitle(QLatin1String("F2"));
    model.addObject(func2.get());
    ci2.model = &model;
    ci2.name = QLatin1String("P1");
    ci2.function = func2.get();
    ci2.type = ivm::AADLIface::IfaceType::Provided;

    std::unique_ptr<ivm::AADLIfaceProvided> provIface2 = std::make_unique<ivm::AADLIfaceProvided>(ci2);
    func2->addChild(provIface2.get());
    model.addObject(provIface2.get());

    ivm::AADLIface::CreationInfo ci3;
    std::unique_ptr<ivm::AADLFunction> func3 = std::make_unique<ivm::AADLFunction>();
    model.addObject(func3.get());
    func3->setTitle(QLatin1String("F3"));
    ci3.model = &model;
    ci3.function = func3.get();
    ci3.type = ivm::AADLIface::IfaceType::Required;

    std::unique_ptr<ivm::AADLIfaceRequired> reqIface1 = std::make_unique<ivm::AADLIfaceRequired>(ci3);
    func3->addChild(reqIface1.get());
    model.addObject(reqIface1.get());
    reqIface1->setPrototype(provIface1.get());
    QCOMPARE(reqIface1->title(), provIface1->title());

    ci3.id = shared::createId();
    std::unique_ptr<ivm::AADLIfaceRequired> reqIface2 = std::make_unique<ivm::AADLIfaceRequired>(ci3);
    func3->addChild(reqIface2.get());
    model.addObject(reqIface2.get());
    reqIface2->setPrototype(provIface2.get());
    const QString reqIface2Name = func2->title() + QLatin1Char('_') + provIface2->title();
    QCOMPARE(reqIface2Name, reqIface2->title());

    provIface2->setTitle(QLatin1String("P2"));
    QCOMPARE(reqIface2Name, reqIface2->title());
}

QTEST_APPLESS_MAIN(tst_AADLIface)

#include "tst_aadliface.moc"
