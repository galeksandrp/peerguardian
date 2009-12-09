/*

  D-Bus messaging interface

  (c) 2008 jpv (jpv950@gmail.com)

  (c) 2008 Jindrich Makovicka (makovick@gmail.com)

  This file is part of pgl.

  pgl is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  pgl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GNU Emacs; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

// #include <syslog.h>
// #include <stdio.h>
#include "dbus.h"

static DBusConnection *dbconn = NULL;

int pgl_dbus_init() {
    DBusError dberr;
    int req;

    dbus_error_init (&dberr);
    dbconn = dbus_bus_get (DBUS_BUS_SYSTEM, &dberr);
    if (dbus_error_is_set (&dberr)) {
        fprintf(stderr, "Connection Error (%s)\n", dberr.message);
        dbus_error_free(&dberr);
    }
    if (dbconn == NULL) {
      return -1;
    }
    do_log(LOG_INFO, "INFO: Connected to system bus.");

    /* need d-bus policy privileges for this to work */
//     dbus_error_init (&dberr);
    req = dbus_bus_request_name (dbconn, NFB_DBUS_PUBLIC_NAME,
                                 DBUS_NAME_FLAG_DO_NOT_QUEUE, &dberr);
    if (dbus_error_is_set (&dberr)) {
        do_log(LOG_ERR, "ERROR: requesting name: %s.", dberr.message);
        dbus_error_free(&dberr);
        return -1;
    }
    if (req == DBUS_REQUEST_NAME_REPLY_EXISTS) {
        /* FIXME: replace the current name owner instead of giving up?
         * Need to request name with DBUS_NAME_FLAG_ALLOW_REPLACEMENT
         * in that case... */
        do_log(LOG_WARNING, "WARN: pgld is already running.");
        return -1;
    }
    return 0;
}

void pgl_dbus_send(const char *format, va_list ap) {
    dbus_uint32_t serial = 0; // unique number to associate replies with requests
    DBusMessage *dbmsg = NULL;
    DBusMessageIter dbiter;
    char msg[MSG_SIZE];
    vsnprintf(msg, sizeof msg, format, ap);

    /* create dbus signal */
    dbmsg = dbus_message_new_signal ("/org/netfilter/pgl",
                                         "org.netfilter.pgl",
                                         "pgld_message");
//     if (!dbmsg)
//         return -1;

    dbus_message_iter_init_append(dbmsg, &dbiter);
    if (!dbus_message_iter_append_basic(&dbiter, DBUS_TYPE_STRING, &msg)) {
      fprintf(stderr, "Out Of Memory!\n");
//       return -1;
   }
    if (!dbus_connection_send (dbconn, dbmsg, &serial)) {
        fprintf(stderr, "Out Of Memory!\n");
//         return -1;
    }
    dbus_connection_flush(dbconn);
    dbus_message_unref(dbmsg);

//     return 0;
}
