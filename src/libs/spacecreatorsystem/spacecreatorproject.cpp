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

#include "spacecreatorproject.h"

#include "interface/interfacedocument.h"
#include "iveditorcore.h"
#include "ivsystemchecks.h"
#include "mainmodel.h"
#include "msceditorcore.h"
#include "mscsystemchecks.h"

#include <QDebug>

namespace scs {

SpaceCreatorProject::SpaceCreatorProject(QObject *parent)
    : QObject(parent)
    , m_mscChecks(new MscSystemChecks)
{
    m_mscChecks->setStorage(this);
}

SpaceCreatorProject::~SpaceCreatorProject() { }

QSharedPointer<dve::DVEditorCore> SpaceCreatorProject::dvData(const QString &fileName) const
{
    qErrnoWarning("SpaceCreatorProject::dvData() not implemented yet.");
    return {};
}

/*!
   Returns the IVEditorCore object for the given file
   If the object does not exist yet, one will be created and the data be loaded
 */
QSharedPointer<ive::IVEditorCore> SpaceCreatorProject::ivData(const QString &fileName) const
{
    if (!m_ivStore.contains(fileName)) {
        QSharedPointer<ive::IVEditorCore> data(new ive::IVEditorCore());
        data->registerBasicActions();
        data->document()->customActions(); // There some further actions are registered

        data->document()->load(fileName);
        const_cast<SpaceCreatorProject *>(this)->setIvData(fileName, data);
        return data;
    }

    return m_ivStore[fileName];
}

/*!
   Returns the MSCEditorCore object for the given file
   If the object does not exist yet, one will be created and the data be loaded
 */
QSharedPointer<msc::MSCEditorCore> SpaceCreatorProject::mscData(const QString &fileName) const
{
    if (!m_mscStore.contains(fileName)) {
        QSharedPointer<msc::MSCEditorCore> data(new msc::MSCEditorCore());
        data->showToolbars(false);
        data->mainModel()->loadFile(fileName);
        const_cast<SpaceCreatorProject *>(this)->setMscData(fileName, data);
        return data;
    }

    return m_mscStore[fileName];
}

/*!
   Returns the iv/aadl data of the project
 */
QSharedPointer<ive::IVEditorCore> SpaceCreatorProject::ivCore() const
{
    QStringList aadlFiles = allAadlFiles();
    if (aadlFiles.empty()) {
        qWarning() << "No AADL file in the project";
        return {};
    }

    return ivData(aadlFiles.first());
}

/*!
   Returns all MSCEditorCore objects, that are used in the current project
 */
QVector<QSharedPointer<msc::MSCEditorCore>> SpaceCreatorProject::allMscCores() const
{
    QStringList mscFiles = allMscFiles();
    QVector<QSharedPointer<msc::MSCEditorCore>> allMscCores;
    for (const QString &mscFile : mscFiles) {
        QSharedPointer<msc::MSCEditorCore> core = mscData(mscFile);
        if (core) {
            allMscCores.append(core);
        }
    }
    return allMscCores;
}

/*!
   Returns if the given \p core is in this storage
 */
bool SpaceCreatorProject::contains(QSharedPointer<shared::EditorCore> core) const
{
    for (QSharedPointer<ive::IVEditorCore> ivCore : m_ivStore) {
        if (core == ivCore) {
            return true;
        }
    }
    for (QSharedPointer<msc::MSCEditorCore> mscCore : m_mscStore) {
        if (core == mscCore) {
            return true;
        }
    }
    return false;
}

/*!
   Returns all aald files of the current project
 */
QStringList SpaceCreatorProject::allAadlFiles() const
{
    return projectFiles("interfaceview.xml");
}

/*!
   Returns all msc files of the current project
 */
QStringList SpaceCreatorProject::allMscFiles() const
{
    return projectFiles(".msc");
}

/*!
   Returns all asn files of the current project
 */
QStringList SpaceCreatorProject::allAsn1Files() const
{
    return projectFiles(".asn");
}

/*!
   \brief SpaceCreatorProject::projectFiles
   \param suffix
   \return
 */
QStringList SpaceCreatorProject::projectFiles(const QString &suffix) const
{
    Q_UNUSED(suffix)
    return {};
}

/*!
   Access to the list of aadl/iv checks done from msc
 */
QVector<IvSystemChecks *> SpaceCreatorProject::ivChecks() const
{
    QVector<IvSystemChecks *> checks;
    for (const QSharedPointer<msc::MSCEditorCore> &core : m_mscStore) {
        if (auto ivChecker = qobject_cast<scs::IvSystemChecks *>(core->systemChecker())) {
            checks.append(ivChecker);
        }
    }
    return checks;
}

/*!
   Removes all data that is stored here, but is not part of the project
 */
void SpaceCreatorProject::purgeNonProjectData()
{
    const QStringList aadlFiles = allAadlFiles();
    auto ivIt = m_ivStore.begin();
    while (ivIt != m_ivStore.end()) {
        if (!aadlFiles.contains(ivIt.key())) {
            ivIt = m_ivStore.erase(ivIt);
        } else {
            ++ivIt;
        }
    }

    const QStringList mscFiles = allMscFiles();
    auto mscIt = m_mscStore.begin();
    while (mscIt != m_mscStore.end()) {
        if (!mscFiles.contains(mscIt.key())) {
            mscIt = m_mscStore.erase(mscIt);
        } else {
            ++mscIt;
        }
    }
}

/*!
   Sets the IVEditorCore object for the given file.
   If the object was already used for another file, that old file/object connection is removed.
 */
void SpaceCreatorProject::setIvData(const QString &fileName, QSharedPointer<ive::IVEditorCore> ivData)
{
    const QString oldKey = m_ivStore.key(ivData, "");
    if (!oldKey.isEmpty()) {
        if (m_ivStore[fileName] == ivData) {
            return;
        }
        QSharedPointer<ive::IVEditorCore> oldData = m_ivStore.take(oldKey);
        disconnect(oldData.data(), nullptr, this, nullptr);
    }

    m_ivStore[fileName] = ivData;
    connect(ivData.data(), &shared::EditorCore::editedExternally, this, &scs::SpaceCreatorProject::editedExternally);
    Q_EMIT ivCoreAdded(ivData);
}

/*!
   Sets the MSCEditorCore object for the given file.
   If the object was already used for another file, that old file/object connection is removed.
 */
void SpaceCreatorProject::setMscData(const QString &fileName, QSharedPointer<msc::MSCEditorCore> mscData)
{
    const QString oldKey = m_mscStore.key(mscData, "");
    if (!oldKey.isEmpty()) {
        if (m_mscStore[fileName] == mscData) {
            return;
        }
        QSharedPointer<msc::MSCEditorCore> oldData = m_mscStore.take(oldKey);
        disconnect(oldData.data(), nullptr, this, nullptr);
    }

    m_mscStore[fileName] = mscData;
    connect(mscData.data(), &shared::EditorCore::editedExternally, this, &scs::SpaceCreatorProject::editedExternally);
    auto checker = new scs::IvSystemChecks(mscData.data());
    checker->setIvCore(ivCore());
    mscData->setSystemChecker(checker);
    Q_EMIT mscCoreAdded(mscData);
}

} // namespace scs