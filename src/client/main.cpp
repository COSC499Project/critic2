// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <synchapi.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "matrix_math.cpp"
#include "tinyfiledialogs.h"

extern "C" void initialize();
extern "C" void init_struct();
extern "C" void call_structure(const char *filename, int size, int isMolecule);
extern "C" void get_num_atoms(int *n);
extern "C" void get_atom_position(int n, int *atomicN, double *x, double *y, double *z);
extern "C" void num_of_bonds(int n, int *nstarN);
extern "C" void get_atom_bond(int n_atom, int nstarIdx, int *connected_atom);

static void ShowAppMainMenuBar();
static void ShowMenuFile();

string charConverter(float t) {
	char buffer[64];
	int len = sprintf(buffer, "%f", t);
	string val = buffer;
	return val;
}

string charConverter(size_t t) {
	char buffer[64];
	int len = sprintf(buffer, "%d", t);
	string val = buffer;
	return val;
}

string charConverter(int t) {
	char buffer[64];
	int len = sprintf(buffer, "%d", t);
	string val = buffer;
	return val;
}

//
//  Global Variables and Structs
//
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

struct {
  GLuint gWorldLocation;
  GLuint gWVPLocation;
  GLuint vColorLocation;
  GLuint lColorLocation;
  GLuint lDirectionLocation;
  GLuint fAmbientIntensityLocation;
} ShaderVarLocations;

struct atom{
	string name = "";
	bool selected = false;
	int atomicNumber;
	float atomPosition[3];
	string atomTreeName; //must be saved to preserve imgui tree Id's
  int numberOfBonds;
  int * loadedBonds;
  int atomTreePosition;
};

struct bond{
    atom * a1;
    atom * a2;
    Vector3f center;
    Matrix4f rotation;
    float length;
};

struct criticalPoint{
	float location[3];
	string pointName;
	int pointType;
};

bond * Bonds;
atom * loadedAtoms;
int loadedAtomsAmount = 0;
int loadedBondsAmount = 0;

criticalPoint * loadedCPoints;
int loadedCPointsAmount;


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

static GLuint LightingShader()
{

  GLuint ShaderProgram = glCreateProgram();
  if (ShaderProgram == 0){
    exit(1);
  }

  const char * vs = "#version 330 \n \
    uniform mat4 gWorld; \n \
    uniform mat4 gWVP; \n \
    layout (location = 0) in vec3 inPosition; \n \
    layout (location = 1) in vec3 inNormal; \n \
    smooth out vec3 vNormal; \n \
    void main() { \n \
      gl_Position = gWVP * vec4(inPosition, 1.0); \n \
      vNormal = (gWorld * vec4(inNormal, 0.0)).xyz; \n \
      }";

  const char * fs = "#version 330 \n \
    smooth in vec3 vNormal; \n \
    uniform vec4 vColor; \n \
    out vec4 outputColor; \n \
    uniform vec3 lColor; \n \
    uniform vec3 lDirection; \n \
    uniform float fAmbientIntensity; \n \
    void main() { \n \
      float fDiffuseIntensity = max(0.0, dot(normalize(vNormal), lDirection)); \n \
      outputColor = vColor; \n \
      }";

//outputColor = vColor * vec4(lColor * (fAmbientIntensity+fDiffuseIntensity), 1.0);
//      outputColor = vColor;

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

void GenerateBondInfo(bond * b, atom * a1, atom * a2)
{
  b->a1 = a1;
  b->a2 = a2;
  float p1[3] = {a1->atomPosition[0], a1->atomPosition[1], a1->atomPosition[2]};
  float p2[3] = {a2->atomPosition[0], a2->atomPosition[1], a2->atomPosition[2]};
  float mid[3] = {(p1[0]+p2[0])/2, (p1[1]+p2[1])/2, (p1[2]+p2[2])/2};
  float q[3] = {(p1[0]-mid[0]), (p1[1]-mid[1]), (p1[2]-mid[2])};
  float d = sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2]);
  b->length = d;
  Vector3f n_q = Vector3f(q);
  n_q.Normalize();

  Vector3f z_axis = Vector3f(0, 0, 1);
  z_axis.Normalize();
  Vector3f v = z_axis.Cross(n_q);
  v.Normalize();
  float c = (1.0f - z_axis.Dot(n_q))/1.0f;

  //v2 = v; n_q2 = n_q; z_ax = z_axis; c2 = c;

  Matrix4f v_x;
  v_x.m[0][0] = 0;     v_x.m[0][1] = -v.z;  v_x.m[0][2] = v.y;  v_x.m[0][3] = 0;
  v_x.m[1][0] =  v.z;  v_x.m[1][1] = 0;     v_x.m[1][2] = -v.x; v_x.m[1][3] = 0;
  v_x.m[2][0] = -v.y;  v_x.m[2][1] = v.x;   v_x.m[2][2] = 0;    v_x.m[2][3] = 0;
  v_x.m[3][0] = 0;     v_x.m[3][1] = 0   ;  v_x.m[3][2] = 0;    v_x.m[3][3] = 1;

  Matrix4f Rot;
  Rot.InitIdentity();
  Rot = Rot + v_x;
  Matrix4f v_x2 = v_x * v_x;
  v_x2 = v_x2 * c;
  Rot = Rot + v_x2;

  Rot.m[3][3] = 1;
  Rot.m[0][3] = 0;
  Rot.m[1][3] = 0;
  Rot.m[2][3] = 0;

  Rot.m[3][0] = 0;
  Rot.m[3][1] = 0;
  Rot.m[3][2] = 0;

  b->center = Vector3f(mid[0], mid[1], mid[2]);
  b->rotation = Rot;
}


void DrawBondLighted(Pipeline * p, GLuint CylVB, GLuint CylIB, bond * b,
                     GLuint SphereVB, GLuint SphereIB)
{
  float grey[3] = {.5, .5, .5};
  float white[3] = {1, 1, 1};

  p->Scale(0.05f, 0.05f, b->length);
  p->Translate(b->center.x, b->center.y, b->center.z);
  p->SetRotationMatrix(b->rotation);

  float dir[3] = {cam.Target[0], cam.Target[1], cam.Target[2]};
  glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWVPTrans());
  glUniformMatrix4fv(ShaderVarLocations.gWorldLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWorldTrans());
  glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&grey);
  glUniform4fv(ShaderVarLocations.lColorLocation, 1, (const GLfloat *)&white);
  glUniform4fv(ShaderVarLocations.lDirectionLocation, 1, (const GLfloat *)&dir);
  glUniform1f(ShaderVarLocations.fAmbientIntensityLocation, 0.8);

  glBindBuffer(GL_ARRAY_BUFFER, CylVB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CylIB);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, 0);

/*
  float red[3] = {1, 0, 0};
  float green[3] = {0, 1, 0};
  float blue[3] = {0, 0, 1};
        p->Scale(0.1f, 0.1f, 0.1f);
        p->Translate(p1[0], p1[1], p1[2]);
        p->Rotate(0.f, 0.f, 0.f);
        glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE, (const GLfloat *)p->GetWVPTrans());
  glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&red);
        glBindBuffer(GL_ARRAY_BUFFER, SphereVB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereIB);
        glDrawElements(GL_TRIANGLES, 6144, GL_UNSIGNED_INT, 0);


        p->Translate(p2[0], p2[1], p2[2]);
        glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE, (const GLfloat *)p->GetWVPTrans());
        glDrawElements(GL_TRIANGLES, 6144, GL_UNSIGNED_INT, 0);

        p->Translate(mid[0], mid[1], mid[2]);
        glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE, (const GLfloat *)p->GetWVPTrans());
  glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&green);
         glDrawElements(GL_TRIANGLES, 6144, GL_UNSIGNED_INT, 0);

        p->Scale(0.05f, 0.05f, 0.05f);
        p->Translate(n_q.x, n_q.y, n_q.z);
        glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE, (const GLfloat *)p->GetWVPTrans());
  glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&blue);
         glDrawElements(GL_TRIANGLES, 6144, GL_UNSIGNED_INT, 0);

        p->Scale(0.05f, 0.05f, 0.05f);
        p->Translate(0, 0, 0);
        glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE, (const GLfloat *)p->GetWVPTrans());
  glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&blue);
         glDrawElements(GL_TRIANGLES, 6144, GL_UNSIGNED_INT, 0);
*/
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

#pragma region atom selection
void selectAtom() {

}

void deselectAll() {

}

#pragma endregion



//TODO call this to load all atoms from the critic2 interface
//the atoms should be loaded into the above array
void loadAtoms() {
  //fill loadedAtoms array
  int n;
  get_num_atoms(&n);
  printf("num of atoms %d\n", n);

  loadedAtomsAmount = n;
	loadedAtoms = new atom[loadedAtomsAmount];
  for (int i=0; i < n; i++) {
    int atomicN;
    double x;
    double y;
    double z;

    get_atom_position(i+1, &atomicN, &x, &y, &z);
    printf("Atoms: %d %d %.10f %.10f %.10f\n",i+1,atomicN, x, y, z);

    loadedAtoms[i].atomicNumber = atomicN;
    loadedAtoms[i].atomPosition[0] = x;
  	loadedAtoms[i].atomPosition[1] = y;
  	loadedAtoms[i].atomPosition[2] = z;
  }

  //tree names must be constant
	for (size_t x = 0; x < loadedAtomsAmount; x++) {
		std::string nodeName = "";
		nodeName += "Elem Name: ";
		nodeName += loadedAtoms[x].name;
		nodeName += "Atomic #:";
		nodeName += charConverter(loadedAtoms[x].atomicNumber);
		nodeName += "  ID: ";
		nodeName += charConverter(x);
		nodeName += "##TreeID = "; //extra info for imgui to find selection
		nodeName += charConverter(x);
		loadedAtoms[x].atomTreeName = nodeName;
		loadedAtoms[x].atomTreePosition = x;
	}

}

void loadBonds() {

  for (int i = 0; i < loadedAtomsAmount; i++) {
    int nstarN;

    num_of_bonds(i+1, &nstarN);

    loadedAtoms[i].numberOfBonds = nstarN;
    loadedAtoms[i].loadedBonds = new int[nstarN];
    loadedBondsAmount += nstarN;

    for (int j = 0; j < nstarN; j++) {
        int connected_atom;

        get_atom_bond(i+1, j+1, &connected_atom);
        loadedAtoms[i].loadedBonds[j] = connected_atom-1;
        printf("%d atom has %d bonds and one is %d\n",i, nstarN, connected_atom-1);
    }
  }

  Bonds = new bond[loadedBondsAmount];
  int bondidx = 0;
  for (int i=0; i<loadedAtomsAmount; i++){
    int numBonds = loadedAtoms[i].numberOfBonds;
    for (int j=0; j<numBonds; j++){
      GenerateBondInfo(&Bonds[bondidx], &loadedAtoms[i], &loadedAtoms[loadedAtoms[i].loadedBonds[j]]);
      bondidx += 1;
    }
  }
}

void drawAllBonds(Pipeline * p, GLuint CylVB, GLuint CylIB,
                     GLuint SphereVB, GLuint SphereIB)
{
    /*
  for (size_t x = 1; x < loadedAtomsAmount; x++){
    int numBonds = sizeof(loadedAtoms[x].loadedBonds) / sizeof(loadedAtoms[x].loadedBonds[0]);
    //std::cout << numBonds << " num of bonds\n";
    for (size_t i = 0; i < numBonds; i++) {
      //std::cout << i << " atom bond " << loadedAtoms[x].loadedBonds[i] << "\n";
      if (loadedAtoms[x].loadedBonds[i] < loadedAtomsAmount && loadedAtoms[x].loadedBonds[i] > -1) {
        DrawBondLighted(p, CylVB, CylIB, loadedAtoms[x].atomPosition, loadedAtoms[loadedAtoms[x].loadedBonds[i]].atomPosition, SphereVB, SphereIB);
      }
    }
	}
   */
   for (int i=0; i< loadedBondsAmount; i++){
       DrawBondLighted(p, CylVB, CylIB, &Bonds[i], SphereVB, SphereIB);
   }
}

///returns the color of an atom based on the atomic number
///and desired color Intesity (brightness)
void getAtomColor(int atomicNumber, float colorIntensity, GLfloat col[4]) {
	if (atomicNumber == 7) {
        col[0] = 0.8; col[1] = 0.8; col[2] = 0.8; col[0] = colorIntensity;
	}else if(atomicNumber == 6) {
        col[0] = 0.8; col[1] = 0.0; col[2] = 0.0; col[0] = colorIntensity;
	} else  {
        col[0] = 0.8; col[1] = 0.8; col[2] = 0.8; col[0] = colorIntensity;
	}
}

///will be used to draw atom number over the atom using imgui window
float* getScreenPositionOfVertex(float *vertexLocation) {
	//TODO transfrom from vertex location to screen location
	return NULL;
}

/*highlighting types
dim color
float inc = 1.f;
if (loadedAtoms[identifyer].selected) { //selection is color based
inc = .5f;
}
const GLfloat n_Color[4]{color[0] * inc,color[1] * inc,color[2] * inc,color[3]};

increase color
float inc = 1.f;
if (loadedAtoms[identifyer].selected) { //selection is color based
inc = 1.5f;
}
const GLfloat n_Color[4]{color[0] * inc,color[1] * inc,color[2] * inc,color[3]};

*/

GLuint gWorldLocation; //made global to make Drawing via methods easer
GLuint mColorLocation;
void drawAtomInstance(int identifyer, float * posVector,const GLfloat color[4],
                      Pipeline * p, GLuint SphereVB, GLuint SphereIB) {
	//selection start
	float inc = 1.f;
	if (loadedAtoms[identifyer].selected) { //selection is color based
		inc = 1.5f;
	}

	GLfloat n_Color[] = {color[0] * inc,color[1] * inc,color[2] * inc, color[3]};
	//selection end

	float scaleAmount = (float)loadedAtoms[identifyer].atomicNumber;
	if (scaleAmount < 4.0f) {
		scaleAmount = 0.25f;
	} else {
		scaleAmount = 0.5f;
	}
	p->Scale(scaleAmount, scaleAmount, scaleAmount);
	p->Translate(posVector[0], posVector[1], posVector[2]);
	p->Rotate(0.f, 0.f, 0.f); //no rotation required
	glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE, (const GLfloat *)p->GetWVPTrans());
	glUniformMatrix4fv(ShaderVarLocations.gWorldLocation, 1, GL_TRUE, (const GLfloat *)p->GetWorldTrans());
	glBindBuffer(GL_ARRAY_BUFFER, SphereVB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereIB);
	glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&n_Color);
	glDrawElements(GL_TRIANGLES, 6144, GL_UNSIGNED_INT, 0);

	//TODO draw atom ID number
	ImGui::SetNextWindowSize(ImVec2(5, 5), ImGuiSetCond_Always);
	ImGui::SetNextWindowCollapsed(true);
	//float * winPos;
	//matrix math to transform posVector to pixel location of an atoms center

	//ImGui::SetNextWindowPos(ImVec2(winPos[0], winPos[1])); //TODO set location of identifying number
	ImGui::Begin(charConverter(identifyer).c_str(), false);
	ImGui::End();
}

///draws all atoms in the loadedAtoms struct
void drawAllAtoms(Pipeline * p, GLuint SphereVB, GLuint SphereIB) {
	for (size_t x = 0; x < loadedAtomsAmount; x++){
        GLfloat c[4] = {0, 0, 0, 0};
        getAtomColor(loadedAtoms[x].atomicNumber, 0.6f, c);
		drawAtomInstance(x, loadedAtoms[x].atomPosition, c, p, SphereVB, SphereIB);
	}
}

/// moves cam over atom (alligned to z axis)
void lookAtAtom(int atomNumber, Pipeline p) {
	//TODO set camara to look at atom
	cam.Pos[0] = -loadedAtoms[atomNumber].atomPosition[0];
	cam.Pos[1] = loadedAtoms[atomNumber].atomPosition[1];
	// z value is preserved
}

#pragma endregion

#pragma region IMGUI
///information to display in the stats list
int selectedAtom = 0;


#pragma region display stats methods
void displayCol(string * displayStats, int numberOfCol) {
	for (size_t i = 0; i < numberOfCol; i++) {
		ImGui::Text(displayStats[i].c_str());
		ImGui::NextColumn();
	}
}

void atomBondAmountInfo(string * displayVars, int atomNumber) {
	displayVars[0] = "number of bonds";
	displayVars[1] = charConverter(loadedAtoms[atomNumber].numberOfBonds);
	displayVars[2] = "";
}

void atomAtomicNumberInfo(string * displayVars, int atomNumber) {
	displayVars[0] = "atomic number";
	displayVars[1] = charConverter(loadedAtoms[atomNumber].atomicNumber);
	displayVars[2] = "";
}

#pragma endregion


void drawSelectedAtomStats() {
	if (loadedAtomsAmount == 0) {
		return;
	}
	ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiSetCond_Appearing);
	ImGui::Begin("Selected Information", false);
	const int numberOfColums = 3;


	ImGui::Columns(numberOfColums, "mycolumns");
	ImGui::Separator();
	ImGui::Text("Info Type"); ImGui::NextColumn();
	ImGui::Text("Value1"); ImGui::NextColumn(); 
	ImGui::Text("Value2"); ImGui::NextColumn();
	ImGui::Separator();
	
	string displayStats[numberOfColums];

	
	atomBondAmountInfo(displayStats, selectedAtom);
	displayCol(displayStats, numberOfColums);

	atomAtomicNumberInfo(displayStats, selectedAtom);
	displayCol(displayStats, numberOfColums);

	ImGui::End();
}



void printCamStats() {
	ImGui::SetNextWindowSize(ImVec2(300, 75), ImGuiSetCond_Appearing);
	ImGui::Begin("cam stats", false);
	string camPos = "cam pos: " + (string)charConverter(cam.Pos[0]) + "," + charConverter(cam.Pos[1]) + "," + charConverter(cam.Pos[2]);
	string camTarget = "cam target: " + (string)charConverter(cam.Target[0]) + "," + charConverter(cam.Target[1]) + "," + charConverter(cam.Target[2]);
	string camUp = "cam up: " + (string)charConverter(cam.Up[0]) + "," + charConverter(cam.Up[1]) + "," + charConverter(cam.Up[2]);

	ImGui::Text(camPos.c_str());
	ImGui::Text(camTarget.c_str());
	ImGui::Text(camUp.c_str());


	ImGui::End();
}

void drawSeachBar() {
	ImGui::SetNextWindowSize(ImVec2(200, 50), ImGuiSetCond_Appearing);
	ImGui::Begin("atom search", false);


	ImGui::End();
}

void drawCrystalTree() {

}


void drawAtomTreeView(Pipeline p) {
	ImGui::SetNextWindowSize(ImVec2(300,500),ImGuiSetCond_Appearing); //this section will be moved to crystle once that section is done
	ImGui::Begin("tree view",false);

	int closeOthers = -1; //id's are context dependent so other tree nodes must be closed outside the main loop
	for (size_t x = 0; x < loadedAtomsAmount; x++){ //all atoms
		if (ImGui::TreeNode(loadedAtoms[x].atomTreeName.c_str())) {
			if (loadedAtoms[x].selected == false) { // not currently true must set all others to false
				loadedAtoms[x].selected = true; //this loop is only run on the frame this tree node is clicked
				lookAtAtom(x, p);
				closeOthers = x;
			}

			//selection based on atoms bonds
			for (size_t i = 0; i < loadedBondsAmount; i++){
				if (Bonds[i].a1 == &loadedAtoms[x]) {
					string bondName = "bondedTo" + Bonds[i].a2->name;
					if (ImGui::TreeNode(bondName.c_str())){
						//select a2
						closeOthers = Bonds[i].a2->atomTreePosition;
					}
				} else if (Bonds[i].a2 == &loadedAtoms[x]) {
					string bondName = "bondedTo" + Bonds[i].a1->name;
					if (ImGui::TreeNode(bondName.c_str())) {
						//select a1
						closeOthers = Bonds[i].a1->atomTreePosition;
					}
				}
			}

			ImGui::TreePop();
		}else{
			loadedAtoms[x].selected = false;
		}
	}
	
	for (size_t i = 0; i < loadedCPointsAmount; i++){
		if (ImGui::TreeNode(loadedCPoints[i].pointName.c_str())) { //critical point tree node
			//TODO: critical point information
		}
	}


	if(closeOthers != -1)
	for (size_t y = 0; y < loadedAtomsAmount; y++) { //close all atom tree nodes exept the current one
		if (closeOthers != y) {
			ImGui::GetStateStorage()->SetInt(ImGui::GetID(loadedAtoms[y].atomTreeName.c_str()), 0); //close tab
		} else {
			selectedAtom = closeOthers;
		}
	}

	ImGui::End();
}



#pragma endregion


int main(int, char**)
{

    initialize();
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#if WIN32
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
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

    //GLuint trishader = CompileShaders();
//    gWorldLocation = glGetUniformLocation(trishader, "gWorld");
  //  mColorLocation = glGetUniformLocation(trishader, "mColor");
    GLuint lightshader = LightingShader();
    ShaderVarLocations.gWorldLocation = glGetUniformLocation(lightshader, "gWorld");
    ShaderVarLocations.gWVPLocation = glGetUniformLocation(lightshader, "gWVP");
    ShaderVarLocations.vColorLocation = glGetUniformLocation(lightshader, "vColor");
    ShaderVarLocations.lColorLocation = glGetUniformLocation(lightshader, "lColor");
    ShaderVarLocations.lDirectionLocation = glGetUniformLocation(lightshader, "lDirection");



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
	// loadedAtomsAmount = 3;
	// loadedAtoms = new atom[loadedAtomsAmount];
	// loadedAtoms[0].atomicNumber = 1;
	// loadedAtoms[0].atomPosition[0] = 0.f;
	// loadedAtoms[0].atomPosition[1] = -1.f;
	// loadedAtoms[0].atomPosition[2] = 0.f;
  //
  //
	// loadedAtoms[1].atomicNumber = 1;
	// loadedAtoms[1].atomPosition[0] = -1.29f;
	// loadedAtoms[1].atomPosition[1] = 1.16f;
	// loadedAtoms[1].atomPosition[2] = 0.f;
  //
  //
	// loadedAtoms[2].atomicNumber = 8;
	// loadedAtoms[2].atomPosition[0] = 0.f;
	// loadedAtoms[2].atomPosition[1] = .715f;
	// loadedAtoms[2].atomPosition[2] = 0.f;
	//loadAtoms();
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

	time_t lastTime = time(0);
	time_t curTime = lastTime;
	double frameTime = 35.0;
    bool show_test_window = true;
    // Main loop ------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
#pragma region frame limiter
		curTime = time(0);
		if ((difftime(lastTime, curTime) < frameTime)) {
			Sleep(frameTime - difftime(lastTime, curTime));
		}
		lastTime = curTime;

#pragma endregion

		glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        ImGuiIO& io = ImGui::GetIO();

		drawAtomTreeView(p);

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

        ShowAppMainMenuBar();
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
		drawAllAtoms(&p, SphereVB, SphereIB);
		printCamStats();
		drawSelectedAtomStats();

        drawAllBonds(&p, CylVB, CylIB, SphereVB, SphereIB);


        glDisableVertexAttribArray(0);

        glUseProgram(lightshader);

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

static void ShowAppMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowMenuFile();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

static void ShowMenuFile()
{
    ImGui::MenuItem("(dummy menu)", NULL, false, false);
    if (ImGui::MenuItem("Molecule")) {
      char const * lTheOpenFileName = tinyfd_openFileDialog(
    		"Select Molecule file",
    		"",
    		0,
    		NULL,
    		NULL,
    		0);

      if (lTheOpenFileName == NULL) {
        return;
      }

      init_struct();
      call_structure(lTheOpenFileName, (int) strlen(lTheOpenFileName), 1);
      loadAtoms();
      loadBonds();
    }
    if (ImGui::MenuItem("Crystal")) {
      char const * lTheOpenFileName = tinyfd_openFileDialog(
    		"Select Crystal file",
    		"",
    		0,
    		NULL,
    		NULL,
    		0);

        if (lTheOpenFileName == NULL) {
          return;
        }

      init_struct();
      call_structure(lTheOpenFileName, (int) strlen(lTheOpenFileName), 0);
      loadAtoms();
    }
}
