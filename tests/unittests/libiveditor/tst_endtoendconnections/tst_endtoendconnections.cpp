/*
   Copyright (C) 2020 European Space Agency - <maxime.perrotin@esa.int>

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

#include "endtoendconnections.h"

#include <QtTest>

// This is copied from a saved MSC file
static const char *mscFile1 = "mscdocument Untitled_Document /* MSC AND */;\n\n\n"
                              "/* CIF MSCDOCUMENT (0, 0) (2410, 1237) */\n\n"
                              "mscdocument Untitled_Leaf /* MSC LEAF */;\n\n\n"
                              "msc Untitled_MSC;\n"
                              "/* CIF INSTANCE (224, 68) (319, 91) (800, 1053) */\n"
                              "instance User_Interface;\n"
                              "/* CIF MESSAGE (0, 350) (384, 350) */\n\n"
                              "in Start_Transaction from env;\n\n"
                              "/* CIF MESSAGE (384, 478) (904, 478) */\n\n"
                              "out Open_File to File_Manager;\n\n\n"
                              "endinstance;\n\n"
                              "/* CIF INSTANCE (762, 68) (284, 91) (800, 1053) */\n\n"
                              "instance File_Manager;\n\n"
                              "/* CIF MESSAGE (384, 478) (904, 478) */\n\n"
                              "in Open_File from User_Interface;\n\n"
                              "/* CIF MESSAGE (904, 606) (1668, 606) */\n\n"
                              "out Access_Database to Database_Manager;\n\n"
                              "endinstance;\n\n"
                              "/* CIF INSTANCE (1470, 68) (396, 91) (800, 1053) */\n\n"
                              "instance Database_Manager;\n\n\n"
                              "/* CIF MESSAGE (904, 606) (1668, 606) */\n\n"
                              "in Access_Database from File_Manager;\n\n"
                              "endinstance;\n\n"
                              "endmsc;\n\n\n"
                              "endmscdocument;\n\n\n"
                              "endmscdocument;";

class tst_EndToEndConnections : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testReader();
};

void tst_EndToEndConnections::testReader()
{
    const auto dataflow = aadlinterface::EndToEndConnections::readDataflow(::mscFile1, false);
    QCOMPARE(dataflow.size(), 3);
    QCOMPARE(dataflow[0].from, QString());
    QCOMPARE(dataflow[0].to, QString("User_Interface"));
    QCOMPARE(dataflow[0].message, QString("Start_Transaction"));

    QCOMPARE(dataflow[1].from, QString("User_Interface"));
    QCOMPARE(dataflow[1].to, QString("File_Manager"));
    QCOMPARE(dataflow[1].message, QString("Open_File"));

    QCOMPARE(dataflow[2].from, QString("File_Manager"));
    QCOMPARE(dataflow[2].to, QString("Database_Manager"));
    QCOMPARE(dataflow[2].message, QString("Access_Database"));
}

QTEST_MAIN(tst_EndToEndConnections)

#include "tst_endtoendconnections.moc"