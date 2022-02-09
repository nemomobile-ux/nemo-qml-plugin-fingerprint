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

#include "fingerprintmodel.h"

#include <QDebug>

FingerPrintModel::FingerPrintModel(QObject* parent)
    : QObject(parent)
    , m_managerDbusInterface(new NetReactivatedFprintManagerInterface(QStringLiteral("net.reactivated.Fprint"), QStringLiteral("/net/reactivated/Fprint/Manager"), QDBusConnection::systemBus(), this))
{
    auto reply = m_managerDbusInterface->GetDefaultDevice();
    reply.waitForFinished();

    if (reply.isError()) {
        qDebug() << reply.error().message();
        setCurrentError(reply.error().message());
        return;
    }

    QDBusObjectPath path = reply.value();
    m_device = new FingerPrintDevice(path, this);

    connect(m_device, &FingerPrintDevice::enrollCompleted, this, &FingerPrintModel::handleEnrollCompleted);
    connect(m_device, &FingerPrintDevice::enrollStagePassed, this, &FingerPrintModel::handleEnrollStagePassed);
    connect(m_device, &FingerPrintDevice::enrollRetryStage, this, &FingerPrintModel::handleEnrollRetryStage);
    connect(m_device, &FingerPrintDevice::enrollFailed, this, &FingerPrintModel::handleEnrollFailed);
}

FingerPrintModel::~FingerPrintModel()
{
    if (m_device) { // just in case device is claimed
        m_device->stopEnrolling();
        m_device->release();
    }
}

QString FingerPrintModel::scanType()
{
    return !m_device ? "" : m_device->scanType();
}

QString FingerPrintModel::currentError()
{
    return m_currentError;
}

void FingerPrintModel::setCurrentError(QString error)
{
    if (m_currentError != error) {
        m_currentError = error;
        Q_EMIT currentErrorChanged();
    }
}

QString FingerPrintModel::enrollFeedback()
{
    return m_enrollFeedback;
}

void FingerPrintModel::setEnrollFeedback(QString feedback)
{
    m_enrollFeedback = feedback;
    Q_EMIT enrollFeedbackChanged();
}

bool FingerPrintModel::currentlyEnrolling()
{
    return m_currentlyEnrolling;
}

bool FingerPrintModel::deviceFound()
{
    return m_device != nullptr;
}

double FingerPrintModel::enrollProgress()
{
    if (!deviceFound()) {
        return 0;
    }
    return (m_device->numOfEnrollStages() == 0) ? 1 : ((double) m_enrollStage) / m_device->numOfEnrollStages();
}

void FingerPrintModel::setEnrollStage(int stage)
{
    m_enrollStage = stage;
    Q_EMIT enrollProgressChanged();
}

FingerPrintModel::DialogState FingerPrintModel::dialogState()
{
    return m_dialogState;
}

void FingerPrintModel::setDialogState(DialogState dialogState)
{
    m_dialogState = dialogState;
    Q_EMIT dialogStateChanged();
}

void FingerPrintModel::switchUser(QString username)
{
    m_username = username;

    if (deviceFound()) {
        stopEnrolling(); // stop enrolling if ongoing
        m_device->release(); // release from old user

        Q_EMIT enrolledFingerprintsChanged();
    }
}

bool FingerPrintModel::claimDevice()
{
    if (!deviceFound()) {
        return false;
    }

    QDBusError error = m_device->claim(m_username);
    if (error.isValid() && error.name() != "net.reactivated.Fprint.Error.AlreadyInUse") {
        qDebug() << "error claiming:" << error.message();
        setCurrentError(error.message());
        return false;
    }
    return true;
}

void FingerPrintModel::startEnrolling(QString finger)
{
    if (!deviceFound()) {
        setCurrentError(tr("No fingerprint device found."));
        setDialogState(DialogState::FingerprintList);
        return;
    }

    setEnrollStage(0);
    setEnrollFeedback({});

    // claim device for user
    if (!claimDevice()) {
        setDialogState(DialogState::FingerprintList);
        return;
    }

    QDBusError error = m_device->startEnrolling(finger);
    if (error.isValid()) {
        qDebug() << "error start enrolling:" << error.message();
        setCurrentError(error.message());
        m_device->release();
        setDialogState(DialogState::FingerprintList);
        return;
    }

    m_currentlyEnrolling = true;
    Q_EMIT currentlyEnrollingChanged();

    setDialogState(DialogState::Enrolling);
}

void FingerPrintModel::stopEnrolling()
{
    setDialogState(DialogState::FingerprintList);
    if (m_currentlyEnrolling) {
        m_currentlyEnrolling = false;
        Q_EMIT currentlyEnrollingChanged();

        QDBusError error = m_device->stopEnrolling();
        if (error.isValid()) {
            qDebug() << "error stop enrolling:" << error.message();
            setCurrentError(error.message());
            return;
        }
        m_device->release();
    }
}

void FingerPrintModel::deleteFingerprint(QString finger)
{
    // claim for user
    if (!claimDevice()) {
        return;
    }

    QDBusError error = m_device->deleteEnrolledFinger(finger);
    if (error.isValid()) {
        qDebug() << "error deleting fingerprint:" << error.message();
        setCurrentError(error.message());
    }

    // release from user
    error = m_device->release();
    if (error.isValid()) {
        qDebug() << "error releasing:" << error.message();
        setCurrentError(error.message());
    }

    Q_EMIT enrolledFingerprintsChanged();
}

void FingerPrintModel::clearFingerprints()
{
    // claim for user
    if (!claimDevice()) {
        return;
    }

    QDBusError error = m_device->deleteEnrolledFingers();
    if (error.isValid()) {
        qDebug() << "error clearing fingerprints:" << error.message();
        setCurrentError(error.message());
    }

    // release from user
    error = m_device->release();
    if (error.isValid()) {
        qDebug() << "error releasing:" << error.message();
        setCurrentError(error.message());
    }

    Q_EMIT enrolledFingerprintsChanged();
}

QStringList FingerPrintModel::enrolledFingerprintsRaw()
{
    if (deviceFound()) {
        QDBusPendingReply reply = m_device->listEnrolledFingers(m_username);
        reply.waitForFinished();
        if (reply.isError()) {
            // ignore net.reactivated.Fprint.Error.NoEnrolledPrints, as it shows up when there are no fingerprints
            if (reply.error().name() != "net.reactivated.Fprint.Error.NoEnrolledPrints") {
                qDebug() << "error listing enrolled fingers:" << reply.error().message();
                setCurrentError(reply.error().message());
            }
            return QStringList();
        }
        return reply.value();
    } else {
        setCurrentError(tr("No fingerprint device found."));
        setDialogState(DialogState::FingerprintList);
        return QStringList();
    }
}

QVariantList FingerPrintModel::enrolledFingerprints()
{
    // convert fingers list to qlist of Finger objects
    QVariantList fingers;
    for (QString &finger : enrolledFingerprintsRaw()) {
        for (Finger *storedFinger : FINGERS) {
            if (storedFinger->name() == finger) {
                fingers.append(QVariant::fromValue(storedFinger));
                break;
            }
        }
    }
    return fingers;
}

QVariantList FingerPrintModel::availableFingersToEnroll()
{
    QVariantList list;
    QStringList enrolled = enrolledFingerprintsRaw();

    // add fingerprints to list that are not in the enrolled list
    for (Finger *finger : FINGERS) {
        if (!enrolledFingerprintsRaw().contains(finger->name())) {
            list.append(QVariant::fromValue(finger));
        }
    }
    return list;
}

void FingerPrintModel::handleEnrollCompleted()
{
    setEnrollStage(m_device->numOfEnrollStages());
    setEnrollFeedback({});
    Q_EMIT enrolledFingerprintsChanged();
    Q_EMIT scanComplete();

    // stopEnrolling not called, as it is triggered only when the "complete" button is pressed
    // (only change dialog state change after button is pressed)
    setDialogState(DialogState::EnrollComplete);
}

void FingerPrintModel::handleEnrollStagePassed()
{
    setEnrollStage(m_enrollStage + 1);
    setEnrollFeedback({});
    Q_EMIT scanSuccess();
    qDebug() << "fingerprint enroll stage pass:" << enrollProgress();
}

void FingerPrintModel::handleEnrollRetryStage(QString feedback)
{
    Q_EMIT scanFailure();
    if (feedback == "enroll-retry-scan") {
        setEnrollFeedback(tr("Retry scanning your finger."));
    } else if (feedback == "enroll-swipe-too-short") {
        setEnrollFeedback(tr("Swipe too short. Try again."));
    } else if (feedback == "enroll-finger-not-centered") {
        setEnrollFeedback(tr("Finger not centered on the reader. Try again."));
    } else if (feedback == "enroll-remove-and-retry") {
        setEnrollFeedback(tr("Remove your finger from the reader, and try again."));
    }
    qDebug() << "fingerprint enroll stage fail:" << feedback;
}

void FingerPrintModel::handleEnrollFailed(QString error)
{
    if (error == "enroll-failed") {
        setCurrentError(tr("Fingerprint enrollment has failed."));
        stopEnrolling();
    } else if (error == "enroll-data-full") {
        setCurrentError(tr("There is no space left for this device, delete other fingerprints to continue."));
        stopEnrolling();
    } else if (error == "enroll-disconnected") {
        setCurrentError(tr("The device was disconnected."));
        m_currentlyEnrolling = false;
        Q_EMIT currentlyEnrollingChanged();
        setDialogState(DialogState::FingerprintList);
    } else if (error == "enroll-unknown-error") {
        setCurrentError(tr("An unknown error has occurred."));
        stopEnrolling();
    }
}
