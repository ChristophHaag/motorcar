#include "depthcompositedsurfacenode.h"
#include "../display/display.h"

using namespace motorcar;


//static const GLfloat defaultTextureCoordinates[] = {
//    0, 1,
//    1, 1,
//    1, 0,
//    0, 0,
//};

DepthCompositedSurfaceNode::DepthCompositedSurfaceNode(WaylandSurface *surface, SceneGraphNode *parent, const glm::mat4 &transform)
    :WaylandSurfaceNode(surface, parent, transform)
    ,m_depthCompositedSurfaceShader(new motorcar::OpenGLShader(std::string("../motorcar/src/shaders/depthcompositedsurface.vert"), std::string("../motorcar/src/shaders/depthcompositedsurface.frag")))
{
//    static const GLfloat vertexCoordinates[] ={
//        -1, -1, 0,
//        -1, 3, 0,
//        1, 3, 0,
//        1, -1, 0
//    };

    const GLfloat vertexCoordinates[] ={
       -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
       -1.0f,  1.0f, 0.0f
    };



    glGenBuffers(1, &m_colorTextureCoordinates);
    glGenBuffers(1, &m_depthTextureCoordinates);
//    glBindBuffer(GL_ARRAY_BUFFER, m_surfaceTextureCoordinates);
//    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), textureCoordinates, GL_STATIC_DRAW);

    glGenBuffers(1, &m_surfaceVertexCoordinates);
    glBindBuffer(GL_ARRAY_BUFFER, m_surfaceVertexCoordinates);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertexCoordinates, GL_STATIC_DRAW);



    h_aPosition =  glGetAttribLocation(m_depthCompositedSurfaceShader->handle(), "aPosition");
    h_aColorTexCoord =  glGetAttribLocation(m_depthCompositedSurfaceShader->handle(), "aColorTexCoord");
    h_aDepthTexCoord =  glGetAttribLocation(m_depthCompositedSurfaceShader->handle(), "aDepthTexCoord");

    if(h_aPosition < 0 || h_aColorTexCoord < 0 || h_aDepthTexCoord < 0 ){
       std::cout << "problem with surface shader handles: " << h_aPosition << ", "<< h_aColorTexCoord << ", " << h_aDepthTexCoord << std::endl;
    }



}


Geometry::RaySurfaceIntersection *DepthCompositedSurfaceNode::intersectWithSurfaces(const Geometry::Ray &ray)
{
    return SceneGraphNode::intersectWithSurfaces(ray);
}



void DepthCompositedSurfaceNode::drawFrameBufferContents(Display *display)
{


    //glBindTexture(GL_TEXTURE_2D, m_colorBufferTexture);


    glBindFramebuffer(GL_READ_FRAMEBUFFER, display->scratchFrameBuffer());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, display->activeFrameBuffer());

    glm::ivec2 res = display->size();
    glBlitFramebuffer(0, 0, res.x - 1, res.y - 1, 0, 0, res.x - 1 , res.y - 1, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    //only for default display
    //glGetError();


    //std::cout << glGetError() <<std::endl <<std::endl;



}

void DepthCompositedSurfaceNode::draw(Scene *scene, Display *display)
{




    GLuint texture = this->surface()->texture();


    glUseProgram(m_depthCompositedSurfaceShader->handle());

    glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, m_surfaceVertexCoordinates);
    glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(h_aColorTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(h_aDepthTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, display->scratchFrameBuffer());

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    glBindTexture(GL_TEXTURE_2D, texture);


    glm::vec4 vp;
    for(ViewPoint *viewpoint : display->viewpoints()){

        viewpoint->viewport()->set();

        vp = viewpoint->clientColorViewport()->viewportParams();

        const GLfloat clientColorTextureCoordinates[] = {
            vp.x, 1 - vp.y,
            vp.x + vp.z, 1 - vp.y,
            vp.x + vp.z, 1 - (vp.y + vp.w),
            vp.x, 1 - (vp.y + vp.w),
        };



//        glEnableVertexAttribArray(h_aColorTexCoord);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindBuffer(GL_ARRAY_BUFFER, m_colorTextureCoordinates);
//        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), clientColorTextureCoordinates, GL_STATIC_DRAW);
//        glVertexAttribPointer(h_aColorTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(h_aColorTexCoord, 2, GL_FLOAT, GL_FALSE, 0, clientColorTextureCoordinates);

        vp = viewpoint->clientDepthViewport()->viewportParams();

        const GLfloat clientDepthTextureCoordinates[] = {
            vp.x, 1 - vp.y,
            vp.x + vp.z, 1 - vp.y,
            vp.x + vp.z, 1 - (vp.y + vp.w),
            vp.x, 1 - (vp.y + vp.w),
        };


//        glEnableVertexAttribArray(h_aDepthTexCoord);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindBuffer(GL_ARRAY_BUFFER, m_depthTextureCoordinates);
//        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), clientDepthTextureCoordinates, GL_STATIC_DRAW);
//        glVertexAttribPointer(h_aDepthTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(h_aDepthTexCoord, 2, GL_FLOAT, GL_FALSE, 0, clientDepthTextureCoordinates);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

//        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
//        glDrawPixels(viewpoint->clientDepthViewport()->size().x, viewpoint->clientDepthViewport()->size().y, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, texture);
//        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }


    this->drawFrameBufferContents(display);



    glBindFramebuffer(GL_FRAMEBUFFER, display->activeFrameBuffer());

    glDisableVertexAttribArray(h_aPosition);
    glDisableVertexAttribArray(h_aColorTexCoord);

    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);


}













