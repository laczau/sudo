/*
 * Copyright (c) 2010 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <config.h>

#include <sys/types.h>
#include <stdio.h>
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif /* STDC_HEADERS */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <libaudit.h>

#include "missing.h"
#include "error.h"
#include "alloc.h"
#include "gettext.h"
#include "sudo_debug.h"
#include "linux_audit.h"

/*
 * Open audit connection if possible.
 * Returns audit fd on success and -1 on failure.
 */
int
static linux_audit_open(void)
{
    static int au_fd = -1;
    debug_decl(linux_audit_open, SUDO_DEBUG_AUDIT)

    if (au_fd != -1)
	debug_return_int(au_fd);
    au_fd = audit_open();
    if (au_fd == -1) {
	/* Kernel may not have audit support. */
	if (errno != EINVAL && errno != EPROTONOSUPPORT && errno != EAFNOSUPPORT)
	    error(1, _("unable to open audit system"));
    } else {
	(void)fcntl(au_fd, F_SETFD, FD_CLOEXEC);
    }
    debug_return_int(au_fd);
}

int
linux_audit_command(char *argv[], int result)
{
    int au_fd, rc;
    char *command, *cp, **av;
    size_t size, n;
    debug_decl(linux_audit_command, SUDO_DEBUG_AUDIT)

    if ((au_fd = linux_audit_open()) == -1)
	debug_return_int(-1);

    /* Convert argv to a flat string. */
    for (size = 0, av = argv; *av != NULL; av++)
	size += strlen(*av) + 1;
    command = cp = emalloc(size);
    for (av = argv; *av != NULL; av++) {
	n = strlcpy(cp, *av, size - (cp - command));
	if (n >= size - (cp - command)) {
	    errorx(1, _("internal error, %s overflow"),
		"linux_audit_command()");
	}
	cp += n;
	*cp++ = ' ';
    }
    *--cp = '\0';

    /* Log command, ignoring ECONNREFUSED on error. */
    rc = audit_log_user_command(au_fd, AUDIT_USER_CMD, command, NULL, result);
    if (rc <= 0 && errno != ECONNREFUSED)
	warning(_("unable to send audit message"));

    efree(command);

    debug_return_int(rc);
}