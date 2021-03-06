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

#pragma once

#include <QObject>
#include <QSharedPointer>

class QIODevice;

namespace Grantlee {
class Engine;
class FileSystemTemplateLoader;
}

namespace templating {

/**
 * @brief The StringTemplate class generates XML document from string template
 * which is based on Django syntax. Also it validates and formats XML schema.
 * Internally it uses Grantlee libary: https://github.com/steveire/grantlee
 * To build and install Grantlee, please read README.md.
 */
class StringTemplate : public QObject
{
    Q_OBJECT
public:
    static StringTemplate *create(QObject *parent = nullptr);

    bool parseFile(const QHash<QString, QVariant> &grouppedObjects, const QString &templateFileName, QIODevice *out);

    QString formatText(const QString &text);

    bool needValidateXMLDocument() const;
    int autoFormattingIndent() const;

public Q_SLOTS:
    void setNeedValidateXMLDocument(bool validate);
    void setEscapeCharacters(bool doEscape);
    void setAutoFormattingIndent(int autoFormattingIndent);

Q_SIGNALS:
    void errorOccurred(const QString &errorString);

private:
    explicit StringTemplate(QObject *parent = nullptr);
    void init();

    Grantlee::Engine *m_engine = nullptr;
    QSharedPointer<Grantlee::FileSystemTemplateLoader> m_fileLoader;
    bool m_validateXMLDocument = false;
    bool m_doEscape = true;
    int m_autoFormattingIndent = 4;
};

}
