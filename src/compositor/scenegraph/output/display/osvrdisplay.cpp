/****************************************************************************
**This file is part of the Motorcar 3D windowing framework
**
**
**Copyright (C) 2014 Forrest Reiling
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
#include <scenegraph/output/display/osvrdisplay.h>

using namespace motorcar;



OSVRDisplay::OSVRDisplay(OpenGLContext *glContext, glm::vec2 displayDimensions, PhysicalNode *parent, const glm::mat4 &transform)
	:Display(glContext, displayDimensions, parent, transform)

{

}

void OSVRDisplay::prepareForDraw() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer);
    m_renderingToTexture = true;
}

void OSVRDisplay::finishDraw() {
}

void OSVRDisplay::setRenderTargetSize(glm::ivec2 size){
    m_renderTargetSize = size;

    createOrUpdateFBO("RenderToTextureDisplay Primary Frame Buffer", m_frameBuffer,
                        m_colorBufferTexture, true,
                        m_depthBufferTexture, false, size);
    createOrUpdateFBO("Display Scratch Frame Buffer", m_scratchFrameBuffer,
                        m_scratchColorBufferTexture, true,
                        m_scratchDepthBufferTexture, true, size);
}


GLuint OSVRDisplay::activeFrameBuffer() const
{
    return m_frameBuffer;
}

GLuint OSVRDisplay::depthBufferTexture() const
{
    return m_depthBufferTexture;
}

OSVRDisplay::~OSVRDisplay()
{
    glDeleteTextures(1, &m_scratchColorBufferTexture);
    glDeleteTextures(1, &m_scratchDepthBufferTexture);
    glDeleteFramebuffers(1, &m_scratchFrameBuffer);
}




