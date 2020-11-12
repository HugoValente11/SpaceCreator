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
#include "indexfindingvisitor.h"

#include <asn1/definitions.h>
#include <asn1/file.h>
#include <asn1/project.h>
#include <asn1/root.h>

using namespace Asn1Acn::Internal::TreeViews::OutlineVisitors;

IndexFindingVisitor::IndexFindingVisitor(const Asn1Acn::Node *child)
    : m_child(child)
{}

IndexFindingVisitor::~IndexFindingVisitor() {}

template<typename Collection>
int IndexFindingVisitor::findIndexIn(const Collection &items) const
{
    using ValueType = typename Collection::value_type;
    const auto it = std::find_if(std::begin(items), std::end(items), [this](const ValueType &item) {
        return item.get() == m_child;
    });
    if (it == items.end())
        return -1;
    return static_cast<int>(std::distance(std::begin(items), it));
}

int IndexFindingVisitor::valueFor(const Asn1Acn::Definitions &defs) const
{
    const auto valueIdx = findIndexIn(defs.values());
    if (valueIdx != -1)
        return static_cast<int>(defs.types().size()) + valueIdx;
    return findIndexIn(defs.types());
}

int IndexFindingVisitor::valueFor(const Asn1Acn::File &file) const
{
    return findIndexIn(file.definitionsList());
}

int IndexFindingVisitor::valueFor(const Asn1Acn::Project &project) const
{
    return findIndexIn(project.files());
}

int IndexFindingVisitor::valueFor(const Asn1Acn::Root &root) const
{
    return findIndexIn(root.projects());
}

int IndexFindingVisitor::valueFor(const Asn1Acn::TypeAssignment &type) const
{
    Q_UNUSED(type);
    return -1;
}

int IndexFindingVisitor::valueFor(const Asn1Acn::ValueAssignment &value) const
{
    Q_UNUSED(value);
    return -1;
}
