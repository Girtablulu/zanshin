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


#ifndef PRESENTATION_PAGEMODEL_H
#define PRESENTATION_PAGEMODEL_H

#include <QObject>

#include <QModelIndex>

#include "domain/project.h"
#include "domain/task.h"
#include "domain/taskqueries.h"

#include "presentation/metatypes.h"
#include "presentation/errorhandlingmodelbase.h"

namespace Presentation {

class PageModel : public QObject, public ErrorHandlingModelBase
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* centralListModel READ centralListModel)
public:
    enum class TaskExtraPart {
        None = 0x00,
        DataSource = 0x01,
        Project = 0x02,
        Contexts = 0x04,
    };
    Q_ENUM(TaskExtraPart)
    Q_DECLARE_FLAGS(TaskExtraParts, TaskExtraPart)
    Q_FLAG(TaskExtraParts)

    explicit PageModel(QObject *parent = nullptr);

    QAbstractItemModel *centralListModel();

public slots:
    virtual Domain::Task::Ptr addItem(const QString &title, const QModelIndex &parentIndex = QModelIndex()) = 0;
    virtual void removeItem(const QModelIndex &index) = 0;
    virtual void promoteItem(const QModelIndex &index) = 0;

protected:
    struct TaskExtraData
    {
        bool isChildTask = false;
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr dataSourceQueryResult;
        Domain::QueryResult<Domain::Project::Ptr>::Ptr projectQueryResult;
        Domain::QueryResult<Domain::Context::Ptr>::Ptr contextQueryResult;
    };
    using TaskExtraDataPtr = QSharedPointer<TaskExtraData>;

    using ProjectQueryPtr = Domain::QueryResult<Domain::Project::Ptr>::Ptr;
    static TaskExtraDataPtr fetchTaskExtraData(Domain::TaskQueries::Ptr taskQueries,
                                               const TaskExtraParts &parts,
                                               const QModelIndex &index,
                                               const Domain::Task::Ptr &task);
    static QVariant defaultTaskData(const Domain::Task::Ptr &task, int role, const TaskExtraDataPtr &info);

private:
    virtual QAbstractItemModel *createCentralListModel() = 0;

    QAbstractItemModel *m_centralListModel;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Presentation::PageModel::TaskExtraParts)

#endif // PRESENTATION_PAGEMODEL_H
