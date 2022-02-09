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

#include "fingerprintdevice.h"

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

#include <QDBusInterface>
#include <QDBusReply>

#include <QtDBus>

FingerPrintDevice::FingerPrintDevice(QDBusObjectPath path, QObject *parent)
    : QObject(parent)
    , m_devicePath(path.path())
    , m_fprintInterface(new NetReactivatedFprintDeviceInterface(QStringLiteral("net.reactivated.Fprint"), path.path(), QDBusConnection::systemBus(), this))
    , m_freedesktopInterface(
          new QDBusInterface(QStringLiteral("net.reactivated.Fprint"), path.path(), "org.freedesktop.DBus.Properties", QDBusConnection::systemBus(), this))
{
    connect(m_fprintInterface, &NetReactivatedFprintDeviceInterface::EnrollStatus, this, &FingerPrintDevice::enrollStatus);
}

QDBusPendingReply<QStringList> FingerPrintDevice::listEnrolledFingers(const QString &username)
{
    return m_fprintInterface->ListEnrolledFingers(username);
}

QDBusError FingerPrintDevice::claim(const QString &username)
{
    auto reply = m_fprintInterface->Claim(username);
    reply.waitForFinished();
    return reply.error();
}

QDBusError FingerPrintDevice::release()
{
    auto reply = m_fprintInterface->Release();
    reply.waitForFinished();
    return reply.error();
}

QDBusError FingerPrintDevice::deleteEnrolledFingers()
{
    auto reply = m_fprintInterface->DeleteEnrolledFingers2();
    reply.waitForFinished();
    return reply.error();
}

QDBusError FingerPrintDevice::deleteEnrolledFinger(QString &finger)
{
    auto reply = m_fprintInterface->DeleteEnrolledFinger(finger);
    reply.waitForFinished();
    return reply.error();
}

QDBusError FingerPrintDevice::startEnrolling(const QString &finger)
{
    auto reply = m_fprintInterface->EnrollStart(finger);
    reply.waitForFinished();
    return reply.error();
}

QDBusError FingerPrintDevice::stopEnrolling()
{
    auto reply = m_fprintInterface->EnrollStop();
    reply.waitForFinished();
    return reply.error();
}

void FingerPrintDevice::enrollStatus(QString result, bool done)
{
    if (result == "enroll-completed") {
        Q_EMIT enrollCompleted();
    } else if (result == "enroll-failed" || result == "enroll-data-full" || result == "enroll-disconnected" || result == "enroll-unknown-error") {
        Q_EMIT enrollFailed(result);
    } else if (result == "enroll-stage-passed") {
        Q_EMIT enrollStagePassed();
    } else if (result == "enroll-retry-scan" || result == "enroll-swipe-too-short" || result == "enroll-finger-not-centered" || result == "enroll-remove-and-retry") {
        Q_EMIT enrollRetryStage(result);
    }
}

int FingerPrintDevice::numOfEnrollStages()
{
    QDBusReply<QDBusVariant> reply = m_freedesktopInterface->call("Get", "net.reactivated.Fprint.Device", "num-enroll-stages");
    if (!reply.isValid()) {
        qDebug() << "error fetching num-enroll-stages:" << reply.error();
        return 0;
    }
    return reply.value().variant().toInt();
}

QString FingerPrintDevice::scanType()
{
    QDBusReply<QDBusVariant> reply = m_freedesktopInterface->call("Get", "net.reactivated.Fprint.Device", "scan-type");
    if (!reply.isValid()) {
        qDebug() << "error fetching scan-type:" << reply.error();
        return "";
    }
    return reply.value().variant().toString();
}

bool FingerPrintDevice::fingerPresent()
{
    QDBusReply<QDBusVariant> reply = m_freedesktopInterface->call("Get", "net.reactivated.Fprint.Device", "finger-present");
    if (!reply.isValid()) {
        qDebug() << "error fetching finger-present:" << reply.error();
        return "";
    }
    return reply.value().variant().toBool();
}

bool FingerPrintDevice::fingerNeeded()
{
    QDBusReply<QDBusVariant> reply = m_freedesktopInterface->call("Get", "net.reactivated.Fprint.Device", "finger-needed");
    if (!reply.isValid()) {
        qDebug() << "error fetching finger-needed:" << reply.error();
        return "";
    }
    return reply.value().variant().toBool();
}
