/* This file is part of Zanshin

   Copyright 2014 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/


#include "newprojectdialog.h"

#include "ui_newprojectdialog.h"

#include <QPushButton>
#include <QSortFilterProxyModel>
#include <KDescendantsProxyModel>

#include "presentation/querytreemodelbase.h"

using namespace Widgets;

class TaskSourceProxy : public QSortFilterProxyModel
{
public:
    explicit TaskSourceProxy(QObject *parent = Q_NULLPTR)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &) const Q_DECL_OVERRIDE
    {
        auto sourceIndex = sourceModel()->index(sourceRow, 0);
        auto source = sourceIndex.data(Presentation::QueryTreeModelBase::ObjectRole)
                                 .value<Domain::DataSource::Ptr>();
        return source && (source->contentTypes() & Domain::DataSource::Tasks);
    }
};

NewProjectDialog::NewProjectDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::NewProjectDialog),
      m_flattenProxy(new KDescendantsProxyModel(this))
{
    ui->setupUi(this);

    QObject::connect(ui->nameEdit, SIGNAL(textChanged(QString)), this, SLOT(onNameTextChanged(QString)));
    onNameTextChanged(m_name);

    auto taskSourceProxy = new TaskSourceProxy(this);
    taskSourceProxy->setSourceModel(m_flattenProxy);
    ui->sourceCombo->setModel(taskSourceProxy);

    m_flattenProxy->setDisplayAncestorData(true);
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

int NewProjectDialog::exec()
{
    return QDialog::exec();
}

void NewProjectDialog::accept()
{
    m_name = ui->nameEdit->text();
    m_source = ui->sourceCombo->itemData(ui->sourceCombo->currentIndex(),
                                         Presentation::QueryTreeModelBase::ObjectRole)
                              .value<Domain::DataSource::Ptr>();
    QDialog::accept();
}

void NewProjectDialog::setDataSourcesModel(QAbstractItemModel *model)
{
    m_flattenProxy->setSourceModel(model);
    auto proxy = ui->sourceCombo->model();
    for (int row = 0; row < proxy->rowCount(); row++) {
        auto index = proxy->index(row, 0);
        if (index.data(Presentation::QueryTreeModelBase::IsDefaultRole).toBool()) {
            ui->sourceCombo->setCurrentIndex(row);
        }
    }
}

QString NewProjectDialog::name() const
{
    return m_name;
}

Domain::DataSource::Ptr NewProjectDialog::dataSource() const
{
    return m_source;
}

void NewProjectDialog::onNameTextChanged(const QString &text)
{
    auto buttonOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    buttonOk->setEnabled(!text.isEmpty());
}