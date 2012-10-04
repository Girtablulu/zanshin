/* This file is part of Zanshin Todo.

   Copyright 2012 Christian Mollekopf <chrigi_1@fastmail.fm>

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

#include <qtest_kde.h>

#include <reparentingmodel/pimitemrelations.h>
#include <utils/incidenceitem.h>

Q_DECLARE_METATYPE(QModelIndex)

class PimItemRelationsTest : public QObject
{
    Q_OBJECT
private slots:
//     void initTestCase()
//     {
//         QTest::setEvaluatedItemRoles(roles);
//     }

    static Akonadi::Item getEventItem(const QList<PimItemRelation> &relations = QList<PimItemRelation>())
    {
        IncidenceItem inc(AbstractPimItem::Event);
        inc.setRelations(relations);
        Akonadi::Item item = inc.getItem();
        item.setId(1);
        return item;
    }
    
    static Akonadi::Item getContextItem(Akonadi::Item::Id id = 1, const PimItemTreeNode &node = PimItemTreeNode("uid", "name"))
    {
        PimItemRelation rel(PimItemRelation::Context, QList<PimItemTreeNode>() << node);
        Akonadi::Item item = getEventItem(QList<PimItemRelation>() << rel);
        item.setId(id);
        return item;
    }

    static Akonadi::Item getProjectItem(Akonadi::Item::Id id = 1, const PimItemTreeNode &node = PimItemTreeNode("uid", "name"))
    {
        PimItemRelation rel(PimItemRelation::Project, QList<PimItemTreeNode>() << node);
        Akonadi::Item item = getEventItem(QList<PimItemRelation>() << rel);
        item.setId(id);
        return item;
    }

    static QByteArray getUid(const Akonadi::Item &item)
    {
        IncidenceItem inc(item);
        return inc.getUid().toLatin1();
    }

    PimItemRelationsStructure *getStructure()
    {
        return new PimItemRelationsStructure(PimItemRelation::Context);
    }

    void returnCorrectItemId()
    {
        Akonadi::Item item = getEventItem();
        
        QScopedPointer<PimItemRelations> relation(getStructure());
        Id id = relation->addItem(item);
        QVERIFY(id >= 0);
        QCOMPARE(relation->getItemId(item), id);
    }
    
    void returnEmptyParents()
    {
        Akonadi::Item item = getEventItem();
        
        QScopedPointer<PimItemRelations> relation(getStructure());
        Id id = relation->addItem(item);
        qDebug() << relation->getParents(id);
        QCOMPARE(relation->getParents(id), IdList());
    }

    void getParents()
    {
        Akonadi::Item item = getContextItem(1, PimItemTreeNode("uid", "name"));
        
        QScopedPointer<PimItemRelations> relation(getStructure());
        Id id = relation->addItem(item);
        QCOMPARE(relation->getItemId(item), id);
        QCOMPARE(relation->getParents(id).size(), 1);
        Id firstParent = relation->getParents(id).first();
        QCOMPARE(relation->getName(firstParent), QLatin1String("name"));
    }
    
    void checkParentHierarchy()
    {
        PimItemTreeNode parent2("uid2", "name2");
        PimItemTreeNode parent1("uid", "name", QList<PimItemTreeNode>() << parent2);
        Akonadi::Item item = getContextItem(1, parent1);
        
        QScopedPointer<PimItemRelations> relation(getStructure());
        Id id = relation->addItem(item);
        Id firstParent = relation->getParents(id).first();
        QCOMPARE(relation->getName(firstParent), QLatin1String("name"));
        QCOMPARE(relation->getParents(firstParent).size(), 1);
        Id secondParent = relation->getParents(firstParent).first();
        QCOMPARE(relation->getName(secondParent), QLatin1String("name2"));
    }
    
    void mergeAndOverwriteName()
    {
        PimItemTreeNode parent2("uid2", "name2");
        PimItemTreeNode parent1("uid", "name", QList<PimItemTreeNode>() << parent2);
        Akonadi::Item item1 = getContextItem(1, parent1);
        
        PimItemTreeNode parent4("uid2", "name3");
        PimItemTreeNode parent3("uid", "name", QList<PimItemTreeNode>() << parent4);
        Akonadi::Item item2 = getContextItem(2, parent3);
        
        QScopedPointer<PimItemRelations> relation(getStructure());
        Id id = relation->addItem(item1);
        relation->addItem(item2);
        Id firstParent = relation->getParents(id).first();
        QCOMPARE(relation->getName(firstParent), QLatin1String("name"));
        QCOMPARE(relation->getParents(firstParent).size(), 1);
        Id secondParent = relation->getParents(firstParent).first();
        QCOMPARE(relation->getName(secondParent), QLatin1String("name3"));
    }
    
    void cleanupParentsAfterChildInsert()
    {
        Akonadi::Item item = getContextItem(1, PimItemTreeNode("uid", "name"));
        Akonadi::Item item2 = getContextItem(2, PimItemTreeNode(getUid(item), "name"));
        
        QScopedPointer<PimItemRelations> relation(getStructure());
        Id id = relation->addItem(item);
        Id id2 = relation->addItem(item2);
        QCOMPARE(relation->getParents(id).size(), 0);
    }
    


    //Project tests
    void getParentProject()
    {
        Akonadi::Item item = getProjectItem();
        
        QScopedPointer<ProjectStructure> relation(new ProjectStructure());
        Id id = relation->addItem(item);
        QCOMPARE(relation->getItemId(item), id);
        QCOMPARE(relation->getParents(id).size(), 1);
        Id firstParent = relation->getParents(id).first();
    }
    
    void matchProjectByUid()
    {
        Akonadi::Item item = getProjectItem(1, PimItemTreeNode("uid", "name"));
        Akonadi::Item item2 = getProjectItem(2, PimItemTreeNode(getUid(item), "name"));
        
        QScopedPointer<ProjectStructure> relation(new ProjectStructure());
        Id id = relation->addItem(item);
        Id id2 = relation->addItem(item2);
        QCOMPARE(relation->getParents(id2).first(), id);
    }
  
    void matchProjectByUidReverse()
    {
        Akonadi::Item item = getProjectItem(1, PimItemTreeNode("uid", "name"));
        Akonadi::Item item2 = getProjectItem(2, PimItemTreeNode(getUid(item), "name"));
        
        QScopedPointer<ProjectStructure> relation(new ProjectStructure());
        Id id2 = relation->addItem(item2);
        Id id = relation->addItem(item);
        QCOMPARE(relation->getParents(id2).first(), id);
    }
    
    void dontLooseParentInformationOnChildInsert()
    {
        Akonadi::Item item = getProjectItem(1, PimItemTreeNode("uid", "name"));
        Akonadi::Item item2 = getProjectItem(2, PimItemTreeNode(getUid(item), "name"));
        
        QScopedPointer<ProjectStructure> relation(new ProjectStructure());
        Id id = relation->addItem(item);
        Id id2 = relation->addItem(item2);
        QCOMPARE(relation->getParents(id).size(), 1);
    }
    
    void getIdFromUid()
    {
        Akonadi::Item item = getProjectItem(1, PimItemTreeNode("uid", "name"));
        QScopedPointer<ProjectStructure> relation(new ProjectStructure());
        Id id = relation->addItem(item);
        QCOMPARE(relation->getId(getUid(item)), id);
    }

};

QTEST_KDEMAIN(PimItemRelationsTest, GUI)

#include "pimitemrelationstest.moc"