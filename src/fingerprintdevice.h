/*
 * Copyright (C) 2022 Chupligin Sergey <neochapay@gmail.com>
 * Copyright 2020  Devin Lin <espidev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FINGERPRINTDEVICE_H
#define FINGERPRINTDEVICE_H

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusPendingReply>
#include <QDBusInterface>

class FingerPrintDevice : public QObject
{
    Q_OBJECT
public:
    explicit FingerPrintDevice(QDBusObjectPath path, QObject* parent = nullptr);

    QDBusPendingReply<QStringList> listEnrolledFingers(const QString &username);

    QDBusError claim(const QString &username);
    QDBusError release();

    QDBusError deleteEnrolledFingers();
    QDBusError deleteEnrolledFinger(QString &finger);
    QDBusError startEnrolling(const QString &finger);
    QDBusError stopEnrolling();

    int numOfEnrollStages();
    QString scanType();
    bool fingerPresent();
    bool fingerNeeded();

public Q_SLOTS:
    void enrollStatus(QString result, bool done);

Q_SIGNALS:
    void enrollCompleted();
    void enrollStagePassed();
    void enrollRetryStage(QString feedback);
    void enrollFailed(QString error);

private:
    QString m_devicePath;
    QString m_username;
    NetReactivatedFprintDeviceInterface *m_fprintInterface;
    QDBusInterface *m_freedesktopInterface;
};

#endif // FINGERPRINTDEVICE_H
