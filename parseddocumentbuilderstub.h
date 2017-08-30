/****************************************************************************
**
** Copyright (C) 2017 N7 Mobile sp. z o. o.
** Contact: http://n7mobile.pl/Space
**
** This file is part of ASN.1/ACN Plugin for QtCreator.
**
** Plugin was developed under a programme and funded by
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

#include <memory>

#include <QHash>
#include <QString>

#include "parseddocument.h"
#include "documentsourceinfo.h"
#include "parseddocumentbuilder.h"

namespace Asn1Acn {
namespace Internal {

class ParsedDocumentBuilderStub
        : public QObject
        , public ParsedDocumentBuilder
{
    Q_OBJECT

public:
    ParsedDocumentBuilderStub(QObject *parent = 0);
    ~ParsedDocumentBuilderStub() = default;

    void setDocumentsToProcess(const QHash<QString, DocumentSourceInfo> *documents) override;
    void run() override;

    std::vector<std::unique_ptr<ParsedDocument>> takeDocuments() override;
    const QStringList &errorMessages() const override;

signals:
    void finished();
    void errored();
    void failed();

private:
    const QHash<QString, DocumentSourceInfo> *m_rawDocuments;
    std::vector<std::unique_ptr<ParsedDocument>> m_parsedDocuments;

    QStringList m_errorMessages;
};

} /* namespace Internal */
} /* namespace Asn1Acn */
