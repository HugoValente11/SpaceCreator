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
#include <map>
#include <vector>

#include "definitions.h"

namespace Asn1Acn {
namespace Internal {
namespace Data {

class Module
{
public:
    // TODO inherit Node ? ~Module() override;

    void add(const DefinitionsPtr &defs)
    {
        m_definitionsByNameMap[defs->name()] = defs;
        m_definitionsList.push_back(defs);
    }

    using DefinitionsList = std::vector<DefinitionsPtr>;
    const DefinitionsList &definitionsList() const { return m_definitionsList; }
    DefinitionsPtr definitions(const QString &name) const;

private:
    DefinitionsList m_definitionsList;
    std::map<QString, DefinitionsPtr> m_definitionsByNameMap;
};

using ModulePtr = std::shared_ptr<Module>;

} // namespace Data
} // namespace Internal
} // namespace Asn1Acn
