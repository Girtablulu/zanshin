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


#ifndef PRESENTATION_APPLICATIONMODEL_H
#define PRESENTATION_APPLICATIONMODEL_H

#include <QObject>

#include "domain/contextqueries.h"
#include "domain/contextrepository.h"
#include "domain/datasourcerepository.h"
#include "domain/datasourcequeries.h"
#include "domain/noterepository.h"
#include "domain/projectqueries.h"
#include "domain/projectrepository.h"
#include "domain/tagqueries.h"
#include "domain/tagrepository.h"
#include "domain/taskqueries.h"
#include "domain/taskrepository.h"

#include "presentation/metatypes.h"

namespace Presentation {

class AvailablePagesModelInterface;
class ErrorHandler;

class ApplicationModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* dataSourcesModel READ dataSourcesModel)
    Q_PROPERTY(Domain::DataSource::Ptr defaultDataSource READ defaultDataSource WRITE setDefaultDataSource)
    Q_PROPERTY(QObject* availableSources READ availableSources)
    Q_PROPERTY(QObject* availablePages READ availablePages)
    Q_PROPERTY(QObject* currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(QObject* editor READ editor)
public:
    explicit ApplicationModel(const Domain::ProjectQueries::Ptr &projectQueries,
                              const Domain::ProjectRepository::Ptr &projectRepository,
                              const Domain::ContextQueries::Ptr &contextQueries,
                              const Domain::ContextRepository::Ptr &contextRepository,
                              const Domain::DataSourceQueries::Ptr &sourceQueries,
                              const Domain::DataSourceRepository::Ptr &sourceRepository,
                              const Domain::TaskQueries::Ptr &taskQueries,
                              const Domain::TaskRepository::Ptr &taskRepository,
                              const Domain::NoteRepository::Ptr &noteRepository,
                              const Domain::TagQueries::Ptr &tagQueries,
                              const Domain::TagRepository::Ptr &tagRepository,
                              QObject *parent = Q_NULLPTR);

    QAbstractItemModel *dataSourcesModel();
    Domain::DataSource::Ptr defaultDataSource();

    QObject *availableSources();
    QObject *availablePages();
    QObject *currentPage();
    QObject *editor();

    ErrorHandler *errorHandler() const;

public slots:
    void setCurrentPage(QObject *page);
    virtual void setDefaultDataSource(const Domain::DataSource::Ptr &source) = 0;
    void setErrorHandler(ErrorHandler *errorHandler);

signals:
    void currentPageChanged(QObject *page);

private:
    virtual Domain::QueryResult<Domain::DataSource::Ptr>::Ptr createDataSourceQueryResult() = 0;
    virtual bool isDefaultSource(const Domain::DataSource::Ptr &source) = 0;
    virtual AvailablePagesModelInterface *createAvailablePagesModel() = 0;

    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr dataSources();

    QObject *m_availableSources;
    QObject *m_availablePages;
    QObject *m_currentPage;
    QObject *m_editor;

protected:
    Domain::QueryResult<Domain::DataSource::Ptr>::Ptr m_dataSources;
    QAbstractItemModel *m_dataSourcesModel;

    Domain::ProjectQueries::Ptr m_projectQueries;
    Domain::ProjectRepository::Ptr m_projectRepository;

    Domain::ContextQueries::Ptr m_contextQueries;
    Domain::ContextRepository::Ptr m_contextRepository;

    Domain::DataSourceQueries::Ptr m_sourceQueries;
    Domain::DataSourceRepository::Ptr m_sourceRepository;

    Domain::TaskQueries::Ptr m_taskQueries;
    Domain::TaskRepository::Ptr m_taskRepository;

    Domain::NoteRepository::Ptr m_noteRepository;

    Domain::TagQueries::Ptr m_tagQueries;
    Domain::TagRepository::Ptr m_tagRepository;

    ErrorHandler *m_errorHandler;
};

}

#endif // PRESENTATION_APPLICATIONMODEL_H
