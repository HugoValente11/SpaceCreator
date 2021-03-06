/*
   Copyright (C) 2018-2019 European Space Agency - <maxime.perrotin@esa.int>

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

#include "mscinstance.h"
#include "mscinstanceevent.h"

#include <QPointer>
#include <QVariantList>

namespace msc {

class MscGate : public MscInstanceEvent
{
    Q_OBJECT
    Q_PROPERTY(msc::MscGate::Direction direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(msc::MscInstance *instance READ instance WRITE setInstance NOTIFY instanceChanged)
    Q_PROPERTY(QString instanceName READ instanceName WRITE setInstanceName NOTIFY instanceNameChanged)
    Q_PROPERTY(QString paramName READ paramName WRITE setParamName NOTIFY paramNameChanged)
    Q_PROPERTY(QVariantList params READ params WRITE setParams NOTIFY paramsChanged)

public:
    enum class Direction
    {
        In = 0,
        Out
    };

    explicit MscGate(QObject *parent = nullptr);
    MscGate(const QString &name, QObject *parent = nullptr);

    MscGate::Direction direction() const;
    MscInstance *instance() const;
    QString instanceName() const;
    QString paramName() const;
    QVariantList params() const;

    MscEntity::EntityType entityType() const override;

    bool relatesTo(const MscInstance *instance) const override;

public Q_SLOTS:
    void setDirection(msc::MscGate::Direction dir);
    void setInstance(msc::MscInstance *instance);
    void setInstanceName(const QString &instanceName);
    void setParamName(const QString &name);
    void setParams(const QVariantList &params);

Q_SIGNALS:
    void directionChanged();
    void instanceChanged();
    void instanceNameChanged();
    void paramNameChanged();
    void paramsChanged();

private:
    Direction m_direction = MscGate::Direction::In;
    QPointer<MscInstance> m_instance = nullptr;
    QString m_instanceName;
    QString m_paramName;
    QVariantList m_params;
};
} // ns name
