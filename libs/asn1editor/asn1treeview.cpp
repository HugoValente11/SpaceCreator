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

#include "asn1treeview.h"

#include "asn1const.h"
#include "asn1itemdelegate.h"
#include "asn1itemmodel.h"

#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>

namespace asn1 {

Asn1TreeView::Asn1TreeView(QWidget *parent)
    : QTreeView(parent)
{
    m_itemDelegate = new Asn1ItemDelegate(this);
    setItemDelegate(m_itemDelegate);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAlternatingRowColors(true);
    setDragEnabled(false);

    connect(m_itemDelegate, &Asn1ItemDelegate::sequenceOfSizeChanged, this, &Asn1TreeView::onSequenceOfSizeChanged);
    connect(m_itemDelegate, &Asn1ItemDelegate::choiceFieldChanged, this, &Asn1TreeView::onChoiceFieldChanged);
}

void Asn1TreeView::setAsn1Model(const QVariantMap &asn1Item, int row)
{
    m_ItemModel.reset(new Asn1ItemModel(asn1Item, this));
    setModel(m_ItemModel.data());

    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    hideExtraFields(m_ItemModel->item(0, 0), true, row);

    expandAll();
}

void Asn1TreeView::setAsn1Value(const QVariantMap &asn1Value)
{
    if (model() == nullptr || asn1Value.empty())
        return;

    auto *nameItem = m_ItemModel->item(0, 0);

    int row = nameItem->row();
    QString asnType = m_ItemModel->item(row, MODEL_TYPE_INDEX)->text();

    if (asnType.startsWith("sequenceOf")) {
        int seqOfSize = asn1Value["seqofvalue"].toList().count();
        m_ItemModel->item(row, MODEL_VALUE_INDEX)->setText(QString::number(seqOfSize));
        setChildValue(nameItem, asn1Value["seqofvalue"], seqOfSize);
    } else if (asnType.startsWith("choice")) {
        QString choiceValue = asn1Value["choice"].toMap()["name"].toString();

        m_ItemModel->item(row, MODEL_VALUE_INDEX)->setText(choiceValue);

        setChildValue(nameItem, asn1Value["choice"], -1, itemChoiceIndex(nameItem, choiceValue));
    } else if (asnType.startsWith("integer") || asnType.startsWith("double") || asnType.startsWith("string")
               || asnType.startsWith("enumerated"))
        m_ItemModel->item(row, MODEL_VALUE_INDEX)->setText(asn1Value["value"].toString());
    else if (asnType.startsWith("bool"))
        m_ItemModel->item(row, MODEL_VALUE_INDEX)->setText(asn1Value["value"].toString().toLower());
    else if (asnType.startsWith("sequence"))
        setChildValue(nameItem, asn1Value["children"]);

    hideExtraFields(nameItem, true, row);
    expandAll();
}

QString Asn1TreeView::getAsn1Value() const
{
    return m_ItemModel ? getItemValue(m_ItemModel->item(0, 0)) : QString();
}

void Asn1TreeView::onSequenceOfSizeChanged(const QModelIndex &index, const QVariant value, const QVariant maxRange)
{
    for (int x = 0; x < maxRange.toInt(); ++x)
        setRowHidden(x, index, x < value.toInt() ? false : true);

    expand(index);
}

void Asn1TreeView::onChoiceFieldChanged(const QModelIndex &index, const QVariant length, const QVariant currentIndex)
{
    for (int x = 0; x < length.toInt(); ++x)
        setRowHidden(x, index, x == currentIndex.toInt() ? false : true);

    expand(index);
}

void Asn1TreeView::hideExtraFields(const QStandardItem *item, bool hide, int row)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
    QString asnType = model->item(row, MODEL_TYPE_INDEX)->text();

    if (asnType == "choice" && hide) {
        QVariantList choices = model->item(row, MODEL_VALUE_INDEX)->data(CHOICE_LIST_ROLE).toList();
        onChoiceFieldChanged(model->item(row)->index(), choices.size(),
                             choices.indexOf(model->item(row, MODEL_VALUE_INDEX)->text()));
    } else if (asnType.indexOf("sequenceOf") >= 0 && hide) {
        onSequenceOfSizeChanged(model->item(row)->index(), model->item(row, MODEL_VALUE_INDEX)->text(),
                                model->item(row, MODEL_VALUE_INDEX)->data(MAX_RANGE_ROLE));
    }

    for (int x = 0; x < item->rowCount(); ++x) {
        hideExtraFields(item->child(x));

        asnType = item->child(x, MODEL_TYPE_INDEX)->text();
        if (asnType.indexOf("sequenceOf") >= 0)
            onSequenceOfSizeChanged(item->child(x)->index(), item->child(x, MODEL_VALUE_INDEX)->text(),
                                    item->child(x, MODEL_VALUE_INDEX)->data(MAX_RANGE_ROLE));
        else if (asnType == "choice") {
            QVariantList choices = item->child(x, MODEL_VALUE_INDEX)->data(CHOICE_LIST_ROLE).toList();
            onChoiceFieldChanged(item->child(x)->index(), choices.size(),
                                 choices.indexOf(item->child(x, MODEL_VALUE_INDEX)->text()));
        }
    }
}

void Asn1TreeView::setChildRowValue(const QStandardItem *rootItem, int childIndex, const QVariant &asn1Value)
{
    QString asnType = rootItem->child(childIndex, MODEL_TYPE_INDEX)->text();
    auto *child = rootItem->child(childIndex, MODEL_VALUE_INDEX);

    if (asn1Value.type() == QVariant::List && asn1Value.toList().count() < childIndex)
        return;

    QVariant value = (asn1Value.type() == QVariant::List) ? asn1Value.toList()[childIndex] : asn1Value;

    if (asnType.startsWith("integer") || asnType.startsWith("double") || asnType.startsWith("string")
        || asnType.startsWith("enumerated"))
        child->setText(value.toMap()["value"].toString());
    else if (asnType.startsWith("bool"))
        child->setText(value.toMap()["value"].toString().toLower());
    else if (asnType.startsWith("sequenceOf")) {
        int seqOfSize = value.toMap()["seqofvalue"].toList().count();
        child->setText(QString::number(seqOfSize));
        setChildValue(rootItem->child(childIndex), value.toMap()["seqofvalue"], seqOfSize);
    } else if (asnType.startsWith("sequence"))
        setChildValue(rootItem->child(childIndex), value);
    else if (asnType.startsWith("choice")) {
        value = value.toMap()["choice"];
        QString choiceValue = value.toMap()["name"].toString();

        child->setText(choiceValue);
        setChildRowValue(rootItem->child(childIndex), itemChoiceIndex(rootItem->child(childIndex), choiceValue), value);
    }
}

void Asn1TreeView::setChildValue(const QStandardItem *rootItem, const QVariant &asn1Value, int seqOfSize, int choiceRow)
{
    if (choiceRow != -1) {
        // asn1Value = map
        setChildRowValue(rootItem, choiceRow, asn1Value);
    } else if (rootItem->hasChildren()) {
        int rowCount = seqOfSize != -1 ? seqOfSize : rootItem->rowCount();

        for (int x = 0; x < rowCount; ++x) {
            setChildRowValue(rootItem, x, asn1Value);

            bool isOptional = rootItem->child(x, MODEL_IS_OPTIONAL_INDEX)->data(OPTIONAL_ROLE).toBool();
            if (isOptional) {
                // asn1Value = list of map
                const auto &valueMap =
                        findValue(rootItem->child(x, MODEL_NAME_INDEX)->text(), asn1Value.toList()[x].toMap());
                rootItem->child(x, MODEL_IS_OPTIONAL_INDEX)
                        ->setCheckState(valueMap.size() ? Qt::Checked : Qt::Unchecked);
            }
        }
    }
}

QVariantMap Asn1TreeView::findValue(const QString &name, const QVariantMap &asn1Value) const
{
    QVariantMap result;

    if (asn1Value["name"] == name)
        return asn1Value;

    if (asn1Value.contains("children")) {
        for (const QVariant &child : asn1Value["children"].toList()) {
            result = findValue(name, child.toMap());
            if (result.size())
                break;
        }
    }

    if (asn1Value.contains("choice"))
        result = findValue(name, asn1Value["choice"].toMap());

    return result;
}

int Asn1TreeView::itemChoiceIndex(const QStandardItem *item, const QString &name) const
{
    int choiceIndex = 0;

    for (choiceIndex = 0; choiceIndex < item->rowCount(); ++choiceIndex)
        if (name == item->child(choiceIndex, MODEL_NAME_INDEX)->text())
            break;

    return choiceIndex;
}

QString Asn1TreeView::getItemValue(const QStandardItem *item, const QString &separator) const
{
    QString itemValue = "";

    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());

    QModelIndex itemIndex = model->indexFromItem(item);
    QModelIndex modelIndex = itemIndex.sibling(item->row(), MODEL_TYPE_INDEX);
    QString asnType = model->itemFromIndex(modelIndex)->text();

    QString asnValue;
    modelIndex = itemIndex.sibling(item->row(), MODEL_VALUE_INDEX);
    if (modelIndex.isValid())
        asnValue = model->itemFromIndex(modelIndex)->text();

    if (!separator.isEmpty())
        itemValue = QString("%1%2 ").arg(item->text(), separator);

    if (asnType.startsWith("bool"))
        itemValue += asnValue.toUpper();
    else if (asnType.startsWith("choice")) {
        itemValue += getItemValue(item->child(itemChoiceIndex(item, asnValue)), " :");
    } else if (asnType.startsWith("sequenceOf") || asnType.startsWith("sequence")) {
        itemValue += "{ ";

        int childCount = asnType.startsWith("sequenceOf") ? asnValue.toInt() : item->rowCount();
        for (int x = 0; x < childCount; ++x) {
            itemValue += getItemValue(item->child(x), asnType.startsWith("sequenceOf") ? "" : " ");
            if (x < childCount - 1)
                itemValue += ", ";
        }
        itemValue += " }";
    } else if (asnType.startsWith("string"))
        itemValue += "\"" + asnValue + "\"";
    else //(asnType == "integer" || asnType == "double" || asnType == "enumerated")
        itemValue += asnValue;

    return itemValue;
}

} // namespace asn1
