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

#include "propertytemplatemanager.h"

#include "addpropertytemplatedialog.h"
#include "propertytemplate.h"
#include "propertytemplateconfig.h"
#include "ui_propertytemplatemanager.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

namespace aadlinterface {

PropertyTemplateManager::PropertyTemplateManager(aadl::PropertyTemplateConfig *dynPropConfig, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PropertyTemplateManager)
    , m_dynPropConfig(dynPropConfig)
{
    ui->setupUi(this);

    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &PropertyTemplateManager::updateErrorInfo);

    readConfig(m_dynPropConfig->configPath());
}

PropertyTemplateManager::~PropertyTemplateManager()
{
    delete ui;
}

bool PropertyTemplateManager::readConfig(const QString &from)
{
    if (from.isEmpty())
        return false;

    QFile configFile(from);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "File opening failed:" << from << configFile.errorString();
        return false;
    }

    ui->plainTextEdit->setPlainText(configFile.readAll());
    return true;
}

void PropertyTemplateManager::updateErrorInfo()
{
    const QString &xmlData = ui->plainTextEdit->toPlainText();

    QColor textColor(Qt::black);
    m_usedNames.clear();

    QString errorMsg;
    int errorLine = -1;
    int errorColumn = -1;

    if (!xmlData.isEmpty()) {
        const QList<aadl::PropertyTemplate *> &attrs =
                m_dynPropConfig->parseAttributesList(xmlData, &errorMsg, &errorLine, &errorColumn);

        if (!errorMsg.isEmpty()) {
            qWarning() << errorMsg << errorLine << errorColumn;
            textColor = Qt::red;
        } else {
            for (aadl::PropertyTemplate *attr : attrs) {
                if (m_usedNames.contains(attr->name())) {
                    errorMsg = tr("Duplicate names found: %1").arg(attr->name());
                    break;
                }
                m_usedNames.append(attr->name());
            }
        }
    }

    ui->errorDisplay->setText(errorMsg);
    setTextColor(textColor);
    ui->btnNewProp->setEnabled(errorMsg.isEmpty() || xmlData.isEmpty());
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(errorMsg.isEmpty());
}

void PropertyTemplateManager::setTextColor(const QColor &color)
{
    QPalette p(ui->plainTextEdit->palette());
    p.setColor(QPalette::Text, color);
    ui->plainTextEdit->setPalette(p);
}

void PropertyTemplateManager::on_btnNewProp_clicked()
{
    AddPropertyTemplateDialog *dlg = new AddPropertyTemplateDialog(m_usedNames, this);
    if (dlg->exec() == QDialog::Accepted) {
        if (aadl::PropertyTemplate *attr = dlg->attribute()) {
            const QString &xmlData = ui->plainTextEdit->toPlainText();
            QDomDocument doc;
            if (doc.setContent(xmlData)) {
                QDomElement rootElement = doc.documentElement();
                rootElement.appendChild(attr->toXml(&doc));

                ui->plainTextEdit->setPlainText(doc.toString());
                ui->plainTextEdit->find(QString("\"name\": \"%1\",").arg(attr->name()));
            }
        }
    }
    delete dlg;
}

void PropertyTemplateManager::accept()
{
    const QString &filePath = m_dynPropConfig->configPath();
    QFile out(filePath);
    if (out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        const QByteArray &content = ui->plainTextEdit->toPlainText().toUtf8();
        out.write(content);
        out.close();
    } else {
        const QString warn(tr("Can't save file %1 - %2").arg(filePath, out.errorString()));
        qWarning() << warn;

        QMessageBox::warning(this, tr("Saving..."), warn);
    }

    QDialog::accept();
}

}