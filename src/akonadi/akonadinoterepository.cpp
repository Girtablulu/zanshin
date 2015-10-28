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


#include "akonadinoterepository.h"

#include <Akonadi/Item>

#include "akonadicollectionfetchjobinterface.h"
#include "akonadiitemfetchjobinterface.h"
#include "akonadistoragesettings.h"

#include "utils/compositejob.h"

using namespace Akonadi;
using namespace Utils;

NoteRepository::NoteRepository(const StorageInterface::Ptr &storage,
                               const SerializerInterface::Ptr &serializer)
    : m_storage(storage),
      m_serializer(serializer)
{
}

bool NoteRepository::isDefaultSource(Domain::DataSource::Ptr source) const
{
    auto settingsCollection = StorageSettings::instance().defaultNoteCollection();
    auto sourceCollection = m_serializer->createCollectionFromDataSource(source);
    return settingsCollection == sourceCollection;
}

void NoteRepository::setDefaultSource(Domain::DataSource::Ptr source)
{
    auto collection = m_serializer->createCollectionFromDataSource(source);
    StorageSettings::instance().setDefaultNoteCollection(collection);
}

KJob *NoteRepository::create(Domain::Note::Ptr note)
{
    auto item = m_serializer->createItemFromNote(note);
    Q_ASSERT(!item.isValid());
    return createItem(item);
}

KJob *NoteRepository::createInTag(Domain::Note::Ptr note, Domain::Tag::Ptr tag)
{
    Item item = m_serializer->createItemFromNote(note);
    Q_ASSERT(!item.isValid());

    Tag akonadiTag = m_serializer->createAkonadiTagFromTag(tag);
    Q_ASSERT(akonadiTag .isValid());
    item.setTag(akonadiTag);

    return createItem(item);
}

KJob *NoteRepository::update(Domain::Note::Ptr note)
{
    auto item = m_serializer->createItemFromNote(note);
    Q_ASSERT(item.isValid());
    return m_storage->updateItem(item);
}

KJob *NoteRepository::remove(Domain::Note::Ptr note)
{
    auto item = m_serializer->createItemFromNote(note);
    return m_storage->removeItem(item);
}

KJob *NoteRepository::createItem(const Item &item)
{
    const Akonadi::Collection defaultCollection = m_storage->defaultNoteCollection();
    if (defaultCollection.isValid()) {
        return m_storage->createItem(item, defaultCollection);
    } else {
        auto job = new CompositeJob();
        CollectionFetchJobInterface *fetchCollectionJob = m_storage->fetchCollections(Akonadi::Collection::root(),
                                                                                      StorageInterface::Recursive,
                                                                                      StorageInterface::Notes);
        job->install(fetchCollectionJob->kjob(), [fetchCollectionJob, item, job, this] {
            if (fetchCollectionJob->kjob()->error() != KJob::NoError)
                return;

            Q_ASSERT(fetchCollectionJob->collections().size() > 0);
            const Akonadi::Collection::List collections = fetchCollectionJob->collections();
            Akonadi::Collection col = *std::find_if(collections.constBegin(), collections.constEnd(),
                                                    [] (const Akonadi::Collection &c) {
                return c.rights() == Akonadi::Collection::AllRights;
            });
            Q_ASSERT(col.isValid());
            auto createJob = m_storage->createItem(item, col);
            job->addSubjob(createJob);
            createJob->start();
        });
        return job;
    }
}
