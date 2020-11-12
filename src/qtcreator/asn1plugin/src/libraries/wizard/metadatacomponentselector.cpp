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

#include "metadatacomponentselector.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QMessageBox>

#include <libraries/librarystorage.h>

using namespace Asn1Acn::Internal::Libraries;
using namespace Asn1Acn::Internal::Libraries::Wizard;

MetadaComponentSelector::MetadaComponentSelector(MetadataModel *model,
                                                 const QString &path,
                                                 QObject *parent)
    : ComponentSelector(parent)
    , m_model(model)
    , m_path(path)
{
    connect(m_model,
            &MetadataModel::conflictOccurred,
            this,
            &MetadaComponentSelector::onConflictOccured);
}

void MetadaComponentSelector::onConflictOccured(const QString &first, const QString &second) const
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Conflict detected");
    msgBox.setText("\"" + first + "\"" + " and " + "\"" + second + "\"" + " conflicts");
    msgBox.exec();
}

QStringList MetadaComponentSelector::pathsToImport()
{
    auto files = fileNamesFromSelectedItems();

    files.removeDuplicates();

    files = pathsFromNames(files);

    return insertAcnFiles(files);
}

QStringList MetadaComponentSelector::fileNamesFromSelectedItems() const
{
    QStringList fileNames;
    const auto &selectedItems = m_model->selectedItems();
    for (auto it = selectedItems.begin(); it != selectedItems.end(); ++it) {
        if (it.value() != Qt::Checked)
            continue;

        const auto libraryNode = m_model->dataNode(it.key());
        fileNames.append(libraryNode->asn1Files());
        fileNames.append(libraryNode->additionalFiles());
    }

    return fileNames;
}

QStringList MetadaComponentSelector::insertAcnFiles(const QStringList &asnFiles) const
{
    QStringList allFiles = asnFiles;

    static const QRegularExpression re("\\.asn1?$");

    for (auto file : asnFiles) {
        const auto match = re.match(file);
        if (!match.hasMatch())
            continue;

        file.replace(match.capturedStart(), match.capturedLength(), ".acn");

        if (QFileInfo::exists(file))
            allFiles << file;
    }

    return allFiles;
}

QStringList MetadaComponentSelector::pathsFromNames(const QStringList &names) const
{
    if (names.empty())
        return {};

    QDirIterator it(m_path, names, QDir::NoFilter, QDirIterator::Subdirectories);

    QStringList paths;
    while (it.hasNext())
        paths.append(it.next());

    return paths;
}
