/* This file is part of Zanshin Todo.

   Copyright 2011 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#include "noteitem.h"
#include "pimitemrelations.h"

#include <Akonadi/EntityDisplayAttribute>
#include <akonadi/notes/noteutils.h>

#include <KMime/Message>
#include <QCoreApplication>

NoteItem::NoteItem(QObject *parent)
:   PimItem(parent)
{
    //init payload, mimetype, and displayattribute
    commitData();
}
/*
NoteItem::Note(const Note &note)
:   PimItem(note.getItem())
{
    m_text = note.m_text;
    m_title = note.m_title;
    m_creationDate = note.m_creationDate;
}*/

NoteItem::NoteItem(const Akonadi::Item &item, QObject *parent)
:   PimItem(item, parent)
{
    fetchData();
}

NoteItem::NoteItem(PimItem &item, QObject* parent)
:   PimItem(item, parent)
{
    commitData();
}


bool NoteItem::hasValidPayload()
{
    if (m_item.hasPayload<KMime::Message::Ptr>()) {
        return true;
    }
    return false;
}


void NoteItem::commitData()
{
    m_item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    Akonadi::NoteUtils::NoteMessageWrapper messageWrapper;
    messageWrapper.setTitle(m_title);
    messageWrapper.setText(m_text, m_textIsRich ? Qt::RichText : Qt::PlainText);
    messageWrapper.setCreationDate(m_creationDate);
    messageWrapper.setFrom(QCoreApplication::applicationName()+QCoreApplication::applicationVersion());
    messageWrapper.setLastModifiedDate(KDateTime::currentUtcDateTime());
    messageWrapper.setUid(m_uid);
    m_item.setPayload(messageWrapper.message());
    
    Akonadi::EntityDisplayAttribute *eda = new Akonadi::EntityDisplayAttribute();
    eda->setIconName(getIconName());
    eda->setDisplayName(m_title);
    m_item.addAttribute(eda);
}

void NoteItem::fetchData()
{
    if (m_dataFetched) {
        return;
    }
    
    if ( !hasValidPayload()) {
        kDebug() << "invalid payload";
        return;
    }
    
    KMime::Message::Ptr msg = m_item.payload<KMime::Message::Ptr>();
    Q_ASSERT(msg.get());
    Akonadi::NoteUtils::NoteMessageWrapper messageWrapper(msg);
    m_textIsRich = messageWrapper.textFormat() == Qt::RichText;
    m_titleIsRich = false;
    m_title = messageWrapper.title();
    m_text = messageWrapper.text();
    m_creationDate = messageWrapper.creationDate();
    m_lastModifiedDate = messageWrapper.lastModifiedDate();
    m_uid = messageWrapper.uid();
    m_dataFetched = true;
}


QString NoteItem::mimeType()
{
    Q_ASSERT(PimItem::mimeType(PimItem::Note) == Akonadi::NoteUtils::noteMimeType());
    return PimItem::mimeType(PimItem::Note);
}

PimItem::ItemStatus NoteItem::getStatus() const
{
    return PimItem::Later;
}


KDateTime NoteItem::getPrimaryDate()
{
    return getLastModifiedDate();
}

QString NoteItem::getIconName()
{
    return Akonadi::NoteUtils::noteIconName();
}

KDateTime NoteItem::getLastModifiedDate()
{
    if (m_lastModifiedDate.isValid()) {
        return m_lastModifiedDate.toLocalZone();
    }
    return PimItem::getLastModifiedDate();
}

PimItem::ItemType NoteItem::itemType()
{
    return PimItem::Note;
}

QList< PimItemRelation > NoteItem::getRelations()
{
    KMime::Message::Ptr msg = m_item.payload<KMime::Message::Ptr>();
    Akonadi::NoteUtils::NoteMessageWrapper messageWrapper(msg);
    QList<QString> xml = messageWrapper.custom().values("x-related");
    QList< PimItemRelation > relations;
    foreach(const QString &x, xml) {
//         kDebug() << xml;
        relations << relationFromXML(x.toLatin1());
    }
    return relations;
}

void NoteItem::setRelations(const QList< PimItemRelation > &relations)
{
    KMime::Message::Ptr msg = m_item.payload<KMime::Message::Ptr>();
    Akonadi::NoteUtils::NoteMessageWrapper messageWrapper(msg);
    messageWrapper.custom().remove("x-related");
    foreach(const PimItemRelation &rel, relations) {
        messageWrapper.custom().insert("x-related", relationToXML(removeDuplicates(rel)));
    }
    m_item.setPayload(messageWrapper.message());
//     kDebug() << messageWrapper.message()->encodedContent();
}

