#pragma once
#ifndef _IMGUIX_PUBSUB_EVENT_LISTENER_HPP_INCLUDED
#define _IMGUIX_PUBSUB_EVENT_LISTENER_HPP_INCLUDED

/// \file EventListener.hpp
/// \brief Defines the EventListener class for receiving event notifications.
/// \ingroup Core

namespace ImGuiX::Pubsub {

    /// \class EventListener
    /// \brief Abstract base class for receiving event notifications.
    ///
    /// Derived classes implement event handlers and can receive events either as shared pointers
    /// or as raw pointers, depending on the dispatch method.
    class EventListener {
    public:
        /// \brief Destroy listener.
        virtual ~EventListener() = default;

        /// \brief Handles an event notification received as a raw pointer.
        /// \param event Raw pointer to the received event.
        virtual void onEvent([[maybe_unused]] const Event* event) {};
    };

} // namespace ImGuiX::Pubsub

#endif // _IMGUIX_PUBSUB_EVENT_LISTENER_HPP_INCLUDED
