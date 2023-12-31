/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2013-2016 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "signond-common.h"
#include "signonauthsession.h"

using namespace SignonDaemonNS;

SignonAuthSession::SignonAuthSession(SignonSessionCore *core,
                                     pid_t ownerPid):
    QObject(core),
    m_ownerPid(ownerPid)
{
    TRACE();

    static quint32 incr = 0;
    QString objectName = SIGNOND_DAEMON_OBJECTPATH +
        QLatin1String("/AuthSession_") + QString::number(incr++, 16);
    TRACE() << objectName;

    setObjectName(objectName);

    connect(core, SIGNAL(stateChanged(const QString&, int, const QString&)),
            this, SLOT(stateChangedSlot(const QString&, int, const QString&)));
}

SignonAuthSession::~SignonAuthSession()
{
    Q_EMIT unregistered();
    TRACE();
}

SignonAuthSession *SignonAuthSession::createAuthSession(const quint32 id,
                                                        const QString &method,
                                                        SignonDaemon *parent,
                                                        pid_t ownerPid)
{
    TRACE();
    SignonSessionCore *core = SignonSessionCore::sessionCore(id, method, parent);
    if (!core) {
        TRACE() << "Cannot retrieve proper tasks queue";
        return NULL;
    }

    SignonAuthSession *sas = new SignonAuthSession(core, ownerPid);

    TRACE() << "SignonAuthSession created successfully:" << sas->objectName();
    return sas;
}

void SignonAuthSession::stopAllAuthSessions()
{
    SignonSessionCore::stopAllAuthSessions();
}

quint32 SignonAuthSession::id() const
{
    return parent()->id();
}

QString SignonAuthSession::method() const
{
    return parent()->method();
}

pid_t SignonAuthSession::ownerPid() const
{
    return m_ownerPid;
}

QStringList
SignonAuthSession::queryAvailableMechanisms(const QStringList &wantedMechanisms)
{
    return parent()->queryAvailableMechanisms(wantedMechanisms);
}

void SignonAuthSession::process(const QVariantMap &sessionDataVa,
                                const QString &mechanism,
                                const PeerContext &peerContext,
                                const ProcessCb &callback)
{
    parent()->process(peerContext,
                      sessionDataVa,
                      mechanism,
                      objectName(),
                      callback);
}

void SignonAuthSession::cancel()
{
    TRACE();
    parent()->cancel(objectName());
}

void SignonAuthSession::setId(quint32 id)
{
    parent()->setId(id);
}

void SignonAuthSession::objectUnref()
{
    //TODO - remove the `objectUnref` functionality from the DBus API
    TRACE();
    cancel();

    deleteLater();
}

void SignonAuthSession::stateChangedSlot(const QString &sessionKey,
                                         int state,
                                         const QString &message)
{
    TRACE();

    if (sessionKey == objectName())
        emit stateChanged(state, message);
}
