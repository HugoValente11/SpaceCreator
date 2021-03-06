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

#include <memory>

#include <QString>

#include <texteditor/codeassist/assistproposalitem.h>

#include <asn1/definitions.h>
#include <asn1/file.h>

#include "proposalsbuilder.h"

namespace Asn1Acn {
namespace Internal {
namespace Completion {

class UserTypesProposalsBuilder : public ProposalsBuilder
{
public:
    UserTypesProposalsBuilder(const Asn1Acn::File *data);

private:
    void fillProposals() override;

    void appendImportedTypes(const Asn1Acn::Definitions::ImportedTypes &importedTypes);
    void appendInternalTypes(const Asn1Acn::Definitions::Types &types);

    void appendInternalValues(const Asn1Acn::Definitions::Values &values);
    void appendImportedValues(const Asn1Acn::Definitions::ImportedValues &importedValues);

    void appendImportedElement(const QString &module, const QString &name);

    const Asn1Acn::File *m_data;
};

} // namespace Completion
} /* namespace Internal */
} /* namespace Asn1Acn */
