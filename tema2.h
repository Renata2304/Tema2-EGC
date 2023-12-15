#pragma once

#include "components/simple_scene.h"
#include "lab_m1/tema2/camera.h"


namespace m1
{   
    struct Proj {
        glm::vec3 coords;
        float alive = true;
        float time;
        float crtAngle;
    };
    struct TankStruct {
        glm::mat4 body;  // body
        glm::mat4 turel; // head
        glm::mat4 weels; // weels
        glm::mat4 pipe;  // pipe
        float pozXTank = 0, pozZTank = 0;
        float rotate, rotateTrl;
        float instructionTime;
        int instruction;
        float projTime;
        int lives;
        bool attack;
        glm::vec3 color;

        std::vector<Proj> projec;

        int serialNr;
    };
    
    struct BuildingStruct {
        glm::vec3 coords;
        glm::vec3 dim;
        int type;
        float r1, r2, d;
    };

    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

        void Init() override;

    private:
        
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, const int& HP, const int& MVP);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void InitBoard();
        void GenerateProj(float pozX, float pozZ, float tankAngle, float turelAngle, TankStruct &tankCrt);
        void BuildProj(float x, float z);
        void BuildBuildings();
        void PrintBuildings();
        void BuildOpponents(glm::vec3 color, int serialNr);
        void MoveTanks(float deltaTimeSeconds);
        void PrintOpponents();
        void TankHit();

        bool VerifCol(float x1, float z1, float x2, float z2, float x3, float z3);
        bool VerifPlaceTank(glm::vec2 coords);

        void ColizBuildProj();
        void ColizTankTank();
        void ColizBuildTank();
        void ColizProjTank(float deltaTimeSeconds);

        int CalcFinalPoints();
        
        glm::vec3 lightPosition;
        glm::vec3 lightPositionTank;
        unsigned int materialShininess;
        float materialKd;
        float materialKs;

    protected:
        TankStruct tank;
        bool wPressed, aPressed, sPressed, dPressed;
        float scale = 0;
        float angleTank = 89.5f;
        float anglCamera = 0;
        

        implemented::Camera* camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;
        float left, right, bottom, top, fov;
        bool projectionOrtho;

        glm::vec3 pozCamera;
        /*float angleCamera = 0.0f;*/

        bool rghButtonPressed = false;
        float myX, myY;
        float pozXturel = 0.5f, pozYturel = 0.0f, pozCrtTurel = 0.0f;

        
        float randProj;
        float timeMove;
        float addition;
        float move = 0;

        float startGX, startGZ;

        glm::vec2 randPosOpp[10];
        float randRotate[10];
        std::vector<TankStruct> opponents;

        float goBackToCameraPos;

        bool first = true, end = true;

        float time = 90.0f;

        std::vector<BuildingStruct> buildings;

        bool pause = false;
        bool isPPressed = false;
        int nrPause = 0;

        float dim[205];
    };
}
