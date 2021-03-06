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

#include "mscmessagedeclarationlist.h"

namespace msc {

MscMessageDeclarationList::MscMessageDeclarationList(QObject *parent)
    : QObjectListModelT<MscMessageDeclaration *>(parent)
{
    setTracking(true);
}

void MscMessageDeclarationList::trackObject(const QObject *obj, const bool on)
{
    QObjectListModel::trackObject(obj, on);
    if (on) {
        auto md = static_cast<const MscMessageDeclaration *>(obj);
        connect(md, &MscMessageDeclaration::dataChanged, this, [&]() {
            auto md1 = static_cast<MscMessageDeclaration *>(sender());
            const int idx = indexOf(md1);
            if (idx >= 0) {
                QModelIndex index = createIndex(idx, 0);
                Q_EMIT dataChanged(index, index);
            }
        });
    } else {
        disconnect(obj, nullptr, this, nullptr);
    }
}

QVariant MscMessageDeclarationList::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_objects.size()) {
        return QVariant();
    }

    switch (role) {
    case ObjectRole:
        return QVariant::fromValue(m_objects.at(index.row()));
    case Qt::DisplayRole:
        return QVariant::fromValue(at(index.row())->names().join(", "));
    }

    return QVariant();
}

MscMessageDeclarationList *MscMessageDeclarationList::clone() const
{
    auto result = new MscMessageDeclarationList();

    QList<MscMessageDeclaration *> list;
    list.reserve(size());
    for (const MscMessageDeclaration *decl : *this) {
        auto copy = new MscMessageDeclaration(result);
        copy->setNames(decl->names());
        copy->setTypeRefList(decl->typeRefList());
        list.append(copy);
    }
    result->setObjectList(list);

    return result;
}

void MscMessageDeclarationList::setObjectList(const QList<MscMessageDeclaration *> &objects)
{
    QObjectListModelT::setObjectList(objects);
    for (MscMessageDeclaration *delc : objects) {
        delc->setParent(this);
    }
}

/*!
   Returns if this list contains a message declaration that is the same as \p declaration.
   Meaning the names and declarations are the same
 */
bool MscMessageDeclarationList::exists(MscMessageDeclaration *declaration) const
{
    for (const MscMessageDeclaration *decl : *this) {
        if (*decl == *declaration) {
            return true;
        }
    }
    return false;
}

} // namespace msc
