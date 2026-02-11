/*
 * MacOSUserFunctions.cpp - implementation of MacOSUserFunctions class
 *
 * Copyright (c) 2017-2026 Tobias Junghans <tobydox@veyon.io>
 *
 * This file is part of Veyon - https://veyon.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <QProcess>
#include <QRegularExpression>

#include <grp.h>
#include <membership.h>
#include <pwd.h>
#include <unistd.h>

#include <Security/Security.h>

#include "MacOSSessionFunctions.h"
#include "MacOSUserFunctions.h"
#include "VeyonConfiguration.h"


QString MacOSUserFunctions::fullName( const QString& username )
{
	const auto pw_entry = getpwnam(username.toUtf8().constData());

	if (pw_entry)
	{
		const auto gecos = QString::fromUtf8(pw_entry->pw_gecos);
		if (!gecos.isEmpty())
		{
			return gecos.split(QLatin1Char(',')).first();
		}
	}

	QProcess dscl;
	dscl.start(QStringLiteral("/usr/bin/dscl"),
		{ QStringLiteral("."), QStringLiteral("-read"),
		  QStringLiteral("/Users/") + username, QStringLiteral("RealName") });

	if (dscl.waitForFinished(5000))
	{
		const auto output = QString::fromUtf8(dscl.readAllStandardOutput()).trimmed();
		const auto lines = output.split(QLatin1Char('\n'));
		if (lines.size() >= 2)
		{
			return lines.last().trimmed();
		}
	}

	return username;
}



QStringList MacOSUserFunctions::userGroups( bool queryDomainGroups )
{
	Q_UNUSED(queryDomainGroups)

	QStringList groups;

	QProcess dscl;
	dscl.start(QStringLiteral("/usr/bin/dscl"),
		{ QStringLiteral("."), QStringLiteral("-list"), QStringLiteral("/Groups") });

	if (dscl.waitForFinished(5000))
	{
		const auto output = QString::fromUtf8(dscl.readAllStandardOutput()).trimmed();
		const auto lines = output.split(QLatin1Char('\n'));
		for (const auto& line : lines)
		{
			const auto groupName = line.trimmed();
			if (!groupName.isEmpty() && !groupName.startsWith(QLatin1Char('_')))
			{
				groups.append(groupName);
			}
		}
	}

	return groups;
}



QStringList MacOSUserFunctions::groupsOfUser( const QString& username, bool queryDomainGroups )
{
	Q_UNUSED(queryDomainGroups)

	QStringList groups;

	QProcess idProcess;
	idProcess.start(QStringLiteral("/usr/bin/id"),
		{ QStringLiteral("-Gn"), username });

	if (idProcess.waitForFinished(5000))
	{
		const auto output = QString::fromUtf8(idProcess.readAllStandardOutput()).trimmed();
		groups = output.split(QLatin1Char(' '), Qt::SkipEmptyParts);
	}

	return groups;
}



QString MacOSUserFunctions::userGroupSecurityIdentifier(const QString& groupName)
{
	const auto grp = getgrnam(groupName.toUtf8().constData());
	if (grp)
	{
		return QString::number(grp->gr_gid);
	}

	return {};
}



bool MacOSUserFunctions::isAnyUserLoggedOn()
{
	return !MacOSSessionFunctions::currentConsoleUser().isEmpty();
}



QString MacOSUserFunctions::currentUser()
{
	const auto consoleUser = MacOSSessionFunctions::currentConsoleUser();
	if (!consoleUser.isEmpty())
	{
		return consoleUser;
	}

	const auto pw = getpwuid(getuid());
	if (pw)
	{
		return QString::fromUtf8(pw->pw_name);
	}

	return {};
}



bool MacOSUserFunctions::prepareLogon( const QString& username, const Password& password )
{
	return m_logonHelper.prepareLogon(username, password);
}



bool MacOSUserFunctions::performLogon( const QString& username, const Password& password )
{
	return m_logonHelper.performLogon(username, password);
}



void MacOSUserFunctions::logoff()
{
	QProcess::startDetached(QStringLiteral("/usr/bin/osascript"),
		{ QStringLiteral("-e"),
		  QStringLiteral("tell application \"System Events\" to log out") });
}



bool MacOSUserFunctions::authenticate( const QString& username, const Password& password )
{
	QProcess authProcess;
	authProcess.start(QStringLiteral("/usr/bin/dscl"),
		{ QStringLiteral("/Local/Default"), QStringLiteral("-authonly"),
		  username, QString::fromUtf8(password.toByteArray()) });

	return authProcess.waitForFinished(10000) && authProcess.exitCode() == 0;
}



uid_t MacOSUserFunctions::userIdFromName( const QString& username )
{
	const auto pw_entry = getpwnam(username.toUtf8().constData());

	if (pw_entry)
	{
		return pw_entry->pw_uid;
	}

	return static_cast<uid_t>(-1);
}
