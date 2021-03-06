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
class IVConnection;
}

namespace ive {

/**
 * @brief The ExportableIVConnection is a class to export IVConnection
 */
class ExportableIVConnection : public ExportableIVObject
{
    Q_GADGET
    Q_PROPERTY(QString source READ sourceName)
    Q_PROPERTY(QString target READ targetName)
    Q_PROPERTY(QString siName READ sourceInterfaceName)
    Q_PROPERTY(QString tiName READ targetInterfaceName)
    Q_PROPERTY(bool siIsRequired READ sourceInterfaceIsRequired)
    Q_PROPERTY(bool siIsProvided READ sourceInterfaceIsProvided)
    Q_PROPERTY(bool tiIsRequired READ targetInterfaceIsRequired)
    Q_PROPERTY(bool tiIsProvided READ targetInterfaceIsProvided)

public:
    explicit ExportableIVConnection(const ivm::IVConnection *connection = nullptr);

    QString sourceName() const;
    QString targetName() const;
    QString sourceInterfaceName() const;
    QString targetInterfaceName() const;
    bool sourceInterfaceIsRequired() const;
    bool sourceInterfaceIsProvided() const;
    bool targetInterfaceIsRequired() const;
    bool targetInterfaceIsProvided() const;
};

}

Q_DECLARE_METATYPE(ive::ExportableIVConnection)
Q_DECLARE_TYPEINFO(ive::ExportableIVConnection, Q_MOVABLE_TYPE);
