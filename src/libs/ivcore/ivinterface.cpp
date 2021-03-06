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

#include "ivinterface.h"

#include "ivcommonprops.h"
#include "ivconnection.h"
#include "ivcoreutils.h"
#include "ivfunction.h"
//#include "ivmyfunction.h"
#include "ivmodel.h"
#include "ivnamevalidator.h"

#include <QDebug>
#include <QMetaEnum>

namespace ivm {

IVInterface::CreationInfo::CreationInfo(IVModel *model, IVFunctionType *function, const QPointF &position,
        IVInterface::InterfaceType type, const shared::Id &id, const QVector<InterfaceParameter> &parameters,
        OperationKind kind, const QString &name, const CreationInfo::Policy policy, IVInterface *source)
    : model(model)
    , function(function)
    , position(position)
    , type(type)
    , id(id)
    , parameters(parameters)
    , kind(kind)
    , name(name)
    , policy(policy)
    , toBeCloned(policy == Policy::Clone ? source : nullptr)
{
}

QVariantList IVInterface::CreationInfo::toVarList() const
{
    return { QVariant::fromValue(*this) };
}

IVInterface::CreationInfo IVInterface::CreationInfo::initFromIface(
        IVInterface *iface, const CreationInfo::Policy policy)
{
    if (!iface)
        return {};

    auto getIfacePos = [](const QVector<qint32> &coordinates) {
        return coordinates.size() < 2 ? QPointF() : QPointF(coordinates.front(), coordinates.back());
    };

    return { iface->model(), (iface->parentObject() ? iface->parentObject()->as<IVFunctionType *>() : nullptr), {},
        iface->direction(), iface->id(), iface->params(), iface->kind(), iface->title(), policy, iface };
}

IVInterface::CreationInfo IVInterface::CreationInfo::fromIface(IVInterface *iface)
{
    IVInterface::CreationInfo info = initFromIface(iface, IVInterface::CreationInfo::Policy::Init);
    if (IVNameValidator::isAutogeneratedName(iface))
        info.name = QString();
    return info;
}

IVInterface::CreationInfo IVInterface::CreationInfo::cloneIface(IVInterface *iface, IVFunction *fn)
{
    IVInterface::CreationInfo info = initFromIface(iface, IVInterface::CreationInfo::Policy::Clone);
    info.function = fn;
    info.model = fn->model();
    return info;
}

void IVInterface::CreationInfo::resetKind()
{
    if (IVInterface::InterfaceType::Required == type)
        kind = IVInterface::OperationKind::Any;
    else {
        if (IVInterface::OperationKind::Any == kind)
            kind = IVInterface::OperationKind::Sporadic;
    }
}

struct IVInterfacePrivate {
    explicit IVInterfacePrivate(IVInterface::InterfaceType dir)
        : m_direction(dir)
    {
    }
    IVInterface::InterfaceType m_direction;
    QVector<InterfaceParameter> m_params = {};
    QPointer<IVInterface> m_cloneOf { nullptr };
    QVector<QPointer<IVInterface>> m_clones {};
};

IVInterface::IVInterface(IVObject::Type ifaceType, const CreationInfo &ci)
    : IVObject(ifaceType, ci.name, ci.function, ci.toBeCloned ? shared::createId() : ci.id)
    , d(new IVInterfacePrivate(Type::InterfaceGroup == ifaceType
                      ? ci.type
                      : Type::RequiredInterface == ifaceType ? IVInterface::InterfaceType::Required
                                                             : IVInterface::InterfaceType::Provided))
{
    setKind(ci.kind);
    setParams(ci.parameters);

    if (ci.toBeCloned)
        setCloneOrigin(ci.toBeCloned);
}

IVInterface::~IVInterface()
{
    if (d->m_cloneOf)
        d->m_cloneOf->forgetClone(this);
}

IVInterface::InterfaceType IVInterface::direction() const
{
    return d->m_direction;
}

bool IVInterface::postInit(QString *warning)
{
    if (!model() || !function()) {
        return false;
    }
    if (!function()->isFunction()) {
        return IVObject::postInit(warning);
    }

    IVFunction *fn = function()->as<IVFunction *>();
    if (!fn) {
        return IVObject::postInit(warning);
    }

    const QString prototypeName =
            fn->entityAttributeValue(meta::Props::token(meta::Props::Token::instance_of)).toString();
    if (prototypeName.isEmpty()) {
        return IVObject::postInit(warning);
    }

    IVFunctionType *prototype = model()->getFunctionType(prototypeName, Qt::CaseInsensitive);
    if (!prototype) {
        prototype = model()->getSharedFunctionType(prototypeName, Qt::CaseInsensitive);
    }
    if (!prototype) {
        return false;
    }
    const QVector<ivm::IVInterface *> &fnTypeIfaces = prototype->interfaces();
    for (auto fnTypeIface : fnTypeIfaces) {
        if (ivm::IVInterface *existingIface = utils::findExistingClone(fn, fnTypeIface)) {
            existingIface->setCloneOrigin(fnTypeIface);
        }
    }
    return IVObject::postInit(warning);
}

bool IVInterface::isProvided() const
{
    return direction() == InterfaceType::Provided;
}
bool IVInterface::isRequired() const
{
    return direction() == InterfaceType::Required;
}

QMap<IVInterface::OperationKind, QString> IVInterface::availableKindNames() const
{
    QMap<IVInterface::OperationKind, QString> result;
    if (result.isEmpty()) {
        const QMetaEnum &me = QMetaEnum::fromType<ivm::IVInterface::OperationKind>();
        for (int i = 0; i < me.keyCount(); ++i) {
            const IVInterface::OperationKind k = static_cast<IVInterface::OperationKind>(me.value(i));
            if ((isProvided() && k == OperationKind::Any) || (isRequired() && k == OperationKind::Cyclic)) {
                continue;
            }
            result.insert(k, QString(me.key(i)));
        }
    }
    return result;
}

QString IVInterface::kindToString(IVInterface::OperationKind k)
{
    const QMetaEnum &me = QMetaEnum::fromType<ivm::IVInterface::OperationKind>();
    return QString::fromLatin1(me.valueToKey(static_cast<int>(k)));
}

IVInterface::OperationKind IVInterface::kindFromString(const QString &k, IVInterface::OperationKind defaultKind)
{
    const QMetaEnum &me = QMetaEnum::fromType<ivm::IVInterface::OperationKind>();
    bool ok = false;
    const auto kind = static_cast<IVInterface::OperationKind>(me.keyToValue(k.toLatin1().data(), &ok));
    return ok ? kind : defaultKind;
}

IVInterface::OperationKind IVInterface::defaultKind() const
{
    return isProvided() ? OperationKind::Sporadic : OperationKind::Any;
}

/*! This returns the title. Unless it's an RI of type inherited, in which case it returns a comma
 * separated list of PI connected to it - unless none are and then it's back to title.
 */
QString IVInterface::ifaceLabel() const
{
    return title();
}

IVInterface::OperationKind IVInterface::kindFromString(const QString &k) const
{
    return kindFromString(k, defaultKind());
}

IVInterface::OperationKind IVInterface::kind() const
{
    return kindFromString(entityAttributeValue<QString>(meta::Props::token(meta::Props::Token::kind)));
}

bool IVInterface::setKind(IVInterface::OperationKind k)
{
    if (this->kind() != k) {
        setEntityAttribute(meta::Props::token(meta::Props::Token::kind), kindToString(k));
        return true;
    }

    return false;
}

QVector<InterfaceParameter> IVInterface::params() const
{
    return d->m_params;
}

InterfaceParameter IVInterface::param(const QString &name) const
{
    if (!name.isEmpty())
        for (const InterfaceParameter &param : params())
            if (param.name() == name)
                return param;
    return {};
}

void IVInterface::setParams(const QVector<InterfaceParameter> &params)
{
    if (d->m_params != params) {
        d->m_params = params;
        Q_EMIT paramsChanged();
    }
}

void IVInterface::addParam(const InterfaceParameter &param)
{
    if (!d->m_params.contains(param)) {
        d->m_params.append(param);
        Q_EMIT paramsChanged();
    }
}

IVFunctionType *IVInterface::function() const
{
    return qobject_cast<IVFunctionType *>(parentObject());
}

/*!
   Returns the parent functions, the grand parent function, ... as a list.
   The first function is the direct parent function
 */
QList<IVFunction *> IVInterface::functionsStack() const
{
    QList<IVFunction *> result;
    IVFunction *parentFunc = qobject_cast<IVFunction *>(parentObject());
    while (parentFunc) {
        result.append(parentFunc);
        parentFunc = qobject_cast<IVFunction *>(parentFunc->parentObject());
    }
    return result;
}

IVInterface *IVInterface::cloneOf() const
{
    return d->m_cloneOf;
}

bool IVInterface::isClone() const
{
    return nullptr != cloneOf();
}

bool IVInterface::isCloned() const
{
    return d->m_clones.size();
}

QVector<QPointer<IVInterface>> IVInterface::clones() const
{
    return d->m_clones;
}

void IVInterface::setCloneOrigin(IVInterface *source)
{
    if (d->m_cloneOf != source) {
        d->m_cloneOf = source;
        if (d->m_cloneOf) {
            d->m_cloneOf->rememberClone(this);
        }
    }
}

void IVInterface::rememberClone(IVInterface *clone)
{
    if (clone && !d->m_clones.contains(clone)) {
        clone->cloneInternals(this);
        d->m_clones.append(clone);
    }
}

void IVInterface::forgetClone(IVInterface *clone)
{
    if (clone)
        clone->restoreInternals(this);
    d->m_clones.removeAll(clone);
}

void IVInterface::setAttributeImpl(const QString &name, const QVariant &value, EntityAttribute::Type type)
{
    if (meta::Props::token(name) == meta::Props::Token::name && value.isValid()) {
        const QString newName = value.toString();
        if (auto fn = function()) {
            for (IVInterface *iface : isRequiredInterface() ? fn->ris() : fn->pis()) {
                if (iface->title() == newName && iface != this) {
                    IVObject::setAttributeImpl(name, IVNameValidator::nextNameFor(iface), type);
                    return;
                }
            }
        }
    }

    IVObject::setAttributeImpl(name, value, type);
}

IVInterfaceProvided::IVInterfaceProvided(const CreationInfo &ci)
    : IVInterface(IVObject::Type::ProvidedInterface, ci)
{
}

IVInterfaceRequired::IVInterfaceRequired(const CreationInfo &ci)
    : IVInterface(IVObject::Type::RequiredInterface, ci)
{
    setEntityProperty(meta::Props::token(meta::Props::Token::Autonamed), IVNameValidator::isAutogeneratedName(this));
}

IVInterface *IVInterface::createIface(const CreationInfo &descr)
{
    IVInterface *iface { nullptr };
    if (descr.type == IVInterface::InterfaceType::Provided)
        iface = new IVInterfaceProvided(descr);
    else if (descr.type == IVInterface::InterfaceType::Required)
        iface = new IVInterfaceRequired(descr);
    else
        qFatal("Unsupported interface type");
    iface->setKind(descr.kind);
    iface->setTitle(descr.name);

    return iface;
}

bool IVInterface::storedKindDiffers() const
{
    const QString nameKind = meta::Props::token(meta::Props::Token::kind);
    auto it = m_originalFields.attrs.constFind(nameKind);
    return it == m_originalFields.attrs.cend() || it.value().value() == entityAttributeValue(nameKind);
}

void IVInterface::cloneInternals(const IVInterface *from)
{
    if (!from)
        return;

    if (!m_originalFields.collected())
        m_originalFields.collect(isClone() ? d->m_cloneOf.data() : this);

    {
        QSignalBlocker sb(this);
        reflectAttrs(from);
        reflectParams(from);
    }

    if (storedKindDiffers()) {
        Q_EMIT attributeChanged(meta::Props::token(meta::Props::Token::kind));
    }

    connect(from, &IVInterface::attributeChanged, this, &IVInterface::onReflectedAttrChanged, Qt::UniqueConnection);
    connect(from, &IVInterface::paramsChanged, this, &IVInterface::onReflectedParamsChanged, Qt::UniqueConnection);
}

void IVInterface::restoreInternals(const IVInterface *disconnectMe)
{
    disconnect(disconnectMe, &IVInterface::attributeChanged, this, &IVInterface::onReflectedAttrChanged);
    disconnect(disconnectMe, &IVInterface::paramsChanged, this, &IVInterface::onReflectedParamsChanged);

    if (!m_originalFields.collected())
        return;

    // keep current coordinates:
    for (meta::Props::Token t : { meta::Props::Token::InnerCoordinates, meta::Props::Token::coordinates }) {
        const QString &name = meta::Props::token(t);
        m_originalFields.attrs[name] = entityAttribute(name);
    }

    const bool kindChanged = storedKindDiffers();
    {
        QSignalBlocker sb(this);

        setEntityAttributes(m_originalFields.attrs);
        setParams(m_originalFields.params);
    }

    if (kindChanged) {
        Q_EMIT attributeChanged(meta::Props::token(meta::Props::Token::kind));
    }

    m_originalFields = {};
}

QVariant IVInterface::originalAttributeValue(const QString &name) const
{
    return m_originalFields.attrs.value(name).value();
}

QVector<InterfaceParameter> IVInterface::originalParams() const
{
    return m_originalFields.params;
}

void IVInterface::onReflectedAttrChanged(const QString &attrName)
{
    Q_UNUSED(attrName);

    if (IVInterface *iface = qobject_cast<IVInterface *>(sender()))
        reflectAttrs(iface);
}

void IVInterface::onReflectedParamsChanged()
{
    if (IVInterface *iface = qobject_cast<IVInterface *>(sender()))
        reflectParams(iface);
}

void IVInterface::reflectAttrs(const IVInterface *from)
{
    if (!from)
        return;

    auto revertAttribute = [](const meta::Props::Token &token, EntityAttributes &attributes,
                                   const EntityAttributes &attrsOriginal) {
        const QString name = meta::Props::token(token);
        auto it = attrsOriginal.constFind(name);
        if (it != attrsOriginal.cend()) {
            attributes[name] = *it;
        }
    };

    EntityAttributes newAttrs = from->entityAttributes();

    const bool isFunctionTypeInherited = from->isNestedInFunctionType();
    static const QString autonamedProp = meta::Props::token(meta::Props::Token::Autonamed);
    const bool isCustomName =
            !(m_originalFields.collected() ? m_originalFields.attrs.value(autonamedProp).value().value<bool>()
                                           : entityAttributeValue(autonamedProp, true));
    const bool keepName = isFunctionTypeInherited || isCustomName;
    if (keepName)
        revertAttribute(meta::Props::Token::name, newAttrs, m_originalFields.attrs);

    for (auto t : { meta::Props::Token::InheritPI, meta::Props::Token::coordinates,
                 meta::Props::Token::InnerCoordinates, meta::Props::Token::Autonamed }) {
        const bool isInheritPIFlag = t == meta::Props::Token::InheritPI;
        const bool isInheritedPI = !isFunctionTypeInherited && isRequired() && from->isProvided();
        if (!isInheritPIFlag || isInheritedPI)
            revertAttribute(t, newAttrs, entityAttributes());
    }

    setEntityAttributes(newAttrs);
}

void IVInterface::reflectParams(const IVInterface *from)
{
    if (!from)
        return;

    setParams(from->params());
}

void IVInterfaceRequired::setAttributeImpl(
        const QString &attributeName, const QVariant &value, EntityAttribute::Type type)
{
    if (!attributeName.isEmpty()) {
        const meta::Props::Token t = meta::Props::token(attributeName);
        switch (t) {
        case meta::Props::Token::InheritPI: {
            const bool newVal = value.toBool();
            if (entityAttributeValue<bool>(attributeName) != newVal) {
                // should be handled in Connection _before_ the actual value change:
                Q_EMIT inheritedLabelsChanged(inheritedLables());

                IVInterface::setAttributeImpl(attributeName, value, type);
                Q_EMIT propChanged_InheritPI(newVal);
            }
        } break;
        case meta::Props::Token::name: {
            QString usedName = value.toString();
            bool autoName = true;
            if (usedName.isEmpty()) {
                if (isInheritPI() && !m_prototypes.isEmpty()) {
                    usedName = m_prototypes.first()->title();
                } else {
                    usedName = IVNameValidator::nextNameFor(this);
                }
                autoName = true;
            } else {
                if (isInheritPI()) {
                    const auto it = std::find_if(m_prototypes.cbegin(), m_prototypes.cend(),
                            [usedName](const ivm::IVInterfaceProvided *iface) { return iface->title() == usedName; });
                    if (it != m_prototypes.cend()) {
                        autoName = true;
                    } else {
                        autoName = false;
                    }
                    if (!IVNameValidator::isAcceptableName(this, usedName)) {
                        if (it != m_prototypes.cend()) {
                            usedName = (*it)->function()->title() + QLatin1Char('_') + (*it)->title();
                            autoName = true;
                        } else {
                            return;
                        }
                    }
                } else {
                    autoName = false;
                }
            }

            static const QString autonamedPropName = meta::Props::token(meta::Props::Token::Autonamed);
            if (m_originalFields.collected()) {
                m_originalFields.attrs[autonamedPropName].setValue(autoName);
                if (!autoName)
                    m_originalFields.attrs[attributeName].setValue(usedName);
            }
            IVInterface::setEntityProperty(autonamedPropName, autoName);
            IVInterface::setAttributeImpl(attributeName, usedName, type);
            return;
        }
        default:
            break;
        }

        IVInterface::setAttributeImpl(attributeName, value, type);
    }
}

QStringList IVInterfaceRequired::inheritedLables() const
{
    QStringList result;

    const QString &currentTitle = title();
    if (currentTitle != m_originalFields.name())
        result.append(currentTitle);
    else if (IVNameValidator::isAutogeneratedName(this) && isInheritPI()) {
        result = collectInheritedLabels();

        // append suffix for connection to the same named PIs with same parent (Function.PI becomes
        // Funtcion.PI#N)
        namesForRIToPIs(result);

        // if 2+ FnA.RI connected to the same FnB.PI, populate the inherited name with number suffix
        // (based on the index of the Connection among related connections)
        namesForRIsToPI(result);
    }

    if (result.isEmpty())
        result.append(currentTitle);

    return result;
}

QStringList IVInterfaceRequired::collectInheritedLabels() const
{
    QStringList result, titles;
    std::transform(m_prototypes.cbegin(), m_prototypes.cend(), std::back_inserter(titles),
            [](const IVInterfaceProvided *pi) { return pi->title(); });

    for (const IVInterfaceProvided *pi : m_prototypes) {
        QString label = pi->title();
        if (titles.count(label) > 1) {
            Q_ASSERT(pi->parentObject());
            // if PIs have same name, prepend it with parent's name
            label = pi->parentObject()->title() + "." + label;
        }
        result.prepend(label);
    }
    return result;
}

void IVInterfaceRequired::namesForRIToPIs(QStringList &result) const
{
    for (const QString &label : result) {
        int count = result.count(label);
        if (count > 1)
            while (count >= 1) {
                const int pos = result.indexOf(label);
                if (pos != -1)
                    result.replace(pos, label + "#" + QString::number(count));
                count = result.count(label);
            }
    }
}

void IVInterfaceRequired::namesForRIsToPI(QStringList &result) const
{
    const IVFunctionType *parentFn = function();
    if (!parentFn)
        return;

    auto findRI = [](const IVConnection *in, const IVInterfaceProvided *pi) -> IVInterfaceRequired * {
        if (in && pi)
            for (IVInterface *iface : { in->sourceInterface(), in->targetInterface() })
                if (iface && iface != pi && iface->isRequired())
                    return qobject_cast<IVInterfaceRequired *>(iface);
        return nullptr;
    };

    const shared::Id &parentId = parentFn->id();
    for (const IVInterfaceProvided *pi : m_prototypes) {
        const QVector<IVConnection *> &relatedConnecions = model()->getConnectionsForIface(pi->id());
        for (auto i = relatedConnecions.crbegin(); i != relatedConnecions.crend(); ++i) {
            IVConnection *c = *i;

            const bool sameSrcFn = c->source() && c->source()->id() == parentId;
            const bool sameDstFn = c->target() && c->target()->id() == parentId;
            const bool sameFn = sameSrcFn || sameDstFn;
            const bool isMeSrc = sameFn && c->sourceInterface() == this;
            const bool isMeDst = sameFn && c->targetInterface() == this;
            const bool toSibling = !isMeSrc && !isMeDst;
            if (toSibling)
                if (IVInterfaceRequired *otherRI = findRI(c, pi)) {
                    if (!otherRI->isInheritPI())
                        continue;

                    const QString &oldLabel = pi->title();
                    const int labelPos = result.indexOf(oldLabel);
                    if (labelPos >= 0) {
                        result.replace(labelPos, oldLabel + "#" + QString::number(relatedConnecions.indexOf(c)));
                    }
                }
        }
    }
}

void IVInterfaceRequired::setPrototype(const IVInterfaceProvided *pi)
{
    if (!pi || !isInheritPI())
        return;

    if (!m_prototypes.contains(pi))
        m_prototypes.append(pi);

    if (!m_prototypes.isEmpty())
        cloneInternals(pi);

    Q_EMIT inheritedLabelsChanged(inheritedLables());
}

void IVInterfaceRequired::unsetPrototype(const IVInterfaceProvided *pi)
{
    if (!pi)
        return;

    m_prototypes.removeAll(pi);
    if (m_prototypes.isEmpty() || !isInheritPI())
        restoreInternals(pi);

    Q_EMIT inheritedLabelsChanged(inheritedLables());
}

QString IVInterfaceRequired::ifaceLabel() const
{
    return ifaceLabelList().join(", ");
}

/*! Get the list of labels for this interface. It will either have the
 * inherited names or, if this is empty, the title. There will always
 * be at least one in the returned list.
 */
QStringList IVInterfaceRequired::ifaceLabelList() const
{
    if (isInheritPI()) {
        const QStringList &labels = inheritedLables();
        if (!labels.isEmpty()) {
            return labels;
        }
    }
    return { IVInterface::ifaceLabel() };
}

bool IVInterfaceRequired::isInheritPI() const
{
    return entityAttributeValue<bool>(meta::Props::token(meta::Props::Token::InheritPI));
}

bool IVInterfaceRequired::hasPrototypePi() const
{
    return m_prototypes.size();
}

void IVInterfaceRequired::cloneInternals(const IVInterface *from)
{
    IVInterface::cloneInternals(from);

    if (const IVFunctionType *fn = function()) {
        const QVector<IVInterface *> ifaces = fn->interfaces();
        const auto it = std::find_if(ifaces.cbegin(), ifaces.cend(), [this](const IVInterface *iface) {
            return iface != this && iface->isRequired() && iface->title() == title();
        });
        if (it != ifaces.cend()) {
            setTitle(from->function()->title() + QLatin1Char('_') + from->title());
        }
    }

    if (const IVInterfaceRequired *ri = from->as<const IVInterfaceRequired *>())
        connect(ri, &IVInterfaceRequired::propChanged_InheritPI, this, &IVInterfaceRequired::propChanged_InheritPI,
                Qt::UniqueConnection);
}

void IVInterfaceRequired::restoreInternals(const IVInterface *disconnectMe)
{
    IVInterface::restoreInternals(disconnectMe);
    if (const IVInterfaceRequired *ri = disconnectMe->as<const IVInterfaceRequired *>())
        disconnect(ri, &IVInterfaceRequired::propChanged_InheritPI, this, &IVInterfaceRequired::propChanged_InheritPI);
}
}
