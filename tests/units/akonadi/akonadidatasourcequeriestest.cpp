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

#include <QtTest>

#include <mockitopp/mockitopp.hpp>

#include "testlib/akonadimocks.h"

#include "akonadi/akonadidatasourcequeries.h"
#include "akonadi/akonadiserializerinterface.h"
#include "akonadi/akonadistorageinterface.h"

using namespace mockitopp;

class AkonadiDataSourceQueriesTest : public QObject
{
    Q_OBJECT
private slots:
    void shouldListAllDataSources()
    {
        // GIVEN

        // Two top level collections and one child collection
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource2(new Domain::DataSource);
        Akonadi::Collection col3(44);
        col3.setParentCollection(col2);
        Domain::DataSource::Ptr dataSource3(new Domain::DataSource);
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).thenReturn(dataSource3);

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         new MockMonitor(this)));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTasks();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), dataSource1);
        QCOMPARE(result->data().at(1), dataSource2);
        QCOMPARE(result->data().at(2), dataSource3);
    }

    void shouldIgnoreCollectionsWhichAreNotDataSources()
    {
        // GIVEN

        // Two top level collections and one child collection
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        // One of the collections not being a data source
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource2;
        Akonadi::Collection col3(44);
        col3.setParentCollection(col2);
        Domain::DataSource::Ptr dataSource3(new Domain::DataSource);
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).thenReturn(dataSource3);

        // WHEN
        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         new MockMonitor(this)));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTasks();

        // THEN
        QVERIFY(result->data().isEmpty());
        QTest::qWait(150);
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), dataSource1);
        QCOMPARE(result->data().at(1), dataSource3);
    }

    void shouldReactToCollectionAddsForDataSourcesOnly()
    {
        // GIVEN

        // Empty collection fetch
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the items
        mock_object<Akonadi::SerializerInterface> serializerMock;

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         monitor));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTasks();
        QTest::qWait(150);
        QVERIFY(result->data().isEmpty());

        // WHEN
        Akonadi::Collection col1(42);
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        Akonadi::Collection col2(43);
        Domain::DataSource::Ptr dataSource2;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        monitor->addCollection(col1);
        monitor->addCollection(col2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));

        QCOMPARE(result->data().size(), 1);
        QCOMPARE(result->data().first(), dataSource1);
    }

    void shouldReactToCollectionRemovesForAllDataSources()
    {
        // GIVEN

        // Two top level collections and one child collection
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource2(new Domain::DataSource);
        Akonadi::Collection col3(44);
        col3.setParentCollection(col2);
        Domain::DataSource::Ptr dataSource3(new Domain::DataSource);
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the collections
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).thenReturn(dataSource3);

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         monitor));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTasks();
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->removeCollection(col3);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).exactly(1));

        QCOMPARE(result->data().size(), 2);
        QCOMPARE(result->data().at(0), dataSource1);
        QCOMPARE(result->data().at(1), dataSource2);
    }

    void shouldReactToCollectionChangesForAllDataSources()
    {
        // GIVEN

        // Two top level collections and one child collection
        Akonadi::Collection col1(42);
        col1.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource1(new Domain::DataSource);
        Akonadi::Collection col2(43);
        col2.setParentCollection(Akonadi::Collection::root());
        Domain::DataSource::Ptr dataSource2(new Domain::DataSource);
        Akonadi::Collection col3(44);
        col3.setParentCollection(col2);
        Domain::DataSource::Ptr dataSource3(new Domain::DataSource);
        MockCollectionFetchJob *collectionFetchJob = new MockCollectionFetchJob(this);
        collectionFetchJob->setCollections(Akonadi::Collection::List() << col1 << col2 << col3);

        // Storage mock returning the fetch jobs
        mock_object<Akonadi::StorageInterface> storageMock;
        storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                       Akonadi::StorageInterface::Recursive,
                                                                       Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                 .thenReturn(collectionFetchJob);

        // Serializer mock returning the data sources from the collections
        mock_object<Akonadi::SerializerInterface> serializerMock;
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).thenReturn(dataSource1);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).thenReturn(dataSource2);
        serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).thenReturn(dataSource3);
        serializerMock(&Akonadi::SerializerInterface::updateDataSourceFromCollection).when(dataSource2, col2).thenReturn();

        // Monitor mock
        MockMonitor *monitor = new MockMonitor(this);

        QScopedPointer<Domain::DataSourceQueries> queries(new Akonadi::DataSourceQueries(&storageMock.getInstance(),
                                                                                         &serializerMock.getInstance(),
                                                                                         monitor));
        Domain::QueryResult<Domain::DataSource::Ptr>::Ptr result = queries->findTasks();
        // Even though the pointer didn't change it's convenient to user if we call
        // the replace handlers
        bool replaceHandlerCalled = false;
        result->addPostReplaceHandler([&replaceHandlerCalled](const Domain::DataSource::Ptr &, int) {
                                          replaceHandlerCalled = true;
                                      });
        QTest::qWait(150);
        QCOMPARE(result->data().size(), 3);

        // WHEN
        monitor->changeCollection(col2);

        // THEN
        QVERIFY(storageMock(&Akonadi::StorageInterface::fetchCollections).when(Akonadi::Collection::root(),
                                                                               Akonadi::StorageInterface::Recursive,
                                                                               Akonadi::StorageInterface::Notes|Akonadi::StorageInterface::Tasks)
                                                                         .exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col1).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col2).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::createDataSourceFromCollection).when(col3).exactly(1));
        QVERIFY(serializerMock(&Akonadi::SerializerInterface::updateDataSourceFromCollection).when(dataSource2, col2).exactly(1));

        QCOMPARE(result->data().size(), 3);
        QCOMPARE(result->data().at(0), dataSource1);
        QCOMPARE(result->data().at(1), dataSource2);
        QCOMPARE(result->data().at(2), dataSource3);
        QVERIFY(replaceHandlerCalled);
    }
};

QTEST_MAIN(AkonadiDataSourceQueriesTest)

#include "akonadidatasourcequeriestest.moc"
