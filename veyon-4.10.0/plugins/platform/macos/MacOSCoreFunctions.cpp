/*
 * MacOSCoreFunctions.cpp - implementation of MacOSCoreFunctions class
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

#include <QDir>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>
#include <QThread>
#include <QWidget>

#include <csignal>
#include <unistd.h>
#include <sys/sysctl.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <ApplicationServices/ApplicationServices.h>

#include "MacOSCoreFunctions.h"
#include "PlatformServiceFunctions.h"
#include "VeyonConfiguration.h"


bool MacOSCoreFunctions::applyConfiguration()
{
	return true;
}



void MacOSCoreFunctions::initNativeLoggingSystem( const QString& appName )
{
	Q_UNUSED(appName)
}



void MacOSCoreFunctions::writeToNativeLoggingSystem( const QString& message, Logger::LogLevel loglevel )
{
	Q_UNUSED(loglevel)
	CFStringRef cfMessage = CFStringCreateWithCString(kCFAllocatorDefault,
		message.toUtf8().constData(), kCFStringEncodingUTF8);
	if (cfMessage)
	{
		CFShow(cfMessage);
		CFRelease(cfMessage);
	}
}



void MacOSCoreFunctions::reboot()
{
	QProcess::startDetached(QStringLiteral("/usr/bin/osascript"),
		{ QStringLiteral("-e"), QStringLiteral("tell application \"System Events\" to restart") });
}



void MacOSCoreFunctions::powerDown( bool installUpdates )
{
	Q_UNUSED(installUpdates)
	QProcess::startDetached(QStringLiteral("/usr/bin/osascript"),
		{ QStringLiteral("-e"), QStringLiteral("tell application \"System Events\" to shut down") });
}



void MacOSCoreFunctions::raiseWindow( QWidget* widget, bool stayOnTop )
{
	if (!widget)
	{
		return;
	}

	widget->activateWindow();
	widget->raise();

	if (stayOnTop)
	{
		widget->setWindowFlags(widget->windowFlags() | Qt::WindowStaysOnTopHint);
		widget->show();
	}
}



void MacOSCoreFunctions::disableScreenSaver()
{
	QProcess::execute(QStringLiteral("/usr/bin/defaults"),
		{ QStringLiteral("-currentHost"), QStringLiteral("write"),
		  QStringLiteral("com.apple.screensaver"), QStringLiteral("idleTime"),
		  QStringLiteral("-int"), QStringLiteral("0") });

	QProcess::execute(QStringLiteral("/usr/bin/caffeinate"), { QStringLiteral("-d") });

	m_screenSaverWasEnabled = true;
}



void MacOSCoreFunctions::restoreScreenSaverSettings()
{
	if (m_screenSaverWasEnabled)
	{
		QProcess::execute(QStringLiteral("/usr/bin/defaults"),
			{ QStringLiteral("-currentHost"), QStringLiteral("write"),
			  QStringLiteral("com.apple.screensaver"), QStringLiteral("idleTime"),
			  QStringLiteral("-int"), QStringLiteral("300") });
		m_screenSaverWasEnabled = false;
	}
}



void MacOSCoreFunctions::setSystemUiState( bool enabled )
{
	Q_UNUSED(enabled)
}



QString MacOSCoreFunctions::activeDesktopName()
{
	return QStringLiteral("default");
}



bool MacOSCoreFunctions::isRunningAsAdmin() const
{
	return getuid() == 0 || geteuid() == 0;
}



bool MacOSCoreFunctions::runProgramAsAdmin( const QString& program, const QStringList& parameters )
{
	const auto parameterString = parameters.join(QLatin1Char(' '));
	const auto command = QStringLiteral("do shell script \"%1 %2\" with administrator privileges")
		.arg(program, parameterString);

	return QProcess::execute(QStringLiteral("/usr/bin/osascript"),
		{ QStringLiteral("-e"), command }) == 0;
}



bool MacOSCoreFunctions::runProgramAsUser( const QString& program,
	const QStringList& parameters,
	const QString& username,
	const QString& desktop )
{
	Q_UNUSED(desktop)

	QStringList args;
	args << QStringLiteral("-u") << username << QStringLiteral("--") << program;
	args.append(parameters);

	return QProcess::startDetached(QStringLiteral("/usr/bin/sudo"), args);
}



QString MacOSCoreFunctions::genericUrlHandler() const
{
	return QStringLiteral("/usr/bin/open");
}



QString MacOSCoreFunctions::queryDisplayDeviceName(const QScreen& screen) const
{
	return screen.name();
}



QString MacOSCoreFunctions::getApplicationName(ProcessId processId) const
{
	if (processId <= 0)
	{
		return {};
	}

	struct kinfo_proc info{};
	size_t length = sizeof(info);
	int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(processId) };

	if (sysctl(mib, 4, &info, &length, nullptr, 0) == 0 && length > 0)
	{
		return QString::fromUtf8(info.kp_proc.p_comm);
	}

	return {};
}



bool MacOSCoreFunctions::waitForProcess( qint64 pid, int timeout, int sleepInterval )
{
	QElapsedTimer timeoutTimer;
	timeoutTimer.start();

	while (kill(static_cast<pid_t>(pid), 0) == 0)
	{
		if (timeoutTimer.elapsed() >= timeout)
		{
			return false;
		}

		QThread::msleep(static_cast<uint64_t>(sleepInterval));
	}

	return true;
}



int MacOSCoreFunctions::launchctl( const QStringList& arguments )
{
	return QProcess::execute(QStringLiteral("/bin/launchctl"), arguments);
}
