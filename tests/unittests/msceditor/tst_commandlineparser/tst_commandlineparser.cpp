/*
   Copyright (C) 2018 European Space Agency - <maxime.perrotin@esa.int>

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

#include <commandlineparser.h>
#include <mainwindow.h>

#include <QtTest>
#include <QTimer>
#include <QMessageBox>

class tst_CommandLineParser : public QObject
{
    Q_OBJECT

public:
private slots:
    /*
      * A test slot name for each option must contain "testCmdArgument" -
      * it's used in the tst_CommandLineParser::testCoverage
      * to detected missed (untested) options.
      * For now it's possible to test all (both) options just by iterating
      * the CommandLineParser::Positional enum,
      * but I'm afraid some options will need some extra config,
      * so lets keep to this way - one option, one dedicated test slot.
      */
    void testCmdArgumentOpenMsc();
    void testCmdArgumentOpenAsn();

    void testCoverage();
};

void tst_CommandLineParser::testCmdArgumentOpenMsc()
{
    static const QCommandLineOption cmdOpenMsc =
            CommandLineParser::positionalArg(CommandLineParser::Positional::OpenFileMsc);
    static const QString FileName(QString(EXAMPLES_STORAGE_PATH).append("example01.msc"));
    static const QString NoFileName("./no-such-file.msc");

    CommandLineParser parser;
    parser.process({ QApplication::instance()->applicationFilePath(),
                     QString("-%1=%2").arg(cmdOpenMsc.names().first(), FileName) });

    QCOMPARE(parser.isSet(CommandLineParser::Positional::Unknown), false);
    QCOMPARE(parser.isSet(CommandLineParser::Positional::OpenFileMsc), true);

    const QString argFromParser1(parser.value(CommandLineParser::Positional::OpenFileMsc));
    QCOMPARE(argFromParser1, FileName);

    MainWindow w;
    const bool processed =
            w.processCommandLineArg(CommandLineParser::Positional::OpenFileMsc,
                                    argFromParser1);
    QCOMPARE(processed, true);

    parser.process({ QApplication::instance()->applicationFilePath(),
                     QString("-%1=%2").arg(cmdOpenMsc.names().first(), NoFileName) });
    QCOMPARE(parser.isSet(CommandLineParser::Positional::OpenFileMsc), true);
    const QString argFromParser2(parser.value(CommandLineParser::Positional::OpenFileMsc));
    QCOMPARE(argFromParser2, NoFileName);

    const bool notProcessed =
            w.processCommandLineArg(CommandLineParser::Positional::OpenFileMsc,
                                    argFromParser2);
    QCOMPARE(notProcessed, false);
}

void tst_CommandLineParser::testCmdArgumentOpenAsn()
{
#ifndef __clang_analyzer__ // Currently clang's scan-build on build server complains on the QTimer by some reason

    // While ASN support is not implemented yet,
    // attempt to open a file will raise a message box.
    // Schedule an Esc press to bypass it:
    auto scheduleMBCloser = [](int interval) {
        QTimer::singleShot(interval, []() {
            if (QMessageBox *mb = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()))
                mb->close();
        });
    };

    static const QCommandLineOption cmdOpenAsn =
            CommandLineParser::positionalArg(CommandLineParser::Positional::OpenFileAsn);
    static const QString FileName(QString(EXAMPLES_STORAGE_PATH).append("no-such-file.asn"));
    static const QString NoFileName("./no-such-file.asn");

    CommandLineParser parser;
    parser.process({ QApplication::instance()->applicationFilePath(),
                     QString("-%1=%2").arg(cmdOpenAsn.names().first(), FileName) });

    QCOMPARE(parser.isSet(CommandLineParser::Positional::Unknown), false);
    QCOMPARE(parser.isSet(CommandLineParser::Positional::OpenFileAsn), true);

    const QString argFromParser1(parser.value(CommandLineParser::Positional::OpenFileAsn));
    QCOMPARE(argFromParser1, FileName);

    MainWindow w;

    scheduleMBCloser(1000);
    const bool processed = w.processCommandLineArg(CommandLineParser::Positional::OpenFileAsn,
                                                   argFromParser1);
    QEXPECT_FAIL("", "ASN support is not implemented yet", Continue);
    QCOMPARE(processed, true);

    parser.process({ QApplication::instance()->applicationFilePath(),
                     QString("-%1=%2").arg(cmdOpenAsn.names().first(), NoFileName) });
    QCOMPARE(parser.isSet(CommandLineParser::Positional::OpenFileAsn), true);

    const QString argFromParser2(parser.value(CommandLineParser::Positional::OpenFileAsn));
    QCOMPARE(argFromParser2, NoFileName);

    scheduleMBCloser(1000);
    const bool notProcessed = w.processCommandLineArg(CommandLineParser::Positional::OpenFileAsn,
                                                      argFromParser2);
    QCOMPARE(notProcessed, false);
#endif // __clang_analyzer__
}

void tst_CommandLineParser::testCoverage()
{
    const QMetaObject *metaMe = metaObject();
    QStringList testMethods;
    for (int i = metaMe->methodOffset(); i < metaMe->methodCount(); ++i) {
        const QString method = QString::fromLatin1(metaMe->method(i).methodSignature());
        if (method.startsWith("testCmdArgument"))
            testMethods << method;
    }

    const QMetaEnum &e = QMetaEnum::fromType<CommandLineParser::Positional>();
    int ignoredCommands(0);
    ++ignoredCommands; // CommandLineParser::PositionalArg::Unknown
    ++ignoredCommands; // CommandLineParser::PositionalArg::DbgOpenMscExamplesChain

    QCOMPARE(testMethods.size(), e.keyCount() - ignoredCommands);
}

QTEST_MAIN(tst_CommandLineParser)

#include "tst_commandlineparser.moc"
