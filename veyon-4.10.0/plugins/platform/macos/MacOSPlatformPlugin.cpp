/*
 * MacOSPlatformPlugin.cpp - implementation of MacOSPlatformPlugin class
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

#include <csignal>
#include <execinfo.h>

#include "MacOSPlatformPlugin.h"
#include "MacOSPlatformConfiguration.h"
#include "MacOSPlatformConfigurationPage.h"

MacOSPlatformPlugin::MacOSPlatformPlugin( QObject* parent ) :
	QObject( parent )
{
	::signal(SIGPIPE, SIG_IGN);

	::signal(SIGBUS, abort);
	::signal(SIGSEGV, abort);
}



MacOSPlatformPlugin::~MacOSPlatformPlugin()
{
	m_macOSInputDeviceFunctions.enableInputDevices();
}



ConfigurationPage* MacOSPlatformPlugin::createConfigurationPage()
{
	return new MacOSPlatformConfigurationPage();
}



void MacOSPlatformPlugin::abort(int signal)
{
	vCritical() << "Received signal" << signal;

	static constexpr int BackTraceMaxDepth = 20;

	void* stackFrame[BackTraceMaxDepth + 1];
	const auto frameCount = backtrace(stackFrame, BackTraceMaxDepth + 1);

	char** humanReadableFrames = backtrace_symbols(stackFrame, frameCount);

	QStringList list{QLatin1String("BACKTRACE:")};
	list.reserve(frameCount);
	for (int i = 1; i < frameCount; i++)
	{
		list.append(QStringLiteral("\t %1").arg(QLatin1String(humanReadableFrames[i])));
	}

	free(humanReadableFrames);

	qCritical().noquote() << list.join(QLatin1String("\n"));

	qFatal("Aborting due to severe error");
	::abort();
}


IMPLEMENT_CONFIG_PROXY(MacOSPlatformConfiguration)
