#include "propertiesview.h"
#include "ui_propertiesview.h"
#include "propertieslistmodel.h"

#include <QDebug>

namespace taste3 {
namespace aadl {

PropertiesView::PropertiesView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertiesView)
{
    ui->setupUi(this);
}

PropertiesView::~PropertiesView()
{
    delete ui;
}

void PropertiesView::setModel(PropertiesListModel* model)
{
    if(model == m_model)
        return;

    if(ui->tableView->selectionModel())
        disconnect(ui->tableView->selectionModel(),&QItemSelectionModel::currentRowChanged,
                this, &PropertiesView::onCurrentRowChanged);

    m_model = model;
    ui->tableView->setModel(m_model);

    if(ui->tableView->selectionModel())
        connect(ui->tableView->selectionModel(),&QItemSelectionModel::currentRowChanged,
                this, &PropertiesView::onCurrentRowChanged);
}

QTableView* PropertiesView::tableView() const
{
    return ui->tableView;
}

void PropertiesView::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &)
{
    if(m_model)
        ui->btnDel->setEnabled(m_model->isProp(current));
}

void PropertiesView::on_btnAdd_clicked()
{
    if(m_model)
    {
        static const QString newNameTmp = tr("New property");
        QString newName(newNameTmp);
        int duplicateCounter(0);
        while(!m_model->findItems(newName).isEmpty())
            newName = QString("%1 #%2").arg(newNameTmp, QString::number(++duplicateCounter));

        if(m_model->createProperty(newName))
        {
            const QModelIndex &added = m_model->index(m_model->rowCount()-1, 0);
//            ui->tableView->scrollTo(added, QAbstractItemView::EnsureVisible);
            ui->tableView->scrollToBottom();
            ui->tableView->edit(added);
        }
    }
}

void PropertiesView::on_btnDel_clicked()
{
    if(m_model)
        m_model->removeProperty(ui->tableView->currentIndex());
}

} // namespace aadl
} // namespace taste3
