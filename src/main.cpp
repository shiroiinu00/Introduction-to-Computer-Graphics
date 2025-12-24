#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "header/cube.h"
#include "header/Object.h"
#include "header/shader.h"
#include "header/stb_image.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void updateCamera();
void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta);
unsigned int loadCubemap(std::vector<std::string> &mFileName);

struct material_t{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float gloss;
};

struct light_t{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct camera_t{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    glm::vec3 target;

    float yaw;
    float pitch;
    float radius;
    float minRadius;
    float maxRadius;
    float orbitRotateSpeed;
    float orbitZoomSpeed;
    float minOrbitPitch;
    float maxOrbitPitch;
    bool enableAutoOrbit;
    float autoOrbitSpeed;
};

// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

// cube map 
unsigned int cubemapTexture;
unsigned int cubemapVAO, cubemapVBO;

// shader programs 
int shaderProgramIndex = 0;
std::vector<shader_program_t*> shaderPrograms;
shader_program_t* cubemapShader;

light_t light;
material_t material;
camera_t camera;

Object* baseballModel = nullptr;
Object* baseballBatModel = nullptr;
Object* microwaveModel = nullptr;
Object* ballparkModel = nullptr;
Object* sharkModel=nullptr;
Object* catModel=nullptr;
Object* handModel = nullptr;
Object* cubeModel = nullptr;
bool isCube = false;
glm::mat4 modelMatrix(1.0f);

float currentTime = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ===== animation parameter =====
// main animation
bool startanimation = false;
float beforeStartDuration = 10.0f;
float animationTime = 0.0f;
bool animationPlaying = false;
float animationDuration = 15.0f;

// microwave explosion animation
bool microwaveVisible = true;
bool microwaveExploding = false;
float microwaveExplodeStart = -1.0f;
float microwaveExplodeDuration = 5.0f;

// ball explosion animation
bool ballVisible = true;
bool ballExploding = false;
float ballExplodeStart = -1.0f;
float ballExlodeDuration = 0.8f;

// camera animation parameter
bool cameraFollowBall = false;
glm::vec3 cameraDefaultTarget(0.0f);
float defaultCameraRadius;
float followCameraRadius = 120.0f;

// cat animation
int catNumber = 0;
float catTime = 2.0f;


// =====================

// ====== ball trail ======
std::vector<glm::vec3> ballTrail;
const int TRAIL_MAX = 80;

unsigned int trailVAO =0, trailVBO = 0;
shader_program_t* trailShader = nullptr;

void trail_setup(){
    glGenVertexArrays(1, &trailVAO);
    glGenBuffers(1, &trailVBO);

    glBindVertexArray(trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);

    // allocate fixed size
    glBufferData(GL_ARRAY_BUFFER, TRAIL_MAX * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

void trail_push(const glm::vec3 &p){
    ballTrail.push_back(p);
    if ((int)ballTrail.size() > TRAIL_MAX) {
        ballTrail.erase(ballTrail.begin());
    }

    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
ballTrail.size() * sizeof(glm::vec3),
        ballTrail.data());
}

void trail_draw(const glm::mat4 &view, const glm::mat4& projection){
    if (ballTrail.size() < 2) return;

    trailShader->use();
    trailShader->set_uniform_value("view", view);
    trailShader->set_uniform_value("projection", projection);
    trailShader->set_uniform_value("time", currentTime);
    trailShader->set_uniform_value("pointCount", (int)ballTrail.size());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(trailVAO);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)ballTrail.size());
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    trailShader->release();
    
}

// =====================


void model_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string obj_path = "..\\..\\src\\asset\\obj\\Mei_Run.obj";
    std::string cube_obj_path = "..\\..\\src\\asset\\obj\\cube.obj";
    std::string texture_path = "..\\..\\src\\asset\\texture\\Mei_TEX.png";
#else
    std::string baseball_obj_path = "..\\..\\src\\asset\\obj\\baseball.obj";
    std::string baseball_texture_path = "..\\..\\src\\asset\\texture\\baseball.jpg";
    std::string baseball_bat_obj_path = "..\\..\\src\\asset\\obj\\baseball_bat.obj";
    std::string baseball_bat_texture_path = "..\\..\\src\\asset\\texture\\baseball_bat.jpg";
    std::string microwave_obj_path = "..\\..\\src\\asset\\obj\\microwave.obj";
    std::string microwave_texture_path = "..\\..\\src\\asset\\texture\\microwave.jpg";
    std::string ballpark_obj_path = "..\\..\\src\\asset\\obj\\CUPIC_BALLPARK.obj";
    std::string ballpark_texture_path = "..\\..\\src\\asset\\texture\\ball_parkSurface_Color.png";
    std::string cube_obj_path = "..\\..\\src\\asset\\obj\\cube.obj";
    std::string shark_texture_path = "..\\..\\src\\asset\\texture\\Tex_Shark.png";
    std::string shark_obj_path="..\\..\\src\\asset\\obj\\Mesh_Shark.obj";
    std::string cat_texture_path = "..\\..\\src\\asset\\texture\\Tex_Cat.png";
    std::string cat_obj_path="..\\..\\src\\asset\\obj\\Mesh_Cat.obj";
    std::string hand_texture_path = "..\\..\\src\\asset\\texture\\hand.jpg";
    std::string hand_obj_path="..\\..\\src\\asset\\obj\\hand.obj";

#endif

    // load baseball
    baseballModel = new Object(baseball_obj_path);
    baseballModel->loadTexture(baseball_texture_path);

    // load baseball_bat
    baseballBatModel = new Object(baseball_bat_obj_path);
    baseballBatModel->loadTexture(baseball_bat_texture_path);

    // load microwave
    microwaveModel = new Object(microwave_obj_path);
    microwaveModel->loadTexture(microwave_texture_path);

    // load ballpark
    ballparkModel = new Object(ballpark_obj_path);
    ballparkModel->loadTexture(ballpark_texture_path);

    // load shark
    sharkModel = new Object(shark_obj_path);
    sharkModel->loadTexture(shark_texture_path);

    // load hand
    handModel = new Object(hand_obj_path);
    handModel->loadTexture(hand_texture_path);

    // load cat
    catModel = new Object(cat_obj_path);
    catModel->loadTexture(cat_texture_path);
    
    cubeModel = new Object(cube_obj_path);

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));
}

void camera_setup(){
    camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.yaw = 90.0f;
    camera.pitch = 10.0f;
    camera.radius = 400.0f;
    camera.minRadius = 150.0f;
    camera.maxRadius = 800.0f;
    camera.orbitRotateSpeed = 60.0f;
    camera.orbitZoomSpeed = 400.0f;
    camera.minOrbitPitch = -80.0f;
    camera.maxOrbitPitch = 80.0f;
    camera.target = glm::vec3(0.0f);
    camera.enableAutoOrbit = true;
    camera.autoOrbitSpeed = 20.0f;
    defaultCameraRadius = camera.radius;

    updateCamera();
}

void updateCamera(){
    float yawRad = glm::radians(camera.yaw);
    float pitchRad = glm::radians(camera.pitch);
    float cosPitch = cos(pitchRad);

    camera.position.x = camera.target.x + camera.radius * cosPitch * cos(yawRad);
    camera.position.y = camera.target.y + camera.radius * sin(pitchRad);
    camera.position.z = camera.target.z + camera.radius * cosPitch * sin(yawRad);

    camera.front = glm::normalize(camera.target - camera.position);
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}

void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta) {
    camera.yaw += yawDelta;
    camera.pitch = glm::clamp(camera.pitch + pitchDelta, camera.minOrbitPitch, camera.maxOrbitPitch);
    camera.radius = glm::clamp(camera.radius + radiusDelta, camera.minRadius, camera.maxRadius);
    updateCamera();
}

void light_setup(){
    light.position = glm::vec3(1000.0, 1000.0, 0.0);
    light.ambient = glm::vec3(1.0);
    light.diffuse = glm::vec3(1.0);
    light.specular = glm::vec3(1.0);
}

void material_setup(){
    material.ambient = glm::vec3(0.5);
    material.diffuse = glm::vec3(1.0);
    material.specular = glm::vec3(0.7);
    material.gloss = 50.0;
}

void shader_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string shaderDir = "..\\..\\src\\shaders\\";
#else
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    std::vector<std::string> shadingMethod = {
        "default"
    };

    for(int i=0; i<shadingMethod.size(); i++){
        std::string vpath = shaderDir + shadingMethod[i] + ".vert";
        std::string fpath = shaderDir + shadingMethod[i] + ".frag";

        shader_program_t* shaderProgram = new shader_program_t();
        shaderProgram->create();
        shaderProgram->add_shader(vpath, GL_VERTEX_SHADER);
        shaderProgram->add_shader(fpath, GL_FRAGMENT_SHADER);
        shaderProgram->link_shader();
        shaderPrograms.push_back(shaderProgram);
    }
    // add the explosion shader to the program
    std::string vpath = shaderDir + "explosion.vert";
    std::string gpath = shaderDir + "explosion.geom";
    std::string fpath = shaderDir + "explosion.frag";
    shader_program_t* explosion = new shader_program_t();
    explosion->create();
    explosion->add_shader(vpath, GL_VERTEX_SHADER);
    explosion->add_shader(gpath, GL_GEOMETRY_SHADER);
    explosion->add_shader(fpath, GL_FRAGMENT_SHADER);
    explosion->link_shader();
    shaderPrograms.push_back(explosion);
    // add the trail shader to the program
    vpath = shaderDir + "trail.vert";
    gpath = shaderDir + "trail.geom";
    fpath = shaderDir + "trail.frag";
    shader_program_t* trail = new shader_program_t();
    trail->create();
    trail->add_shader(vpath, GL_VERTEX_SHADER);
    trail->add_shader(gpath, GL_GEOMETRY_SHADER);
    trail->add_shader(fpath, GL_FRAGMENT_SHADER);
    trail->link_shader();
    shaderPrograms.push_back(trail);
    trailShader = trail;
    // add the base shader to the program
    vpath = shaderDir + "base.vert";
    // gpath = shaderDir + "trail.geom";
    fpath = shaderDir + "base.frag";
    shader_program_t* base = new shader_program_t();
    base->create();
    base->add_shader(vpath, GL_VERTEX_SHADER);
    // trail->add_shader(gpath, GL_GEOMETRY_SHADER);
    base->add_shader(fpath, GL_FRAGMENT_SHADER);
    base->link_shader();
    shaderPrograms.push_back(base);
    // add the trail shader to the program
    vpath = shaderDir + "flame.vert";
    gpath = shaderDir + "flame.geom";
    fpath = shaderDir + "flame.frag";
    shader_program_t* flame = new shader_program_t();
    flame->create();
    flame->add_shader(vpath, GL_VERTEX_SHADER);
    flame->add_shader(gpath, GL_GEOMETRY_SHADER);
    flame->add_shader(fpath, GL_FRAGMENT_SHADER);
    flame->link_shader();
    shaderPrograms.push_back(flame);

    

}

void cubemap_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string cubemapDir = "..\\..\\src\\asset\\texture\\skybox\\";
    std::string shaderDir = "..\\..\\src\\shaders\\";
#else
    std::string cubemapDir = "..\\..\\src\\asset\\texture\\skybox\\";
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    std::vector<std::string> faces
    {
        cubemapDir + "cube_right.jpg",
        cubemapDir + "cube_left.jpg",
        cubemapDir + "cube_top.jpg",
        cubemapDir + "cube_bottom.jpg",
        cubemapDir + "cube_back.jpg",
        cubemapDir + "cube_front.jpg"
    };
    cubemapTexture = loadCubemap(faces);   

    std::string vpath = shaderDir + "cubemap.vert";
    std::string fpath = shaderDir + "cubemap.frag";
    
    cubemapShader = new shader_program_t();
    cubemapShader->create();
    cubemapShader->add_shader(vpath, GL_VERTEX_SHADER);
    cubemapShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    cubemapShader->link_shader();

    glGenVertexArrays(1, &cubemapVAO);
    glGenBuffers(1, &cubemapVBO);
    glBindVertexArray(cubemapVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), &cubemapVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void setup(){
    light_setup();
    model_setup();
    shader_setup();
    camera_setup();
    cubemap_setup();
    material_setup();
    
    // trail_setup
    trail_setup();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
}

void update(){
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;

    if(startanimation){
        animationTime += deltaTime;
        
        float t = animationTime / beforeStartDuration;
        camera.yaw = glm::mix(90.0f, 630.0f, t);
        camera.radius=glm::mix(500.0f,100.0f,t);
        if (animationTime > catTime){
            catTime+=2.0f;
            catNumber++;
        }
        if(t >= 1){
            startanimation = false;
            animationPlaying = true;
            animationTime = 0.0f;
        }
        
    }

    if (animationPlaying) {
        animationTime += deltaTime;
        if (animationTime > animationDuration) {
            // animationPlaying = false;
            ballVisible = false;

            // cameraFollowBall = false; // stop following the ball
            // camera.target = cameraDefaultTarget;
            // camera.radius = defaultCameraRadius;
            // updateCamera();
        }
    }

    // camera rotater
    // if (camera.enableAutoOrbit) {
    //     float yawDelta = camera.autoOrbitSpeed * deltaTime;
    //     applyOrbitDelta(yawDelta, 0.0f, 0.0f);
    // }
}

void render() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::lookAt(camera.position - glm::vec3(0.0f, 0.2f, 0.1f), camera.position + camera.front, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

    glDepthMask(GL_FALSE);
    
    cubemapShader->use();

    glm::mat4 viewWithNoTrans = glm::mat4(glm::mat3(view));
    cubemapShader->set_uniform_value("view", viewWithNoTrans);
    cubemapShader->set_uniform_value("projection", projection);

    // bind texture unit 0
    cubemapShader->set_uniform_value("skybox", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glBindVertexArray(cubemapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    cubemapShader->release();
    glDepthMask(GL_TRUE);

    shaderPrograms[shaderProgramIndex]->use();
    shaderPrograms[shaderProgramIndex]->set_uniform_value("view", view);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("projection", projection);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("viewPos", camera.position - glm::vec3(0.0f, 0.2f, 0.1f));

    // Light uniforms
    shaderPrograms[shaderProgramIndex]->set_uniform_value("light.position", light.position);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("light.ambient", light.ambient);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("light.diffuse", light.diffuse);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("light.specular", light.specular);
    
    // Material uniforms
    shaderPrograms[shaderProgramIndex]->set_uniform_value("material.ambient", material.ambient);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("material.diffuse", material.diffuse);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("material.specular", material.specular);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("material.gloss", material.gloss);
    
    // Skybox uniform
    shaderPrograms[shaderProgramIndex]->set_uniform_value("skybox", 0);

    // Special shader uniforms
    if (shaderProgramIndex == 3) {
        shaderPrograms[shaderProgramIndex]->set_uniform_value("bias", 0.2f);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("alpha", 0.4f);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("lightIntensity", 1.0f);
    }

    if (shaderProgramIndex == 4) { 
        shaderPrograms[shaderProgramIndex]->set_uniform_value("AIR_coeff", 1.0f);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("GLASS_coeff", 1.52f);
    }

    // Define key positions for animation
    glm::vec3 baseballStartPos(10.0f, 60.0f, 180.0f);   // Starting position
    glm::vec3 baseballBatPos(10.0f, 60.0f, 300.0f);     // Bat position
    glm::vec3 handPos(-10.0f, 60.0f, 285.0f);     // hand position
    glm::vec3 baseballHitPos(10.0f, 60.0f, 282.0f);     // Position just before bat
    glm::vec3 microwavePos(-220.0f, 40.0f, 70.0f);        // Microwave position

    
    // Default positions
    glm::vec3 baseballPos = baseballStartPos;
    glm::vec3 batPos = baseballBatPos;
    float batRotationAngle = -45.0f;  
    float baseballSpinSpeed = 0.0f; 
    
    if (animationTime>0.0f && animationPlaying) {
        float t = glm::min(animationTime / animationDuration, 1.0f); 
        
        float t0 = 0.35f;
        float t1=0.4f;
        float t2=0.42f;

        // Baseball moves toward bat
        if (t <= t1) { 
            float phaseT = t / t1; 
            baseballPos = glm::mix(baseballStartPos, baseballHitPos, phaseT);
            // batRotationAngle = -45.0f; // Bat initial angle
            baseballSpinSpeed=360.0f;
            // camera.yaw = 90.0f;
            camera.yaw = glm::mix(270.0f, 450.0f, phaseT);
        }
        // Bat swings and hits ball
        else if (t <= t2) { 
            // float phaseT = (t - t1) / (t2-t1); 
            baseballPos = baseballHitPos;
            batRotationAngle = 75.0f;
            baseballSpinSpeed=1440.0f;
        }
        // Baseball to microwave
        else {
            float phaseT = (t - t2) / (1.0f-t2); 
            
            float parabolicHeight = 50.0f * sin(1.05*phaseT * glm::pi<float>());
            
            baseballPos.x = glm::mix(baseballHitPos.x, -220.0f, phaseT);
            baseballPos.y = 60.0f + parabolicHeight;
            baseballPos.z = glm::mix(baseballHitPos.z, 70.0f, phaseT);
            batRotationAngle = 75.0f;
            baseballSpinSpeed=720.0f;

            camera.yaw = glm::mix(450.0f, 405.0f, phaseT);
        }

        if(t < t0){
            batRotationAngle = -45.0f; // Bat initial angle
        }else if (t <= (t1 - 0.005f)){
            float phaseT = (t - t0) / ((t1 - 0.005f) - t0);
            batRotationAngle = -45.0f + 120.0f * phaseT;
        }else if ((t1 - 0.005f) < t && t <= t1){
            batRotationAngle = 75.0f;
        }
    }

    // let camera follow the ball
    if(cameraFollowBall){
        float followSpeed = 5.0f;
        glm::vec3 followTarget = baseballPos + glm::vec3(0.0f, -15.0f, 20.0f);
        camera.target = glm::mix(camera.target, followTarget, followSpeed * deltaTime);
        updateCamera();
    }

    auto baseShader = shaderPrograms[shaderProgramIndex];
    baseShader->use();

    if(isCube){
        // Render cube if toggled
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", modelMatrix);
        cubeModel->draw();
    } else {
        // Render ballpark
        glm::mat4 ballparkMat = glm::mat4(1.0f);
        ballparkMat = glm::translate(ballparkMat, glm::vec3(0.0f, 30.0f, 0.0f)); 
        ballparkMat = glm::scale(ballparkMat, glm::vec3(0.75f)); 
        ballparkMat = glm::rotate(ballparkMat, glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", ballparkMat);
        ballparkModel->draw();
        
        // Render baseball bat
        glm::mat4 batMat = glm::mat4(1.0f);
        batMat = glm::translate(batMat, batPos);
        batMat = glm::rotate(batMat, glm::radians(batRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        batMat = glm::rotate(batMat, glm::radians(batRotationAngle * 1.2f), glm::vec3(1.0f, 0.0f, 0.0f));
        // batMat = glm::rotate(batMat, glm::radians(-10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        batMat = glm::rotate(batMat, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
        batMat = glm::scale(batMat, glm::vec3(0.5f));
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", batMat);
        baseballBatModel->draw();

        // Render Shark
        glm::mat4 sharkMat = glm::mat4(1.0f);
        sharkMat = glm::translate(sharkMat, glm::vec3(0.0f, 50.0f+20.0f*sin(currentTime*10.0f),465.0f));
        sharkMat = glm::rotate(sharkMat, glm::radians(currentTime*720.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        sharkMat = glm::scale(sharkMat, glm::vec3(0.4f));
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", sharkMat);
        sharkModel->draw();

        glm::mat4 handMat = batMat; 
        handMat = glm::translate(handMat, glm::vec3(-50.0f, 5.0f, 3.0f));
        // handMat = glm::rotate(handMat, glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // handMat = glm::rotate(handMat, glm::radians(-40.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
        handMat = glm::rotate(handMat, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); 
        handMat=glm::scale(handMat,glm::vec3(1.0f));
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", handMat);
        handModel->draw();

         // Render Cat
        std::vector<glm::vec3> catPos = {
            glm::vec3(200.0f,60.0f+5.0f*sin(currentTime*10.0f),-30.0f), // RF
            glm::vec3(80.0f, 60.0f+5.0f*sin(currentTime*10.0f), 100.0f), // 2B
            // glm::vec3(-200.0f,60.0f+5.0f*sin(currentTime*10.0f),-30.0f), //LF
            glm::vec3(-95.0f, 60.0f+5.0f*sin(currentTime*10.0f), 180.0f), // 3B
            glm::vec3(-40.0f, 60.0f+5.0f*sin(currentTime*10.0f), 110.0f), // SS
            glm::vec3(130.0f, 60.0f+5.0f*sin(currentTime*10.0f), 150.0f), // 1B
            glm::vec3(20.0f, 60.0f+5.0f*sin(currentTime*10.0f),-180.0f), // CF
        };

        int baseIndex = 3;
        int flameIndex = 4;
        auto catBaseShader = shaderPrograms[baseIndex];
        auto flameShader = shaderPrograms[flameIndex];

        float burn = glm::clamp((animationTime - 0.0f) * 0.1f, 0.0f, 1.0f);

        for(int i = 0; i < catNumber; i++){
            glm::mat4 catMat = glm::mat4(1.0f);
            catMat = glm::translate(catMat, catPos[i]);
            catMat = glm::rotate(catMat, glm::radians(currentTime*720.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
            catMat = glm::scale(catMat, glm::vec3(0.4f));
            // draw base
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
            catBaseShader->use();
            catBaseShader->set_uniform_value("view", view);
            catBaseShader->set_uniform_value("projection", projection);
            catBaseShader->set_uniform_value("model", catMat);
            catBaseShader->set_uniform_value("uTime", currentTime);
            catBaseShader->set_uniform_value("uBurn", burn);
            catBaseShader->set_uniform_value("diffuseTex", 0);
            catModel->draw();
            catBaseShader->release();
            // draw flame
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDepthMask(GL_FALSE);
            flameShader->use();
            flameShader->set_uniform_value("view", view);
            flameShader->set_uniform_value("projection", projection);
            flameShader->set_uniform_value("model", catMat);
            flameShader->set_uniform_value("uTime", currentTime);
            flameShader->set_uniform_value("uBurn", burn);
            flameShader->set_uniform_value("uUp", glm::vec3(0.0f, 1.0f, 0.0f));

            catModel->draw();
            flameShader->release();
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
        
    }

    // Determine whether the ball hits the microwave
    float explodeAmt = 0.0f;
    float dist = glm::length(baseballPos - microwavePos);
    if(microwaveVisible && !microwaveExploding && dist < 20.0f){
        microwaveExploding = true;
        ballExploding = true;
        microwaveExplodeStart = currentTime;
        printf("explosion trigger!\n");
    }
    
    if(microwaveExploding){
        float t = (currentTime - microwaveExplodeStart) / microwaveExplodeDuration;
        explodeAmt = 1.0f - pow(1.0f - t, 3.0f);
        // explodeAmt = explodeAmt * explodeAmt;

        if (t >= 1.0f){
            microwaveVisible = false;
            microwaveExploding = false;
        }
    }

    // draw microwave
    
    int explosionIndex = 1; // 5 is the explosion shader index

    // Render microwave
    if (microwaveVisible) {
        glm::mat4 microwaveMat = glm::mat4(1.0f);
        microwaveMat = glm::translate(microwaveMat, microwavePos);
        microwaveMat = glm::rotate(microwaveMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        microwaveMat = glm::rotate(microwaveMat, glm::radians(40.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        microwaveMat = glm::scale(microwaveMat, glm::vec3(0.6f));
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", microwaveMat);
        auto explosionShader = shaderPrograms[explosionIndex];
        if(microwaveExploding) {
            explosionShader->use();
            explosionShader->set_uniform_value("view", view);
            explosionShader->set_uniform_value("projection", projection);
            explosionShader->set_uniform_value("model", microwaveMat);
            explosionShader->set_uniform_value("explode", explodeAmt);
            explosionShader->set_uniform_value("time", currentTime);
            explosionShader->set_uniform_value("outTexture", 0);
            explosionShader->set_uniform_value("isExplosion", GL_TRUE);
            microwaveModel->draw();
            explosionShader->release();
        }else{
            baseShader->use();
            baseShader->set_uniform_value("model", microwaveMat);
            explosionShader->set_uniform_value("isExplosion", GL_FALSE);
            microwaveModel->draw();
        }
    }
    if (ballVisible) {
        // Render baseball
        glm::mat4 baseballMat = glm::mat4(1.0f);
        baseballMat = glm::translate(baseballMat, baseballPos);
        float baseballRotation = currentTime * baseballSpinSpeed;
        baseballMat = glm::rotate(baseballMat, glm::radians(baseballRotation), glm::vec3(0.0f, 0.0f, 1.0f));
        baseballMat = glm::scale(baseballMat, glm::vec3(0.8f));
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", baseballMat);
        auto explosionShader = shaderPrograms[explosionIndex];
        if(ballExploding) {
            explosionShader->use();
            explosionShader->set_uniform_value("view", view);
            explosionShader->set_uniform_value("projection", projection);
            explosionShader->set_uniform_value("model", baseballMat);
            float strength = pow(explodeAmt, 3);
            explosionShader->set_uniform_value("explode", strength);
            explosionShader->set_uniform_value("time", currentTime);
            explosionShader->set_uniform_value("outTexture", 0);
            explosionShader->set_uniform_value("isExplosion", GL_TRUE);
            baseballModel->draw();
            explosionShader->release();
        }else{
            baseShader->use();
            baseShader->set_uniform_value("model", baseballMat);
            baseballModel->draw();
        }
    }

    if(animationPlaying) trail_push(baseballPos);
    if(ballVisible) trail_draw(view, projection);
    // trail -> add vertex to the vector
    
    
    baseShader->release();


}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HW3-Static Model", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    setup();
    
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        update(); 
        render(); 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete baseballModel;
    delete baseballBatModel;
    delete ballparkModel;
    delete microwaveModel;
    delete cubeModel;
    delete sharkModel;
    delete handModel;
    for (auto shader : shaderPrograms) {
        delete shader;
    }
    delete cubemapShader;

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec2 orbitInput(0.0f);
    float zoomInput = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        orbitInput.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        orbitInput.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        orbitInput.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        orbitInput.y -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        zoomInput -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        zoomInput += 1.0f;

    if (orbitInput.x != 0.0f || orbitInput.y != 0.0f || zoomInput != 0.0f) {
        float yawDelta = orbitInput.x * camera.orbitRotateSpeed * deltaTime;
        float pitchDelta = orbitInput.y * camera.orbitRotateSpeed * deltaTime;
        float radiusDelta = zoomInput * camera.orbitZoomSpeed * deltaTime;
        applyOrbitDelta(yawDelta, pitchDelta, radiusDelta);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // if (key == GLFW_KEY_0 && (action == GLFW_REPEAT || action == GLFW_PRESS)) 
    //     shaderProgramIndex = 0;
    // if (key == GLFW_KEY_1 && (action == GLFW_REPEAT || action == GLFW_PRESS)) 
    //     shaderProgramIndex = 1;
    // if (key == GLFW_KEY_2 && (action == GLFW_REPEAT || action == GLFW_PRESS)) 
    //     shaderProgramIndex = 2;
    // if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    //     shaderProgramIndex = 3;
    // if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    //     shaderProgramIndex = 4;
    // if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    //     shaderProgramIndex = 5;
    // if (key == GLFW_KEY_6 && action == GLFW_PRESS)
    //     shaderProgramIndex = 6;
    // if (key == GLFW_KEY_7 && action == GLFW_PRESS)
    //     shaderProgramIndex = 7;
    // if (key == GLFW_KEY_8 && action == GLFW_PRESS)
    //     shaderProgramIndex = 8;
    if( key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
        animationTime += 0.1f;
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        // before throw the ball
        startanimation = true;
        
        // animation of throwing the ball
        animationPlaying = false;
        animationTime = 0.0f;

        // microwave explosion
        microwaveVisible = true;
        microwaveExploding = false;
        microwaveExplodeStart = -1.0f;
        
        // ball explosion
        ballExploding = false;
        ballVisible = true;

        // show the cat
        catTime = 2.0f;
        catNumber = 1;

        // camera animation
        cameraFollowBall = true;
        camera.radius = followCameraRadius;

        // trail animation
        ballTrail.clear();


    }
    
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        animationPlaying = false;
    }
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

unsigned int loadCubemap(vector<std::string>& faces)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        stbi_set_flip_vertically_on_load(false);
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}   