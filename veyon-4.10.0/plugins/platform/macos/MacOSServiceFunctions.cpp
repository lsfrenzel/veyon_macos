/*
 * MacOSServiceFunctions.cpp - implementation of MacOSServiceFunctions class
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
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>

#include "MacOSCoreFunctions.h"
#include "MacOSServiceCore.h"
#include "MacOSServiceFunctions.h"


QString MacOSServiceFunctions::veyonServiceName() const
{
	return QStringLiteral("io.veyon.service");
}



bool MacOSServiceFunctions::isRegistered( const QString& name )
{
	return QFileInfo::exists(launchdPlistPath(name));
}



bool MacOSServiceFunctions::isRunning( const QString& name )
{
	QProcess listProcess;
	listProcess.start(QStringLiteral("/bin/launchctl"),
		{ QStringLiteral("list") });

	if (listProcess.waitForFinished(5000))
	{
		const auto output = QString::fromUtf8(listProcess.readAllStandardOutput());
		return output.contains(launchdLabel(name));
	}

	return false;
}



bool MacOSServiceFunctions::start( const QString& name )
{
	return MacOSCoreFunctions::launchctl(
		{ QStringLiteral("load"), launchdPlistPath(name) }) == 0;
}



bool MacOSServiceFunctions::stop( const QString& name )
{
	return MacOSCoreFunctions::launchctl(
		{ QStringLiteral("unload"), launchdPlistPath(name) }) == 0;
}



bool MacOSServiceFunctions::install( const QString& name, const QString& filePath,
	StartMode startMode, const QString& displayName )
{
	Q_UNUSED(displayName)

	const auto label = launchdLabel(name);
	const auto plistPath = launchdPlistPath(name);

	QFile plistFile(plistPath);
	if (!plistFile.open(QFile::WriteOnly | QFile::Truncate))
	{
		vCritical() << "Failed to create launchd plist at" << plistPath;
		return false;
	}

	QTextStream stream(&plistFile);
	stream << QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
		   << QStringLiteral("<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n")
		   << QStringLiteral("<plist version=\"1.0\">\n")
		   << QStringLiteral("<dict>\n")
		   << QStringLiteral("    <key>Label</key>\n")
		   << QStringLiteral("    <string>") << label << QStringLiteral("</string>\n")
		   << QStringLiteral("    <key>ProgramArguments</key>\n")
		   << QStringLiteral("    <array>\n")
		   << QStringLiteral("        <string>") << filePath << QStringLiteral("</string>\n")
		   << QStringLiteral("    </array>\n")
		   << QStringLiteral("    <key>RunAtLoad</key>\n")
		   << QStringLiteral("    <") << (startMode == StartMode::Auto ? QStringLiteral("true") : QStringLiteral("false")) << QStringLiteral("/>\n")
		   << QStringLiteral("    <key>KeepAlive</key>\n")
		   << QStringLiteral("    <true/>\n")
		   << QStringLiteral("    <key>StandardErrorPath</key>\n")
		   << QStringLiteral("    <string>/var/log/veyon-service.log</string>\n")
		   << QStringLiteral("    <key>StandardOutPath</key>\n")
		   << QStringLiteral("    <string>/var/log/veyon-service.log</string>\n")
		   << QStringLiteral("</dict>\n")
		   << QStringLiteral("</plist>\n");

	plistFile.close();

	if (startMode == StartMode::Auto)
	{
		return start(name);
	}

	return true;
}



bool MacOSServiceFunctions::uninstall( const QString& name )
{
	stop(name);

	const auto plistPath = launchdPlistPath(name);
	if (QFile::exists(plistPath))
	{
		return QFile::remove(plistPath);
	}

	return true;
}



bool MacOSServiceFunctions::setStartMode( const QString& name, PlatformServiceFunctions::StartMode startMode )
{
	Q_UNUSED(name)
	Q_UNUSED(startMode)

	vWarning() << "Changing start mode is not directly supported on macOS – please reinstall the service";
	return true;
}



bool MacOSServiceFunctions::runAsService( const QString& name, const ServiceEntryPoint& serviceEntryPoint )
{
	Q_UNUSED(name);

	serviceEntryPoint();

	return true;
}



void MacOSServiceFunctions::manageServerInstances()
{
	MacOSServiceCore serviceCore;
	serviceCore.run();
}



QString MacOSServiceFunctions::launchdPlistPath(const QString& name)
{
	return QStringLiteral("/Library/LaunchDaemons/%1.plist").arg(launchdLabel(name));
}



QString MacOSServiceFunctions::launchdLabel(const QString& name)
{
	if (name.contains(QLatin1Char('.')))
	{
		return name;
	}
	return QStringLiteral("io.veyon.%1").arg(name);
}
