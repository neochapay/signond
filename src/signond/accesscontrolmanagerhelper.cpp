/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2011 Intel Corporation.
 * Copyright (C) 2013-2016 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 * Contact: Elena Reshetova <elena.reshetova@intel.com>
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

#include <QBuffer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#ifdef ENABLE_P2P
#include <dbus/dbus.h>
#endif

#include "accesscontrolmanagerhelper.h"
#include "signond-common.h"
#include "credentialsaccessmanager.h"
#include "signonidentity.h"

using namespace SignonDaemonNS;

AccessControlManagerHelper *AccessControlManagerHelper::m_pInstance = NULL;

AccessControlManagerHelper *AccessControlManagerHelper::instance()
{
    return m_pInstance;
}

AccessControlManagerHelper::AccessControlManagerHelper(
                                SignOn::AbstractAccessControlManager *acManager)
{
    if (!m_pInstance) {
        m_pInstance = this;
        m_acManager = acManager;
    } else {
        BLAME() << "Creating a second instance of the CAM";
    }
}

AccessControlManagerHelper::~AccessControlManagerHelper()
{
    m_acManager = NULL;
    m_pInstance = NULL;
}


bool AccessControlManagerHelper::isPeerAllowedToUseIdentity(
                                       const PeerContext &peerContext,
                                       const quint32 identityId)
{
    // TODO - improve this, the error handling and more precise behaviour

    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    if (db == 0) {
        TRACE() << "NULL db pointer, secure storage might be unavailable,";
        return false;
    }
    QStringList acl = db->accessControlList(identityId);

    TRACE() << QString(QLatin1String("Access control list of identity: "
                                 "%1: [%2].Tokens count: %3\t"))
                                .arg(identityId)
                                .arg(acl.join(QLatin1String(", ")))
                                .arg(acl.size());

    if (db->errorOccurred())
        return false;

    IdentityOwnership ownership =
        isPeerOwnerOfIdentity(peerContext, identityId);
    if (ownership == ApplicationIsOwner)
        return true;

    if (acl.isEmpty())
        return false;

    if (acl.contains(QLatin1String("*")))
        return true;

    return peerHasOneOfAccesses(peerContext, acl);
}

AccessControlManagerHelper::IdentityOwnership
AccessControlManagerHelper::isPeerOwnerOfIdentity(
                                       const PeerContext &peerContext,
                                       const quint32 identityId)
{
    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    if (db == 0) {
        TRACE() << "NULL db pointer, secure storage might be unavailable,";
        return ApplicationIsNotOwner;
    }
    QStringList ownerSecContexts = db->ownerList(identityId);

    if (db->errorOccurred())
        return ApplicationIsNotOwner;

    if (ownerSecContexts.isEmpty())
        return IdentityDoesNotHaveOwner;

    return peerHasOneOfAccesses(peerContext, ownerSecContexts) ?
        ApplicationIsOwner : ApplicationIsNotOwner;
}

bool
AccessControlManagerHelper::isPeerKeychainWidget(
                                       const PeerContext &peerContext)
{
    static QString keychainWidgetAppId = m_acManager->keychainWidgetAppId();
    QString peerAppId = appIdOfPeer(peerContext);
    return (peerAppId == keychainWidgetAppId);
}

QString AccessControlManagerHelper::appIdOfPeer(
                                       const PeerContext &peerContext)
{
    TRACE() << m_acManager->appIdOfPeer(peerContext.connection(),
                                        peerContext.message());
    return m_acManager->appIdOfPeer(peerContext.connection(),
                                    peerContext.message());
}

bool
AccessControlManagerHelper::peerHasOneOfAccesses(
                                       const PeerContext &peerContext,
                                       const QStringList secContexts)
{
    foreach(QString securityContext, secContexts)
    {
        TRACE() << securityContext;
        if (isPeerAllowedToAccess(peerContext, securityContext))
            return true;
    }

    BLAME() << "given peer does not have needed permissions";
    return false;
}

bool
AccessControlManagerHelper::isPeerAllowedToAccess(
                                       const PeerContext &peerContext,
                                       const QString securityContext)
{
    TRACE() << securityContext;
    return m_acManager->isPeerAllowedToAccess(peerContext.connection(),
                                              peerContext.message(),
                                              securityContext);
}

pid_t AccessControlManagerHelper::pidOfPeer(const PeerContext &peerContext)
{
    QString service = peerContext.message().service();
    if (service.isEmpty()) {
#ifdef ENABLE_P2P
        DBusConnection *connection =
            (DBusConnection *)peerContext.connection().internalPointer();
        unsigned long pid = 0;
        dbus_bool_t ok = dbus_connection_get_unix_process_id(connection,
                                                             &pid);
        if (Q_UNLIKELY(!ok)) {
            BLAME() << "Couldn't get PID of caller!";
            return 0;
        }
        return pid;
#else
        BLAME() << "Empty caller name, and no P2P support enabled";
        return 0;
#endif
    } else {
        return peerContext.connection().interface()->servicePid(service).value();
    }
}

SignOn::AccessReply *
AccessControlManagerHelper::requestAccessToIdentity(
                                       const PeerContext &peerContext,
                                       quint32 id)
{
    SignOn::AccessRequest request;
    request.setPeer(peerContext.connection(), peerContext.message());
    request.setIdentity(id);
    return m_acManager->handleRequest(request);
}
