/*
 * MacOSFilesystemFunctions.cpp - implementation of MacOSFilesystemFunctions class
 *
 * Copyright (c) 2018-2026 Tobias Junghans <tobydox@veyon.io>
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
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <fcntl.h>
#include <grp.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "MacOSFilesystemFunctions.h"


QString MacOSFilesystemFunctions::personalAppDataPath() const
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
		+ QDir::separator() + QStringLiteral("veyon");
}



QString MacOSFilesystemFunctions::globalAppDataPath() const
{
	return QStringLiteral("/Library/Application Support/Veyon");
}



QString MacOSFilesystemFunctions::globalTempPath() const
{
	return QStringLiteral("/tmp");
}



QString MacOSFilesystemFunctions::fileOwnerGroup( const QString& filePath )
{
	return QFileInfo(filePath).group();
}



bool MacOSFilesystemFunctions::setFileOwnerGroup( const QString& filePath, const QString& ownerGroup )
{
	struct stat statBuffer{};
	if (stat(filePath.toUtf8().constData(), &statBuffer) != 0)
	{
		vCritical() << "failed to stat file" << filePath;
		return false;
	}

	const auto grp = getgrnam(ownerGroup.toUtf8().constData());
	if (grp == nullptr)
	{
		vCritical() << "failed to get gid for" << ownerGroup;
		return false;
	}

	const auto gid = grp->gr_gid;

	if (chown(filePath.toUtf8().constData(), statBuffer.st_uid, gid) != 0)
	{
		vCritical() << "failed to change owner group of file" << filePath;
		return false;
	}

	return true;
}



bool MacOSFilesystemFunctions::setFileOwnerGroupPermissions( const QString& filePath, QFile::Permissions permissions )
{
	QFile file(filePath);
	return file.setPermissions(permissions);
}



bool MacOSFilesystemFunctions::openFileSafely( QFile* file, QFile::OpenMode openMode, QFile::Permissions permissions )
{
	if (!file)
	{
		return false;
	}

	const auto filePath = file->fileName().toUtf8();

	mode_t fileMode = 0;
	if (permissions & QFile::ReadOwner) fileMode |= S_IRUSR;
	if (permissions & QFile::WriteOwner) fileMode |= S_IWUSR;
	if (permissions & QFile::ExeOwner) fileMode |= S_IXUSR;
	if (permissions & QFile::ReadGroup) fileMode |= S_IRGRP;
	if (permissions & QFile::WriteGroup) fileMode |= S_IWGRP;
	if (permissions & QFile::ExeGroup) fileMode |= S_IXGRP;
	if (permissions & QFile::ReadOther) fileMode |= S_IROTH;
	if (permissions & QFile::WriteOther) fileMode |= S_IWOTH;
	if (permissions & QFile::ExeOther) fileMode |= S_IXOTH;

	int flags = O_NOFOLLOW;
	if (openMode & QFile::ReadOnly) flags |= O_RDONLY;
	if (openMode & QFile::WriteOnly) flags |= O_WRONLY;
	if (openMode & QFile::ReadWrite) flags |= O_RDWR;
	if (openMode & QFile::Append) flags |= O_APPEND;
	if (openMode & QFile::Truncate) flags |= O_TRUNC;

	int fd = open(filePath.constData(), flags | O_CREAT, fileMode);
	if (fd < 0)
	{
		return false;
	}

	struct stat s{};
	if (fstat(fd, &s) != 0 || !S_ISREG(s.st_mode))
	{
		close(fd);
		return false;
	}

	if (s.st_uid != getuid())
	{
		close(fd);
		return false;
	}

	if (fileMode)
	{
		(void)fchmod(fd, fileMode);
	}

	return file->open(fd, openMode, QFileDevice::AutoCloseHandle);
}



PlatformCoreFunctions::ProcessId MacOSFilesystemFunctions::findFileLockingProcess(const QString& filePath) const
{
	QProcess lsofProcess;
	lsofProcess.start(QStringLiteral("/usr/sbin/lsof"),
		{ QStringLiteral("-t"), filePath });

	if (lsofProcess.waitForFinished(5000))
	{
		const auto output = lsofProcess.readAllStandardOutput().trimmed();
		if (!output.isEmpty())
		{
			bool ok = false;
			const auto pid = output.split('\n').first().toLongLong(&ok);
			if (ok)
			{
				return pid;
			}
		}
	}

	return PlatformCoreFunctions::InvalidProcessId;
}
