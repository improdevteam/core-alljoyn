﻿//-----------------------------------------------------------------------
// <copyright file="App.xaml.cpp" company="AllSeen Alliance.">
//        Copyright (c) Open Connectivity Foundation (OCF), AllJoyn Open Source
//        Project (AJOSP) Contributors and others.
//        
//        SPDX-License-Identifier: Apache-2.0
//        
//        All rights reserved. This program and the accompanying materials are
//        made available under the terms of the Apache License, Version 2.0
//        which accompanies this distribution, and is available at
//        http://www.apache.org/licenses/LICENSE-2.0
//        
//        Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
//        Alliance. All rights reserved.
//        
//        Permission to use, copy, modify, and/or distribute this software for
//        any purpose with or without fee is hereby granted, provided that the
//        above copyright notice and this permission notice appear in all
//        copies.
//        
//        THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
//        WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
//        WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
//        AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
//        DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
//        PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
//        TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
//        PERFORMANCE OF THIS SOFTWARE.
// </copyright>
//-----------------------------------------------------------------------

#include "pch.h"
#include "MainPage.xaml.h"

using namespace BasicClient;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Application template is documented at http://go.microsoft.com/fwlink/?LinkId=234227

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App() : uiPage(nullptr)
{
    InitializeComponent();
    Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used when the application is launched to open a specific file, to display
/// search results, and so forth.
/// </summary>
/// <param name="pArgs">Details about the launch request and process.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs ^ pArgs)
{
    // Do not repeat app initialization when already running, just ensure that
    // the window is active
    if (pArgs->PreviousExecutionState == ApplicationExecutionState::Running) {
        Window::Current->Activate();
        return;
    }

    if (pArgs->PreviousExecutionState == ApplicationExecutionState::Terminated) {
        //TODO: Load state from previously suspended application
    }

    // Create a Frame to act navigation context and navigate to the first page
    auto rootFrame = ref new Frame();
    if (!rootFrame->Navigate(TypeName(MainPage::typeid ))) {
        throw ref new FailureException("Failed to create initial page");
    }

    // Place the frame in the current Window and ensure that it is active
    Window::Current->Content = rootFrame;
    Window::Current->Activate();
}

/// <summary>
/// Invoked when application execution is being suspended.  Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
/// <param name="sender">The source of the suspend request.</param>
/// <param name="e">Details about the suspend request.</param>
void App::OnSuspending(Object ^ sender, SuspendingEventArgs ^ e)
{
    (void) sender;      // Unused parameter
    (void) e;           // Unused parameter

    //TODO: Save application state and stop any background activity
}

void App::OutputLine(Platform::String ^ msg)
{
    uiPage->OutputLine(msg);
}