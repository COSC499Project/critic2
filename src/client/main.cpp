// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "matrix_math.cpp"

struct {
  bool LeftMouseButton = 0;
  bool RightMouseButton = 0;
  bool MiddleMouseButton = 0;
  double ScrollYOffset = 0.f;
  double MPosX = 0.f;
  double MPosY = 0.f;
  double lastX = 0.f;
  double lastY = 0.f;
  double diffX = 0.f;
  double diffY = 0.f;
  bool ShiftKey = 0;
  bool CtrlKey = 0;
  bool AltKey = 0;
} input;

static CameraInfo cam;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

#pragma region shaders

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(0);
	}

	const GLchar * p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}
	glAttachShader(ShaderProgram, ShaderObj);
}

static GLuint LightingShader() {
	GLuint ShaderProgram = glCreateProgram();
	if (ShaderProgram == 0) {
		exit(1);
	}

	const char * vs = "#version 400 \n \
		varying vec3 N; \
		varying vec3 v; \
		void main(void){ \
		v = vec3(gl_ModelViewMatrix * gl_Vertex); \
		N = normalize(gl_NormalMatrix * gl_Normal); \
		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \
		}";


	const char * fs = "#version 400 \n \
		varying vec3 N;\
		varying vec3 v;\
		void main(void){ \
		vec3 L = normalize(gl_LightSource[0].position.xyz - v);\
		vec3 E = normalize(-v); // we are in Eye Coordinates, so EyePos is (0,0,0)\
		vec3 R = normalize(-reflect(L, N));\
		//calculate Ambient Term: \
		vec4 Iamb = gl_FrontLightProduct[0].ambient; \
		//calculate Diffuse Term:\
		vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N, L), 0.0); \
		// calculate Specular Term: \
		vec4 Ispec = gl_FrontLightProduct[0].specular \
			* pow(max(dot(R, E), 0.0), 0.3*gl_FrontMaterial.shininess); \
		// write Total Color:\
		gl_FragColor = gl_FrontLightModelProduct.sceneColor + Iamb + Idiff + Ispec;\
		}";

	AddShader(ShaderProgram, vs, GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs, GL_FRAGMENT_SHADER);

	GLint success = 0;

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
	if (success == 0) exit(1);

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &success);
	if (success == 0) exit(1);

	return ShaderProgram;
}

static GLuint CompileShaders()
{
	GLuint ShaderProgram = glCreateProgram();
	if (ShaderProgram == 0) {
		exit(1);
	}


	const char * vs = "#version 330 \n \
      layout (location = 0) in vec3 Position; \n \
      layout (location = 1) in vec3 Normal; \n \
      uniform mat4 gWorld; \n \
      uniform vec4 mColor; \n \
      out vec4 Color; \n \
      out vec3 Normal0; \n \
      void main() { \n \
        gl_Position = gWorld * vec4(Position, 1.0); \n \
        Normal0 = (gWorld * vec4(Normal, 0.0)).xyz; \n \
        Color = mColor;}";

	const char * fs = "#version 330 \n \
      in vec4 Color; \n \
      in vec3 Normal0; \n \
      vec3 Direction = vec3(0.0, 0.0, 1.0);\n \
      float DiffuseFactor = dot(normalize(Normal0), Direction); \n \
      out vec4 FragColor; \n \
      vec4 DiffuseColor; \n \
      float DiffuseIntensity = 1.0; \n \
      void main() { \n \
        if (DiffuseFactor > 0) { \n \
          DiffuseColor = vec4(Color * DiffuseFactor * DiffuseIntensity); \n \
        } else { \n \
          DiffuseColor = vec4(0, 0, 0, 0);} \n \
        FragColor = Color; }";


	AddShader(ShaderProgram, vs, GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs, GL_FRAGMENT_SHADER);

	GLint success = 0;

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
	if (success == 0) exit(1);

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &success);
	if (success == 0) exit(1);

	return ShaderProgram;
}

#pragma endregion

void CreateAndFillBuffers(GLuint * VertexBuffer, GLuint * IndexBuffer, 
                          GLfloat * Vertices, unsigned int * Indices,
                          unsigned int NumVertices, unsigned int NumIndices)
{
  glGenBuffers(1, VertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, *VertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, NumVertices*sizeof(GLfloat), Vertices, GL_STATIC_DRAW);

  glGenBuffers(1, IndexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumIndices*sizeof(unsigned int), Indices, GL_STATIC_DRAW);

}

void ScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
  cam.Pos[2] += yoffset * 0.5f;
}

void DrawBond(GLuint WorldLocation, GLuint ColorLocation, Pipeline * p, 
              GLuint CylVB, GLuint CylIB,
              const float p1[3], const float p2[3])
{
  float df[3] = {p2[0]-p1[0], p2[1]-p1[1], p2[2]-p1[2]};
  float d = sqrt(df[0]*df[0] + df[1]*df[1] + df[2]*df[2]);
  float mid[3] = {(p1[0]+p2[0])/2, (p1[1]+p2[1])/2, (p1[2]+p2[2])/2};
  float grey[3] = {.5, .5, .5};


  p->Scale(0.1f, 0.1f, d);
  p->Translate(mid[0], mid[1], mid[2]); 
  p->Rotate(0.f, 0.f, 0.f);

  glUniformMatrix4fv(WorldLocation, 1, GL_TRUE, (const GLfloat *)p->GetTrans());
  glBindBuffer(GL_ARRAY_BUFFER, CylVB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CylIB);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glUniform4fv(ColorLocation, 1, (const GLfloat *)&grey);
  glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, 0);
}

#pragma region atom loading and drawing
//global vars for an atom's mesh
GLuint atomVB;
GLuint atomIB;
unsigned int numbIndeces;

void loadAtomObject() {
	// Load sphere mesh
	unsigned int numbVerteces = 3078;
	numbIndeces = 6144;
	static GLfloat *atomVerteces = (GLfloat *)malloc(sizeof(GLfloat)*numbVerteces);
	static unsigned int *atomIndeces = (unsigned int *)malloc(sizeof(unsigned int)*numbIndeces);
	ReadMesh(atomVerteces, atomIndeces, "./sphere.v", "./sphere.i");
	CreateAndFillBuffers(&atomVB, &atomIB, atomVerteces, atomIndeces,
		numbVerteces, numbIndeces);
}

struct atom{
	bool selected = false;
	int atomicNumber;
	int indentifyingNumber;
	float* atomPosition = new float[3];
};

int loadedAtomsAmount;
atom *loadedAtoms;

//TODO call this to load all atoms from the critic2 interface
void loadAtoms() {
	
}

///returns the color of an atom based on the atomic number
///and desired color Intesity (brightness)
const GLfloat* getAtomColor(int atomicNumber,float colorIntesity) {
	if (atomicNumber == 1) {
		return new GLfloat[4]{ 1.f, 1.f, 1.f, colorIntesity}; //white 
	}else if(atomicNumber == 8) {
		return new GLfloat[4]{ 1.0f,0.0f, 0.0f, colorIntesity }; //red
	} else  {
		return new GLfloat[4]{ 0.8f,0.8f, 0.8f, colorIntesity }; //brown
	}
}

///will be used to draw atom number over the atom using imgui window
float* getScreenPositionOfVertex(float *vertexLocation) {
	//TODO transfrom from vertex location to screen location 
	return NULL;
}

GLuint gWorldLocation; //made global to make Drawing via methods easer
GLuint mColorLocation;
void drawAtomInstance(int identifyer, float * posVector,const GLfloat color[4], Pipeline p) {
	float inc = 1.f;
	if (loadedAtoms[identifyer].selected) { //selection is color based
		inc = .5f;
	}
	const GLfloat n_Color[4]{color[0] * inc,color[1] * inc,color[2] * inc,color[3]};

	float scaleAmount = (float)loadedAtoms[identifyer].atomicNumber;
	if (scaleAmount < 4.0f) {
		scaleAmount = 0.25f;
	} else {
		scaleAmount = 0.5f;
	}

	p.Scale(scaleAmount, scaleAmount, scaleAmount);
	p.Translate(posVector[0], posVector[1], posVector[2]);
	p.Rotate(0.f, 0.f, 0.f); //no rotation required
	glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat *)p.GetTrans());

	glBindBuffer(GL_ARRAY_BUFFER, atomVB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, atomIB);
	glUniform4fv(mColorLocation, 1, (const GLfloat *)&n_Color);
	glDrawElements(GL_TRIANGLES, numbIndeces, GL_UNSIGNED_INT, 0);
	
}

///draws all atoms in the loadedAtoms struct
void drawAllAtoms(Pipeline p) {
	for (size_t x = 0; x < loadedAtomsAmount; x++){
		drawAtomInstance(x,loadedAtoms[x].atomPosition,getAtomColor(loadedAtoms[x].atomicNumber, 0.6f),p);
	}
}

#pragma endregion

#pragma region IMGUI


void drawTreeViewer() {
	//TODO tree nodes
	for (size_t x = 0; x < loadedAtomsAmount; x++){
		string nodeName = "Atomic #:";		
		nodeName += to_string(loadedAtoms[x].atomicNumber);
		nodeName += "  ID: ";
		nodeName += to_string(loadedAtoms[x].indentifyingNumber);
		if (ImGui::TreeNode(nodeName.c_str())) {
			loadedAtoms[x].selected = true;
			ImGui::TreePop();
		}else{
			loadedAtoms[x].selected = false;
		}
	}

}



#pragma endregion


int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Critic2", NULL, NULL);
    glfwMakeContextCurrent(window);
    gl3wInit();
    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

    // Event callbacks
    glfwSetScrollCallback(window, ScrollCallback);
    
    
    //Setup up OpenGL stuff
    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    GLuint trishader = CompileShaders();
    gWorldLocation = glGetUniformLocation(trishader, "gWorld");
    mColorLocation = glGetUniformLocation(trishader, "mColor");
    
	//glEnables
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // initialize pipeline
    Pipeline p;

    //define some colors
	float colorIntesity = 0.6f;
    GLfloat color[4] = {0.f, 0.f, 0.f, 1.f};
    const GLfloat red[4] = {1.f, 0.f, 0.f, colorIntesity};
    const GLfloat green[4] = {0.f, 1.f, 0.f, colorIntesity};
    const GLfloat blue[4] = {0.f, 0.f, 1.f, colorIntesity};
    const GLfloat black[4] = {0.f, 0.f, 0.f, colorIntesity};
    const GLfloat white[4] = {1.f, 1.f, 1.f, colorIntesity};
    const GLfloat grey[4] = {.5f, .5f, .5f, colorIntesity};

	//load all atom information ---------------------------------------------
	loadAtomObject();
	
	//this section will be replaced by the loadAtoms function
#pragma region atom loading test
	loadedAtomsAmount = 3;
	loadedAtoms = new atom[loadedAtomsAmount];
	loadedAtoms[0].atomicNumber = 1;
	loadedAtoms[0].indentifyingNumber = 1;
	loadedAtoms[0].atomPosition[0] = 0.f;
	loadedAtoms[0].atomPosition[1] = -1.f;
	loadedAtoms[0].atomPosition[2] = 0.f;


	loadedAtoms[1].atomicNumber = 1;
	loadedAtoms[1].indentifyingNumber = 2;
	loadedAtoms[1].atomPosition[0] = -1.29f;
	loadedAtoms[1].atomPosition[1] = 1.16f;
	loadedAtoms[1].atomPosition[2] = 0.f;


	loadedAtoms[2].atomicNumber = 8;
	loadedAtoms[2].indentifyingNumber = 3;
	loadedAtoms[2].atomPosition[0] = 0.f;
	loadedAtoms[2].atomPosition[1] = .715f;
	loadedAtoms[2].atomPosition[2] = 0.f;

#pragma endregion

    // Load sphere mesh
    GLuint SphereIB;
    GLuint SphereVB;
    unsigned int SphereNumV = 3078;
    unsigned int SphereNumI = 6144;
    static GLfloat * SphereV = (GLfloat *) malloc(sizeof(GLfloat)*SphereNumV);
    static unsigned int * SphereI = (unsigned int *) malloc(sizeof(unsigned int)*SphereNumI);
    ReadMesh(SphereV, SphereI, "./sphere.v", "./sphere.i");
    CreateAndFillBuffers(&SphereVB, &SphereIB, SphereV, SphereI, 
                         SphereNumV, SphereNumI);

    // Load cylinder mesh
    GLuint CylIB;
    GLuint CylVB;
    int CylNumV = 240;
    int CylNumI = 240;
    static GLfloat * CylV = (GLfloat *) malloc(sizeof(GLfloat)*CylNumV);
    static unsigned int * CylI = (unsigned int *) malloc(sizeof(unsigned int)*CylNumI);
    ReadMesh(CylV, CylI, "./cylinder.v", "./cylinder.i");
    CreateAndFillBuffers(&CylVB, &CylIB, CylV, CylI, CylNumV, CylNumI);

    // input variables;
    // c means for current loop, l means last loop
    static int cLMB;
    static int cRMB;
    static int lLMB;
    static int lRMB;
    static double cMPosX;
    static double cMPosY;
    static double lMPosX;
    static double lMPosY;
    static double scrollY;

    cam.Pos[0] = 0.f; cam.Pos[1] = 0.f; cam.Pos[2] = -3.f;
    cam.Target[0] = 0.f; cam.Target[1] = 0.f; cam.Target[2] = 1.f;
    cam.Up[0] = 0.f; cam.Up[1] = 1.f; cam.Up[2] = 0.f;


    bool show_test_window = true;
    // Main loop ------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        ImGuiIO& io = ImGui::GetIO();
	

		drawTreeViewer();

        // get input
        lLMB = cLMB;
        lRMB = cRMB;
        cLMB = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        cRMB = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        lMPosX = cMPosX;
        lMPosY = cMPosY;
        glfwGetCursorPos(window, &cMPosX, &cMPosY);


        float camPanFactor = 0.008f;
        float camZoomFactor = 1.f;
        if (!io.WantCaptureMouse) {
          if (cLMB == GLFW_PRESS){
            cam.Pos[0] += camPanFactor * (cMPosX - lMPosX);
            cam.Pos[1] += camPanFactor * (cMPosY - lMPosY);
          }
        }

        /*
        static float sx=1.f, sy=1.f, sz=1.f;
        static float sf = 0.5f;
        static bool lockScale = true;
        static float rx = 0.f, ry = 0.f, rz = 0.f;
        static float tx = 0.5f, ty = 0.f, tz = 0.f;
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Checkbox("Lock Scale Ratio", &lockScale);
            if(lockScale){
              ImGui::DragFloat("Scale", &sf, 0.005f);
            } else {
              ImGui::DragFloat("Scale X", &sx, 0.005f);
              ImGui::DragFloat("Scale Y", &sy, 0.005f);
              ImGui::DragFloat("Scale Z", &sz, 0.005f);
            }
            ImGui::DragFloat("Rotate X", &rx, 0.5f);
            ImGui::DragFloat("Rotate Y", &ry, 0.5f);
            ImGui::DragFloat("Rotate Z", &rz, 0.5f);

            ImGui::DragFloat("Translate X", &tx, 0.005f);
            ImGui::DragFloat("Translate Y", &ty, 0.005f);
            ImGui::DragFloat("Translate Z", &tz, 0.005f);

            ImGui::DragFloat("Translate CamX", &camPos[0], 0.005f);
            ImGui::DragFloat("Translate CamY", &camPos[1], 0.005f);
            ImGui::DragFloat("Translate CamZ", &camPos[2], 0.005f);
  
            ImGui::DragFloat("CamTargetX", &camTarget[0], 0.005f);
            ImGui::DragFloat("CamTargetY", &camTarget[1], 0.005f);
            ImGui::DragFloat("CamTargetZ", &camTarget[2], 0.005f);

            ImGui::DragFloat("CamUpX", &camUp[0], 0.005f);
            ImGui::DragFloat("CamUpY", &camUp[1], 0.005f);
            ImGui::DragFloat("CamUpZ", &camUp[2], 0.005f);

        }
*/

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow
        if (!show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }


        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        p.SetPersProjInfo(45, 500, 500, 1.f, 1000.f);
        p.SetOrthoProjInfo(-10.f, 10.f, -10.f, 10.f, -1000.f, 1000.f);
        p.SetCamera(cam);

    
        glEnableVertexAttribArray(0);
		drawAllAtoms(p);
		/* old atom drawing
        p.Scale(0.25f, 0.25f, 0.25f);
        p.Translate(0.f, -1.f, 0.f); 
        p.Rotate(0.f, 0.f, 0.f);
        glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat *)p.GetTrans());

        glBindBuffer(GL_ARRAY_BUFFER, SphereVB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereIB);
        glUniform4fv(mColorLocation, 1, (const GLfloat *)&white);
        glDrawElements(GL_TRIANGLES, SphereNumI, GL_UNSIGNED_INT, 0);

        p.Scale(0.25f, 0.25f, 0.25f);
        p.Translate(-1.29, 1.16, 0.f); 
        p.Rotate(0.f, 0.f, 0.f);
        glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat *)p.GetTrans());

        glBindBuffer(GL_ARRAY_BUFFER, SphereVB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereIB);
        glUniform4fv(mColorLocation, 1, (const GLfloat *)&white);
        glDrawElements(GL_TRIANGLES, SphereNumI, GL_UNSIGNED_INT, 0);


        p.Scale(0.5f, 0.5f, 0.5f);
        p.Translate(0.f, .715, 0.f); 
        p.Rotate(0.f, 0.f, 0.f);
        glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat *)p.GetTrans());

        glBindBuffer(GL_ARRAY_BUFFER, SphereVB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereIB);
        glUniform4fv(mColorLocation, 1, (const GLfloat *)&red);
        glDrawElements(GL_TRIANGLES, SphereNumI, GL_UNSIGNED_INT, 0);
		*/

        p.Scale(0.1f, 0.1f, .5f);
        p.Translate(0.f, -.275, 0.f); 
        p.Rotate(-90.f, 0.f, 0.f);
        glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat *)p.GetTrans());

        glBindBuffer(GL_ARRAY_BUFFER, CylVB);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CylIB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glUniform4fv(mColorLocation, 1, (const GLfloat *)&grey);
        glDrawElements(GL_TRIANGLES, CylNumI, GL_UNSIGNED_INT, 0);

        p.Scale(0.1f, 0.1f, .5f);
        p.Translate(-.7, 1, 0); 
        p.Rotate(90, 0, 75);
        glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat *)p.GetTrans());

        glBindBuffer(GL_ARRAY_BUFFER, CylVB);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CylIB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glUniform4fv(mColorLocation, 1, (const GLfloat *)&grey);
        glDrawElements(GL_TRIANGLES, CylNumI, GL_UNSIGNED_INT, 0);


        const float p1[3] = {-1, 2, 0};
        const float p2[3] = {1, 2, 0};
//        DrawBond(gWorldLocation, mColorLocation, &p, 
  //               CylVB, CylIB, p1, p2);



        glDisableVertexAttribArray(0);
        
        glUseProgram(trishader);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        ImGui::Render();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
