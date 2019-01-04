/* This file is part of Zanshin

   Copyright 2015 Kevin Ottens <ervin@kde.org>

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

#include <testlib/qtest_zanshin.h>

#include "akonadi/akonadilivequeryhelpers.h"

#include <functional>

#include <KCalCore/Todo>
#include <KMime/Message>
#include <Akonadi/Notes/NoteUtils>

#include "akonadi/akonadiserializer.h"

#include "testlib/akonadifakedata.h"
#include "testlib/gencollection.h"
#include "testlib/gentag.h"
#include "testlib/gentodo.h"
#include "testlib/testhelpers.h"

using namespace Testlib;

using namespace std::placeholders;

Q_DECLARE_METATYPE(Akonadi::StorageInterface::FetchContentTypes)

static QString titleFromItem(const Akonadi::Item &item)
{
    if (item.hasPayload<KCalCore::Todo::Ptr>()) {
        const auto todo = item.payload<KCalCore::Todo::Ptr>();
        return todo->summary();

    } else if (item.hasPayload<KMime::Message::Ptr>()) {
        const auto message = item.payload<KMime::Message::Ptr>();
        const Akonadi::NoteUtils::NoteMessageWrapper note(message);
        return note.title();

    } else {
        return QString();
    }
}

class AkonadiLiveQueryHelpersTest : public QObject
{
    Q_OBJECT

private:
    Akonadi::LiveQueryHelpers::Ptr createHelpers(AkonadiFakeData &data)
    {
        return Akonadi::LiveQueryHelpers::Ptr(new Akonadi::LiveQueryHelpers(createSerializer(), createStorage(data)));
    }

    Akonadi::StorageInterface::Ptr createStorage(AkonadiFakeData &data)
    {
        return Akonadi::StorageInterface::Ptr(data.createStorage());
    }

    Akonadi::SerializerInterface::Ptr createSerializer()
    {
        return Akonadi::SerializerInterface::Ptr(new Akonadi::Serializer);
    }

private slots:
    void shouldFetchAllCollectionsForType_data()
    {
        QTest::addColumn<Akonadi::StorageInterface::FetchContentTypes>("contentTypes");

        QTest::newRow("task collections only") << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Tasks);
        QTest::newRow("note collections only") << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Notes);
        QTest::newRow("task and note collections only") << (Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes);
        QTest::newRow("all collections") << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::AllContent);
    }

    void shouldFetchAllCollectionsForType()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Three top level collections (any content, tasks and notes)
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());
        data.createCollection(GenCollection().withId(44).withRootAsParent().withName(QStringLiteral("44")).withNoteContent());

        // Three children under each of the top level for each content type
        data.createCollection(GenCollection().withId(45).withParent(42).withName(QStringLiteral("45")));
        data.createCollection(GenCollection().withId(46).withParent(42).withName(QStringLiteral("46")).withTaskContent());
        data.createCollection(GenCollection().withId(47).withParent(42).withName(QStringLiteral("47")).withNoteContent());
        data.createCollection(GenCollection().withId(48).withParent(43).withName(QStringLiteral("48")));
        data.createCollection(GenCollection().withId(49).withParent(43).withName(QStringLiteral("49")).withTaskContent());
        data.createCollection(GenCollection().withId(50).withParent(43).withName(QStringLiteral("50")).withNoteContent());
        data.createCollection(GenCollection().withId(51).withParent(44).withName(QStringLiteral("51")));
        data.createCollection(GenCollection().withId(52).withParent(44).withName(QStringLiteral("52")).withTaskContent());
        data.createCollection(GenCollection().withId(53).withParent(44).withName(QStringLiteral("53")).withNoteContent());

        // The list which will be filled by the fetch function
        auto collections = Akonadi::Collection::List();
        auto add = [&collections] (const Akonadi::Collection &collection) {
            collections.append(collection);
        };

        // WHEN
        QFETCH(Akonadi::StorageInterface::FetchContentTypes, contentTypes);
        auto fetch = helpers->fetchAllCollections(contentTypes);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(collections.constBegin(), collections.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Collection::displayName, _1));
        result.sort();

        // THEN
        auto expected = QStringList();

        if (contentTypes == Akonadi::StorageInterface::AllContent) {
            expected << QStringLiteral("42") << QStringLiteral("45") << QStringLiteral("48") << QStringLiteral("51");
        }

        if ((contentTypes & Akonadi::StorageInterface::Tasks) || contentTypes == Akonadi::StorageInterface::AllContent) {
            expected << QStringLiteral("43") << QStringLiteral("46") << QStringLiteral("49") << QStringLiteral("52");
        }

        if ((contentTypes & Akonadi::StorageInterface::Notes) || contentTypes == Akonadi::StorageInterface::AllContent) {
            expected << QStringLiteral("44") << QStringLiteral("47") << QStringLiteral("50") << QStringLiteral("53");
        }

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        collections.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(collections.constBegin(), collections.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Collection::displayName, _1));
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchCollectionsForRootAndType_data()
    {
        QTest::addColumn<Akonadi::Collection>("root");
        QTest::addColumn<Akonadi::StorageInterface::FetchContentTypes>("contentTypes");

        QTest::newRow("task collections only from root") << Akonadi::Collection::root()
                                                         << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Tasks);
        QTest::newRow("note collections only from root") << Akonadi::Collection::root()
                                                         << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Notes);
        QTest::newRow("task and note collections only from root") << Akonadi::Collection::root()
                                                                  << (Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes);
        QTest::newRow("all collections from root") << Akonadi::Collection::root()
                                                   << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::AllContent);

        QTest::newRow("task collections only from 'all branch'") << Akonadi::Collection(42)
                                                                 << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Tasks);
        QTest::newRow("note collections only from 'all branch'") << Akonadi::Collection(42)
                                                                 << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Notes);
        QTest::newRow("task and note collections only from 'all branch'") << Akonadi::Collection(42)
                                                                          << (Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes);
        QTest::newRow("all collections from 'all branch'") << Akonadi::Collection(42)
                                                           << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::AllContent);

        QTest::newRow("task collections only from 'task branch'") << Akonadi::Collection(43)
                                                                  << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Tasks);
        QTest::newRow("note collections only from 'task branch'") << Akonadi::Collection(43)
                                                                  << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Notes);
        QTest::newRow("task and note collections only from 'task branch'") << Akonadi::Collection(43)
                                                                           << (Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes);
        QTest::newRow("all collections from 'task branch'") << Akonadi::Collection(43)
                                                            << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::AllContent);

        QTest::newRow("task collections only from 'note branch'") << Akonadi::Collection(44)
                                                                  << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Tasks);
        QTest::newRow("note collections only from 'note branch'") << Akonadi::Collection(44)
                                                                  << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::Notes);
        QTest::newRow("task and note collections only from 'note branch'") << Akonadi::Collection(44)
                                                                           << (Akonadi::StorageInterface::Tasks | Akonadi::StorageInterface::Notes);
        QTest::newRow("all collections from 'note branch'") << Akonadi::Collection(44)
                                                            << Akonadi::StorageInterface::FetchContentTypes(Akonadi::StorageInterface::AllContent);
    }

    void shouldFetchCollectionsForRootAndType()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Three top level collections (any content, tasks and notes)
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());
        data.createCollection(GenCollection().withId(44).withRootAsParent().withName(QStringLiteral("44")).withNoteContent());

        // Three children under each of the top level for each content type
        data.createCollection(GenCollection().withId(45).withParent(42).withName(QStringLiteral("45")));
        data.createCollection(GenCollection().withId(46).withParent(42).withName(QStringLiteral("46")).withTaskContent());
        data.createCollection(GenCollection().withId(47).withParent(42).withName(QStringLiteral("47")).withNoteContent());
        data.createCollection(GenCollection().withId(48).withParent(43).withName(QStringLiteral("48")));
        data.createCollection(GenCollection().withId(49).withParent(43).withName(QStringLiteral("49")).withTaskContent());
        data.createCollection(GenCollection().withId(50).withParent(43).withName(QStringLiteral("50")).withNoteContent());
        data.createCollection(GenCollection().withId(51).withParent(44).withName(QStringLiteral("51")));
        data.createCollection(GenCollection().withId(52).withParent(44).withName(QStringLiteral("52")).withTaskContent());
        data.createCollection(GenCollection().withId(53).withParent(44).withName(QStringLiteral("53")).withNoteContent());

        // The list which will be filled by the fetch function
        auto collections = Akonadi::Collection::List();
        auto add = [&collections] (const Akonadi::Collection &collection) {
            collections.append(collection);
        };

        // WHEN
        QFETCH(Akonadi::Collection, root);
        QFETCH(Akonadi::StorageInterface::FetchContentTypes, contentTypes);
        auto fetch = helpers->fetchCollections(root, contentTypes);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(collections.constBegin(), collections.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Collection::displayName, _1));
        result.sort();

        // THEN
        auto expected = QStringList();

        if (root == Akonadi::Collection::root()) {
            expected << QStringLiteral("42") << QStringLiteral("43") << QStringLiteral("44");
        } else {
            const qint64 baseId = root.id() == 42 ? 45
                                : root.id() == 43 ? 48
                                : root.id() == 44 ? 51
                                : -1;
            QVERIFY(baseId > 0);

            if (contentTypes == Akonadi::StorageInterface::AllContent) {
                expected << QString::number(baseId);
            }

            if ((contentTypes & Akonadi::StorageInterface::Tasks) || contentTypes == Akonadi::StorageInterface::AllContent) {
                expected << QString::number(baseId + 1);
            }

            if ((contentTypes & Akonadi::StorageInterface::Notes) || contentTypes == Akonadi::StorageInterface::AllContent) {
                expected << QString::number(baseId + 2);
            }
        }

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        collections.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(collections.constBegin(), collections.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Collection::displayName, _1));
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchItemsByContentTypes()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two top level collections, one with no particular content, one with tasks
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")));
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());

        // One note collection as child of the first one
        data.createCollection(GenCollection().withId(44).withParent(42).withName(QStringLiteral("44")).withNoteContent());

        // One task collection as child of the note collection
        data.createCollection(GenCollection().withId(45).withParent(44).withName(QStringLiteral("45")).withTaskContent());

        // One task in the first collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")));

        // Two tasks in all the other collections
        data.createItem(GenTodo().withId(43).withParent(43).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withId(44).withParent(43).withTitle(QStringLiteral("44")));
        data.createItem(GenTodo().withId(45).withParent(44).withTitle(QStringLiteral("45")));
        data.createItem(GenTodo().withId(46).withParent(44).withTitle(QStringLiteral("46")));
        data.createItem(GenTodo().withId(47).withParent(45).withTitle(QStringLiteral("47")));
        data.createItem(GenTodo().withId(48).withParent(45).withTitle(QStringLiteral("48")));

        // The list which will be filled by the fetch function
        auto items = Akonadi::Item::List();
        auto add = [&items] (const Akonadi::Item &item) {
            items.append(item);
        };

        // WHEN
        auto fetch = helpers->fetchItems(Akonadi::StorageInterface::Tasks);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();

        // THEN
        auto expected = QStringList();
        expected << QStringLiteral("43") << QStringLiteral("44") << QStringLiteral("47") << QStringLiteral("48");
        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        items.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchItemsByCollection_data()
    {
        QTest::addColumn<Akonadi::Collection>("collection");

        QTest::newRow("first collection") << Akonadi::Collection(42);
        QTest::newRow("second collection") << Akonadi::Collection(43);
    }

    void shouldFetchItemsByCollection()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two top level collections with tasks
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());

        // Two items in each collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withId(44).withParent(43).withTitle(QStringLiteral("44")));
        data.createItem(GenTodo().withId(45).withParent(43).withTitle(QStringLiteral("45")));

        // The list which will be filled by the fetch function
        auto items = Akonadi::Item::List();
        auto add = [&items] (const Akonadi::Item &item) {
            items.append(item);
        };

        // WHEN
        QFETCH(Akonadi::Collection, collection);
        auto fetch = helpers->fetchItems(collection);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();

        // THEN
        auto expected = QStringList();

        switch (collection.id()) {
        case 42:
            expected << QStringLiteral("42") << QStringLiteral("43");
            break;
        case 43:
            expected << QStringLiteral("44") << QStringLiteral("45");
            break;
        }
        QVERIFY(!expected.isEmpty());

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        items.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchItemsByTag_data()
    {
        QTest::addColumn<Akonadi::Tag>("tag");

        QTest::newRow("first tag") << Akonadi::Tag(42);
        QTest::newRow("second tag") << Akonadi::Tag(43);
    }

    void shouldFetchItemsByTag()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two top level collections with tasks
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withTaskContent());

        // Two tags
        data.createTag(GenTag().withId(42));
        data.createTag(GenTag().withId(43));

        // Four items in each collection, one with no tag, one with the first tag,
        // one with the second tag, last one with both tags
        data.createItem(GenTodo().withId(42).withParent(42).withTags({}).withTitle(QStringLiteral("42")));
        data.createItem(GenTodo().withId(43).withParent(42).withTags({42}).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withId(44).withParent(42).withTags({43}).withTitle(QStringLiteral("44")));
        data.createItem(GenTodo().withId(45).withParent(42).withTags({42, 43}).withTitle(QStringLiteral("45")));
        data.createItem(GenTodo().withId(46).withParent(43).withTags({}).withTitle(QStringLiteral("46")));
        data.createItem(GenTodo().withId(47).withParent(43).withTags({42}).withTitle(QStringLiteral("47")));
        data.createItem(GenTodo().withId(48).withParent(43).withTags({43}).withTitle(QStringLiteral("48")));
        data.createItem(GenTodo().withId(49).withParent(43).withTags({42, 43}).withTitle(QStringLiteral("49")));

        // The list which will be filled by the fetch function
        auto items = Akonadi::Item::List();
        auto add = [&items] (const Akonadi::Item &item) {
            items.append(item);
        };

        // WHEN
        QFETCH(Akonadi::Tag, tag);
        auto fetch = helpers->fetchItems(tag);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();

        // THEN
        auto expected = QStringList();

        switch (tag.id()) {
        case 42:
            expected << QStringLiteral("43") << QStringLiteral("45") << QStringLiteral("47") << QStringLiteral("49");
            break;
        case 43:
            expected << QStringLiteral("44") << QStringLiteral("45") << QStringLiteral("48") << QStringLiteral("49");
            break;
        }
        QVERIFY(!expected.isEmpty());

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        items.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchSiblings_data()
    {
        QTest::addColumn<Akonadi::Item>("item");

        QTest::newRow("item in first collection") << Akonadi::Item(43);
        QTest::newRow("item in second collection") << Akonadi::Item(48);
    }

    void shouldFetchSiblings()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two top level collections (one with notes, one with tasks)
        data.createCollection(GenCollection().withId(42).withRootAsParent().withName(QStringLiteral("42")).withTaskContent());
        data.createCollection(GenCollection().withId(43).withRootAsParent().withName(QStringLiteral("43")).withNoteContent());

        // Four items in each collection
        data.createItem(GenTodo().withId(42).withParent(42).withTitle(QStringLiteral("42")));
        data.createItem(GenTodo().withId(43).withParent(42).withTitle(QStringLiteral("43")));
        data.createItem(GenTodo().withId(44).withParent(42).withTitle(QStringLiteral("44")));
        data.createItem(GenTodo().withId(45).withParent(42).withTitle(QStringLiteral("45")));
        data.createItem(GenTodo().withId(46).withParent(43).withTitle(QStringLiteral("46")));
        data.createItem(GenTodo().withId(47).withParent(43).withTitle(QStringLiteral("47")));
        data.createItem(GenTodo().withId(48).withParent(43).withTitle(QStringLiteral("48")));
        data.createItem(GenTodo().withId(49).withParent(43).withTitle(QStringLiteral("49")));

        // The list which will be filled by the fetch function
        auto items = Akonadi::Item::List();
        auto add = [&items] (const Akonadi::Item &item) {
            items.append(item);
        };

        // WHEN
        QFETCH(Akonadi::Item, item);
        auto fetch = helpers->fetchSiblings(item);
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();

        // THEN
        auto expected = QStringList();

        switch (item.id()) {
        case 43:
            expected << QStringLiteral("42") << QStringLiteral("43") << QStringLiteral("44") << QStringLiteral("45");
            break;
        case 48:
            expected << QStringLiteral("46") << QStringLiteral("47") << QStringLiteral("48") << QStringLiteral("49");
            break;
        }
        QVERIFY(!expected.isEmpty());

        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        items.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(items.constBegin(), items.constEnd(),
                       std::back_inserter(result),
                       titleFromItem);
        result.sort();
        QCOMPARE(result, expected);
    }

    void shouldFetchTags()
    {
        // GIVEN
        auto data = AkonadiFakeData();
        auto helpers = createHelpers(data);

        // Two tags (one plain, one context)
        data.createTag(GenTag().withId(42).withName(QStringLiteral("42")).asPlain());
        data.createTag(GenTag().withId(43).withName(QStringLiteral("43")).asContext());

        // The list which will be filled by the fetch function
        auto tags = Akonadi::Tag::List();
        auto add = [&tags] (const Akonadi::Tag &tag) {
            tags.append(tag);
        };

        // WHEN
        auto fetch = helpers->fetchTags();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        auto result = QStringList();
        std::transform(tags.constBegin(), tags.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Tag::name, _1));
        result.sort();

        // THEN
        auto expected = QStringList({"42", "43"});
        expected.sort();
        QCOMPARE(result, expected);

        // WHEN (should not crash when the helpers object is deleted)
        helpers.clear();
        tags.clear();
        fetch(add);
        TestHelpers::waitForEmptyJobQueue();

        // THEN
        result.clear();
        std::transform(tags.constBegin(), tags.constEnd(),
                       std::back_inserter(result),
                       std::bind(&Akonadi::Tag::name, _1));
        result.sort();
        QCOMPARE(result, expected);
    }
};

ZANSHIN_TEST_MAIN(AkonadiLiveQueryHelpersTest)

#include "akonadilivequeryhelperstest.moc"
