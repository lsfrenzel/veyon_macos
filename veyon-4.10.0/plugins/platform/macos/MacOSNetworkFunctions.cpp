/*
 * MacOSNetworkFunctions.cpp - implementation of MacOSNetworkFunctions class
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

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <QProcess>
#include <QRegularExpression>

#include "MacOSNetworkFunctions.h"
#include "ProcessHelper.h"

MacOSNetworkFunctions::PingResult MacOSNetworkFunctions::ping(const QString& hostAddress)
{
	QProcess pingProcess;
	pingProcess.start(QStringLiteral("/sbin/ping"),
		{ QStringLiteral("-c"), QStringLiteral("1"),
		  QStringLiteral("-t"), QString::number(PingTimeout / 1000),
		  hostAddress });

	if (pingProcess.waitForFinished(PingProcessTimeout))
	{
		switch (pingProcess.exitCode())
		{
		case 0: return PingResult::ReplyReceived;
		case 2: return PingResult::TimedOut;
		case 68: return PingResult::NameResolutionFailed;
		default:
			break;
		}
	}

	return PingResult::Unknown;
}



bool MacOSNetworkFunctions::configureFirewallException( const QString& applicationPath, const QString& description, bool enabled )
{
	Q_UNUSED(applicationPath)
	Q_UNUSED(description)
	Q_UNUSED(enabled)

	return true;
}



bool MacOSNetworkFunctions::configureSocketKeepalive( Socket socket, bool enabled, int idleTime, int interval, int probes )
{
	int optval;
	socklen_t optlen = sizeof(optval);
	auto fd = static_cast<int>(socket);

	optval = enabled ? 1 : 0;
	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0)
	{
		vWarning() << "could not set SO_KEEPALIVE";
		return false;
	}

	optval = std::max<int>(1, idleTime / 1000);
	if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &optval, optlen) < 0)
	{
		vWarning() << "could not set TCP_KEEPALIVE";
		return false;
	}

	optval = std::max<int>(1, interval / 1000);
	if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &optval, optlen) < 0)
	{
		vWarning() << "could not set TCP_KEEPINTVL";
		return false;
	}

	optval = probes;
	if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &optval, optlen) < 0)
	{
		vWarning() << "could not set TCP_KEEPCNT";
		return false;
	}

	return true;
}



QNetworkInterface MacOSNetworkFunctions::defaultRouteNetworkInterface()
{
	QProcess routeProcess;
	routeProcess.start(QStringLiteral("/sbin/route"),
		{ QStringLiteral("-n"), QStringLiteral("get"), QStringLiteral("default") });

	if (routeProcess.waitForFinished(5000))
	{
		const auto output = QString::fromUtf8(routeProcess.readAllStandardOutput());
		const QRegularExpression interfaceRegex(QStringLiteral("interface:\\s*(\\S+)"));
		const auto match = interfaceRegex.match(output);

		if (match.hasMatch())
		{
			return QNetworkInterface::interfaceFromName(match.captured(1));
		}
	}

	const auto interfaces = QNetworkInterface::allInterfaces();
	for (const auto& iface : interfaces)
	{
		if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
			iface.flags().testFlag(QNetworkInterface::IsRunning) &&
			!iface.flags().testFlag(QNetworkInterface::IsLoopBack))
		{
			return iface;
		}
	}

	return {};
}



int MacOSNetworkFunctions::networkInterfaceSpeedInMBitPerSecond(const QNetworkInterface& networkInterface)
{
	if (!networkInterface.isValid())
	{
		return 0;
	}

	QProcess networksetupProcess;
	networksetupProcess.start(QStringLiteral("/usr/sbin/networksetup"),
		{ QStringLiteral("-getMedia"), networkInterface.humanReadableName() });

	if (networksetupProcess.waitForFinished(5000))
	{
		const auto output = QString::fromUtf8(networksetupProcess.readAllStandardOutput());
		const QRegularExpression speedRegex(QStringLiteral("(\\d+)baseT"));
		const auto match = speedRegex.match(output);

		if (match.hasMatch())
		{
			return match.captured(1).toInt();
		}
	}

	const auto name = networkInterface.name();
	if (name.startsWith(QStringLiteral("en")) &&
		networkInterface.type() == QNetworkInterface::Wifi)
	{
		QProcess airportProcess;
		airportProcess.start(QStringLiteral("/usr/sbin/system_profiler"),
			{ QStringLiteral("SPAirPortDataType") });

		if (airportProcess.waitForFinished(5000))
		{
			const auto output = QString::fromUtf8(airportProcess.readAllStandardOutput());
			const QRegularExpression txRateRegex(QStringLiteral("Transmit Rate:\\s*(\\d+)"));
			const auto match = txRateRegex.match(output);
			if (match.hasMatch())
			{
				return match.captured(1).toInt();
			}
		}
	}

	return 0;
}
