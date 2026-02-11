/*
 * MacOSServiceCore.h - declaration of MacOSServiceCore class
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

#pragma once

#include <QObject>
#include <QMap>

#include "PlatformSessionManager.h"
#include "ServiceDataManager.h"

class MacOSServerProcess;

class MacOSServiceCore : public QObject
{
	Q_OBJECT
public:
	explicit MacOSServiceCore( QObject* parent = nullptr );
	~MacOSServiceCore() override;

	void run();

private:
	static constexpr auto SessionPollingInterval = 5000;
	static constexpr auto ServerRestartInterval = 5000;

	void checkSessions();
	void startServerForUser( const QString& username );
	void stopServerForUser( const QString& username );
	void stopAllServers();

	QMap<QString, MacOSServerProcess *> m_serverProcesses;

	ServiceDataManager m_dataManager{};
	PlatformSessionManager m_sessionManager{};
};
