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

#include "projectcontenthandler.h"

#include <QList>
#include <QSet>

#include <utils/qtcassert.h>

#include "asn1sccdocumentprocessor.h"
#include "filesourcereader.h"

using namespace Asn1Acn::Internal;
using FileNameList = Utils::FileNameList;

ProjectContentHandler *ProjectContentHandler::create()
{
    return new ProjectContentHandler([](const QString &project)
                                         -> DocumentProcessor
                                             * { return Asn1SccDocumentProcessor::create(project); },
                                     new FileSourceReader,
                                     ParsedDataStorage::instance(),
                                     ModelValidityGuard::instance());
}

ProjectContentHandler::ProjectContentHandler(
    std::function<DocumentProcessor *(const QString &)> createProcessor,
    const SourceReader *sourceReader,
    ParsedDataStorage *storage,
    ModelValidityGuard *guard)
    : m_storage(storage)
    , m_guard(guard)
    , m_projectsChanged(0)
    , m_sourceReader(sourceReader)
    , m_createProcessor(createProcessor)
{
    m_guard->invalidate();
}

ProjectContentHandler::~ProjectContentHandler()
{
    delete m_sourceReader;
}

void ProjectContentHandler::handleProjectAdded(const QString &projectName)
{
    m_storage->addProject(projectName);
    allProcessingFinished();
}

void ProjectContentHandler::handleProjectRemoved(const QString &projectName)
{
    m_storage->removeProject(projectName);
    allProcessingFinished();
}

void ProjectContentHandler::handleFileListChanged(const QString &projectName,
                                                  const FileNameList &fileList)
{
    removeStaleFiles(projectName, fileList);
    processFiles(projectName, fileList);
}

void ProjectContentHandler::handleFileContentChanged(const Utils::FileName &path)
{
    QStringList projects = m_storage->getProjectsForFile(path);

    if (projects.empty()) {
        allProcessingFinished();
        return;
    }

    for (const QString &projectName : projects) {
        const auto paths = m_storage->getFilesPathsFromProject(projectName);
        processFiles(projectName, paths);
    }
}

void ProjectContentHandler::removeStaleFiles(const QString &projectName,
                                             const FileNameList &filePaths)
{
    const auto stalePaths = getStaleFilesPaths(projectName, filePaths);

    foreach (const auto &stalePath, stalePaths)
        m_storage->removeFileFromProject(projectName, stalePath);
}

void ProjectContentHandler::processFiles(const QString &projectName, const FileNameList &filePaths)
{
    DocumentProcessor *dp = createDocumentProcessor(projectName, filePaths);
    startProcessing(dp);
}

FileNameList ProjectContentHandler::getStaleFilesPaths(const QString &projectName,
                                                       const FileNameList &filePaths) const
{
    const auto staleFileList = m_storage->getFilesPathsFromProject(projectName);
    const auto pathsToRemove = staleFileList.toSet().subtract(filePaths.toSet());
    return pathsToRemove.toList();
}

DocumentProcessor *ProjectContentHandler::createDocumentProcessor(
    const QString &projectName, const FileNameList &filePaths) const
{
    DocumentProcessor *docProcessor = m_createProcessor(projectName);

    for (const auto &path : filePaths)
        docProcessor->addToRun(path.toString(), m_sourceReader->readContent(path.toString()));

    return docProcessor;
}

void ProjectContentHandler::startProcessing(DocumentProcessor *dp)
{
    connect(dp,
            &DocumentProcessor::processingFinished,
            this,
            &ProjectContentHandler::onFilesProcessingFinished);

    m_projectsChanged++;

    dp->run();
}

void ProjectContentHandler::onFilesProcessingFinished(const QString &projectName)
{
    DocumentProcessor *dp = qobject_cast<DocumentProcessor *>(sender());

    switch (dp->state()) {
    case DocumentProcessor::State::Successful:
        handleFilesProcesedWithSuccess(projectName, dp->takeResults());
        break;
    case DocumentProcessor::State::Errored:
    case DocumentProcessor::State::Failed:
        handleFilesProcesedWithFailure(projectName, dp->takeResults(), dp->errorMessages());
        break;
    case DocumentProcessor::State::Outdated:
        break;
    case DocumentProcessor::State::Unfinished:
        QTC_CHECK(false && "Wrong state in onFilesProcessingFinished");
        break;
    }

    dp->deleteLater();

    if (--m_projectsChanged == 0)
        allProcessingFinished();
}

void ProjectContentHandler::handleFilesProcesedWithSuccess(
    const QString &projectName, std::vector<std::unique_ptr<Asn1Acn::File>> parsedDocuments)
{
    for (size_t i = 0; i < parsedDocuments.size(); i++)
        m_storage->addFileToProject(projectName, std::move(parsedDocuments[i]));
}

void ProjectContentHandler::handleFilesProcesedWithFailure(
    const QString &projectName,
    std::vector<std::unique_ptr<Asn1Acn::File>> parsedDocuments,
    const std::vector<Asn1Acn::ErrorMessage> &errorMessages)
{
    for (size_t i = 0; i < parsedDocuments.size(); i++) {
        const auto path = Utils::FileName::fromString(parsedDocuments[i]->location().path());
        auto file = m_storage->getFileForPathFromProject(projectName, path);
        if (file == nullptr) {
            file = parsedDocuments[i].get();
            m_storage->addFileToProject(projectName, std::move(parsedDocuments[i]));
        }

        refreshErrorMessages(file, errorMessages);
    }
}

void ProjectContentHandler::refreshErrorMessages(
    Asn1Acn::File *file, const std::vector<Asn1Acn::ErrorMessage> &errorMessages)
{
    file->clearErrors();

    for (const auto &message : errorMessages)
        if (message.location().path() == file->location().path())
            file->addErrorMessage(message);
}

void ProjectContentHandler::allProcessingFinished()
{
    m_guard->validate();
    deleteLater();
}
