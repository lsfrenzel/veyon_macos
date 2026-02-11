/*
 * MacOSPlatformPlugin.h - declaration of MacOSPlatformPlugin class
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

#include "ConfigurationPagePluginInterface.h"
#include "PluginInterface.h"
#include "PlatformPluginInterface.h"
#include "MacOSCoreFunctions.h"
#include "MacOSFilesystemFunctions.h"
#include "MacOSInputDeviceFunctions.h"
#include "MacOSNetworkFunctions.h"
#include "MacOSServiceFunctions.h"
#include "MacOSSessionFunctions.h"
#include "MacOSUserFunctions.h"

class MacOSPlatformPlugin : public QObject, PlatformPluginInterface, PluginInterface, ConfigurationPagePluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.MacOSPlatform")
	Q_INTERFACES(PluginInterface PlatformPluginInterface ConfigurationPagePluginInterface)
public:
	explicit MacOSPlatformPlugin( QObject* parent = nullptr );
	~MacOSPlatformPlugin() override;

	Plugin::Uid uid() const override
	{
		return Plugin::Uid{ QStringLiteral("a2d78c61-5f03-4c8b-b548-7d89f2e3a7b1") };
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 0 );
	}

	QString name() const override
	{
		return QStringLiteral( "MacOSPlatformPlugin" );
	}

	QString description() const override
	{
		return tr( "Plugin implementing abstract functions for the macOS platform" );
	}

	QString vendor() const override
	{
		return QStringLiteral( "Veyon Community" );
	}

	QString copyright() const override
	{
		return QStringLiteral( "Tobias Junghans" );
	}

	Plugin::Flags flags() const override
	{
		return Plugin::ProvidesDefaultImplementation;
	}

	PlatformCoreFunctions& coreFunctions() override
	{
		return m_macOSCoreFunctions;
	}

	PlatformFilesystemFunctions& filesystemFunctions() override
	{
		return m_macOSFilesystemFunctions;
	}

	PlatformInputDeviceFunctions& inputDeviceFunctions() override
	{
		return m_macOSInputDeviceFunctions;
	}

	PlatformNetworkFunctions& networkFunctions() override
	{
		return m_macOSNetworkFunctions;
	}

	PlatformServiceFunctions& serviceFunctions() override
	{
		return m_macOSServiceFunctions;
	}

	PlatformSessionFunctions& sessionFunctions() override
	{
		return m_macOSSessionFunctions;
	}

	PlatformUserFunctions& userFunctions() override
	{
		return m_macOSUserFunctions;
	}

	ConfigurationPage* createConfigurationPage() override;

	static void abort(int signal);

private:
	MacOSCoreFunctions m_macOSCoreFunctions{};
	MacOSFilesystemFunctions m_macOSFilesystemFunctions{};
	MacOSInputDeviceFunctions m_macOSInputDeviceFunctions{};
	MacOSNetworkFunctions m_macOSNetworkFunctions{};
	MacOSServiceFunctions m_macOSServiceFunctions{};
	MacOSSessionFunctions m_macOSSessionFunctions{};
	MacOSUserFunctions m_macOSUserFunctions{};
};
