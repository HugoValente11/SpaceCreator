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

#include "ivfunctiontype.h"
#include "ivobject.h"
#include "ui/verectgraphicsitem.h"

namespace ive {

class IVFunctionNameGraphicsItem;

class IVFunctionTypeGraphicsItem : public shared::ui::VERectGraphicsItem
{
    Q_OBJECT
public:
    explicit IVFunctionTypeGraphicsItem(ivm::IVFunctionType *entity, QGraphicsItem *parentItem = nullptr);
    enum
    {
        Type = UserType + static_cast<int>(ivm::IVObject::Type::FunctionType)
    };
    int type() const override { return Type; }

    ivm::IVFunctionType *entity() const override;

    void init() override;

    void enableEditMode() override;

    QSizeF minimalSize() const override;

    QString prepareTooltip() const override;

    bool isRootItem() const;

    int itemLevel(bool isSelected) const override;

protected Q_SLOTS:
    void applyColorScheme() override;
    void updateNameFromUi(const QString &name);
    void updateText();

protected:
    void layoutInterfaces();
    void onManualResizeProgress(shared::ui::GripPoint *grip, const QPointF &from, const QPointF &to) override;
    void onManualResizeFinish(shared::ui::GripPoint *grip, const QPointF &from, const QPointF &to) override;
    void rebuildLayout() override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    shared::ColorManager::HandledColors handledColorType() const override;

    virtual void updateTextPosition();

    template<class Type>
    static QString uniteNames(const QVector<Type> &collection, const QString &prefix)
    {
        QStringList result;
        std::transform(collection.cbegin(), collection.cend(), std::back_inserter(result),
                [](Type obj) { return obj ? obj->titleUI() : QString(); });
        const QString line = joinNonEmpty(result, QStringLiteral(", "));
        return line.isEmpty() ? QString() : QString("<b>%1</b>%2").arg(prefix, line);
    }

    static QString joinNonEmpty(const QStringList &values, const QString &lineBreak)
    {
        QStringList filtered(values);
        filtered.removeAll(QString());
        return filtered.join(lineBreak);
    }

    virtual void prepareTextRect(QRectF &textRect, const QRectF &targetTextRect) const;

protected:
    IVFunctionNameGraphicsItem *m_textItem = nullptr;
};

}
