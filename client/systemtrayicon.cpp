/**************************************************************************
 *                                                                        *
 * Copyright (C) 2016 Felix Rohrbach <kde@fxrh.de>                        *
 *                                                                        *
 * This program is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU General Public License            *
 * as published by the Free Software Foundation; either version 3         *
 * of the License, or (at your option) any later version.                 *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                        *
 **************************************************************************/

#include "systemtrayicon.h"

#include <QtGui/QWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>

#include "mainwindow.h"
#include "quaternionroom.h"
#include "linuxutils.h"
#include <settings.h>
#include <qt_connection_util.h>

SystemTrayIcon::SystemTrayIcon(MainWindow* parent)
    : QSystemTrayIcon(parent)
    , m_parent(parent)
{
    auto contextMenu = new QMenu(parent);
    auto showHideAction = contextMenu->addAction(tr("Hide"), this, &SystemTrayIcon::showHide);
    contextMenu->addAction(tr("Quit"), this, QApplication::quit);
    connect(m_parent->windowHandle(), &QWindow::visibleChanged, [showHideAction](bool visible) {
        showHideAction->setText(visible ? tr("Hide") : tr("Show"));
    });

    setIcon(QIcon::fromTheme(appIconName(), QIcon(":/icon.png")));
    setToolTip("Quaternion");
    setContextMenu(contextMenu);
    connect( this, &SystemTrayIcon::activated, this, &SystemTrayIcon::systemTrayIconAction);
}

void SystemTrayIcon::newRoom(Quotient::Room* room)
{
    connect(room, &Quotient::Room::highlightCountChanged,
            this, [this,room] { highlightCountChanged(room); });
}

void SystemTrayIcon::highlightCountChanged(Quotient::Room* room)
{
    using namespace Quotient;
    const auto mode = Settings().get<QString>("UI/notifications", "intrusive");
    if (mode == "none")
        return;
    if( room->highlightCount() > 0 ) {
        showMessage(
            //: %1 is the room display name
            tr("Highlight in %1").arg(room->displayName()),
            tr("%Ln highlight(s)", "", room->highlightCount()));
        if (mode != "non-intrusive")
            m_parent->activateWindow();
        connectSingleShot(this, &SystemTrayIcon::messageClicked, m_parent,
                          [this,qRoom=static_cast<QuaternionRoom*>(room)]
                          { m_parent->selectRoom(qRoom); });
    }
}

void SystemTrayIcon::systemTrayIconAction(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger
        || reason == QSystemTrayIcon::DoubleClick)
        showHide();
}

void SystemTrayIcon::showHide()
{
    if (m_parent->isVisible())
        m_parent->hide();
    else {
        m_parent->show();
        m_parent->activateWindow();
        m_parent->raise();
        m_parent->setFocus();
    }
}
