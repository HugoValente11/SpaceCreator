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

#include "aadlcomment.h"
#include "aadlfunction.h"
#include "aadlparameter.h"
#include "asn1modelstorage.h"
#include "interface/interfacedocument.h"
#include "iveditor.h"
#include "xmldocexporter.h"

#include <QFile>
#include <QObject>
#include <QtTest>

static QString testFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/tst_xmldocex.xml";

class tst_XmlDocExporter : public QObject
{
    Q_OBJECT
public:
    tst_XmlDocExporter();

private:
    QByteArray testFileContent() const;
    std::unique_ptr<ive::InterfaceDocument> m_doc;

private Q_SLOTS:
    void init();
    void cleanup();

    void testExportEmptyDoc();
    void testExportFunctions();
    void testExportComment();
    void testExportNestedComment();
    void testExportAsn1File();
    void testExportToBuffer();
};

tst_XmlDocExporter::tst_XmlDocExporter()
{
    ive::initIvEditor();
}

QByteArray tst_XmlDocExporter::testFileContent() const
{
    QFile file(testFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        return file.readAll();
    }

    return QByteArray();
}

void tst_XmlDocExporter::init()
{
    m_doc = std::make_unique<ive::InterfaceDocument>(this);
    m_doc->asn1DataTypes()->clear();
    if (QFile::exists(testFilePath)) {
        QFile::remove(testFilePath);
    }
}

void tst_XmlDocExporter::cleanup()
{
    if (QFile::exists(testFilePath)) {
        QFile::remove(testFilePath);
    }
}

void tst_XmlDocExporter::testExportEmptyDoc()
{
    ive::XmlDocExporter::exportDocSilently(m_doc.get(), testFilePath);
    QByteArray text = testFileContent();
    QByteArray expectedRaw = "<InterfaceView>\n</InterfaceView>";
    QByteArray expectedFormatted = "<?xml version=\"1.0\"?>\n<InterfaceView/>";
    QVERIFY(text == expectedRaw || text == expectedFormatted);
}

void tst_XmlDocExporter::testExportFunctions()
{
    auto testfunc1 = new ivm::AADLFunction("TestFunc1", m_doc.get());
    testfunc1->setAttr("foo", QVariant::fromValue(11));
    testfunc1->setProp("bar", QVariant::fromValue(22));
    testfunc1->addContextParam(
            ivm::ContextParameter("Mo", ivm::BasicParameter::Type::Other, "MyInt", QVariant::fromValue(33)));

    QVector<ivm::AADLObject *> objects;
    objects.append(testfunc1);
    m_doc->setObjects(objects);

    ive::XmlDocExporter::exportDocSilently(m_doc.get(), testFilePath);
    QByteArray text = testFileContent();

    QByteArray expectedFormatted =
            "<?xml version=\"1.0\"?>\n<InterfaceView>\n"
            "    <Function name=\"TestFunc1\" language=\"\" is_type=\"NO\" instance_of=\"\" foo=\"11\">\n"
            "        <Property name=\"bar\" value=\"22\"/>\n"
            "        <ContextParameter name=\"Mo\" type=\"MyInt\" value=\"33\"/>\n"
            "    </Function>\n"
            "</InterfaceView>";
    QByteArray expectedRaw = "<InterfaceView>\n"
                             "<Function name=\"TestFunc1\" language=\"\" is_type=\"NO\" instance_of=\"\" foo=\"11\">\n"
                             "    <Property name=\"bar\" value=\"22\"/>\n"
                             "    <ContextParameter name=\"Mo\" type=\"MyInt\" value=\"33\"/>\n"
                             "</Function>\n\n"
                             "</InterfaceView>";
    qDebug() << text;
    QVERIFY(text == expectedRaw || text == expectedFormatted);
}

void tst_XmlDocExporter::testExportComment()
{
    auto testcomment1 = new ivm::AADLComment("TestComment1", m_doc.get());
    testcomment1->setAttr("foo", QVariant::fromValue(11));
    testcomment1->setProperty("bar", QVariant::fromValue(22)); // ignored for comment

    QVector<ivm::AADLObject *> objects;
    objects.append(testcomment1);
    m_doc->setObjects(objects);

    ive::XmlDocExporter::exportDocSilently(m_doc.get(), testFilePath);
    QByteArray text = testFileContent();

    QByteArray expectedFormatted = "<?xml version=\"1.0\"?>\n<InterfaceView>\n"
                                   "    <Comment name=\"TestComment1\" foo=\"11\"/>\n"
                                   "</InterfaceView>";
    QByteArray expectedRaw =
            "<InterfaceView>\n<Comment name=\"TestComment1\" foo=\"11\">\n</Comment>\n\n</InterfaceView>";
    qDebug() << text;
    QVERIFY(text == expectedRaw || text == expectedFormatted);
}

void tst_XmlDocExporter::testExportNestedComment()
{
    auto testfunc1 = new ivm::AADLFunction("TestFunc1", m_doc.get());
    auto testcomment1 = new ivm::AADLComment("TestComment1", testfunc1);
    testfunc1->addChild(testcomment1);

    QVector<ivm::AADLObject *> objects;
    objects.append(testfunc1);
    m_doc->setObjects(objects);

    ive::XmlDocExporter::exportDocSilently(m_doc.get(), testFilePath);
    QByteArray text = testFileContent();

    QByteArray expectedFormatted = "<?xml version=\"1.0\"?>\n<InterfaceView>\n"
                                   "    <Function name=\"TestFunc1\" language=\"\" is_type=\"NO\" instance_of=\"\">\n"
                                   "        <Comment name=\"TestComment1\"/>\n"
                                   "    </Function>\n"
                                   "</InterfaceView>";
    QByteArray expectedRaw =
            "<InterfaceView>\n<Function name=\"TestFunc1\" language=\"\" is_type=\"NO\" instance_of=\"\">\n<Comment "
            "name=\"TestComment1\">\n</Comment>\n\n</Function>\n\n</InterfaceView>";
    qDebug() << text;
    QVERIFY(text == expectedRaw || text == expectedFormatted);
}

void tst_XmlDocExporter::testExportAsn1File()
{
    m_doc->setAsn1FileName("fake.asn");
    ive::XmlDocExporter::exportDocSilently(m_doc.get(), testFilePath);
    QByteArray text = testFileContent();

    QByteArray expectedFormatted = "<?xml version=\"1.0\"?>\n<InterfaceView asn1file=\"fake.asn\"/>";
    QByteArray expectedRaw = "<InterfaceView asn1file=\"fake.asn\">\n</InterfaceView>";

    QVERIFY(text == expectedRaw || text == expectedFormatted);
}

void tst_XmlDocExporter::testExportToBuffer()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    bool ok = ive::XmlDocExporter::exportDoc(m_doc.get(), &buffer);
    QCOMPARE(ok, true);
    QByteArray expectedRaw = "<InterfaceView>\n</InterfaceView>";
    QByteArray expectedFormatted = "<?xml version=\"1.0\"?>\n<InterfaceView/>";
    QVERIFY(buffer.data() == expectedRaw || buffer.data() == expectedFormatted);
}

QTEST_MAIN(tst_XmlDocExporter)

#include "tst_xmldocexporter.moc"