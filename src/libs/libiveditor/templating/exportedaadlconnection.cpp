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

#include "exportedaadlconnection.h"

#include "tab_aadl/aadlconnection.h"

namespace taste3 {
namespace templating {

ExportedAADLConnection::ExportedAADLConnection(const aadl::AADLConnection *connection)
    : ExportedAADLObject(connection)
{
}

QString ExportedAADLConnection::sourceName() const
{
    return exportedObject<aadl::AADLConnection>()->sourceName();
}

QString ExportedAADLConnection::targetName() const
{
    return exportedObject<aadl::AADLConnection>()->targetName();
}

QString ExportedAADLConnection::sourceInterfaceName() const
{
    return exportedObject<aadl::AADLConnection>()->sourceInterfaceName();
}

QString ExportedAADLConnection::targetInterfaceName() const
{
    return exportedObject<aadl::AADLConnection>()->targetInterfaceName();
}

bool ExportedAADLConnection::sourceInterfaceIsRequired() const
{
    auto o = exportedObject<aadl::AADLConnection>();
    return o->sourceInterface() ? o->sourceInterface()->isRequired() : false;
}

bool ExportedAADLConnection::sourceInterfaceIsProvided() const
{
    auto o = exportedObject<aadl::AADLConnection>();
    return o->sourceInterface() ? o->sourceInterface()->isProvided() : false;
}

bool ExportedAADLConnection::targetInterfaceIsRequired() const
{
    auto o = exportedObject<aadl::AADLConnection>();
    return o->targetInterface() ? o->targetInterface()->isRequired() : false;
}

bool ExportedAADLConnection::targetInterfaceIsProvided() const
{
    auto o = exportedObject<aadl::AADLConnection>();
    return o->targetInterface() ? o->targetInterface()->isProvided() : false;
}

} // ns templating
} // ns taste3
