/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012-2013 Jolla Ltd.
** Contact: Robin Burchell <robin.burchell@jollamobile.com>
**
** This file is part of lipstick.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include <QDBusConnection>
#include <QTimer>
#include "homeapplication.h"
#include "lipsticknotification.h"
#include "notificationmanager.h"
#include "diskspacenotifier.h"

DiskSpaceNotifier::DiskSpaceNotifier(QObject *parent) : QObject(parent),
    notificationId(0)
{
    QDBusConnection::systemBus().connect(QString(), "/com/nokia/diskmonitor/signal", "com.nokia.diskmonitor.signal", "disk_space_change_ind", this, SLOT(handleDiskSpaceChange(QString, int)));

    QTimer::singleShot(0, this, SLOT(removeDiskSpaceNotifications()));
}

DiskSpaceNotifier::~DiskSpaceNotifier()
{
}

void DiskSpaceNotifier::handleDiskSpaceChange(const QString &path, int percentage)
{
    bool notificationShouldBeVisible = false;

    if (percentage == 100) {
        // Disk space usage for path is 100%
        if (!notificationsSentForPath[path].second) {
            notificationShouldBeVisible = true;
            notificationsSentForPath[path].second = true;
        }
    } else {
        // Disk space usage for path is above the notification threshold
        if (!notificationsSentForPath[path].first) {
            notificationShouldBeVisible = true;
            notificationsSentForPath[path].first = true;
        }
    }

    if (notificationShouldBeVisible) {
        if (notificationId != 0) {
            // Destroy any previous notification
            NotificationManager *manager = NotificationManager::instance();
            manager->CloseNotification(notificationId);
        }

        // Show a notification
        NotificationManager *manager = NotificationManager::instance();
        QVariantHash hints;
        hints.insert(NotificationManager::HINT_CATEGORY, "x-nemo.system.diskspace");
        //% "Getting low with storage. Please check."
        hints.insert(NotificationManager::HINT_PREVIEW_BODY, qtTrId("qtn_memu_memlow_notification_src"));
        // TODO go to some relevant place when clicking the notification
        notificationId = manager->Notify(qApp->applicationName(), 0, QString(), QString(), QString(), QStringList(), hints, -1);
    }
}

void DiskSpaceNotifier::removeDiskSpaceNotifications()
{
    NotificationManager *manager = NotificationManager::instance();
    foreach (uint id, manager->notificationIds()) {
        LipstickNotification *notification = NotificationManager::instance()->notification(id);
        if (notification->appName() == qApp->applicationName() && notification->category() == "x-nemo.system.diskspace") {
            manager->CloseNotification(id);
        }
    }
}
