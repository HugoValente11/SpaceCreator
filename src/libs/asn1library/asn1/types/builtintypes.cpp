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

#include "builtintypes.h"

using namespace Asn1Acn::Types;

std::unique_ptr<Type> BuiltinType::createBuiltinType(const QString &name)
{
    if (name.startsWith(QStringLiteral("boolean"), Qt::CaseInsensitive))
        return std::make_unique<Boolean>();

    if (name == QStringLiteral("NULL") || name == QStringLiteral("NullType"))
        return std::make_unique<Null>();

    if (name.startsWith(QStringLiteral("Integer"), Qt::CaseInsensitive))
        return std::make_unique<Integer>();

    if (name.startsWith(QStringLiteral("real"), Qt::CaseInsensitive))
        return std::make_unique<Real>();

    if (name == QStringLiteral("BIT_STRING") || name == QStringLiteral("BitStringType"))
        return std::make_unique<BitString>();

    if (name == QStringLiteral("OCTET_STRING") || name == QStringLiteral("OctetStringType"))
        return std::make_unique<OctetString>();

    if (name.startsWith(QStringLiteral("IA5String"), Qt::CaseInsensitive))
        return std::make_unique<IA5String>();

    if (name.startsWith(QStringLiteral("NumericString"), Qt::CaseInsensitive))
        return std::make_unique<NumericString>();

    if (name.startsWith(QStringLiteral("enumerated"), Qt::CaseInsensitive))
        return std::make_unique<Enumerated>();

    if (name.startsWith(QStringLiteral("choice"), Qt::CaseInsensitive))
        return std::make_unique<Choice>();

    if (name == QStringLiteral("SEQUENCE") || name == QStringLiteral("SequenceType"))
        return std::make_unique<Sequence>();

    if (name == QStringLiteral("SEQUENCE_OF") || name == QStringLiteral("SequenceOfType"))
        return std::make_unique<SequenceOf>();

    return nullptr;
}
