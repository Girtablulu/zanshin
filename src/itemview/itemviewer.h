/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) <year>  <name of author>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef ITEMVIEWER_H
#define ITEMVIEWER_H

#include "ui_itemview.h"
#include <QFrame>
#include <KXMLGUIClient>

#include <Akonadi/Item>

#include "abstractpimitem.h"

class ItemContext;
class QFocusEvent;

class KXMLGUIClient;

class QTimer;

namespace Ui {
    class tags;
    class properties;
}

/**
 * The editor part for editing notes/todos/events
 */
class ItemViewer : public QFrame, private Ui_itemView, public KXMLGUIClient
{
    Q_OBJECT
public:
    explicit ItemViewer(QWidget* parent, KXMLGUIClient *parentClient);
    virtual ~ItemViewer();

protected:
    //void focusOutEvent ( QFocusEvent * event );
    //void focusInEvent ( QFocusEvent * event );

public slots:
    /**
     * Set new Akonadi::Item in the viewer, this function also stores the (changed) content to the old item
     *
     */
    void setItem(const Akonadi::Item &);
    
    void setItem(const Nepomuk::Resource &res);

    /**
     * This will fetch the given item first, before setting it on the editor
     */
    void setItem(const KUrl &);
    
private slots:
    ///add tag from tag edit
    void itemsReceived(const Akonadi::Item::List&);
    void addTag();
    void saveItem();
    void updateContent(AbstractPimItem::ChangedParts parts = AbstractPimItem::AllParts);
    void itemRemoved();
    void clearView();

    void setDueDate(KDateTime, bool);
    void setStatus(int);

    void setEventDate(KDateTime);

    void autosave();
    
    void setFullscreenEditor();
    void restoreState();

private:

    AbstractPimItem *m_currentItem;
    QTimer *m_autosaveTimer;
    int m_autosaveTimeout;
    ItemContext *m_itemContext;
    Ui::tags *ui_tags;
    Ui::properties *ui_properties;

};

#endif // ITEMVIEWER_H