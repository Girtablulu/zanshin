/* This file is part of Zanshin Todo.

   Copyright 2008-2010 Kevin Ottens <ervin@kde.org>
   Copyright 2008, 2009 Mario Bensi <nef@ipsquad.net>

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

#ifndef ZANSHIN_TODOHELPERS_H
#define ZANSHIN_TODOHELPERS_H

#include <KDE/Akonadi/Collection>
#include <KDE/Akonadi/Item>
#include <KDE/Akonadi/TransactionSequence>
#include <QModelIndex>

#include "todomodel.h"

namespace TodoHelpers
{
    void addProject(const QString &summary, const Akonadi::Collection &collection);
    void addProject(const QString &summary, const Akonadi::Item &parentProject);
    void addCategory(const QString &category, const QString &parentCategory = QString());
    bool removeProject(QWidget *parent, const QModelIndex &project);
    bool removeCategory(QWidget *parent, const QModelIndex &categoryIndex);
    bool removeTodoFromCategory(const QModelIndex &todo, const QString &category);
    bool moveTodoToProject(const QModelIndex &todo, const QString &parentUid, const TodoModel::ItemType parentType, const Akonadi::Collection &parentCollection);
    bool moveTodoToCategory(const QModelIndex &todo, const QString &category, const TodoModel::ItemType parentType);
    void moveCategory(const QString &oldCategoryPath, const QString &parentPath, const TodoModel::ItemType parentType);
    void renameCategory(const QString &oldCategoryPath, const QString &newCategoryName);
}

#endif

