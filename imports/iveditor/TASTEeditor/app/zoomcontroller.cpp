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

#include "zoomcontroller.h"

#include "baseitems/graphicsview.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>

namespace taste3 {

/*!
 * \class taste3::ZoomValidator
 * \brief validator used in ZoomController's combo box.
 * \sa ZoomController
 */
ZoomValidator::ZoomValidator(QObject *parent)
    : QValidator(parent)
    , m_validator(new QIntValidator(this))
{
}

void ZoomValidator::addSuffix(QString &text) const
{
    if (!text.endsWith('%'))
        text.append('%');
}

QString ZoomValidator::clearSuffix(QString &value) const
{
    value.remove('%');
    return value;
}

QValidator::State ZoomValidator::validate(QString &s, int &i) const
{
    clearSuffix(s);

    QValidator::State validated = m_validator->validate(s, i);

    addSuffix(s);

    return validated;
}

void ZoomValidator::fixup(QString &s) const
{
    clearSuffix(s);
    m_validator->fixup(s);
    addSuffix(s);
}

void ZoomValidator::setBottom(int min)
{
    m_validator->setBottom(min);
}

void ZoomValidator::setRange(int bottom, int top)
{
    m_validator->setRange(bottom, top);
}

void ZoomValidator::setTop(int top)
{
    m_validator->setTop(top);
}

/*!
 * \class taste3::ZoomController
 * \brief Control to change, reset and display the zoom level of the graphics scene. Uses editable combo box.
 * \sa ZoomValidator
 */
ZoomController::ZoomController(QWidget *parent)
    : QWidget(parent)
    , m_combo(new QComboBox(this))
    , m_validator(new ZoomValidator(this))
{
    QHBoxLayout *hBox = new QHBoxLayout(this);
    hBox->addWidget(new QLabel(tr("Zoom:")));
    hBox->addWidget(m_combo);
    hBox->setContentsMargins({ 0, 0, 0, 0 });
}

void ZoomController::refill()
{
    QSignalBlocker sb(this);
    m_combo->clear();

    m_combo->setEditable(true);
    m_combo->setInsertPolicy(QComboBox::InsertAtBottom);
    m_combo->setValidator(m_validator);

    for (int z : { 5, 10, 25, 50, 75, 100, 150, 200, 400 }) {
        QString s(QString::number(z));
        m_validator->fixup(s);
        m_combo->addItem(s);
    }
}

void ZoomController::setView(aadlinterface::GraphicsView *view)
{
    if (m_view == view)
        return;

    if (m_view) {
        disconnect(m_combo, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this,
                   &ZoomController::onCurrentIndexChanged);
        disconnect(m_view, &aadlinterface::GraphicsView::zoomChanged, this, &ZoomController::displayZoomLevel);
    }

    m_view = view;
    setEnabled(nullptr != m_view);

    if (!m_view)
        return;

    m_validator->setBottom(qRound(m_view->minZoomPercent()));
    m_validator->setTop(qRound(m_view->maxZoomPercent()));

    refill();

    connect(m_view, &aadlinterface::GraphicsView::zoomChanged, this, &ZoomController::displayZoomLevel);
    connect(m_combo, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this,
            &ZoomController::onCurrentIndexChanged);

    displayZoomLevel(m_view->zoom());
}

void ZoomController::onCurrentIndexChanged(const QString &text)
{
    QString strPercent(text);
    bool ok(false);
    const qreal percent = m_validator->clearSuffix(strPercent).toDouble(&ok);
    if (ok)
        setZoomLevel(percent);
}

void ZoomController::setZoomLevel(qreal percent)
{
    if (m_view)
        m_view->setZoom(percent);
}

void ZoomController::displayZoomLevel(qreal percent)
{
    m_combo->setCurrentText(QString::number(percent));
}

} // ns taste3
