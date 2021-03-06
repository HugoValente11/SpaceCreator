/****************************************************************************
**
** Copyright (C) 2017-2019 N7 Space sp. z o. o.
** Contact: http://n7space.com
**
** This file is part of ASN.1/ACN Plugin for QtCreator.
**
** Plugin was developed under a program and funded by
** European Space Agency.
**
** This Plugin is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This Plugin is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#pragma once

#include "astxmlparser.h"

#include <QObject>
#include <QXmlStreamReader>

namespace Asn1Acn {
namespace Tests {

class AstXmlParserTests : public QObject
{
    Q_OBJECT
public:
    explicit AstXmlParserTests(QObject *parent = 0);

private Q_SLOTS:
    void cleanup();
    void test_emptyFile();
    void test_badXmlRoot();
    void test_emptyDefinitions();
    void test_singleTypeAssignment();
    void test_builtinTypeReference();
    void test_builtinTypeReference_data();
    void test_userDefinedTypeReference();
    void test_multipleTypeAssignments();
    void test_importedType();
    void test_multipleImportedType();
    void test_assignmentsAreTypeReferences();
    void test_sequenceTypeAssingment();
    void test_sequenceOfTypeAssingment();
    void test_choiceTypeAssignment();
    void test_valueAssignment();
    void test_importedValues();
    void test_multipleImportedValues();
    void test_multipleModules();
    void test_multipleFiles();
    void test_parametrizedInstancesContentsAreIgnored();
    void test_asn1AstParsing();
    void test_asn1AstReferenceParsing();

private:
    void setXmlData(const QString &str);
    void parsingFails(const QString &xmlData);
    void parse(const QString &xmlData);
    void parseTestFile(const QString &fileName);

    QXmlStreamReader m_xmlReader;
    std::map<QString, std::unique_ptr<Asn1Acn::File>> m_parsedData;
};

}
}
