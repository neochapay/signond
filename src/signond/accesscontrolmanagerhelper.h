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

/*!
  @file accesscontrolmanagerhelper.h
  Helper class for access control-related functionality
  @ingroup Accounts_and_SSO_Framework
 */

#ifndef ACCESSCONTROLMANAGERHELPER_H
#define ACCESSCONTROLMANAGERHELPER_H

#include "peercontext.h"
#include "signonauthsession.h"
#include "SignOn/abstract-access-control-manager.h"

namespace SignonDaemonNS {

/*!
 * @class AccessControlManagerHelper
 * Contains helper functions related to Access Control
 * @ingroup Accounts_and_SSO_Framework
 */
class AccessControlManagerHelper
{
public:
    /*!
     * @enum IdentityOwnership
     * Specifies the owner relationship of an application over a specific
     * identity, or the lack of ownership over that specific identity.
     * @see isPeerOwnerOfIdentity().
     */
    enum IdentityOwnership {
        ApplicationIsOwner = 0,
        ApplicationIsNotOwner,
        IdentityDoesNotHaveOwner
    };

    AccessControlManagerHelper(SignOn::AbstractAccessControlManager *acManager);
    ~AccessControlManagerHelper();

    /*!
     * @param peerContext, the context, which process id we want to know
     * @returns process id of service client.
     */
    static pid_t pidOfPeer(const PeerContext &peerContext);

    /* creating an instance of a class */
    static AccessControlManagerHelper *instance();

    /*!
     * Checks if a client process is allowed to use a specific SignonIdentity.
     * @param peerContext the peer connection over which the message was sent.
     * @param identityId, the SignonIdentity to be used.
     * @returns true, if the peer is allowed, false otherwise.
     */
    bool isPeerAllowedToUseIdentity(const PeerContext &peerContext,
                                    const quint32 identityId);

    /*!
     * Checks if a specific process is the owner of a SignonIdentity, thus
     * having full control over it.
     * @param peerContext the peer connection over which the message was sent.
     * @param identityId, the SignonIdentity in context.
     * @retval ApplicationIsOwner/ApplicationIsNotOwner if the identity
     * is/isn't the owner or IdentityDoesNotHaveOwner if the identity does not
     * have an owner at all.
     */
    IdentityOwnership isPeerOwnerOfIdentity(const PeerContext &peerContext,
                                            const quint32 identityId);

    /*!
     * Checks if a specific process is allowed to use the SignonAuthSession
     * functionality.
     * @param peerContext the peer connection over which the message was sent.
     * @param authSession, the authentication session to be used by the peer
     * request.
     * @returns true, if the peer is allowed, false otherwise.
     */
    bool isPeerAllowedToUseAuthSession(const PeerContext &peerContext,
                                       const SignonAuthSession &authSession)
    {
        return isPeerAllowedToUseIdentity(peerContext, authSession.id());
    }

    /*!
     * Checks if a specific process is allowed to use the SignonAuthSession
     * functionality.
     * @param peerContext the peer connection over which the message was sent.
     * @param ownerIdentityId, id of the Identity owning the authentication
     * session.
     * @returns true, if the peer is allowed, false otherwise.
     */
    bool isPeerAllowedToUseAuthSession(const PeerContext &peerContext,
                                       const quint32 ownerIdentityId)
    {
        return isPeerAllowedToUseIdentity(peerContext, ownerIdentityId);
    }

    /*!
     * @param peerContext the peer connection over which the message was sent.
     * @returns true, if the peer is the Keychain Widget, false otherwise.
     */
    bool isPeerKeychainWidget(const PeerContext &peerContext);

    /*!
     * Looks up for the application identifier of a specific client process.
     * @param peerContext the peer connection over which the message was sent.
     * @returns the application identifier of the process, or an empty string
     * if none found.
     */
    QString appIdOfPeer(const PeerContext &peerContext);

    /*!
     * Checks if a client process is allowed to access objects with a certain
     * security context.
     * The access type to be checked depends on the concrete implementation of
     * this function.
     * @param peerContext the peer connection over which the message was sent.
     * @param securityContext, the securityContext to be checked against.
     * @returns true, if the peer is allowed, false otherwise.
     */
    bool isPeerAllowedToAccess(const PeerContext &peerContext,
                               const QString securityContext);

    /*!
     * Checks if a client process is allowed to access at least one object from
     * the list with a certain security context.
     * The access type to be checked depends on the concrete implementation of
     * this function.
     * @param peerContext the peer connection over which the message was sent.
     * @param secContexts, the objects' securityContexts to be checked against.
     * @returns true, if the peer is allowed, false otherwise.
     */
    bool peerHasOneOfAccesses(const PeerContext &peerContext,
                              const QStringList secContexts);

    SignOn::AccessReply *
        requestAccessToIdentity(const PeerContext &peerContext,
                                quint32 id);

private:
    SignOn::AbstractAccessControlManager *m_acManager;
    static AccessControlManagerHelper* m_pInstance;
};

} // namespace SignonDaemonNS

#endif // ACCESSCONTROLMANAGER_H
