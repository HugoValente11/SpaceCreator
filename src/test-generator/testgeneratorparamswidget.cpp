/****************************************************************************
**
** Copyright (C) 2018 N7 Space sp. z o. o.
** Contact: http://n7space.com
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

#include "testgeneratorparamswidget.h"

using namespace Asn1Acn::Internal::TestGenerator;

TestGeneratorParamsWidget::TestGeneratorParamsWidget(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
    m_ui.pathChooser->setExpectedKind(Utils::PathChooser::ExistingDirectory);
}

QString TestGeneratorParamsWidget::path() const
{
    return m_ui.pathChooser->path();
}

void TestGeneratorParamsWidget::setPath(const QString &path)
{
    m_ui.pathChooser->setPath(path);
}

QString TestGeneratorParamsWidget::rootType() const
{
    return m_ui.comboBox->currentText();
}

void TestGeneratorParamsWidget::addRootTypeCandidate(const QString &rootType)
{
    m_ui.comboBox->addItem(rootType);
}

void TestGeneratorParamsWidget::clearRootTypeCandidates()
{
    m_ui.comboBox->clear();
}
