#pragma once
#ifndef _IMGUIX_CONFIG_NOTIFICATIONS_HPP_INCLUDED
#define _IMGUIX_CONFIG_NOTIFICATIONS_HPP_INCLUDED

/// \file notifications.hpp
/// \brief Default icons and colors for toast notifications.

#include "colors.hpp"

/// \brief Success notification icon (Material: check_circle).
#define IMGUIX_NOTIFY_ICON_SUCCESS u8"\uE876"

/// \brief Warning notification icon (Material: warning).
#define IMGUIX_NOTIFY_ICON_WARNING u8"\uE002"

/// \brief Error notification icon (Material: error).
#define IMGUIX_NOTIFY_ICON_ERROR   u8"\uE000"

/// \brief Information notification icon (Material: info).
#define IMGUIX_NOTIFY_ICON_INFO    u8"\uE88F"

/// \brief Dismiss button icon (Material: close).
#define IMGUIX_NOTIFY_ICON_CLOSE   u8"\uE5CD"

/// \brief Success toast color.
#define IMGUIX_NOTIFY_COLOR_SUCCESS IMGUIX_COLOR_SUCCESS

/// \brief Warning toast color.
#define IMGUIX_NOTIFY_COLOR_WARNING IMGUIX_COLOR_WARNING

/// \brief Error toast color.
#define IMGUIX_NOTIFY_COLOR_ERROR   IMGUIX_COLOR_ERROR

/// \brief Info toast color.
#define IMGUIX_NOTIFY_COLOR_INFO    IMGUIX_COLOR_INFO

/// \brief Default toast text color.
#define IMGUIX_NOTIFY_COLOR_DEFAULT ImVec4(1.0f, 1.0f, 1.0f, 1.0f)

#endif // _IMGUIX_CONFIG_NOTIFICATIONS_HPP_INCLUDED
