/*
  Copyright (C) 2020 European Space Agency - <maxime.perrotin@esa.int>

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

#include "aadlnamevalidator.h"

#include "aadlobject.h"
#include "aadlconnection.h"
#include "aadlconnectiongroup.h"
#include "aadlfunction.h"
#include "aadlfunctiontype.h"
#include "aadlmodel.h"

#include <QDebug>
#include <QRegularExpression>

namespace ivm {

AADLNameValidator *AADLNameValidator::m_instance = nullptr;

static const QString namePattern("^[a-zA-Z][a-zA-Z0-9_]*[a-zA-Z0-9]$");
static const QString namePatternUI("^[a-zA-Z][a-zA-Z0-9 ]*[a-zA-Z0-9]$");

AADLNameValidator::AADLNameValidator()
    : m_typePrefixes {
        { AADLObject::Type::FunctionType, QObject::tr("Function_Type_") },
        { AADLObject::Type::Function, QObject::tr("Function_") },
        { AADLObject::Type::RequiredInterface, QObject::tr("RI_") },
        { AADLObject::Type::ProvidedInterface, QObject::tr("PI_") },
        { AADLObject::Type::InterfaceGroup, QObject::tr("Interface_Group_") },
        { AADLObject::Type::Comment, QObject::tr("Comment_") },
        { AADLObject::Type::Connection, QObject::tr("Connection_") },
        { AADLObject::Type::ConnectionGroup, QObject::tr("Connection_Group_") },
    }
{
}

AADLNameValidator *AADLNameValidator::instance()
{
    if (!m_instance)
        m_instance = new AADLNameValidator;
    return m_instance;
}

QString AADLNameValidator::encodeName(const AADLObject::Type t, const QString &name)
{
    if (name.isEmpty())
        return QString();

    switch (t) {
    case AADLObject::Type::ConnectionGroup:
    case AADLObject::Type::Connection:
    case AADLObject::Type::Function:
    case AADLObject::Type::FunctionType:
    case ivm::AADLObject::Type::ProvidedInterface:
    case ivm::AADLObject::Type::RequiredInterface: {
        QString result;
        std::transform(name.cbegin(), name.cend(), std::back_inserter(result),
                [](const QChar &ch) { return ch.isLetterOrNumber() ? ch : QLatin1Char('_'); });
        return result;
    }
    case AADLObject::Type::Comment: {
        QString result(name);
        result.replace('\n', "\\n");
        return result;
    }
    case AADLObject::Type::Unknown: {
        qWarning() << "Unknown object does not support naming";
        break;
    }
    default: {
        qWarning() << "Unsupported object type" << t;
        break;
    }
    }

    return name;
}

QString AADLNameValidator::decodeName(const AADLObject::Type t, const QString &name)
{
    if (name.isEmpty())
        return QString();

    switch (t) {
    case AADLObject::Type::ConnectionGroup:
    case AADLObject::Type::Connection:
    case AADLObject::Type::Function:
    case AADLObject::Type::FunctionType:
    case ivm::AADLObject::Type::ProvidedInterface:
    case ivm::AADLObject::Type::RequiredInterface: {
        QString result;
        std::transform(name.cbegin(), name.cend(), std::back_inserter(result),
                [](const QChar &ch) { return ch.isLetterOrNumber() ? ch : QLatin1Char(' '); });
        return result;
    }
    case AADLObject::Type::Comment: {
        QString result(name);
        result.replace("\\n", "\n");
        return result;
    }
    case AADLObject::Type::Unknown: {
        qWarning() << "Unknown object does not support naming";
        break;
    }
    default: {
        qWarning() << "Unsupported object type" << t;
        break;
    }
    }

    return name;
}

/*!
   Returns is the gieven \p name is usable as name in general.
 */
bool AADLNameValidator::isValidName(const QString &name)
{
    if (name.isEmpty()) {
        return false;
    }

    static QRegularExpression re(ivm::namePattern);
    QRegularExpressionMatch match = re.match(name);
    return match.hasMatch();
}

/*!
   The regualr expression pattern for aadl names, used for raw/system/model
 */
const QString &AADLNameValidator::namePattern()
{
    return ivm::namePattern;
}

/*!
   The regualr expression pattern for aadl names, used for the UI (encoded name)
 */
const QString &AADLNameValidator::namePatternUI()
{
    return ivm::namePatternUI;
}

/*!
   Check if the name can be used for that object.
   It checks if the name is usable at all, and if the names is used already by another relevant object
 */
bool AADLNameValidator::isAcceptableName(const AADLObject *object, const QString &name)
{
    if (!object || !isValidName(name)) {
        return false;
    }

    const AADLObject::Type t = object->aadlType();
    switch (t) {
    case AADLObject::Type::FunctionType: {
        return instance()->isFunctionTypeNameUsed(name, object);
    }
    case AADLObject::Type::Function: {
        return instance()->isFunctionNameUsed(name, object);
    }
    case AADLObject::Type::RequiredInterface: {
        auto parent = object->parentObject() ? object->parentObject()->as<const AADLFunctionType *>() : nullptr;
        return instance()->isRequiredInterfaceNameUsed(name, parent);
    }
    case AADLObject::Type::ProvidedInterface: {
        auto parent = object->parentObject() ? object->parentObject()->as<const AADLFunctionType *>() : nullptr;
        return instance()->isProvidedInterfaceNameUsed(name, parent);
    }
    case AADLObject::Type::InterfaceGroup:
    case AADLObject::Type::ConnectionGroup:
    case AADLObject::Type::Connection:
    case AADLObject::Type::Comment: {
        return true;
    }
    case AADLObject::Type::Unknown: {
        qWarning() << "Unsupported object type" << t;
        return false;
    }
    default: {
        qWarning() << "Not implemented yet for" << t;
        return false;
    }
    }

    Q_UNREACHABLE();
    return false;
}

bool AADLNameValidator::isAutogeneratedName(const AADLObject *object, const QString &nameForCheck)
{
    if (!object)
        return false;

    const QString name = nameForCheck.isEmpty() ? object->title() : nameForCheck;
    if (name.isEmpty())
        return false;

    const AADLObject::Type t = object->aadlType();
    switch (t) {
    case AADLObject::Type::Function:
    case AADLObject::Type::FunctionType:
    case AADLObject::Type::RequiredInterface:
    case AADLObject::Type::ProvidedInterface:
    case AADLObject::Type::Comment: {
        const QString preffix = instance()->m_typePrefixes[t];
        if (!name.startsWith(preffix))
            return false;

        if (name == preffix)
            return false;

        for (int i = preffix.length(); i < name.length() - 1; ++i)
            if (!name.at(i).isDigit())
                return false;

        return true;
    }
    case AADLObject::Type::ConnectionGroup:
    case AADLObject::Type::InterfaceGroup:
    case AADLObject::Type::Connection: {
        return false;
    }
    case AADLObject::Type::Unknown: {
        qWarning() << "unknown object type";
        return false;
    }
    default: {
        qWarning() << "unsupported object type";
        return false;
    }
    }

    Q_UNREACHABLE();
    return false;
}

QString AADLNameValidator::nameForInstance(const AADLFunction *object, const QString &suggestedName)
{
    if (suggestedName.isEmpty()) {
        return nextNameFor(object);
    } else if (isAcceptableName(object, suggestedName)) {
        return suggestedName;
    }
    return instance()->makeCountedName(object, suggestedName, 2);
}

QString AADLNameValidator::nameOfType(AADLObject::Type t)
{
    switch (t) {
    case AADLObject::Type::Function:
    case AADLObject::Type::FunctionType:
    case AADLObject::Type::InterfaceGroup:
    case AADLObject::Type::RequiredInterface:
    case AADLObject::Type::ProvidedInterface:
    case AADLObject::Type::Comment:
    case AADLObject::Type::ConnectionGroup:
    case AADLObject::Type::Connection: {
        return instance()->m_typePrefixes[t];
    }
    case AADLObject::Type::Unknown: {
        const QString wrn = QObject::tr("Unknown object type");
        qWarning() << wrn;
        return wrn;
    }
    default: {
        const QString wrn = QObject::tr("Unsupported object type");
        qWarning() << wrn;
        return wrn;
    }
    }
    Q_UNREACHABLE();
    return QString();
}

QString AADLNameValidator::nextNameFor(const AADLObject *object)
{
    return instance()->nextName(object);
}

QString AADLNameValidator::nextName(const AADLObject *object) const
{
    if (!object) {
        return QString();
    }

    const AADLObject::Type t = object->aadlType();
    switch (t) {
    case AADLObject::Type::Function:
        return nameFunction(object);
    case AADLObject::Type::FunctionType:
        return nameFunctionType(object);
    case AADLObject::Type::RequiredInterface:
        return nameRequiredInterface(object);
    case AADLObject::Type::ProvidedInterface:
        return nameProvidedInterface(object);
    case AADLObject::Type::Comment:
        return nameComment(object);
    case AADLObject::Type::ConnectionGroup:
    case AADLObject::Type::Connection:
        return nameConnection(object);
    case AADLObject::Type::InterfaceGroup:
    case AADLObject::Type::Unknown:
        return QString();
    default:
        break;
    }

    qWarning() << "Unsupported object type:" << t;
    return QString();
}

QString AADLNameValidator::makeCountedName(const AADLObject *object, const QString &nameTemplate, int counter) const
{
    QString name = nameTemplate + QString::number(counter);
    while (!isAcceptableName(object, name)) {
        name = nameTemplate + QString::number(++counter);
    }
    return name;
}

QString AADLNameValidator::nameFunctionType(const AADLObject *functionType) const
{
    const QString nameTemplate =
            functionType->title().isEmpty() ? m_typePrefixes[functionType->aadlType()] : functionType->title();

    int counter = 0;
    if (functionType && functionType->objectsModel()) {
        for (const auto fn : functionType->objectsModel()->objects()) {
            if (fn->isFunctionType()) {
                ++counter;
            }
        }
    } else {
        counter = 0;
    }
    ++counter;

    return makeCountedName(functionType, nameTemplate, counter);
}

QString AADLNameValidator::nameFunction(const AADLObject *function) const
{
    const QString nameTemplate = function->title().isEmpty() ? m_typePrefixes[function->aadlType()] : function->title();

    int counter = 0;
    if (function && function->objectsModel()) {
        for (const auto fn : function->objectsModel()->objects()) {
            if (fn->isFunction()) {
                ++counter;
            }
        }
    } else {
        counter = 0;
    }
    ++counter;

    return makeCountedName(function, nameTemplate, counter);
}

QString AADLNameValidator::nameRequiredInterface(const AADLObject *iface) const
{
    Q_ASSERT(iface);

    const QString nameTemplate = m_typePrefixes[iface->aadlType()];

    const auto parent = iface->parentObject()->as<const AADLFunctionType *>();
    int counter = parent ? parent->ris().size() : 0;
    ++counter;

    return makeCountedName(iface, nameTemplate, counter);
}

QString AADLNameValidator::nameProvidedInterface(const AADLObject *iface) const
{
    Q_ASSERT(iface);

    const QString nameTemplate = m_typePrefixes[iface->aadlType()];

    const auto parent = iface->parentObject()->as<const AADLFunctionType *>();
    int counter = parent ? parent->pis().size() : 0;
    ++counter;

    return makeCountedName(iface, nameTemplate, counter);
}

QString AADLNameValidator::nameComment(const AADLObject *comment) const
{
    const QString nameTemplate = m_typePrefixes[comment->aadlType()];

    int counter = 0;
    if (comment && comment->objectsModel()) {
        for (const auto fn : comment->objectsModel()->objects())
            if (fn->isComment())
                ++counter;
    } else
        counter = 0;
    ++counter;

    return makeCountedName(comment, nameTemplate, counter);
}

QString AADLNameValidator::nameConnection(const AADLObject *connection) const
{
    if (auto connectionPtr = qobject_cast<const AADLConnection *>(connection)) {
        return QString("%1.%2 <-> %3.%4")
                .arg(connectionPtr->sourceName(), connectionPtr->sourceInterfaceName(), connectionPtr->targetName(),
                        connectionPtr->targetInterfaceName());
    } else if (auto connectionPtr = qobject_cast<const AADLConnectionGroup *>(connection)) {
        QStringList sourceNames, targetNames;
        for (const auto sourceIface : connectionPtr->groupedSourceInterfaces()) {
            sourceNames.append(sourceIface->title());
        }
        for (const auto targetIface : connectionPtr->groupedTargetInterfaces()) {
            targetNames.append(targetIface->title());
        }
        return QString("%1.{%2} <-> %3.{%4}")
                .arg(connectionPtr->sourceName(), sourceNames.join(QLatin1Char('|')), connectionPtr->targetName(),
                        targetNames.join(QLatin1Char('|')));
    }
    return {};
}

bool AADLNameValidator::isFunctionTypeNameUsed(const QString &name, const AADLObject *fnType) const
{
    if (name.isEmpty() || !fnType) {
        return false;
    }

    if (fnType->objectsModel()) {
        for (const auto fn : fnType->objectsModel()->objects()) {
            if (fn->isFunctionType()) {
                if (fn->title() == name) {
                    return false;
                }
            }
        }

        return true;
    }

    if (fnType->parentObject()) {
        if (const AADLFunction *parent = fnType->parentObject()->as<const AADLFunction *>()) {
            for (const auto c : parent->children()) {
                if (c->isFunctionType()) {
                    if (c->title() == name) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool AADLNameValidator::isFunctionNameUsed(const QString &name, const AADLObject *function) const
{
    if (name.isEmpty() || !function) {
        return false;
    }

    if (function->objectsModel()) {
        for (const auto fn : function->objectsModel()->objects()) {
            if (fn->isFunction()) {
                if (fn->title() == name) {
                    return false;
                }
            }
        }

        return true;
    }

    if (auto fn = function->as<const AADLFunction *>()) {
        for (const auto c : fn->children()) {
            if (c->isFunction()) {
                if (c->title() == name) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool AADLNameValidator::isRequiredInterfaceNameUsed(const QString &name, const AADLFunctionType *parent) const
{
    if (name.isEmpty()) {
        return false;
    }

    for (const auto ri : parent->ris()) {
        if (ri->title() == name) {
            return false;
        }
    }

    return true;
}

bool AADLNameValidator::isProvidedInterfaceNameUsed(const QString &name, const AADLFunctionType *parent) const
{
    if (name.isEmpty()) {
        return false;
    }

    for (const auto pi : parent->pis()) {
        if (pi->title() == name) {
            return false;
        }
    }

    return true;
}

}
