/*
   Copyright (C) 2019 European Space Agency - <maxime.perrotin@esa.int>

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

#include "exportableivobject.h"

namespace ivm {
class IVFunctionType;
}

namespace ive {

/**
 * @brief The ExportableIVFunction is a class to export IVFunctionType and IVFunction
 */
class ExportableIVFunction : public ExportableIVObject
{
    Q_GADGET
    Q_PROPERTY(QVariantList interfaces READ interfaces)
    Q_PROPERTY(QVariantList functions READ functions)
    Q_PROPERTY(QVariantList comments READ comments)
    Q_PROPERTY(QVariantList connections READ connections)
    Q_PROPERTY(QVariantList connectionGroups READ connectionGroups)
    Q_PROPERTY(QVariantList connectedFunctions READ connectedFunctions)
    Q_PROPERTY(QVariantList contextParameters READ contextParameters)

public:
    explicit ExportableIVFunction(const ivm::IVFunctionType *function = nullptr);

    QVariantList interfaces() const;
    QVariantList functions() const;
    QVariantList comments() const;
    QVariantList connections() const;
    QVariantList connectionGroups() const;
    QVariantList connectedFunctions() const;
    QVariantList contextParameters() const;
};

}

Q_DECLARE_METATYPE(ive::ExportableIVFunction)
Q_DECLARE_TYPEINFO(ive::ExportableIVFunction, Q_MOVABLE_TYPE);
