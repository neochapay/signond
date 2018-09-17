/*
 * This file is part of signon
 *
 * Copyright (C) 2018 elementary, Inc
 *
 * Contact: Corentin Noël <corentin@elementary.io>
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

#ifndef LIBSIGNON_SECURITY_CONTEXT_PRIV_H
#define LIBSIGNON_SECURITY_CONTEXT_PRIV_H

#include <QDBusMetaType>

#include "securitycontext.h"

namespace SignOn {
QDBusArgument &operator<<(QDBusArgument &argument,
                          const SecurityContext &securityContext);
const QDBusArgument &operator>>(const QDBusArgument &argument,
                                SecurityContext &securityContext);
} //namespace SignOn

#endif /* LIBSIGNON_SECURITY_CONTEXT_PRIV_H */
