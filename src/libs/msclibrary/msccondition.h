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

#include "mscinstanceevent.h"

namespace msc {

class MscInstance;

class MscCondition : public MscInstanceEvent
{
    Q_OBJECT
    Q_PROPERTY(msc::MscInstance *instance READ instance WRITE setInstance NOTIFY instanceChanged)
    Q_PROPERTY(bool shared READ shared WRITE setShared NOTIFY sharedChanged)

public:
    explicit MscCondition(QObject *parent = nullptr);
    MscCondition(const QString &name, QObject *parent = nullptr);

    bool shared() const;
    void setShared(bool shared);

    MscInstance *instance() const;
    void setInstance(MscInstance *instance);

    MscEntity::EntityType entityType() const override;

    bool relatesTo(const MscInstance *instance) const override;

Q_SIGNALS:
    void instanceChanged();
    void sharedChanged();

private:
    MscInstance *m_instance = nullptr;
    bool m_shared = false;
};

} // namespace msc
