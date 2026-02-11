/*
 * MacOSSessionFunctions.cpp - implementation of MacOSSessionFunctions class
 *
 * Copyright (c) 2020-2026 Tobias Junghans <tobydox@veyon.io>
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

#include <QHostInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSettings>

#include <SystemConfiguration/SystemConfiguration.h>
#include <unistd.h>
#include <sys/sysctl.h>

#include "MacOSSessionFunctions.h"


PlatformSessionFunctions::SessionId MacOSSessionFunctions::currentSessionId()
{
	CFStringRef consoleUser = SCDynamicStoreCopyConsoleUser(nullptr, nullptr, nullptr);
	if (consoleUser)
	{
		CFRelease(consoleUser);
		return static_cast<SessionId>(getuid());
	}

	return DefaultSessionId;
}



PlatformSessionFunctions::SessionUptime MacOSSessionFunctions::currentSessionUptime() const
{
	struct timeval bootTime{};
	size_t size = sizeof(bootTime);
	int mib[2] = { CTL_KERN, KERN_BOOTTIME };

	if (sysctl(mib, 2, &bootTime, &size, nullptr, 0) == 0)
	{
		time_t now = time(nullptr);
		return static_cast<SessionUptime>(now - bootTime.tv_sec);
	}

	return InvalidSessionUptime;
}



QString MacOSSessionFunctions::currentSessionClientAddress() const
{
	return {};
}



QString MacOSSessionFunctions::currentSessionClientName() const
{
	return {};
}



QString MacOSSessionFunctions::currentSessionHostName() const
{
	return QHostInfo::localHostName();
}



QString MacOSSessionFunctions::currentSessionType() const
{
	return QStringLiteral("Aqua");
}



bool MacOSSessionFunctions::currentSessionHasUser() const
{
	return !currentConsoleUser().isEmpty();
}



PlatformSessionFunctions::EnvironmentVariables MacOSSessionFunctions::currentSessionEnvironmentVariables() const
{
	EnvironmentVariables envVars;
	const auto sysEnv = QProcessEnvironment::systemEnvironment();
	const auto keys = sysEnv.keys();

	for (const auto& key : keys)
	{
		envVars[key] = sysEnv.value(key);
	}

	return envVars;
}



QVariant MacOSSessionFunctions::querySettingsValueInCurrentSession(const QString& key) const
{
	QSettings settings(QStringLiteral("io.veyon"), QStringLiteral("Veyon"));
	return settings.value(key);
}



QString MacOSSessionFunctions::currentConsoleUser()
{
	uid_t uid = 0;
	gid_t gid = 0;
	CFStringRef consoleUser = SCDynamicStoreCopyConsoleUser(nullptr, &uid, &gid);

	if (consoleUser)
	{
		const auto length = CFStringGetLength(consoleUser);
		const auto maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
		QByteArray buffer(static_cast<int>(maxSize), 0);
		CFStringGetCString(consoleUser, buffer.data(), maxSize, kCFStringEncodingUTF8);
		CFRelease(consoleUser);

		const auto username = QString::fromUtf8(buffer.constData());
		if (username != QStringLiteral("loginwindow"))
		{
			return username;
		}
	}

	return {};
}
