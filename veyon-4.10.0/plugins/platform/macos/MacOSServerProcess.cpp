/*
 * MacOSServerProcess.cpp - implementation of MacOSServerProcess class
 *
 * Copyright (c) 2021-2026 Tobias Junghans <tobydox@veyon.io>
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

#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>

#include "Filesystem.h"
#include "MacOSCoreFunctions.h"
#include "MacOSServerProcess.h"
#include "VeyonConfiguration.h"


MacOSServerProcess::MacOSServerProcess( const QString& username, QObject* parent ) :
	QProcess( parent ),
	m_username( username )
{
}



MacOSServerProcess::~MacOSServerProcess()
{
	stop();
}



void MacOSServerProcess::start()
{
	if (VeyonCore::config().logToSystem())
	{
		setProcessChannelMode(QProcess::ForwardedChannels);
	}

	QProcess::start(VeyonCore::filesystem().serverFilePath(), QStringList{});
}



void MacOSServerProcess::stop()
{
	const auto pid = static_cast<pid_t>(processId());

	if (pid <= 0)
	{
		return;
	}

	setProcessState(QProcess::NotRunning);

	::kill(pid, SIGINT);

	if (MacOSCoreFunctions::waitForProcess(pid, ServerShutdownTimeout, ServerWaitSleepInterval) == false)
	{
		::kill(pid, SIGTERM);

		if (MacOSCoreFunctions::waitForProcess(pid, ServerTerminateTimeout, ServerWaitSleepInterval) == false)
		{
			vWarning() << "server for user" << m_username << "still running - killing now";
			::kill(pid, SIGKILL);
			MacOSCoreFunctions::waitForProcess(pid, ServerKillTimeout, ServerWaitSleepInterval);
		}
	}

	waitpid(pid, nullptr, WNOHANG);
}
