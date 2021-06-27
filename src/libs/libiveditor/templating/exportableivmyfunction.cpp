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

#include "exportableivmyfunction.h"

#include "ivcomment.h"
#include "ivconnection.h"
#include "ivconnectiongroup.h"
#include "ivmyfunction.h"
#include "ivfunctiontype.h"
#include "ivinterface.h"
#include "ivinterfacechain.h"
#include "ivmodel.h"

namespace ive {

ExportableIVMyFunction::ExportableIVMyFunction(const ivm::IVFunctionType *function)
    : ExportableIVObject(function)
{
}

QVariantList ExportableIVMyFunction::interfaces() const
{
    QVariantList ifaces;
    for (const auto iface : exportedObject<ivm::IVFunctionType>()->interfaces())
        ifaces << createFrom(iface);
    return ifaces;
}

QVariantList ExportableIVMyFunction::functions() const
{
    QVariantList functions;
    const ivm::IVFunctionType *o = exportedObject<ivm::IVFunctionType>();
    for (const auto function : o->functionTypes())
        functions << createFrom(function);
    for (const auto function : o->functions())
        functions << createFrom(function);
    return functions;
}

QVariantList ExportableIVMyFunction::comments() const
{
    QVariantList comments;
    for (const auto comment : exportedObject<ivm::IVFunctionType>()->comments())
        comments << createFrom(comment);
    return comments;
}

QVariantList ExportableIVMyFunction::connections() const
{
    QVariantList connections;
    for (const auto connection : exportedObject<ivm::IVFunctionType>()->connections())
        connections << createFrom(connection);
    return connections;
}

QVariantList ExportableIVMyFunction::connectionGroups() const
{
    QVariantList connectionGroups;
    for (const auto connectionGroup : exportedObject<ivm::IVFunctionType>()->connectionGroups())
        connectionGroups << createFrom(connectionGroup);
    return connectionGroups;
}

QVariantList ExportableIVMyFunction::connectedFunctions() const
{
    const auto ivFunction = exportedObject<ivm::IVFunctionType>();
    QVariantList list;
    for (auto chain : ivm::IVInterfaceChain::linkedFunctions(ivFunction)) {
        list << QVariant::fromValue(chain);
    }
    return list;
}

QVariantList ExportableIVMyFunction::contextParameters() const
{
    QVariantList parameters;
    for (const ivm::ContextParameter &param : exportedObject<ivm::IVFunctionType>()->contextParams()) {
        parameters << QVariant::fromValue(param);
    }
    return parameters;
}

}
