/****************************************************************************
**
** Copyright (C) 2017-2019 N7 Space sp. z o. o.
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

#pragma once

#include "type.h"

#include <QString>

namespace Asn1Acn {
namespace Types {

class UserdefinedType : public Type
{
public:
    UserdefinedType(const QString &typeName, const QString &module, const TypeAssignment *referencedType);

    QString typeName() const override;
    QString label() const override;

    ASN1Type typeEnum() const override;

    QString baseIconFile() const override;

    const TypeAssignment *referencedType() const;

private:
    QString m_name;
    QString m_module;
    const TypeAssignment *m_referencedType = nullptr;
};

}
}
