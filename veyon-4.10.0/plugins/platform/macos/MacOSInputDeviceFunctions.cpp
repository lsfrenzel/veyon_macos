/*
 * MacOSInputDeviceFunctions.cpp - implementation of MacOSInputDeviceFunctions class
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

#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/hidsystem/IOHIDLib.h>

#include "MacOSInputDeviceFunctions.h"
#include "MacOSKeyboardShortcutTrapper.h"
#include "PlatformServiceFunctions.h"


void MacOSInputDeviceFunctions::enableInputDevices()
{
	if (m_inputDevicesDisabled)
	{
		CGAssociateMouseAndMouseCursorPosition(true);
		m_inputDevicesDisabled = false;
	}
}



void MacOSInputDeviceFunctions::disableInputDevices()
{
	if (m_inputDevicesDisabled == false)
	{
		CGAssociateMouseAndMouseCursorPosition(false);
		m_inputDevicesDisabled = true;
	}
}



KeyboardShortcutTrapper* MacOSInputDeviceFunctions::createKeyboardShortcutTrapper( QObject* parent )
{
	return new MacOSKeyboardShortcutTrapper(parent);
}
