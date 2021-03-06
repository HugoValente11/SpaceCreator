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

#include "abstractexportableobject.h"
#include "entityattribute.h"

#include <QVariant>

namespace ivm {
class IVObject;
}

namespace ive {

/**
 * @brief The ExportableIVObject is a common class to export ivm::IVObject and its successors
 */
class ExportableIVObject : public templating::AbstractExportableObject
{
    Q_GADGET
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString subscriber_name READ subscriber_name)
    Q_PROPERTY(QVariantList attributes READ attributes)
    Q_PROPERTY(QVariantList properties READ properties)
    Q_PROPERTY(QStringList path READ path)

public:
    explicit ExportableIVObject(const ivm::IVObject *ivObject = nullptr);

    QString name() const;
    QString subscriber_name() const;

    static QVariant createFrom(const ivm::IVObject *ivObject);

    QVariantList attributes() const;
    QVariantList properties() const;
    QStringList path() const;

protected:
    static QVariantList generateProperties(const EntityAttributes &attributes, bool isProperty);
};

}

Q_DECLARE_METATYPE(ive::ExportableIVObject)
Q_DECLARE_TYPEINFO(ive::ExportableIVObject, Q_MOVABLE_TYPE);

template<>
inline ive::ExportableIVObject qvariant_cast<ive::ExportableIVObject>(const QVariant &v)
{
    return *static_cast<const ive::ExportableIVObject *>(v.constData());
}
