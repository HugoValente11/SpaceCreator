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
#include <QStyledItemDelegate>

class QStringListModel;

namespace aadlinterface {

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ComboBoxDelegate(QObject *parent = nullptr);
    ~ComboBoxDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

protected:
    virtual QAbstractItemModel *editorModel(const QModelIndex &id) const = 0;
};

class StringListComboDelegate : public ComboBoxDelegate
{
    Q_OBJECT
public:
    StringListComboDelegate(const QStringList &data, QObject *parent = nullptr);
    ~StringListComboDelegate() override;

protected:
    virtual QAbstractItemModel *editorModel(const QModelIndex &id) const override;

    QStringListModel *m_model { nullptr };
};

}