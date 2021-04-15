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

#include "ivcommonprops.h"
#include "common.h"

#include <QObject>
#include <QVariant>
#include <QVector>
#include <memory>

namespace ivm {

struct IVObjectPrivate;
class IVModel;
class IVObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(shared::Id id READ id CONSTANT)

public:
    enum class Type
    {
        Unknown = 0,

        Function,
        FunctionType,
        RequiredInterface,
        ProvidedInterface,
        InterfaceGroup,
        Comment,
        Connection,
        ConnectionGroup,
    };
    Q_ENUM(Type)

    explicit IVObject(const IVObject::Type t, const QString &title = QString(), QObject *parent = nullptr,
            const shared::Id &id = shared::InvalidId);
    virtual ~IVObject();

    QString title() const;
    QString titleUI() const;
    shared::Id id() const;

    IVObject::Type type() const;

    QVector<qint32> coordinates() const;
    void setCoordinates(const QVector<qint32> &coordinates);
    meta::Props::Token coordinatesType() const;

    QStringList path() const;
    static QStringList path(const IVObject *obj);

    IVObject *parentObject() const;
    bool isFunction() const;
    bool isFunctionType() const;
    bool isInterfaceGroup() const;
    bool isRequiredInterface() const;
    bool isProvidedInterface() const;
    bool isInterface() const;
    bool isComment() const;
    bool isConnection() const;
    bool isConnectionGroup() const;
    bool isNestedInFunction() const;
    bool isNestedInFunctionType() const;
    bool isNested() const;

    QString groupName() const;
    void setGroupName(const QString &groupName);

    // "attributes" - payload data in the opening XML tag,
    // like "name" and "kind" below:
    // <Required_Interface name="run_forrest" kind="Sporadic">

    QHash<QString, QVariant> attrs() const;
    void setAttrs(const QHash<QString, QVariant> &attrs);
    QVariant attr(const QString &name, const QVariant &defaultValue = QVariant()) const;
    virtual void setAttr(const QString &name, const QVariant &val);
    void removeAttr(const QString &name);
    bool hasAttribute(const QString &attributeName, const QVariant &value = QVariant()) const;
    bool hasAttributes(const QHash<QString, QVariant> &attrs) const;

    // "properties" - XML children <Property>
    QHash<QString, QVariant> props() const;
    void setProps(const QHash<QString, QVariant> &props);
    QVariant prop(const QString &name, const QVariant &defaultValue = QVariant()) const;
    virtual void setProp(const QString &name, const QVariant &val);
    void removeProp(const QString &name);
    bool hasProperty(const QString &propertyName, const QVariant &value = QVariant()) const;
    bool hasProperties(const QHash<QString, QVariant> &props) const;

    void setObjectsModel(IVModel *model);
    IVModel *model() const;

    bool isRootObject() const;

    bool isGrouped() const;

    void setVisible(bool isVisible);
    bool isVisible() const;

    virtual bool postInit();
    virtual bool aboutToBeRemoved();

    template<class T>
    inline T as()
    {
        return qobject_cast<T>(this);
    }

    template<class T>
    inline const T as() const
    {
        return qobject_cast<const T>(this);
    }

    static void sortObjectList(QList<ivm::IVObject *> &objects);
    static QVector<qint32> coordinatesFromString(const QString &strCoordinates);
    static QString coordinatesToString(const QVector<qint32> &coordinates);

Q_SIGNALS:
    void titleChanged(const QString &title);
    void coordinatesChanged(const QVector<qint32> &coordinates);
    void visibilityChanged(bool visible);
    void groupChanged(const QString &groupName);
    void attributeChanged(const QString &name);
    void propertyChanged(const QString &name);
    void attributeChanged(ivm::meta::Props::Token attr = ivm::meta::Props::Token::Unknown);
    void propertyChanged(ivm::meta::Props::Token prop = ivm::meta::Props::Token::Unknown);

public Q_SLOTS:
    bool setTitle(const QString &title);
    bool setParentObject(ivm::IVObject *parentObject);

private:
    const std::unique_ptr<IVObjectPrivate> d;
};

inline uint qHash(const IVObject::Type &key, uint seed)
{
    return ::qHash(static_cast<uint>(key), seed);
}

}