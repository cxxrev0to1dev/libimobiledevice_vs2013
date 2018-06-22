/*
 * ideviceprovision.c
 * Simple utility to install, get, or remove provisioning profiles
 *   to/from idevices
 *
 * Copyright (c) 2012 Nikias Bassen, All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#pragma comment(lib,"libusbmuxd.lib")
#include <windows.h>
#define ssize_t size_t
#else
#include <arpa/inet.h>
#endif

#include <libusbmuxd/Multi_usbmuxd.h>

int main(int argc, char *argv[])
{
  usbmuxd::Multi_usbmuxd multi_usbmuxd;
	return 0;
}

