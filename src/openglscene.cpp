#include "openglscene.h"

#include "scene.h"
#include "objmodel.h"
#include "light.h"
#include <iostream>
#include <QDebug>
#include <QTimer>

// for point shadow
//const float near_plane = 1.0f;
//const float far_plane = 25.0f;

OpenGLScene::OpenGLScene(QWidget *parent) : QOpenGLWidget (parent)
{
    this->setFocusPolicy(Qt::StrongFocus);
    this->w = this->width();
    this->h = this->height();
    this->ka = QVector3D(1.0f, 1.0f, 1.0f);
    this->ks = QVector3D(1.0f, 1.0f, 1.0f);
    this->n = 1.0f;

    m_bLeftPressed = false;

    m_pTimer = new QTimer(this);
    connect(m_pTimer, &QTimer::timeout, this, [=]{
        m_nTimeValue += 1;
        update();
    });
    m_pTimer->start(40);//25 fps

    mode = Perspective;
    shadowMode = SHADOW_MAPPING;
}

OpenGLScene::~OpenGLScene(){
    makeCurrent();
    shadow_mapping_phong_shader.release();
    shadow_mapping_depth_shader.release();
    point_shadow_phong_shader.release();
    point_shadow_depth_shader.release();
    delete camera;
    foreach(PointLight* pLight, pointLightv){
        delete pLight;
    }
    doneCurrent();
}

void OpenGLScene::setMode(OpenGLScene::ProjectionMode mode)
{
    this->mode = mode;
    update();
}

void OpenGLScene::setShadowMode(OpenGLScene::ShadowMode mode)
{
    this->shadowMode = mode;
    update();
}

void OpenGLScene::setPhongParams(QVector3D ka, QVector3D ks, double n)
{
    this->ka = ka;
    this->ks = ks;
    this->n = n;
    update();
}

void OpenGLScene::setCameraParams(float nearPlane, float farPlane, float speed)
{
    this->camera->nearPlane = nearPlane;
    this->camera->farPlane = farPlane;
    this->camera->movementSpeed = speed;
    update();
}

void OpenGLScene::resetCameraPose()
{
    this->camera->position = CAMERA_DEFAULT_POSITION;
    this->camera->up = CAMERA_DEFAULT_UP;
    this->camera->yaw = CAMERA_DEFAULT_YAW;
    this->camera->pitch = CAMERA_DEFAULT_PITCH;
    this->camera->front = -CAMERA_DEFAULT_POSITION;
    this->camera->zoom = CAMERA_DEFAULT_ZOOM;
    update();
}

void OpenGLScene::setLight(Light *light)
{
    // for point shadow
//    if(light == nullptr)
//        return
//        if(light->m_type == Light::PointLight){
//            PointLight* pLight = (PointLight*)light;
//            pointLight->m_position = pLight->m_position;
//            pointLight->m_color = pLight->m_color;
//        }
//        else{ // convert parallel light to point light
//            ParallelLight* pLight = (ParallelLight*)light;
//            pointLight->m_color = pLight->m_color;
//            pointLight->m_position = -1.0e20f * pLight->m_direction;
//        }
//        QMatrix4x4 shadowProj;
//        shadowProj.perspective(90.0f, 1.0 * width() / height(), near_plane, far_plane);
//        QMatrix4x4 shadowView;
//        shadowView.lookAt(pointLight->m_position.toQVector3D(),
//                          QVector3D(1.0f, 0.0f, 0.0f),
//                          QVector3D(0.0f, -1.0f, 0.0f));
//        shadowTransforms.push_back(shadowProj * shadowView);
//        shadowView.setToIdentity();
//        shadowView.lookAt(pointLight->m_position.toQVector3D(),
//                          QVector3D(-1.0f, 0.0f, 0.0f),
//                          QVector3D(0.0f, -1.0f, 0.0f));
//        shadowTransforms.push_back(shadowProj * shadowView);
//        shadowView.setToIdentity();
//        shadowView.lookAt(pointLight->m_position.toQVector3D(),
//                          QVector3D(0.0f, 1.0f, 0.0f),
//                          QVector3D(0.0f, 0.0f, 1.0f));
//        shadowTransforms.push_back(shadowProj * shadowView);
//        shadowView.setToIdentity();
//        shadowView.lookAt(pointLight->m_position.toQVector3D(),
//                          QVector3D(0.0f, -1.0f, 0.0f),
//                          QVector3D(0.0f, 0.0f, -1.0f));
//        shadowTransforms.push_back(shadowProj * shadowView);
//        shadowView.setToIdentity();
//        shadowView.lookAt(pointLight->m_position.toQVector3D(),
//                          QVector3D(0.0f, 0.0f, 1.0f),
//                          QVector3D(0.0f, -1.0f, 0.0f));
//        shadowTransforms.push_back(shadowProj * shadowView);
//        shadowView.setToIdentity();
//        shadowView.lookAt(pointLight->m_position.toQVector3D(),
//                          QVector3D(0.0f, 0.0f, -1.0f),
//                          QVector3D(0.0f, -1.0f, 0.0f));
//        shadowTransforms.push_back(shadowProj * shadowView);
//    }
}

void OpenGLScene::initializeGL(){
    this->initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    createShader();

    size_t light_num = scene->lights.size();
    depthMapFBOv = std::vector<GLuint>(light_num);
    depthMapv = std::vector<GLuint>(light_num);
    for(size_t i=0; i<light_num; i++){
        // init lights
        if(scene->lights[i]->m_type == Light::PointLight){
            PointLight* pLight = (PointLight*)scene->lights[i];
            PointLight* light = new PointLight();
            light->m_position = pLight->m_position;
            light->m_color = pLight->m_color;
            pointLightv.push_back(light);
        }
        else{ // convert parallel light to point light
            ParallelLight* pLight = (ParallelLight*)scene->lights[i];
            PointLight* light = new PointLight();
            light->m_color = pLight->m_color;
            light->m_position = -1.0e2f * pLight->m_direction;
            pointLightv.push_back(light);
        }
        // configure depth map FBO
        // -----------------------
        glGenFramebuffers(1, &depthMapFBOv[i]);

        // for shadow mapping
        // create depth texture
        glGenTextures(1, &depthMapv[i]);
        glBindTexture(GL_TEXTURE_2D, depthMapv[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width(), height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOv[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapv[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());
    }

//    // for point shadow
//    glGenTextures(1, &depthCubeMap);
//    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
//    for (unsigned int i = 0; i < 6; ++i)
//        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width(), height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//    // attach depth texture as FBO's depth buffer
//    glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
//    glDrawBuffer(GL_NONE);
//    glReadBuffer(GL_NONE);
//    glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());

    // load models
    scene->loadModels();
    size_t modelNum = scene->models.size();
    objModelAABBs = std::vector<AABB>(modelNum);
}

void OpenGLScene::resizeGL(int w, int h){
    this->w = w;
    this->h = h;
    glViewport(0, 0, w, h);
}

void OpenGLScene::paintGL(){

    QVector3D oldCameraPos = camera->position;

    camera->processInput(0.5f);//speed
    foreach(AABB objModelAABB, this->objModelAABBs){
        if(objModelAABB.pointInAABB(camera->position)) {
            camera->position = oldCameraPos;
            break;
        }
    }
    emit updateCameraPosition();

    // render
    glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<QMatrix4x4> lightSpaceMatrixv;

    if(shadowMode == ShadowMode::NO_SHADOW ||
        shadowMode == ShadowMode::SHADOW_MAPPING)
    {
        size_t light_num = scene->lights.size();
        for(size_t i=0; i<light_num; i++)
        {
            if(!scene->lightFlag[i]) continue;

            // 1. render depth of scene to texture (from light's perspective)
            QMatrix4x4 lightProjection, lightView;
            QMatrix4x4 lightSpaceMatrix;
            lightProjection.ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 150.0f);
            lightView.lookAt(pointLightv[i]->m_position.toQVector3D(), QVector3D(0.0f, 0.0f, 0.0f),  QVector3D(0.0f, 1.0f, 0.0f));
            lightSpaceMatrix = lightProjection * lightView;
            lightSpaceMatrixv.push_back(lightSpaceMatrix);

            // render scene from light's point of view
            shadow_mapping_depth_shader.bind();
            shadow_mapping_depth_shader.setUniformValue("lightSpaceMatrix", lightSpaceMatrix);
            glViewport(0, 0, width(), height());
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBOv[i]);
            glClear(GL_DEPTH_BUFFER_BIT);
            renderScene(shadow_mapping_depth_shader, false);
            glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());
        }

        // 2. render scene as normal using the generated depth/shadow map
        glViewport(0, 0, width(), height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shadow_mapping_phong_shader.bind();
        shadow_mapping_phong_shader.setUniformValue("useShadow", shadowMode==SHADOW_MAPPING);
        shadow_mapping_phong_shader.setUniformValue("viewPos", camera->position);

        int light_index = -1;
        for(size_t i=0; i<light_num; i++)
        {
            if(!scene->lightFlag[i]) continue;
            light_index++;
            shadow_mapping_phong_shader.setUniformValue(std::string("lightSpaceMatrix["+std::to_string(light_index)+"]").c_str(), lightSpaceMatrixv[light_index]);
            if(scene->lights[i]->m_type == Light::PointLight){
                shadow_mapping_phong_shader.setUniformValue(std::string("lights["+std::to_string(light_index)+"].type").c_str(), 1);
                shadow_mapping_phong_shader.setUniformValue(std::string("lights["+std::to_string(light_index)+"].attrib").c_str(), ((PointLight*)(scene->lights[i]))->m_position.toQVector3D());
                shadow_mapping_phong_shader.setUniformValue(std::string("lights["+std::to_string(light_index)+"].color").c_str(), pointLightv[i]->m_color.toQVector3D());
            }
            else
            {
                shadow_mapping_phong_shader.setUniformValue(std::string("lights["+std::to_string(light_index)+"].type").c_str(), 2);
                shadow_mapping_phong_shader.setUniformValue(std::string("lights["+std::to_string(light_index)+"].attrib").c_str(), ((ParallelLight*)(scene->lights[i]))->m_direction.toQVector3D());
                shadow_mapping_phong_shader.setUniformValue(std::string("lights["+std::to_string(light_index)+"].color").c_str(), pointLightv[i]->m_color.toQVector3D());
            }
            glActiveTexture(GL_TEXTURE10 + light_index);
            glBindTexture(GL_TEXTURE_2D, depthMapv[i]);
            shadow_mapping_phong_shader.setUniformValue(std::string("shadowMap["+std::to_string(light_index)+"]").c_str(), 10+light_index);
        }
        GLint light_number = (GLint)(light_index+1);
        shadow_mapping_phong_shader.setUniformValue("numLights", light_number);

        size_t modelNum = scene->models.size();
        for(size_t i=0; i<modelNum; i++){
            Model* model = scene->models[i];
            // light model parameters
            shadow_mapping_phong_shader.setUniformValue("ambientStrength", model->ka * this->ka);
            shadow_mapping_phong_shader.setUniformValue("specularStrength", model->ks * this->ks);
            shadow_mapping_phong_shader.setUniformValue("n", GLfloat(model->n * this->n));
            // view/projection transformations
            QMatrix4x4 projection;
            if(this->mode == Perspective){
                projection.perspective(camera->zoom, 1.0f * width() / height(), camera->nearPlane, camera->farPlane);
            }
            else{ // ortho
                projection.ortho(-1.0f, 1.0f, -1.0f, 1.0f, camera->nearPlane, camera->farPlane);
            }
            shadow_mapping_phong_shader.setUniformValue("projection", projection);
            QMatrix4x4 view = camera->getViewMatrix();
            shadow_mapping_phong_shader.setUniformValue("view", view);
            // set light uniforms
            shadow_mapping_phong_shader.setUniformValue("model", model->modelMatrix);
            if(model->type == Model::OBJMODEL){
                ObjModel* objModel = (ObjModel*)model;
                objModel->Draw(shadow_mapping_phong_shader, *this);
            }
            objModelAABBs[i] = model->aabb;
            objModelAABBs[i].transform(model->modelMatrix);
        }
    }
//    else // point shadow
//    {
//        // 1. render scene to depth cubemap
//        glViewport(0, 0, width(), height());
//        glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
//        glClear(GL_DEPTH_BUFFER_BIT);
//        point_shadow_depth_shader.bind();
//        for(unsigned int i=0; i<6; i++){
//            string str = "shadowMatrices["+std::to_string(i)+"]";
//            point_shadow_depth_shader.setUniformValue(str.c_str(), shadowTransforms[i]);
//        }
//        point_shadow_depth_shader.setUniformValue("far_plane", far_plane);
//        point_shadow_depth_shader.setUniformValue("lightPos", pointLight->m_position.toQVector3D());
//        renderScene(point_shadow_depth_shader, false);
//        glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());

//        // 2. render scene as normal
//        glViewport(0, 0, width(), height());
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        point_shadow_phong_shader.bind();
//        point_shadow_phong_shader.setUniformValue("viewPos", camera->position);
//        point_shadow_phong_shader.setUniformValue("lightPos", pointLight->m_position.toQVector3D());
//        point_shadow_phong_shader.setUniformValue("lightColor", pointLight->m_color.toQVector3D());
//        point_shadow_phong_shader.setUniformValue("far_plane", far_plane);
//        glActiveTexture(GL_TEXTURE10);
//        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
//        point_shadow_phong_shader.setUniformValue("depthMap", 10);

//        size_t modelNum = scene->models.size();
//        for(size_t i=0; i<modelNum; i++){
//            Model* model = scene->models[i];
//            // light model parameters
//            point_shadow_phong_shader.setUniformValue("ambientStrength", model->ka * this->ka);
//            point_shadow_phong_shader.setUniformValue("specularStrength", model->ks * this->ks);
//            point_shadow_phong_shader.setUniformValue("n", GLfloat(model->n * this->n));
//            // view/projection transformations
//            QMatrix4x4 projection;
//            if(this->mode == Perspective){
//                projection.perspective(camera->zoom, 1.0f * width() / height(), camera->nearPlane, camera->farPlane);
//            }
//            else{ // ortho
//                projection.ortho(-1.0f, 1.0f, -1.0f, 1.0f, camera->nearPlane, camera->farPlane);
//            }
//            point_shadow_phong_shader.setUniformValue("projection", projection);
//            QMatrix4x4 view = camera->getViewMatrix();
//            point_shadow_phong_shader.setUniformValue("view", view);
//            // set light uniforms
//            point_shadow_phong_shader.setUniformValue("model", model->modelMatrix);
//            if(model->type == Model::OBJMODEL){
//                ObjModel* objModel = (ObjModel*)model;
//                objModel->Draw(point_shadow_phong_shader, *this);
//            }
//            objModelAABBs[i] = model->aabb;
//            objModelAABBs[i].transform(model->modelMatrix);
//        }
//    }
}

void OpenGLScene::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key >= 0 && key < 1024)
        camera->keys[key] = true;
}

void OpenGLScene::keyReleaseEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key >= 0 && key < 1024)
        camera->keys[key] = false;
}

void OpenGLScene::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        m_bLeftPressed = true;
        m_lastPos = event->pos();
    }
}

void OpenGLScene::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    m_bLeftPressed = false;
}

void OpenGLScene::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bLeftPressed) {
        int xpos = event->pos().x();
        int ypos = event->pos().y();

        int xoffset = xpos - m_lastPos.x();
        int yoffset = m_lastPos.y() - ypos;
        m_lastPos = event->pos();
        camera->processMouseMovement(xoffset, yoffset);
    }
}

void OpenGLScene::wheelEvent(QWheelEvent *event)
{
    QPoint offset = event->angleDelta();
    camera->processMouseScroll(offset.y() / 20.0f);
}

bool OpenGLScene::createShader()
{
    // shadow mapping
    bool success = shadow_mapping_phong_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/phong_shadow_mapping.vert");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shadow_mapping_phong_shader.log();
        return success;
    }

    success = shadow_mapping_phong_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/phong_shadow_mapping.frag");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shadow_mapping_phong_shader.log();
        return success;
    }

    success = shadow_mapping_phong_shader.link();
    if(!success) {
        qDebug() << "shaderProgram link failed!" << shadow_mapping_phong_shader.log();
    }

    success = shadow_mapping_depth_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/shadow_mapping_depth.vert");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shadow_mapping_depth_shader.log();
        return success;
    }

    success = shadow_mapping_depth_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/shadow_mapping_depth.frag");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << shadow_mapping_depth_shader.log();
        return success;
    }

    success = shadow_mapping_depth_shader.link();
    if(!success) {
        qDebug() << "shaderProgram link failed!" << shadow_mapping_depth_shader.log();
    }

    // point shadow
    success = point_shadow_phong_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/phong_point_shadow.vert");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << point_shadow_phong_shader.log();
        return success;
    }

    success = point_shadow_phong_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/phong_point_shadow.frag");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << point_shadow_phong_shader.log();
        return success;
    }

    success = point_shadow_phong_shader.link();
    if(!success) {
        qDebug() << "shaderProgram link failed!" << point_shadow_phong_shader.log();
    }

    success = point_shadow_depth_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shader/point_shadow_depth.vert");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << point_shadow_depth_shader.log();
        return success;
    }

    success = point_shadow_depth_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shader/point_shadow_depth.frag");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << point_shadow_depth_shader.log();
        return success;
    }

    success = point_shadow_depth_shader.addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shader/point_shadow_depth.geom");
    if (!success) {
        qDebug() << "shaderProgram addShaderFromSourceFile failed!" << point_shadow_depth_shader.log();
        return success;
    }

    return success;
}

void OpenGLScene::renderScene(QOpenGLShaderProgram &shader, bool light)
{
    size_t modelNum = scene->models.size();
    for(size_t i=0; i<modelNum; i++){
        Model* model = scene->models[i];
        // set light uniforms
        shader.setUniformValue("model", model->modelMatrix);
        if(light){
            // light model parameters
            shader.setUniformValue("ambientStrength", model->ka);
            shader.setUniformValue("specularStrength", model->ks);
            shader.setUniformValue("n", GLfloat(model->n));
        }
        if(model->type == Model::OBJMODEL){
            ObjModel* objModel = (ObjModel*)model;
            objModel->Draw(shader, *this);
        }
        objModelAABBs[i] = model->aabb;
        objModelAABBs[i].transform(model->modelMatrix);
    }
}


