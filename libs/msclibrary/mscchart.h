/*
   Copyright (C) 2018 European Space Agency - <maxime.perrotin@esa.int>

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

#ifndef MSCCHART_H
#define MSCCHART_H

#include "mscentity.h"

#include <QObject>
#include <QString>
#include <QVector>

namespace msc {
class MscDocument;
class MscInstance;
class MscMessage;
class MscGate;

class MscChart : public MscEntity
{
    Q_OBJECT
public:
    explicit MscChart(QObject *parent = nullptr);
    MscChart(const QString &name, QObject *parent = nullptr);
    ~MscChart();

    const QVector<MscInstance *> &instances() const;
    void addInstance(MscInstance *instance);
    void removeInstance(MscInstance *instance);

    MscInstance *instanceByName(const QString &name) const;

    const QVector<MscMessage *> &messages() const;
    void addMessage(MscMessage *message);
    void removeMessage(MscMessage *message);
    MscMessage *messageByName(const QString &name) const;

    const QVector<MscGate *> &gates() const;
    void addGate(MscGate *gate);
    void removeGate(MscGate *gate);

    MscDocument *parentDocument() const;

    MscEntity::EntityType elementType() const override;

Q_SIGNALS:
    void instanceAdded(MscInstance *instance);
    void instanceRemoved(MscInstance *instance);
    void messageAdded(MscMessage *message);
    void messageRemoved(MscMessage *message);
    void gateAdded(MscGate *gate);
    void gateRemoved(MscGate *gate);

private:
    QVector<MscInstance *> m_instances;
    QVector<MscMessage *> m_messages;
    QVector<MscGate *> m_gates;
};

} // namespace msc

#endif // MSCCHART_H
