//========================================================================
// GLFW 3.4 Win32 - www.opengl.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@opengl.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"

#if defined(GLFW_BUILD_WIN32_TIMER)

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformInitTimer(void)
{
    QueryPerformanceFrequency((LARGE_INTEGER*) &_glfw.timer.win32.frequency);
}

uint64_t _glfwPlatformGetTimerValue(void)
{
    uint64_t value;
    QueryPerformanceCounter((LARGE_INTEGER*) &value);
    return value;
}

uint64_t _glfwPlatformGetTimerFrequency(void)
{
    return _glfw.timer.win32.frequency;
}

#endif // GLFW_BUILD_WIN32_TIMER

