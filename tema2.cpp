#include "lab_m1/tema2/tema2.h"

#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <glm/gtx/vector_angle.hpp>

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
}


Tema2::~Tema2()
{
}


void Tema2::Init()
{
    left = 0.0f;
    right = 10.0f;
    bottom = 0.01f;
    top = 0.0f;
    fov = 90.0f;

    pozCamera = glm::vec3(0, 2.5f, 3);

    camera = new implemented::Camera();
    camera->Set(pozCamera, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

    camera->RotateFirstPerson_OX(0.25);

    {
        Mesh* mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // Create a shader program for drawing face polygon with the color of the normal
    {
        Shader* shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        vector<VertexFormat> vertices
        {
            VertexFormat(glm::vec3(-1, -1,  1), glm::vec3(0, 1, 1), glm::vec3(0.2, 0.8, 0.2)),
            VertexFormat(glm::vec3(1, -1,  1), glm::vec3(1, 0, 1), glm::vec3(0.9, 0.4, 0.2)),
            VertexFormat(glm::vec3(-1,  1,  1), glm::vec3(1, 0, 0), glm::vec3(0.7, 0.7, 0.1)),
            VertexFormat(glm::vec3(1,  1,  1), glm::vec3(0, 1, 0), glm::vec3(0.7, 0.3, 0.7)),
            VertexFormat(glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1), glm::vec3(0.3, 0.5, 0.4)),
            VertexFormat(glm::vec3(1, -1, -1), glm::vec3(0, 1, 1), glm::vec3(0.5, 0.2, 0.9)),
            VertexFormat(glm::vec3(-1,  1, -1), glm::vec3(1, 1, 0), glm::vec3(0.7, 0.0, 0.7)),
            VertexFormat(glm::vec3(1,  1, -1), glm::vec3(0, 0, 1), glm::vec3(0.1, 0.5, 0.8)),
        };

        vector<unsigned int> indices =
        {
            0, 1, 2,        1, 3, 2,
            2, 3, 7,        2, 7, 6,
            1, 7, 3,        1, 5, 7,
            6, 7, 4,        7, 5, 4,
            0, 4, 1,        1, 4, 5,
            2, 6, 4,        0, 2, 4,
        };

        Mesh* mesh = new Mesh("building");
        mesh->InitFromData(vertices, indices);
        AddMeshToList(mesh);
    }

    {
        Mesh* mesh = new Mesh("proj");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("tank");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank"), "TNK.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("turel");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank"), "turel.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("weels");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank"), "WEELS.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("pipe");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "tank"), "PIPE.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    //void glLineWidth(GLfloat width);
    // Light & material properties
    {   
        lightPositionTank = glm::vec3(2, 7, 0);
        lightPositionTank = glm::rotate(lightPositionTank, 0.25f, glm::vec3(0,1,0));
        lightPosition = glm::vec3(0, 20, 0);
        materialShininess = 30;
        materialKd = 0.5;
        materialKs = 0.5;
    }

    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

    tank.pozXTank = tank.pozZTank = 0;

    randProj = 0.5f;

    goBackToCameraPos = 0;

    tank.lives = 5;

    nrPause = 1;
}

void Tema2::FrameStart() {
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0.3, 0.6, 0.9, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::InitBoard() {
    // Render ground
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(10));
    RenderSimpleMesh(meshes["plane"], shaders["LabShader"], modelMatrix, glm::vec3(0.3f, 0.9f, 0.3f), 3, 2);

    // Render the parts of the tank
    tank.body = glm::mat4(1);
    tank.body = glm::translate(tank.body, glm::vec3(tank.pozXTank, 0.55, tank.pozZTank));
    tank.body = glm::scale(tank.body, glm::vec3(0.5f));
    tank.body = glm::rotate(tank.body, scale, glm::vec3(0, 1, 0));
    RenderSimpleMesh(meshes["tank"], shaders["LabShader"], tank.body, glm::vec3(0.9, 0, 0.4), tank.lives, 1);

    tank.turel = glm::mat4(1);
    tank.turel = glm::translate(tank.turel, glm::vec3(tank.pozXTank, 1.3, tank.pozZTank));
    tank.turel = glm::scale(tank.turel, glm::vec3(0.45f));
    tank.turel = glm::rotate(tank.turel, scale + pozCrtTurel, glm::vec3(0, 1, 0));
    RenderSimpleMesh(meshes["turel"], shaders["LabShader"], tank.turel, glm::vec3(0.9, 0, 0.4), tank.lives, 1);

    tank.pipe = glm::mat4(1);
    tank.pipe = glm::translate(tank.pipe, glm::vec3(tank.pozXTank, 1.6, tank.pozZTank));
    tank.pipe = glm::scale(tank.pipe, glm::vec3(0.5f));
    tank.pipe = glm::rotate(tank.pipe, scale + pozCrtTurel, glm::vec3(0, 1, 0));
    RenderSimpleMesh(meshes["pipe"], shaders["LabShader"], tank.pipe, glm::vec3(1, 1, 1), tank.lives, 1);

    tank.weels = glm::mat4(1);
    tank.weels = glm::translate(tank.weels, glm::vec3(tank.pozXTank, 0.55, tank.pozZTank));
    tank.weels = glm::scale(tank.weels, glm::vec3(0.5f));
    tank.weels = glm::rotate(tank.weels, scale, glm::vec3(0, 1, 0));
    RenderSimpleMesh(meshes["weels"], shaders["LabShader"], tank.weels, glm::vec3(1, 1, 1), tank.lives, 1);
}

void Tema2::GenerateProj(float pozX, float pozZ, float tankAngle, float turelAngle, TankStruct &tankCrt) {
    
    Proj newProj;
    newProj.coords.x = pozX;
    newProj.coords.y = 1.6;
    newProj.coords.z = pozZ;
    newProj.time = 5;
    newProj.alive = true;
    newProj.crtAngle = tankAngle + turelAngle - 4.72;// + 0.035;//scale + pozCrtTurel + 1.57;// 4.71;

    tankCrt.projec.emplace(tankCrt.projec.end(), newProj);
}

void Tema2::BuildProj(float x, float z) {
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(x, 1.6, z));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f));
    RenderSimpleMesh(meshes["proj"], shaders["LabShader"], modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f), 3, 2);
}

void Tema2::BuildBuildings() {
    BuildingStruct newBuilding;
    newBuilding.coords.x = pow(-1, (rand() % 2 + 1)) * (rand() % 200 + 5);
    newBuilding.coords.y = 1;
    newBuilding.coords.z = pow(-1, (rand() % 2 + 1)) * (rand() % 250 + 5);

    newBuilding.dim.x = 5 + 0.5 * pow(-1, (rand() % 2 + 1)) * (rand() % 4 + 1);
    newBuilding.dim.y = 7 + 2 * (rand() % 5 + 1);
    newBuilding.dim.z = 5 + 0.5 * pow(-1, (rand() % 2 + 1)) * (rand() % 4 + 1);

    newBuilding.r1 =  newBuilding.dim.x / 2; // inaltime / 2
    newBuilding.r2 =  newBuilding.dim.z / 2; // lungime / 2

    newBuilding.d = sqrt(newBuilding.dim.x * newBuilding.dim.x + newBuilding.dim.z * newBuilding.dim.z);

    buildings.emplace(buildings.end(), newBuilding);
}

void Tema2::PrintBuildings() {
    int i = 0;
    for (auto crtBuilding = buildings.begin(); crtBuilding != buildings.end(); ++crtBuilding) {
        dim[2 * i] = crtBuilding->dim.x / 2; dim[2 * i + 1] = crtBuilding->dim.z / 2;
        glm::mat4 modelMatrix2 = glm::mat4(1);
        modelMatrix2 = glm::translate(modelMatrix2, crtBuilding->coords);
        modelMatrix2 = glm::scale(modelMatrix2, crtBuilding->dim);
        RenderSimpleMesh(meshes["building"], shaders["VertexNormal"], modelMatrix2, glm::vec3(0.9f, 0.3f, 0.2f), 3, 2);
        i++;
    }
}

// pct 1 -> centru tank
// pct 2 -> centru opponent
// pct 3 -> vf pipe oppoenent
bool Tema2::VerifCol(float x1, float z1, float x2, float z2, float x3, float z3) {
    float detPoz = x1 * z2 + x3 * z1 + x2 * z3;
    float detNeg = x2 * z1 + x1 * z3 + x3 * z2;
    float det = detPoz - detNeg;

    // verif if the pipe is pointing towards the tank
    bool point = false;
    if (abs(x1 - x2) > abs(x1 - x3) && abs(z1 - z2) > abs(z1 - z3))
        point = true;

    if (abs(det) <= 0.2 && point)
        return true;
    return false;
}

bool Tema2::VerifPlaceTank(glm::vec2 coords) {
    for (auto crtBuilding = buildings.begin(); crtBuilding != buildings.end(); ++crtBuilding) {
        if (glm::distance(coords,
            glm::vec2(crtBuilding->coords.x, crtBuilding->coords.x)) < 6)
                return false;
    }

    return true;
}

void Tema2::BuildOpponents(glm::vec3 color, int serialNr) {
    TankStruct newTank;
    for (int i = 0; i < 1000; i++) {
        newTank.pozXTank = pow(-1, (rand() % 2 + 1)) * (rand() % 70 + 5);
        newTank.pozZTank = pow(-1, (rand() % 2 + 1)) * (rand() % 100 + 5);
        if (VerifPlaceTank(glm::vec2(newTank.pozXTank, newTank.pozZTank)))
            break;
    }
    newTank.rotate = rand() % 90 + 5;
    newTank.rotateTrl = 0;
    newTank.projTime = 0.2f;
    newTank.lives = 3;
    newTank.attack = false;
    newTank.color = color;
    newTank.serialNr = serialNr;
    newTank.instruction = rand() % 4 + 1; // 1 = w, 2 = s, 3 = a, 4 = d
    if (newTank.instruction >= 3) {
        newTank.instructionTime = 0.3 + (rand() % 5 + 1) * 0.1;
    }
    else {
        newTank.instructionTime = 4 + (rand() % 5 + 1) * 0.5;
    }

    opponents.emplace(opponents.end(), newTank);
}

void Tema2::MoveTanks(float deltaTimeSeconds){
    for (auto crtOpp = opponents.begin(); crtOpp != opponents.end(); ++crtOpp) {
        
        if (abs(crtOpp->pozXTank - tank.pozXTank) <= 15 && 
            abs(crtOpp->pozZTank - tank.pozZTank) <= 15)
            crtOpp->attack = true;
        else
            crtOpp->attack = false;

        if (crtOpp->lives > 0) {
            if (crtOpp->attack) {
                if(!VerifCol(tank.pozXTank, tank.pozZTank,
                        crtOpp->pozXTank, crtOpp->pozZTank,
                        crtOpp->pozXTank + 2 * cos(crtOpp->rotateTrl + crtOpp->rotate - 4.72),
                        crtOpp->pozZTank - 2 * sin(crtOpp->rotateTrl + crtOpp->rotate - 4.72))) 
                    crtOpp->rotateTrl += deltaTimeSeconds / 2;
                else {
                    crtOpp->projTime -= deltaTimeSeconds;
                    if (crtOpp->projTime <= 0) {
                        crtOpp->projTime = 3;
                        GenerateProj(crtOpp->pozXTank + 2 * cos(crtOpp->rotateTrl + crtOpp->rotate - 4.72),
                                     crtOpp->pozZTank - 2 * sin(crtOpp->rotateTrl + crtOpp->rotate - 4.72),
                                     crtOpp->rotate, crtOpp->rotateTrl, *crtOpp);
                    }
                }
                for (auto crtProj = crtOpp->projec.begin(); crtProj != crtOpp->projec.end(); ++crtProj) {
                    crtProj->time -= deltaTimeSeconds;
                    if (crtProj->time <= 0 || crtProj->alive == false) {
                        crtProj = crtOpp->projec.erase(crtProj);
                        if (crtProj == crtOpp->projec.end()) {
                            break;
                        }
                    }
                    crtProj->coords.x += (cos(crtProj->crtAngle) * deltaTimeSeconds * 3.5);
                    crtProj->coords.z -= (sin(crtProj->crtAngle) * deltaTimeSeconds * 3.5);
                    BuildProj(crtProj->coords.x, crtProj->coords.z);
                }
            }
            else {
                crtOpp->attack = false;
                float angleOpp = crtOpp->rotate + 89.5;
                if (crtOpp->instruction == 1) { // w
                    crtOpp->pozXTank += (cos(angleOpp) * deltaTimeSeconds * 2);
                    crtOpp->pozZTank -= (sin(angleOpp) * deltaTimeSeconds * 2);
                }
                if (crtOpp->instruction == 2) { // s
                    crtOpp->pozXTank -= (cos(angleOpp) * deltaTimeSeconds * 2);
                    crtOpp->pozZTank += (sin(angleOpp) * deltaTimeSeconds * 2);
                }
                if (crtOpp->instruction == 3) { // a
                    crtOpp->rotate += deltaTimeSeconds / 2;
                }
                if (crtOpp->instruction == 4) { // d
                    crtOpp->rotate -= deltaTimeSeconds / 2;
                }
                crtOpp->instructionTime -= deltaTimeSeconds;
                if (crtOpp->instructionTime <= 0) {
                    crtOpp->instruction = rand() % 3 + 1; // 1 = w, 2 = s, 3 = a, 4 = d
                    if (crtOpp->instruction >= 3) {
                        crtOpp->instructionTime = 0.5 + (rand() % 5 + 1) * 0.2;
                    }
                    else {
                        crtOpp->instructionTime = 1 + (rand() % 5 + 1) * 0.2;
                    }
                }
            }
        }
    }
}

void Tema2::PrintOpponents() {
    for (auto crtOpp = opponents.begin(); crtOpp != opponents.end(); ++crtOpp) {
        crtOpp->body = glm::mat4(1);
        crtOpp->body = glm::translate(crtOpp->body, glm::vec3(crtOpp->pozXTank, 0.55, crtOpp->pozZTank));
        crtOpp->body = glm::scale(crtOpp->body, glm::vec3(0.5f));
        crtOpp->body = glm::rotate(crtOpp->body, crtOpp->rotate, glm::vec3(0, 1, 0));
        RenderSimpleMesh(meshes["tank"], shaders["LabShader"], crtOpp->body, crtOpp->color, crtOpp->lives, 0);
        // glm::vec3((3 - max(0, crtOpp->lives)) * 0.1)
        crtOpp->turel = glm::mat4(1);
        crtOpp->turel = glm::translate(crtOpp->turel, glm::vec3(crtOpp->pozXTank, 1.3, crtOpp->pozZTank));
        crtOpp->turel = glm::scale(crtOpp->turel, glm::vec3(0.45f));
        crtOpp->turel = glm::rotate(crtOpp->turel, crtOpp->rotate + crtOpp->rotateTrl, glm::vec3(0, 1, 0));
        RenderSimpleMesh(meshes["turel"], shaders["LabShader"], crtOpp->turel, crtOpp->color, crtOpp->lives, 0);

        crtOpp->pipe = glm::mat4(1);
        crtOpp->pipe = glm::translate(crtOpp->pipe, glm::vec3(crtOpp->pozXTank, 1.6, crtOpp->pozZTank));
        crtOpp->pipe = glm::scale(crtOpp->pipe, glm::vec3(0.5f));
        crtOpp->pipe = glm::rotate(crtOpp->pipe, crtOpp->rotate + crtOpp->rotateTrl, glm::vec3(0, 1, 0));
        RenderSimpleMesh(meshes["pipe"], shaders["LabShader"], crtOpp->pipe, glm::vec3(1, 1, 1), crtOpp->lives, 0);

        crtOpp->weels = glm::mat4(1);
        crtOpp->weels = glm::translate(crtOpp->weels, glm::vec3(crtOpp->pozXTank, 0.55, crtOpp->pozZTank));
        crtOpp->weels = glm::scale(crtOpp->weels, glm::vec3(0.5f - 0.005 * (3 - max(0, crtOpp->lives))));
        crtOpp->weels = glm::rotate(crtOpp->weels, crtOpp->rotate, glm::vec3(0, 1, 0));
        RenderSimpleMesh(meshes["weels"], shaders["LabShader"], crtOpp->weels, glm::vec3(1, 1, 1), crtOpp->lives, 0);
    }
}

void Tema2::TankHit() {
    for (auto crtOpp = opponents.begin(); crtOpp != opponents.end(); ++crtOpp) {
        for (auto crtProj = crtOpp->projec.begin(); crtProj != crtOpp->projec.end(); ++crtProj) {
            /*glm::distance(glm::vec2(tank.pozXTank, tank.pozZTank),
            glm::vec2(crtProj->coords.x, crtProj->coords.z)*/
            if (abs(tank.pozXTank - crtProj->coords.x) <= 1 &&
                abs(tank.pozZTank - crtProj->coords.z) <= 1) {
                crtProj->alive = false;
                tank.lives--;
            }
        }
    }
}

void Tema2::ColizProjTank(float deltaTimeSeconds) {
    // verify collision Opponent tank - proj
    for (auto crtProj = tank.projec.begin(); crtProj != tank.projec.end(); ++crtProj) {
        bool ok = true;
        for (auto crtOpp = opponents.begin(); crtOpp != opponents.end(); ++crtOpp) {
            if (abs(crtOpp->pozXTank - crtProj->coords.x) <= 1.5 &&
                abs(crtOpp->pozZTank - crtProj->coords.z) <= 1.5) {
                crtProj->alive = false;
                crtOpp->lives--;
                ok = false;
            }
            if (!ok)
                break;
        }
        if (!ok)
            break;
    }

    for (auto crtProj = tank.projec.begin(); crtProj != tank.projec.end(); ++crtProj) {
        crtProj->time -= deltaTimeSeconds;
        if (crtProj->time <= 0 || crtProj->alive == false) {
            crtProj = tank.projec.erase(crtProj);
            if (crtProj == tank.projec.end()) {
                break;
            }
        }
        crtProj->coords.x += (cos(crtProj->crtAngle) * deltaTimeSeconds * 4);
        crtProj->coords.z -= (sin(crtProj->crtAngle) * deltaTimeSeconds * 4);
        BuildProj(crtProj->coords.x, crtProj->coords.z);
    }
}

void Tema2::ColizBuildProj() {
    // building - other ooponent's projectiles
    for (auto crtBuilding = buildings.begin(); crtBuilding != buildings.end(); ++crtBuilding) {
        for (auto crtOpp = opponents.begin(); crtOpp != opponents.end(); ++crtOpp) {
            for (auto crtProj = crtOpp->projec.begin(); crtProj != crtOpp->projec.end(); ++crtProj) {
                float dist = glm::distance(glm::vec2(crtProj->coords.x, crtProj->coords.z),
                    glm::vec2(crtBuilding->coords.x, crtBuilding->coords.z));
                if (dist < crtBuilding->d) {
                    crtProj->alive = false;
                }
            }
        }
    }

    // building - mvp projectiles
    for (auto crtBuilding = buildings.begin(); crtBuilding != buildings.end(); ++crtBuilding) {
        bool ok = true;
        for (auto crtProj = tank.projec.begin(); crtProj != tank.projec.end(); ++crtProj) {
            float dist = glm::distance(glm::vec2(crtProj->coords.x, crtProj->coords.z),
                glm::vec2(crtBuilding->coords.x, crtBuilding->coords.z));
            if (dist < crtBuilding->d) {
                crtProj->alive = false;
                ok = false;
                //tank.pozXTank -= dist * 0.01; tank.pozZTank -= dist * 0.01;
            }
            if (!ok) break;
        }
    }

}

void Tema2::ColizTankTank() {
    for (auto crtOpp = opponents.begin(); crtOpp != opponents.end(); ++crtOpp) {
        float dist = glm::distance(glm::vec3(tank.pozXTank, 1, tank.pozZTank),
            glm::vec3(crtOpp->pozXTank, 1, crtOpp->pozZTank));
        if (abs(tank.pozXTank - crtOpp->pozXTank) <= 4 && 
            abs(tank.pozZTank - crtOpp->pozZTank) <= 4 ) {
            glm::vec3 P = glm::vec3(4.2 - abs(tank.pozXTank - crtOpp->pozXTank), 1, 3.7 - abs(tank.pozZTank - crtOpp->pozZTank));
            glm::vec3 Dif = glm::vec3(crtOpp->pozXTank - tank.pozXTank, 0,
                crtOpp->pozZTank - tank.pozZTank);
            P = abs(P) * glm::normalize(Dif);
            tank.pozXTank += P.x * (-0.1);
            tank.pozZTank += P.z * (-0.1);
            crtOpp->pozXTank += P.x * 0.1;
            crtOpp->pozZTank += P.z * 0.1;
        }
    }

    for (auto crtOpp1 = opponents.begin(); crtOpp1 != opponents.end(); ++crtOpp1) {
        for (auto crtOpp2 = opponents.begin(); crtOpp2 != opponents.end(); ++crtOpp2) {
            if (crtOpp1->serialNr != crtOpp2->serialNr) {
                if (abs(crtOpp1->pozXTank - crtOpp2->pozXTank) <= 4 &&
                    abs(crtOpp1->pozZTank - crtOpp2->pozZTank) <= 4) {
                    glm::vec3 P = glm::vec3(4.2 - abs(crtOpp1->pozXTank - crtOpp2->pozXTank), 1, 
                                            3.7 - abs(crtOpp1->pozZTank - crtOpp2->pozZTank));
                    glm::vec3 Dif = glm::vec3(crtOpp1->pozXTank - crtOpp1->pozXTank, 0,
                                              crtOpp1->pozZTank - crtOpp1->pozZTank);
                    P = abs(P) * glm::normalize(Dif);
                    crtOpp1->pozXTank += P.x * (-0.1);
                    crtOpp1->pozZTank += P.z * (-0.1);
                    crtOpp2->pozXTank += P.x * 0.1;
                    crtOpp2->pozZTank += P.z * 0.1;
                }
            }
        }
    }


}

void Tema2::ColizBuildTank() {

    for (auto crtBuilding = buildings.begin(); crtBuilding != buildings.end(); ++crtBuilding) {
        float r = 0, dist = 0;
        
        if (tank.pozXTank >= (crtBuilding->coords.x - crtBuilding->dim.x / 2) - 2 &&
            tank.pozXTank <= (crtBuilding->coords.x + crtBuilding->dim.x / 2) + 2) {
            r = 6.2 + crtBuilding->r2;
            dist = abs(tank.pozZTank - crtBuilding->coords.z);
        } 
        if (tank.pozZTank >= (crtBuilding->coords.z - crtBuilding->dim.z / 2) - 2 &&
            tank.pozZTank <= (crtBuilding->coords.z + crtBuilding->dim.z / 2) + 2) {
            r = 6.2 + crtBuilding->r1;
            dist = abs(tank.pozXTank - crtBuilding->coords.x);
        }

        if (dist < r && r != 0 && dist != 0) {
            glm::vec3 Dif = glm::vec3(crtBuilding->coords.x - tank.pozXTank, 0,
                                      crtBuilding->coords.z - tank.pozZTank);
            glm::vec3 P = (r - dist) * glm::normalize(Dif);
            tank.pozXTank += P.x * (-0.5);
            tank.pozZTank += P.z * (-0.5);
        }
    }

    for (auto crtBuilding = buildings.begin(); crtBuilding != buildings.end(); ++crtBuilding) {
        for (auto crtOpp = opponents.begin(); crtOpp != opponents.end(); ++crtOpp) {
            float r = 0, dist = 0;

            if (crtOpp->pozXTank >= (crtBuilding->coords.x - crtBuilding->dim.x / 2) - 3 &&
                tank.pozXTank <= (crtBuilding->coords.x + crtBuilding->dim.x / 2) + 3) {
                r = 6.2 + crtBuilding->r2;
                dist = abs(crtOpp->pozZTank - crtBuilding->coords.z);
            }
            if (crtOpp->pozZTank >= (crtBuilding->coords.z - crtBuilding->dim.z / 2) - 3 &&
                crtOpp->pozZTank <= (crtBuilding->coords.z + crtBuilding->dim.z / 2) + 3) {
                r = 6.2 + crtBuilding->r1;
                dist = abs(crtOpp->pozXTank - crtBuilding->coords.x);
            }

            if (dist < r && r != 0 && dist != 0) {
                glm::vec3 Dif = glm::vec3(crtBuilding->coords.x - crtOpp->pozXTank, 0,
                    crtBuilding->coords.z - crtOpp->pozZTank);
                glm::vec3 P = (r - dist) * glm::normalize(Dif);
                crtOpp->pozXTank += P.x * (-0.5);
                crtOpp->pozZTank += P.z * (-0.5);
            }
        }
    }
}

int Tema2::CalcFinalPoints() {
    int points = 0;
    for (auto crtOpp = opponents.begin(); crtOpp != opponents.end(); ++crtOpp) {
        if (crtOpp->lives <= 0) points++;
    }
    return points;
}

void Tema2::Update(float deltaTimeSeconds)
{   
    camera->SetPos(glm::vec3(tank.pozXTank, 3, tank.pozZTank));
    camera->MoveForward(-4.5f);
    if (!pause) {
        if (first) {
            for (int i = 0; i < 100; i++) {
                BuildBuildings();
            }
            for (int i = 0; i < 35; i++) {
                float red = 0.7 + 0.05 * pow(-1, (rand() % 2 + 1)) * (rand() % 4 + 1);
                float green = 0.2 + 0.03 * pow(-1, (rand() % 2 + 1)) * (rand() % 6);
                float blue = 0.7 + 0.05 * pow(-1, (rand() % 2 + 1)) * (rand() % 3 + 1);
                BuildOpponents(glm::vec3(red, green, blue), i);
            }
            first = false;
        }
        PrintBuildings();
        PrintOpponents();
    }

    InitBoard();

    if (!pause)
        time -= deltaTimeSeconds;
    if (time <= 0) {
        if (end) {
            printf("Jocul s-a terminat! Ai acumulat %d puncte!\n", CalcFinalPoints());
            end = false;
        }
    }
    if (time > 0 && !pause) {
        randProj -= deltaTimeSeconds;
        // building the buildings and the opponent tanks

        MoveTanks(deltaTimeSeconds);

        // shoot proj
        if (randProj <= 0 && window->MouseHold(GLFW_MOUSE_BUTTON_LEFT)) {
            randProj = 2.5f;
            float angleCrt = glm::angle(glm::vec3(0, 0, scale), glm::vec3(0, 0, pozCrtTurel));
            GenerateProj(tank.pozXTank + 2 * cos(pozCrtTurel + scale - 4.72),
                tank.pozZTank - 2 * sin(pozCrtTurel + scale - 4.72),
                scale, pozCrtTurel, tank);
        }

        TankHit();
        ColizProjTank(deltaTimeSeconds);
        ColizBuildProj();
        ColizTankTank();
        ColizBuildTank();

        if (tank.lives <= 0) {
            printf("Ai murit! :(\n");
            exit(0);
        }
    }
}

void Tema2::FrameEnd()
{
    //DrawCoordinateSystem();
}

void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, const int& HP, const int& MVP)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // Set shader uniforms for light & material properties
    // TODO(student): Set light position uniform
    GLint loc_light = glGetUniformLocation(shader->program, "light_position");
    glUniform3fv(shader->loc_light_pos, 1, glm::value_ptr(lightPosition));
    glUniform3fv(shader->loc_light_pos, 1, glm::value_ptr(lightPositionTank));

    glm::vec3 eyePosition = camera->position;

    // TODO(student): Set eye position (camera position) uniform
    GLint loc_eye = glGetUniformLocation(shader->program, "eye_position");
    glUniform3fv(shader->loc_eye_pos, 1, glm::value_ptr(eyePosition));

    glUniform1i(glGetUniformLocation(shader->program, "hp"), HP);
    glUniform1i(glGetUniformLocation(shader->program, "mvp"), MVP);

    // TODO(student): Set material property uniforms (shininess, kd, ks, object color)
    GLint loc_shininess = glGetUniformLocation(shader->program, "material_shininess");
    glUniform1i(loc_shininess, materialShininess);
    GLint loc_kd = glGetUniformLocation(shader->program, "material_kd");
    glUniform1f(loc_kd, materialKd);
    GLint loc_ks = glGetUniformLocation(shader->program, "material_ks");
    glUniform1f(loc_ks, materialKs);
    GLint loc_color = glGetUniformLocation(shader->program, "object_color");
    glUniform3fv(loc_color, 1, glm::value_ptr(color));

    // Bind model matrix
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Bind view matrix
    glm::mat4 viewMatrix = camera->GetViewMatrix();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    float speed = 4.0f;
    addition = 0;

    if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT) && time >= 0  && !pause) {
        anglCamera = 0;
        if (window->KeyHold(GLFW_KEY_W) && aPressed == false && dPressed == false) {
            tank.pozXTank += (cos(angleTank) * deltaTime * speed);
            tank.pozZTank -= (sin(angleTank) * deltaTime * speed);
            lightPositionTank.x += (cos(angleTank) * deltaTime * speed);
            lightPositionTank.z -= (sin(angleTank) * deltaTime * speed);
        }
        if (window->KeyHold(GLFW_KEY_S) && aPressed == false && dPressed == false) {
            tank.pozXTank -= (cos(angleTank) * deltaTime * speed);
            tank.pozZTank += (sin(angleTank) * deltaTime * speed);
            lightPositionTank.x -= (cos(angleTank) * deltaTime * speed);
            lightPositionTank.z += (sin(angleTank) * deltaTime * speed);
        }
        if (window->KeyHold(GLFW_KEY_A)) {
            angleTank += deltaTime;
            anglCamera += deltaTime;
            scale += deltaTime;
            camera->RotateThirdPerson_OY(deltaTime);
            aPressed = true;
        }
        if (window->KeyHold(GLFW_KEY_D)) {
            angleTank -= deltaTime;
            anglCamera -= deltaTime;
            scale -= deltaTime;
            camera->RotateThirdPerson_OY(-deltaTime);
            dPressed = true;
        }
        if (!window->KeyHold(GLFW_KEY_A)) aPressed = false;
        if (!window->KeyHold(GLFW_KEY_D)) dPressed = false;
    }
}


void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (window->KeyHold(GLFW_KEY_P)) {
        isPPressed = true;
        if (nrPause == 4)
            nrPause = 1;
        if (nrPause == 2)
            nrPause = 3;
        if (pause = false)
            pause = true;
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
    if (isPPressed) {
        if (!window->KeyHold(GLFW_KEY_P) && nrPause == 1) {
            nrPause = 2;
            pause = true;
        }
        if (!window->KeyHold(GLFW_KEY_P) && nrPause == 3) {
            nrPause = 4;
            pause = false;
        }
        isPPressed = false;
    }
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
    myX = mouseX;
    myY = window->GetResolution().y - mouseY;

    if (!pause) {   
        if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
            float sensivityOX = 0.001f;
            float sensivityOY = 0.001f;
            pozCrtTurel -= 5 * 0.001f * deltaX;
            camera->RotateThirdPerson_OY(-goBackToCameraPos);
            goBackToCameraPos = 0;
        }

        if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
            float sensivityOX = 0.001f;
            float sensivityOY = 0.001f;
            goBackToCameraPos += -sensivityOY * deltaX;
            camera->RotateThirdPerson_OY(-sensivityOY * deltaX);

        }
    }
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
        rghButtonPressed = true;
    }
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
    if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
       rghButtonPressed = false;
    }
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}
