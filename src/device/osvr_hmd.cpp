/****************************************************************************
**This file is part of the Motorcar 3D windowing framework
**
**
**Copyright (C) 2015 Forrest Reiling
**
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/
#include "osvr_hmd.h"
#include <GL/glx.h>

using namespace motorcar;

void OsvrHMD::prepareForDraw()
{

	OSVRDisplay::prepareForDraw();


    osvrcontext.update();
    OSVR_PoseState state;
    OSVR_TimeValue timestamp;
    OSVR_ReturnCode ret = osvrGetPoseState(head.get(), &timestamp, &state);
    if (OSVR_RETURN_SUCCESS != ret) {
        std::cout << "No pose state!" << std::endl;
        return;
    } else {
    	/*
        std::cout << "Got POSE state: Position = ("
                  << state.translation.data[0] << ", "
                  << state.translation.data[1] << ", "
                  << state.translation.data[2] << "), orientation = ("
                  << osvrQuatGetW(&(state.rotation)) << ", ("
                  << osvrQuatGetX(&(state.rotation)) << ", "
                  << osvrQuatGetY(&(state.rotation)) << ", "
                  << osvrQuatGetZ(&(state.rotation)) << ")"
                  << std::endl;
                  */
    }

	glm::vec3 position = glm::vec3(state.translation.data[0], state.translation.data[1], state.translation.data[2]);
    glm::quat orientation;
    orientation.x = osvrQuatGetX(&(state.rotation));
    orientation.y = osvrQuatGetY(&(state.rotation));
    orientation.z = osvrQuatGetZ(&(state.rotation));
    orientation.w = osvrQuatGetW(&(state.rotation));
    glm::mat4 transform = glm::translate(glm::mat4(), position) * glm::mat4_cast(orientation);
    this->setTransform(transform);

}


void OsvrHMD::finishDraw()
{
	OSVRDisplay::finishDraw();

	m_frameIndex++;

}


// Callback to set up for rendering into a given eye (viewpoint and projection).
void SetupEye(
    void* userData //< Passed into SetViewProjectionCallback
    , osvr::renderkit::GraphicsLibrary library //< Graphics library context to use
    , osvr::renderkit::RenderBuffer buffers //< Buffers to use
    , osvr::renderkit::OSVR_ViewportDescription
        viewport //< Viewport set by RenderManager
    , osvr::renderkit::OSVR_ProjectionMatrix
        projection //< Projection matrix set by RenderManager
    , size_t whichEye //< Which eye are we setting up for?
    ) {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr
            << "SetupEye: No OpenGL GraphicsLibrary, this should not happen"
            << std::endl;
        return;
    }
    if (buffers.OpenGL == nullptr) {
        std::cerr << "SetupEye: No OpenGL RenderBuffer, this should not happen"
                  << std::endl;
        return;
    }

    // Set the viewport
    glViewport(static_cast<GLint>(viewport.left),
      static_cast<GLint>(viewport.lower),
      static_cast<GLint>(viewport.width),
      static_cast<GLint>(viewport.height));
}

// Callback to set up a given display, which may have one or more eyes in it
void SetupDisplay(
    void* userData //< Passed into SetDisplayCallback
    , osvr::renderkit::GraphicsLibrary library //< Graphics library context to use
    , osvr::renderkit::RenderBuffer buffers //< Buffers to use
    ) {
    // Make sure our pointers are filled in correctly.  The config file selects
    // the graphics library to use, and may not match our needs.
    if (library.OpenGL == nullptr) {
        std::cerr
            << "SetupDisplay: No OpenGL GraphicsLibrary, this should not happen"
            << std::endl;
        return;
    }
    if (buffers.OpenGL == nullptr) {
        std::cerr
            << "SetupDisplay: No OpenGL RenderBuffer, this should not happen"
            << std::endl;
        return;
    }

    osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

    // Clear the screen to black and clear depth
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool SetupRendering(osvr::renderkit::GraphicsLibrary library) {
    // Make sure our pointers are filled in correctly.
    if (library.OpenGL == nullptr) {
        std::cerr << "SetupRendering: No OpenGL GraphicsLibrary, this should "
                     "not happen"
                  << std::endl;
        return false;
    }

    osvr::renderkit::GraphicsLibraryOpenGL* glLibrary = library.OpenGL;

    // Turn on depth testing, so we get correct ordering.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    return true;
}

OsvrHMD::OsvrHMD(Skeleton *skeleton, OpenGLContext *glContext, PhysicalNode *parent)
    : OSVRDisplay(glContext, glm::vec2(0.126, 0.0706), parent, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.10f)))
    , m_initialized(true)
    , m_frameIndex(0)
	, osvrcontext("com.motorcar")
{
    

	head = osvrcontext.getInterface("/me/head");

    //TODO: Proper condition
	if (head.notEmpty()) {
		printf("detected %d hmds\n", 1);
	} else {
		printf("detected %d hmds\n", 0);
		return;
	}

	osvr::clientkit::DisplayConfig display;
	do {
		std::cout << "Trying to get the display config" << std::endl;
		osvrcontext.update();
		display = osvr::clientkit::DisplayConfig(osvrcontext);
	} while (!display.valid());

	/*
    display.forEachViewer([](osvr::clientkit::Viewer viewer){
        std::cout << "Viewer " << viewer.getViewerID() << "\n";
        viewer.forEachEye([](osvr::clientkit::Eye eye) {
            std::cout << "\tEye " << int(eye.getEyeID()) << "\n";
            eye.forEachSurface([](osvr::clientkit::Surface surface) {
                std::cout << "\t\tSurface " << surface.getSurfaceID() << "\n";
            });
        });
    });
    */

    osvr::clientkit::DisplayDimensions dimensions;
    dimensions = display.getDisplayDimensions(0);
    //std::cout << "Display: " << dimensions.width << "x" << dimensions.height << std::endl;

	int win_width = dimensions.width, win_height = dimensions.height;
	printf("Reported hmd size: %d, %d. Default Framebuffer size: %d, %d\n",win_width, win_height, glContext->defaultFramebufferSize().x, glContext->defaultFramebufferSize().y );

	setRenderTargetSize(glm::ivec2(win_width, win_height));

	float near = .01f, far = 10.0f;

	glm::vec4 normalizedViewportParams = glm::vec4( 0,
													0,
													win_width,
													win_height);

	glm::vec3 HmdToEyeViewOffset = -1.0f * glm::vec3(1,
												1,
												1);
    ViewPoint *vp = new ViewPoint(near, far, this, this, glm::translate(glm::mat4(), HmdToEyeViewOffset), normalizedViewportParams, glm::vec3(0));
	addViewpoint(vp);

	auto oglContext = glXGetCurrentContext();
	auto drawable = glXGetCurrentDrawable();

    // Open OpenGL and set up the context for rendering to
    // an HMD.  Do this using the OSVR RenderManager interface,
    // which maps to the nVidia or other vendor direct mode
    // to reduce the latency.
    //  As of the 7/13/2016 version of RenderManager, we don't
    // need to explicitly ask it to share the OpenGL context with
    // us -- it does so by default.
    osvr::renderkit::RenderManager* render =
        osvr::renderkit::createRenderManager(osvrcontext.get(), "OpenGL");

    if ((render == nullptr) || (!render->doingOkay())) {
        std::cerr << "Could not create RenderManager" << std::endl;
        return;
    }

    // Set callback to handle setting up rendering in an eye
    render->SetViewProjectionCallback(SetupEye);

    // Set callback to handle setting up rendering in a display
    render->SetDisplayCallback(SetupDisplay);


    osvr::renderkit::RenderManager::OpenResults ret = render->OpenDisplay();
    if (ret.status == osvr::renderkit::RenderManager::OpenStatus::FAILURE) {
        std::cerr << "Could not open display" << std::endl;
        delete render;
        return;
    }
    if (ret.library.OpenGL == nullptr) {
        std::cerr << "Attempted to run an OpenGL program with a config file "
                  << "that specified a different rendering library."
                  << std::endl;
        return;
    }

    // Set up the rendering state we need.
    if (!SetupRendering(ret.library)) {
    	std::cerr << "Setup Rendering failed" << std::endl;
        return;
    }

    glXMakeCurrent(glXGetCurrentDisplay(), drawable, oglContext);
}

OsvrHMD::~OsvrHMD()
{
}

