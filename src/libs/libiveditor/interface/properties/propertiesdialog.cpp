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

#include "propertiesdialog.h"

#include "asn1/file.h"
#include "asn1/types/builtintypes.h"
#include "baseitems/common/ivutils.h"
#include "commandsstack.h"
#include "contextparametersmodel.h"
#include "delegates/asn1valuedelegate.h"
#include "delegates/attributedelegate.h"
#include "ifaceparametersmodel.h"
#include "interface/commands/cmdentityattributechange.h"
#include "interface/ivconnectiongroupmodel.h"
#include "ivcomment.h"
#include "ivconnectiongroup.h"
#include "ivinterface.h"
#include "ivnamevalidator.h"
#include "ivobject.h"
#include "propertieslistmodel.h"
#include "propertiesviewbase.h"
#include "ui_propertiesdialog.h"

#include <QDebug>
#include <QHeaderView>
#include <QListView>
#include <QPlainTextEdit>
#include <QTableView>
#include <QTimer>
#include <QUndoCommand>

namespace ive {

PropertiesDialog::PropertiesDialog(ivm::PropertyTemplateConfig *dynPropConfig, ivm::IVObject *obj,
        const QSharedPointer<Asn1Acn::File> &dataTypes, cmd::CommandsStack *commandsStack, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PropertiesDialog)
    , m_dataObject(obj)
    , m_dynPropConfig(dynPropConfig)
    , m_cmdMacro(new cmd::CommandsStack::Macro(commandsStack->undoStack(),
              tr("Edit %1 - %2")
                      .arg(ivm::IVNameValidator::nameOfType(m_dataObject->type()).trimmed(), m_dataObject->titleUI())))
    , m_dataTypes(dataTypes)
    , m_commandsStack(commandsStack)
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    initTabs();
}

PropertiesDialog::~PropertiesDialog()
{
    delete m_cmdMacro;
    delete ui;
}

QString PropertiesDialog::objectTypeName() const
{
    if (!m_dataObject)
        return QString();

    switch (m_dataObject->type()) {
    case ivm::IVObject::Type::FunctionType:
        return tr("Function Type");
    case ivm::IVObject::Type::Function:
        return tr("Function");
    case ivm::IVObject::Type::MyFunction:
        return tr("My Function");
    case ivm::IVObject::Type::RequiredInterface:
        return tr("RI");
    case ivm::IVObject::Type::ProvidedInterface:
        return tr("PI");
    case ivm::IVObject::Type::Comment:
        return tr("Comment");
    case ivm::IVObject::Type::Connection:
        return tr("Connection");
    case ivm::IVObject::Type::ConnectionGroup:
        return tr("Connection Group");
    default:
        return QString();
    }
}

void PropertiesDialog::done(int r)
{
    m_cmdMacro->setComplete(r == QDialog::Accepted);
    QDialog::done(r);
}

void PropertiesDialog::initTabs()
{
    if (!m_dataObject)
        return;

    switch (m_dataObject->type()) {
    case ivm::IVObject::Type::FunctionType:
    case ivm::IVObject::Type::MyFunction:
    case ivm::IVObject::Type::Function: {
        initContextParams();
        initAttributesView();
        break;
    }
    case ivm::IVObject::Type::RequiredInterface:
    case ivm::IVObject::Type::ProvidedInterface: {
        initIfaceParams();
        initAttributesView();
        break;
    }
    case ivm::IVObject::Type::ConnectionGroup: {
        initConnectionGroup();
        initAttributesView();
        break;
    }
    case ivm::IVObject::Type::Comment: {
        initCommentView();
        break;
    }
    default:
        break;
    }

    setWindowTitle(tr("Edit %1").arg(objectTypeName()));
    ui->tabWidget->setCurrentIndex(0);
}

void PropertiesDialog::initConnectionGroup()
{
    auto model = new IVConnectionGroupModel(qobject_cast<ivm::IVConnectionGroup *>(m_dataObject), m_cmdMacro, this);
    auto connectionsView = new QListView;
    connectionsView->setModel(model);
    ui->tabWidget->insertTab(0, connectionsView, tr("Connections"));
}

void PropertiesDialog::initAttributesView()
{
    auto viewAttrs = new AttributesView(this);
    PropertiesListModel *modelAttrs { nullptr };
    QStyledItemDelegate *attrDelegate = new AttributeDelegate(viewAttrs->tableView());

    switch (m_dataObject->type()) {
    case ivm::IVObject::Type::Function: {
        modelAttrs = new FunctionPropertiesListModel(m_cmdMacro, m_dynPropConfig, this);
        break;
    }
    default:
        modelAttrs = new InterfacePropertiesListModel(m_cmdMacro, m_dynPropConfig, this);
        break;
    }

    modelAttrs->setDataObject(m_dataObject);
    viewAttrs->tableView()->setItemDelegateForColumn(PropertiesListModel::Column::Value, attrDelegate);
    viewAttrs->setModel(modelAttrs);

    ui->tabWidget->insertTab(0, viewAttrs, tr("Attributes"));

    QTimer::singleShot(0, viewAttrs, [this, viewAttrs, modelAttrs]() {
        const int nameColumn = m_dataObject->isFunction() ? FunctionPropertiesListModel::Column::Name
                                                          : InterfacePropertiesListModel::Column::Name;
        const int valueColumn = m_dataObject->isFunction() ? FunctionPropertiesListModel::Column::Value
                                                           : InterfacePropertiesListModel::Column::Value;

        const QModelIndexList indexes = modelAttrs->match(modelAttrs->index(0, nameColumn),
                FunctionPropertiesListModel::DataRole, ivm::meta::Props::token(ivm::meta::Props::Token::name));
        if (!indexes.isEmpty()) {
            viewAttrs->tableView()->edit(indexes.front().siblingAtColumn(valueColumn));
        }
    });
}

void PropertiesDialog::initContextParams()
{
    ContextParametersModel *modelCtxParams = new ContextParametersModel(m_cmdMacro, this);
    modelCtxParams->setDataTypes(m_dataTypes);
    modelCtxParams->setDataObject(m_dataObject);

    PropertiesViewBase *viewAttrs = new ContextParametersView(this);
    viewAttrs->tableView()->setItemDelegateForColumn(
            ContextParametersModel::Column::Type, new AttributeDelegate(viewAttrs->tableView()));
    viewAttrs->tableView()->setItemDelegateForColumn(
            ContextParametersModel::Column::Value, new Asn1ValueDelegate(m_dataTypes, viewAttrs->tableView()));
    viewAttrs->tableView()->horizontalHeader()->show();
    viewAttrs->setModel(modelCtxParams);
    ui->tabWidget->insertTab(0, viewAttrs, tr("Context Parameters"));
}

void PropertiesDialog::initIfaceParams()
{
    IfaceParametersModel *modelIfaceParams = new IfaceParametersModel(m_cmdMacro, asn1Names(m_dataTypes.get()), this);
    modelIfaceParams->setDataObject(m_dataObject);

    PropertiesViewBase *viewAttrs = new IfaceParametersView(this);
    viewAttrs->tableView()->setItemDelegateForColumn(
            IfaceParametersModel::Column::Type, new AttributeDelegate(viewAttrs->tableView()));
    viewAttrs->tableView()->setItemDelegateForColumn(
            IfaceParametersModel::Column::Encoding, new AttributeDelegate(viewAttrs->tableView()));
    viewAttrs->tableView()->setItemDelegateForColumn(
            IfaceParametersModel::Column::Direction, new AttributeDelegate(viewAttrs->tableView()));
    viewAttrs->tableView()->horizontalHeader()->show();
    viewAttrs->setModel(modelIfaceParams);
    ui->tabWidget->insertTab(0, viewAttrs, tr("Parameters"));
}

void PropertiesDialog::initCommentView()
{
    if (auto comment = qobject_cast<ivm::IVComment *>(m_dataObject)) {
        auto commentEdit = new QPlainTextEdit(this);
        commentEdit->setPlainText(comment->titleUI());
        ui->tabWidget->insertTab(0, commentEdit, tr("Comment content"));
        connect(this, &QDialog::accepted, this, [comment, commentEdit, this]() {
            const QString text = commentEdit->toPlainText();
            if (comment->titleUI() == text)
                return;

            const QString encodedText = ivm::IVNameValidator::encodeName(comment->type(), text);
            const QVariantHash textArg { { ivm::meta::Props::token(ivm::meta::Props::Token::name), encodedText } };
            auto commentTextCmd = new cmd::CmdEntityAttributeChange(comment, textArg);
            commentTextCmd->setText(tr("Edit Comment"));
            m_commandsStack->push(commentTextCmd);
        });
    }
}

} // namespace ive
