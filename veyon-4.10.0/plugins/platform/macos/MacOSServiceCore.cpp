/*
 * MacOSServiceCore.cpp - implementation of MacOSServiceCore class
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

#include <QEventLoop>
#include <QTimer>

#include "MacOSServerProcess.h"
#include "MacOSServiceCore.h"
#include "MacOSSessionFunctions.h"
#include "VeyonConfiguration.h"


MacOSServiceCore::MacOSServiceCore( QObject* parent ) :
	QObject( parent )
{
}



MacOSServiceCore::~MacOSServiceCore()
{
	stopAllServers();
}



void MacOSServiceCore::run()
{
	auto* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &MacOSServiceCore::checkSessions);
	timer->start(SessionPollingInterval);

	checkSessions();

	QEventLoop eventLoop;
	eventLoop.exec();
}



void MacOSServiceCore::checkSessions()
{
	const auto currentUser = MacOSSessionFunctions::currentConsoleUser();

	if (currentUser.isEmpty())
	{
		stopAllServers();
		return;
	}

	if (!m_serverProcesses.contains(currentUser))
	{
		startServerForUser(currentUser);
	}
	else
	{
		auto* serverProcess = m_serverProcesses.value(currentUser);
		if (serverProcess && serverProcess->state() == QProcess::NotRunning)
		{
			vInfo() << "Server crashed for user" << currentUser << "- restarting";
			QTimer::singleShot(ServerRestartInterval, serverProcess,
				[serverProcess]() { serverProcess->start(); });
		}
	}

	QStringList usersToRemove;
	for (auto it = m_serverProcesses.constBegin(); it != m_serverProcesses.constEnd(); ++it)
	{
		if (it.key() != currentUser)
		{
			usersToRemove.append(it.key());
		}
	}

	for (const auto& user : usersToRemove)
	{
		stopServerForUser(user);
	}
}



void MacOSServiceCore::startServerForUser( const QString& username )
{
	vInfo() << "Starting server for user" << username;

	auto* serverProcess = new MacOSServerProcess(username, this);
	m_serverProcesses[username] = serverProcess;
	serverProcess->start();

	m_sessionManager.openSession(username);
}



void MacOSServiceCore::stopServerForUser( const QString& username )
{
	if (!m_serverProcesses.contains(username))
	{
		return;
	}

	vInfo() << "Stopping server for user" << username;

	m_sessionManager.closeSession(username);

	auto* serverProcess = m_serverProcesses.take(username);
	serverProcess->stop();
	serverProcess->deleteLater();
}



void MacOSServiceCore::stopAllServers()
{
	while (!m_serverProcesses.isEmpty())
	{
		stopServerForUser(m_serverProcesses.firstKey());
	}
}
