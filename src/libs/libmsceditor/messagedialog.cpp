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

#include "messagedialog.h"

#include "asn1editor.h"
#include "asn1valueparser.h"
#include "commands/common/commandsstack.h"
#include "messagedeclarationsdialog.h"
#include "mscchart.h"
#include "mscdocument.h"
#include "mscinstance.h"
#include "mscmessage.h"
#include "mscmessagedeclarationlist.h"
#include "mscmodel.h"
#include "mscreader.h"
#include "mscwriter.h"
#include "ui_messagedialog.h"

#include <QDebug>
#include <QKeyEvent>
#include <QRegExp>
#include <QRegExpValidator>
#include <QStyledItemDelegate>
#include <QUndoStack>

/*!
   \brief MessageDialog::MessageDialog

   The name is checked using a validator.
   But the parameters a only checked after editing using the msc parser, as the parameters can be
   quite complex. See paramaterDefn in the msc.g4 grammar file.
 */
MessageDialog::MessageDialog(msc::MscMessage *message, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MessageDialog)
    , m_message(message)
{
    Q_ASSERT(message);
    ui->setupUi(this);

    QRegExpValidator *nameValidator = new QRegExpValidator(msc::MscEntity::nameVerifier(), this);
    ui->nameLineEdit->setValidator(nameValidator);

    ui->nameLineEdit->setText(m_message->name());

    connect(ui->nameLineEdit, &QLineEdit::editingFinished, this, &MessageDialog::selectDeclarationFromName);
    connect(ui->addParameterButton, &QPushButton::clicked, this, &MessageDialog::addParameter);
    connect(ui->removeParameterButton, &QPushButton::clicked, this, &MessageDialog::removeParameter);
    connect(ui->parameterTable->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &MessageDialog::checkRemoveButton);
    connect(ui->parameterTable, QOverload<QTableWidgetItem *>::of(&QTableWidget::itemDoubleClicked), this,
            &MessageDialog::editItem);
    connect(ui->parameterTable, &QTableWidget::cellChanged, this, &MessageDialog::checkTextValidity);

    connect(ui->declarationsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [&]() { ui->declarationButton->setEnabled(ui->declarationsComboBox->currentIndex() > 0); });
    connect(ui->declarationButton, &QPushButton::clicked, this, &MessageDialog::copyDeclarationData);
    connect(ui->editDeclarationsButton, &QPushButton::clicked, this, &MessageDialog::editDeclarations);

    fillMessageDeclartionBox();
    selectDeclarationFromName();

    fillParameters();
    setParameterEditState();

    checkTextValidity();

    ui->nameLineEdit->setFocus();
}

MessageDialog::~MessageDialog()
{
    delete ui;
}

void MessageDialog::accept()
{
    msc::cmd::CommandsStack::current()->beginMacro("Edit message");
    msc::cmd::CommandsStack::push(msc::cmd::RenameEntity,
            { QVariant::fromValue(m_message.data()), QVariant::fromValue(ui->nameLineEdit->text()) });

    msc::MscParameterList parameters;
    for (int i = 0; i < ui->parameterTable->rowCount(); ++i) {
        const QString &text = ui->parameterTable->item(i, 0)->text();
        if (!text.isEmpty()) {
            parameters << msc::MscParameter(text);
        } else {
            qWarning() << "An empty parameter is not allowed";
        }
    }
    msc::cmd::CommandsStack::push(
            msc::cmd::SetParameterList, { QVariant::fromValue(m_message.data()), QVariant::fromValue(parameters) });
    msc::cmd::CommandsStack::current()->endMacro();

    QDialog::accept();
}

void MessageDialog::copyDeclarationData()
{
    const msc::MscMessageDeclarationList *declarations = messageDeclarations();
    if (!declarations)
        return;

    const int idx = ui->declarationsComboBox->currentIndex() - 1;
    if (idx < 0 || idx >= declarations->size())
        return;

    const msc::MscMessageDeclaration *declaration = declarations->at(idx);
    Q_ASSERT(declaration != nullptr);
    Q_ASSERT(!declaration->names().isEmpty());
    const QString name = declaration->names().at(0);
    ui->nameLineEdit->setText(name);
    selectDeclarationFromName();
}

void MessageDialog::selectDeclarationFromName()
{
    msc::MscMessageDeclarationList *declarations = messageDeclarations();
    if (!declarations)
        return;

    int currentIdx = 0;
    int i = 0;
    m_selectedDeclaration.clear();
    for (msc::MscMessageDeclaration *declaration : *declarations) {
        ++i;
        const QStringList &names = declaration->names();
        if (names.contains(ui->nameLineEdit->text())) {
            currentIdx = i;
            m_selectedDeclaration = declaration;
            break;
        }
    }
    if (ui->declarationsComboBox->currentIndex() != currentIdx)
        ui->declarationsComboBox->setCurrentIndex(currentIdx);

    if (m_selectedDeclaration) {
        const QStringList &types = m_selectedDeclaration->typeRefList();
        ui->parameterTable->setRowCount(types.size());
        ui->parameterTable->setVerticalHeaderLabels(types);
        ui->parameterTable->verticalHeader()->setVisible(true);

        ui->addParameterButton->setEnabled(false);
    } else {
        ui->parameterTable->verticalHeader()->setVisible(false);
        ui->addParameterButton->setEnabled(true);
    }

    checkRemoveButton();
    setParameterEditState();
    checkTextValidity();
}

void MessageDialog::editDeclarations()
{
    msc::MscMessageDeclarationList *declarations = messageDeclarations();
    msc::MscModel *model = mscModel();
    if (!declarations || !model)
        return;

    const QVector<msc::MscDocument *> &docs = model->documents();
    if (docs.isEmpty())
        return;

    MessageDeclarationsDialog dialog(declarations, mscModel()->asn1TypesData(), this);
    dialog.setFileName(model->dataDefinitionString());

    int result = dialog.exec();

    if (result == QDialog::Accepted) {
        const QVariantList cmdParams = { QVariant::fromValue<msc::MscDocument *>(docs.at(0)),
            QVariant::fromValue<msc::MscMessageDeclarationList *>(dialog.declarations()) };
        msc::cmd::CommandsStack::push(msc::cmd::Id::SetMessageDeclarations, cmdParams);
        const QVariantList params { QVariant::fromValue(model), dialog.fileName(), "ASN.1" };
        msc::cmd::CommandsStack::push(msc::cmd::Id::SetAsn1File, params);
        model->setDataDefinitionString(dialog.fileName());
        model->setAsn1TypesData(dialog.asn1Types());
        fillMessageDeclartionBox();
        selectDeclarationFromName();
    }
}

void MessageDialog::addParameter()
{
    const int count = ui->parameterTable->rowCount();
    ui->parameterTable->setRowCount(count + 1);
    QTableWidgetItem *tableItem = ui->parameterTable->item(count, 0);
    if (!tableItem) {
        tableItem = new QTableWidgetItem();
        ui->parameterTable->setItem(count, 0, tableItem);
    }
    setItemFlags(tableItem);
    ui->parameterTable->editItem(tableItem);
}

void MessageDialog::removeParameter()
{
    QTableWidgetItem *tableItem = ui->parameterTable->currentItem();
    if (tableItem) {
        const int idx = tableItem->row();
        ui->parameterTable->removeRow(idx);
    }
}

void MessageDialog::checkRemoveButton()
{
    bool enabled = !m_selectedDeclaration && !ui->parameterTable->selectedItems().isEmpty();
    ui->removeParameterButton->setEnabled(enabled);
}

void MessageDialog::setParameterEditState()
{
    for (int row = 0; row < ui->parameterTable->rowCount(); ++row) {
        QTableWidgetItem *tableItem = ui->parameterTable->item(row, 0);
        if (!tableItem) {
            tableItem = new QTableWidgetItem();
            ui->parameterTable->setItem(row, 0, tableItem);
        }
        setItemFlags(tableItem);
    }
}

void MessageDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(event);
}

void MessageDialog::editItem(QTableWidgetItem *item)
{
    if (!m_selectedDeclaration)
        return;

    asn1::Asn1Editor editor(this);
    editor.setValueEditOnlyMode();
    const QVariantList &types = mscModel()->asn1TypesData();
    editor.setAsn1Types(types);
    const QString type = ui->parameterTable->verticalHeaderItem(item->row())->text();
    editor.showAsn1Type(type);
    editor.setValue(item->text());

    const int result = editor.exec();
    if (result == QDialog::Accepted)
        item->setText(editor.value());

    ui->parameterTable->closePersistentEditor(item);
}

void MessageDialog::setItemFlags(QTableWidgetItem *item)
{
    Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    if (m_selectedDeclaration)
        itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    item->setFlags(itemFlags);
}

void MessageDialog::checkTextValidity()
{
    msc::MscMessage msg;
    msg.setTargetInstance(m_message->targetInstance());
    msg.setSourceInstance(m_message->sourceInstance());

    msg.setName(ui->nameLineEdit->text());

    msc::MscParameterList parameters;
    for (int i = 0; i < ui->parameterTable->rowCount(); ++i) {
        QTableWidgetItem *item = ui->parameterTable->item(i, 0);
        if (item) {
            const QString &text = item->text();
            if (!text.isEmpty())
                parameters << msc::MscParameter(text);
            else
                m_isValid = false;
        }
    }
    msg.setParameters(parameters);

    msc::MscInstance *instance =
            m_message->sourceInstance() ? m_message->sourceInstance() : m_message->targetInstance();
    if (!instance)
        return;

    msc::MscWriter writer;
    QString text = writer.serialize(&msg, instance);
    text.remove("\n");

    try {
        msc::MscReader reader;
        const QString mscText = QString("msc chart; instance %1; %2 endinstance; endmsc;").arg(instance->name(), text);
        QScopedPointer<msc::MscModel> model(reader.parseText(mscText));
        m_isValid = true;
    } catch (...) {
        m_isValid = false;
    }

    if (m_selectedDeclaration) {
        asn1::Asn1ValueParser parser;
        const QVariantList &asn1Types = mscModel()->asn1TypesData();
        for (int i = 0; i < ui->parameterTable->rowCount(); ++i) {
            QTableWidgetItem *item = ui->parameterTable->item(i, 0);
            if (item) {
                const QString &value = ui->parameterTable->item(i, 0)->text();
                const QString &typeName = ui->parameterTable->verticalHeaderItem(i)->text();
                auto find = std::find_if(asn1Types.begin(), asn1Types.end(),
                        [&](const QVariant &asn1Var) { return asn1Var.toMap()["name"] == typeName; });
                if (find != asn1Types.end()) {
                    bool ok;
                    parser.parseAsn1Value((*find).toMap(), value, &ok);
                    m_isValid = m_isValid && ok;
                } else
                    m_isValid = false;
            }
        }
    }

    if (m_isValid)
        ui->previewLabel->setText("");
    else
        ui->previewLabel->setText(tr("<font color='red'>Invalid message:<br>%1</font>").arg(text));

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_isValid);
}

void MessageDialog::fillMessageDeclartionBox()
{
    ui->declarationsComboBox->clear();

    const msc::MscMessageDeclarationList *declarations = messageDeclarations();
    if (!declarations) {
        ui->declarationsComboBox->setEnabled(false);
        ui->declarationButton->setEnabled(false);
        return;
    }

    ui->declarationsComboBox->addItem(" ");

    for (const msc::MscMessageDeclaration *declaration : *declarations) {
        const QStringList &names = declaration->names();
        ui->declarationsComboBox->addItem(names.join(","));
    }
}

void MessageDialog::fillParameters()
{
    const int size = m_message->parameters().size();
    ui->parameterTable->setRowCount(size);
    int row = 0;
    for (const msc::MscParameter &parameter : m_message->parameters()) {
        QTableWidgetItem *tableItem = ui->parameterTable->item(row, 0);
        if (!tableItem) {
            tableItem = new QTableWidgetItem();
            ui->parameterTable->setItem(row, 0, tableItem);
        }
        setItemFlags(tableItem);
        tableItem->setText(parameter.parameter());
        ++row;
    }
}

msc::MscModel *MessageDialog::mscModel() const
{
    msc::MscModel *model = nullptr;
    const auto chart = qobject_cast<msc::MscChart *>(m_message->parent());
    if (chart) {
        msc::MscDocument *doc = chart->parentDocument();
        if (doc) {
            while (!model && doc) {
                model = qobject_cast<msc::MscModel *>(doc->parent());
                doc = doc->parentDocument();
            }
        } else {
            model = qobject_cast<msc::MscModel *>(chart->parent());
        }
    }

    Q_ASSERT(model);
    return model;
}

msc::MscMessageDeclarationList *MessageDialog::messageDeclarations() const
{
    const msc::MscModel *model = mscModel();
    if (model) {
        QVector<msc::MscDocument *> docs = model->documents();
        if (!docs.isEmpty())
            return docs.at(0)->messageDeclarations();
    }
    return nullptr;
}