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

#ifndef FINGERPRINTMODEL_H
#define FINGERPRINTMODEL_H

#include <QObject>

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

#include "finger.h"
#include "fingerprintdevice.h"

class FingerPrintModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString scanType READ scanType CONSTANT)
    Q_PROPERTY(QString currentError READ currentError WRITE setCurrentError NOTIFY currentErrorChanged) // error for ui to display
    Q_PROPERTY(QString enrollFeedback READ enrollFeedback WRITE setEnrollFeedback NOTIFY enrollFeedbackChanged)
    Q_PROPERTY(QVariantList enrolledFingerprints READ enrolledFingerprints NOTIFY enrolledFingerprintsChanged)
    Q_PROPERTY(QVariantList availableFingersToEnroll READ availableFingersToEnroll NOTIFY enrolledFingerprintsChanged)
    Q_PROPERTY(bool deviceFound READ deviceFound NOTIFY devicesFoundChanged)
    Q_PROPERTY(bool currentlyEnrolling READ currentlyEnrolling NOTIFY currentlyEnrollingChanged)
    Q_PROPERTY(double enrollProgress READ enrollProgress NOTIFY enrollProgressChanged)
    Q_PROPERTY(DialogState dialogState READ dialogState WRITE setDialogState NOTIFY dialogStateChanged)

public:
    explicit FingerPrintModel(QObject* parent = nullptr);
    ~FingerPrintModel();

    enum DialogState {
        FingerprintList,
        PickFinger,
        Enrolling,
        EnrollComplete,
    };
    Q_ENUM(DialogState)

    const QList<Finger *> FINGERS = {
        new Finger("right-index-finger", tr("Right index finger"), this),
        new Finger("right-middle-finger", tr("Right middle finger"), this),
        new Finger("right-ring-finger", tr("Right ring finger"), this),
        new Finger("right-little-finger", tr("Right little finger"), this),
        new Finger("right-thumb", tr("Right thumb"), this),
        new Finger("left-index-finger", tr("Left index finger"), this),
        new Finger("left-middle-finger", tr("Left middle finger"), this),
        new Finger("left-ring-finger", tr("Left ring finger"), this),
        new Finger("left-little-finger", tr("Left little finger"), this),
        new Finger("left-thumb", tr("Left thumb"), this)
    };

    Q_INVOKABLE void switchUser(QString username);
    bool claimDevice();

    Q_INVOKABLE void startEnrolling(QString finger);
    Q_INVOKABLE void stopEnrolling();
    Q_INVOKABLE void deleteFingerprint(QString finger);
    Q_INVOKABLE void clearFingerprints();

    QStringList enrolledFingerprintsRaw();
    QVariantList enrolledFingerprints();
    QVariantList availableFingersToEnroll();

    QString scanType();
    QString currentError();
    void setCurrentError(QString error);
    QString enrollFeedback();
    void setEnrollFeedback(QString feedback);
    bool currentlyEnrolling();
    bool deviceFound();
    double enrollProgress();
    void setEnrollStage(int stage);
    DialogState dialogState();
    void setDialogState(DialogState dialogState);

public Q_SLOTS:
    void handleEnrollCompleted();
    void handleEnrollStagePassed();
    void handleEnrollRetryStage(QString feedback);
    void handleEnrollFailed(QString error);

Q_SIGNALS:
    void currentErrorChanged();
    void enrollFeedbackChanged();
    void enrolledFingerprintsChanged();
    void devicesFoundChanged();
    void currentlyEnrollingChanged();
    void enrollProgressChanged();
    void dialogStateChanged();

    void scanComplete();
    void scanSuccess();
    void scanFailure();

private:
    QString m_username; // set to "" if it is the currently logged in user
    QString m_currentError, m_enrollFeedback;

    DialogState m_dialogState = DialogState::FingerprintList;

    bool m_currentlyEnrolling = false;
    int m_enrollStage = 0;

    FingerPrintDevice *m_device = nullptr;
    NetReactivatedFprintManagerInterface *m_managerDbusInterface;
};

#endif // FINGERPRINTMODEL_H
