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
extern "C" void auto_cp();
extern "C" void num_of_crit_points(int *n_critp);
extern "C" void get_cp_pos_type(int cpIdx, int *type, double *x, double *y, double *z);

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
  int * bonds;
  int atomTreePosition;
};

struct bond{
    atom * a1;
    atom * a2;
    Vector3f center;
    Matrix4f rotation;
    float length;
};

struct criticalPoint {
    float cpPosition[3];
    int type;
    string typeName = "";
    bool selected = false;
};

bond * bonds;
criticalPoint * loadedCriticalPoints;

atom * loadedAtoms;
int loadedAtomsAmount = 0;
int selectedAtom = 0;
int bondsAmount = 0;
int loadedCPAmount = 0;


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

void DrawBond(Pipeline * p, GLuint CylVB, GLuint CylIB, bond * b)
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
}

void DrawRotationAxes(Pipeline * p, GLuint CylVB, GLuint CylIB)
{
  float white[3] = {1, 1, 1};
  float red[3] = {1, 0, 0};
  float green[3] = {0, 1, 0};
  float blue[3] = {0, 0, 1};

  float dir[3] = {cam.Target[0], cam.Target[1], cam.Target[2]};
  glUniform4fv(ShaderVarLocations.lColorLocation, 1, (const GLfloat *)&white);
  glUniform4fv(ShaderVarLocations.lDirectionLocation, 1, (const GLfloat *)&dir);
  glUniform1f(ShaderVarLocations.fAmbientIntensityLocation, 0.8);
  glBindBuffer(GL_ARRAY_BUFFER, CylVB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CylIB);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  // X axis - red
  p->Scale(1, 1, 0.05);
  p->Translate(0, 0, 0);
  p->Rotate(0, 90, 0);
  glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWVPTrans());
  glUniformMatrix4fv(ShaderVarLocations.gWorldLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWorldTrans());
  glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&red);
  glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, 0);

  // Y axis - green
  p->Scale(1, 1, 0.05);
  p->Translate(0, 0, 0);
  p->Rotate(90, 0, 0);
  glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWVPTrans());
  glUniformMatrix4fv(ShaderVarLocations.gWorldLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWorldTrans());
  glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&green);
  glDrawElements(GL_TRIANGLES, 240, GL_UNSIGNED_INT, 0);

  // Z axis - blue
  p->Scale(1, 1, 0.05);
  p->Translate(0, 0, 0);
  p->Rotate(0, 0, 0);
  glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWVPTrans());
  glUniformMatrix4fv(ShaderVarLocations.gWorldLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWorldTrans());
  glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&blue);
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

#pragma region atom selection
void selectAtom() {

}

void deselectAll() {

}

#pragma endregion

void destructLoadedMolecule(){
  if (loadedAtomsAmount > 0){
    for (int i=loadedAtomsAmount-1; i>=0; i--){
      delete loadedAtoms[i].bonds;
    }
    loadedAtomsAmount = 0;
  }
  if (bondsAmount > 0){
    delete bonds;
    bondsAmount = 0;
  }
}

void destructCriticalPoints() {
  if (loadedCPAmount > 0) {
    loadedCriticalPoints = NULL;
    loadedCPAmount = 0;
  }
}



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
    loadedAtoms[i].bonds = new int[nstarN];
    bondsAmount += nstarN;

    for (int j = 0; j < nstarN; j++) {
        int connected_atom;

        get_atom_bond(i+1, j+1, &connected_atom);
        loadedAtoms[i].bonds[j] = connected_atom-1;
        printf("%d atom has %d bonds and one is %d\n",i, nstarN, connected_atom-1);
    }
  }

  bonds = new bond[bondsAmount];
  int bondidx = 0;
  for (int i=0; i<loadedAtomsAmount; i++){
    int numBonds = loadedAtoms[i].numberOfBonds;
    for (int j=0; j<numBonds; j++){
      GenerateBondInfo(&bonds[bondidx], &loadedAtoms[i], &loadedAtoms[loadedAtoms[i].bonds[j]]);
      bondidx += 1;
    }
  }
}

void loadCriticalPoints() {
  int numCP;
  num_of_crit_points(&numCP);

  loadedCPAmount += (numCP - loadedAtomsAmount);
  loadedCriticalPoints = new criticalPoint[loadedCPAmount];

  for (int i = loadedAtomsAmount + 1; i <= numCP; i++) {
    int cpType;
    double x;
    double y;
    double z;

    get_cp_pos_type(i, &cpType, &x, &y, &z);

    printf("Critical Points: %d %d %.10f %.10f %.10f\n",i,cpType, x, y, z);

    loadedCriticalPoints[(i-(loadedAtomsAmount+1))].cpPosition[0] = x;
    loadedCriticalPoints[(i-(loadedAtomsAmount+1))].cpPosition[1] = y;
    loadedCriticalPoints[(i-(loadedAtomsAmount+1))].cpPosition[2] = z;

    loadedCriticalPoints[(i-(loadedAtomsAmount+1))].type = cpType;

    if (cpType == -1) {
      loadedCriticalPoints[(i-(loadedAtomsAmount+1))].typeName += "bond";
    } else if (cpType == 1) {
      loadedCriticalPoints[(i-(loadedAtomsAmount+1))].typeName += "ring";
    } else if (cpType == 3) {
      loadedCriticalPoints[(i-(loadedAtomsAmount+1))].typeName += "cage";
    }
  }
}

void drawAllBonds(Pipeline * p, GLuint CylVB, GLuint CylIB)
{
   for (int i=0; i< bondsAmount; i++){
     DrawBond(p, CylVB, CylIB, &bonds[i]);
   }
}

///returns the color of an atom based on the atomic number
///and desired color Intesity (brightness)
Vector3f getAtomColor(int atomicNumber) {
  Vector3f color;
  if (atomicNumber == 1) {     // Hydrogen = white
    color = Vector3f(1, 1, 1);
	} else if (atomicNumber == 6) {   // Carbon = black
    color = Vector3f(0, 0, 0);
	} else if (atomicNumber == 7) {   // Nitrogen = dark blue
    color = Vector3f(0, 0, 0.5);
	} else if (atomicNumber == 8) {   // Oxygen = red
    color = Vector3f(1, 0, 0);
	} else if (atomicNumber == 9 || atomicNumber == 17) {   // Fluorine & Chlorine = green
    color = Vector3f(0, 1, 0);
	} else if (atomicNumber == 35) {  // Bromine = dark red
    color = Vector3f(0.5, 0, 0);
	} else if (atomicNumber == 53) {  // Iodine = dark violet
    color = Vector3f(0.5, 0, 0.5);
	} else if (atomicNumber == 2 ||    // noble gases (He, Ne, Ar, Kr, Xe, Rn) = cyan
        atomicNumber == 10 ||
        atomicNumber == 18 ||
        atomicNumber == 36 ||
        atomicNumber == 54 ||
        atomicNumber == 86) {
    color = Vector3f(0, 1, 1);
	} else if (atomicNumber == 15) {  // Potassium = orange
    color = Vector3f(1, 0.5, 0);
	} else if (atomicNumber == 16) {  // sulfur = yellow
    color = Vector3f(1, 1, 0);
	} else if (atomicNumber == 3 ||   // alkali metals = violet
             atomicNumber == 11 ||
             atomicNumber == 19 ||
             atomicNumber == 37 ||
             atomicNumber == 55 ||
             atomicNumber == 87){
    color = Vector3f(1, 0, 1);
	} else if (atomicNumber == 4 ||   // alkaline earth metals = dark green
             atomicNumber == 12 ||
             atomicNumber == 20 ||
             atomicNumber == 38 ||
             atomicNumber == 56 ||
             atomicNumber == 88){
    color = Vector3f(0, 0.5, 0);
	} else if (atomicNumber == 81) {  // titaniam = gray
    color = Vector3f(0.75, 0.75, 0.75);
	} else if (atomicNumber == 26) {  // iron = dark orange
    color = Vector3f(0.75, 0.25, 0);
	} else if ((atomicNumber >= 21 && atomicNumber <= 30) ||  // transition metals = orange/pink
             (atomicNumber >= 39 && atomicNumber <= 48) ||
             (atomicNumber >= 57 && atomicNumber <= 80) ||
             (atomicNumber >= 89 && atomicNumber <= 112)){
    color = Vector3f(1, 0.5, 0.3);
	} else  {   // all other atoms = pink
    color = Vector3f(1, 0.25, 0.5);
	}
  return color;
}

Vector3f getCritPointColor(int cpType) {
  Vector3f color;
  if (cpType == -1) {     // bond cp = yellow
    color = Vector3f(1, 1, 0);
	} else if (cpType == 1) {   // ring cp = light grey
    color = Vector3f(0.6588, 0.6588, 0.6588);
	} else  {   // cage cp = light purple
    color = Vector3f(0.94, 0.81, 0.99);
	}
  return color;
}

#pragma region color legend

//basic color Legend
void atomLegend_displayColorBoxAndName(int colorNumber, string * colorNames) {
	Vector3f AtomicNumberColor = getAtomColor(colorNumber);
	ImVec4 color = ImColor(AtomicNumberColor.x, AtomicNumberColor.y, AtomicNumberColor.z, 1.0);
	ImGui::ColorButton(color); ImGui::SameLine();
	ImGui::Text(colorNames[colorNumber].c_str());
}


void atomColorLegend() {
	ImGui::SetNextWindowPos(ImVec2(0, 500), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiSetCond_Appearing);
	ImGui::Begin("Atom color legend", false);
	string * colorNames = new string[101];
	colorNames[1] = "Hydrogen";
	colorNames[2] = "Noble Gas";
	colorNames[3] = "Alkali Metals";
	colorNames[4] = "Akaline Earth";

	colorNames[6] = "Carbon";
	colorNames[7] = "Nitrogen";
	colorNames[8] = "Oxygen";
	colorNames[9] = "Flourine & Chlorine";

	colorNames[15] = "Potassium";
	colorNames[16] = "Sulfur";

	colorNames[21] = "transition Metal";

	colorNames[26] = "iron";

	colorNames[35] = "Bromine";

	colorNames[53] = "Iodine";

	colorNames[81] = "titaniam";
	
	colorNames[100] = "other";

	for (size_t i = 1; i < 5; i++) {
		atomLegend_displayColorBoxAndName(i, colorNames);
	}
	for (size_t i = 6; i < 10; i++) {
		atomLegend_displayColorBoxAndName(i, colorNames);
	}

	atomLegend_displayColorBoxAndName(15, colorNames);
	atomLegend_displayColorBoxAndName(16, colorNames);
	atomLegend_displayColorBoxAndName(21, colorNames);
	atomLegend_displayColorBoxAndName(26, colorNames);
	atomLegend_displayColorBoxAndName(35, colorNames);
	atomLegend_displayColorBoxAndName(53, colorNames);
	atomLegend_displayColorBoxAndName(81, colorNames);
	atomLegend_displayColorBoxAndName(100, colorNames);


	ImGui::End();
}

#pragma endregion

///will be used to draw atom number over the atom using imgui window
float* getScreenPositionOfVertex(float *vertexLocation) {
	//TODO transfrom from vertex location to screen location
	return NULL;
}

void drawAtomInstance(int id, float * posVector, Vector3f color,
                      Pipeline * p, GLuint SphereVB, GLuint SphereIB) {

  // if atom is selected, brighten it
  if (loadedAtoms[id].selected) {
    color = color * 1.5;
	}

	float scaleAmount = (float)loadedAtoms[id].atomicNumber;
	if (scaleAmount < 4.0f) {
		scaleAmount = 0.2f;
	} else {
		scaleAmount = 0.4f;
	}
	p->Scale(scaleAmount, scaleAmount, scaleAmount);
	p->Translate(posVector[0], posVector[1], posVector[2]);
	p->Rotate(0.f, 0.f, 0.f);

	glUniformMatrix4fv(ShaderVarLocations.gWVPLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWVPTrans());
	glUniformMatrix4fv(ShaderVarLocations.gWorldLocation, 1, GL_TRUE,
                     (const GLfloat *)p->GetWorldTrans());
	glBindBuffer(GL_ARRAY_BUFFER, SphereVB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereIB);
	glUniform4fv(ShaderVarLocations.vColorLocation, 1, (const GLfloat *)&color);
	glDrawElements(GL_TRIANGLES, 6144, GL_UNSIGNED_INT, 0);
/*
	//TODO draw atom ID number
	ImGui::SetNextWindowSize(ImVec2(5, 5), ImGuiSetCond_Always);
	ImGui::SetNextWindowCollapsed(true);
	//float * winPos;
	//matrix math to transform posVector to pixel location of an atoms center

	//ImGui::SetNextWindowPos(ImVec2(winPos[0], winPos[1])); //TODO set location of identifying number
	ImGui::Begin(std::to_string(identifyer).c_str(), false);
	ImGui::End();
  */
}

void drawCritPointInstance(int identifier, float * posVector, const GLfloat color[4],
                      Pipeline * p, GLuint SphereVB, GLuint SphereIB) {
	//selection start
	float inc = 1.f;
	if (loadedCriticalPoints[identifier].selected) { //selection is color based
		inc = 1.5f;
	}

	GLfloat n_Color[] = {color[0] * inc,color[1] * inc,color[2] * inc, color[3]};
	//selection end

	float scaleAmount = 0.05f;

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
/*
	//TODO draw crit point ID number or type?
	ImGui::SetNextWindowSize(ImVec2(5, 5), ImGuiSetCond_Always);
	ImGui::SetNextWindowCollapsed(true);
	//float * winPos;
	//matrix math to transform posVector to pixel location of an atoms center

	//ImGui::SetNextWindowPos(ImVec2(winPos[0], winPos[1])); //TODO set location of identifying number
	ImGui::Begin(charConverter(identifyer).c_str(), false);
	ImGui::End();
  */
}

#pragma region atom flashing
bool flashAtoms = false; // toggle with selection toggles (in gui)
//selctedAtom from tree selection
//the number of frames the deselected atoms stay invisable
int framesMax = 15; // ~0.5 seconds
int framesLeft = 0;
bool otherAtomsVisable = true;

#pragma endregion

///draws all atoms in the loadedAtoms struct
void drawAllAtoms(Pipeline * p, GLuint SphereVB, GLuint SphereIB) {
	if (flashAtoms) { //flash mode
		if (framesLeft <= 0) {
			otherAtomsVisable = !otherAtomsVisable;
			framesLeft = framesMax;
		}
		if (otherAtomsVisable) { //all visible
			for (size_t x = 0; x < loadedAtomsAmount; x++) {
				Vector3f color = getAtomColor(loadedAtoms[x].atomicNumber);
				drawAtomInstance(x, loadedAtoms[x].atomPosition, color, p, SphereVB, SphereIB);
			}
		} else { // only selected atom visable
			Vector3f color = getAtomColor(loadedAtoms[selectedAtom].atomicNumber);
			drawAtomInstance(selectedAtom, loadedAtoms[selectedAtom].atomPosition, color, p, SphereVB, SphereIB);
		}
		framesLeft--;
	} else { // regular drawing
		for (size_t x = 0; x < loadedAtomsAmount; x++){
		Vector3f color = getAtomColor(loadedAtoms[x].atomicNumber);
			drawAtomInstance(x, loadedAtoms[x].atomPosition, color, p, SphereVB, SphereIB);
		}
	}
}

void drawAllCPs(Pipeline * p, GLuint SphereVB, GLuint SphereIB) {
	for (int x = 0; x < loadedCPAmount; x++){
    Vector3f color = getCritPointColor(loadedCriticalPoints[x].type);
		drawCritPointInstance(x, loadedCriticalPoints[x].cpPosition, color, p, SphereVB, SphereIB);
	}
}

/// moves cam over atom (alligned to z axis)
void lookAtAtom(int atomNumber) {
	cam.Pos[0] = loadedAtoms[atomNumber].atomPosition[0];
	cam.Pos[1] = loadedAtoms[atomNumber].atomPosition[1];
}

/// moves cam over crit point (alligned to z axis)
void lookAtCritPoint(int critPointNum) {
	cam.Pos[0] = loadedCriticalPoints[critPointNum].cpPosition[0];
	cam.Pos[1] = loadedCriticalPoints[critPointNum].cpPosition[1];
}

#pragma endregion

#pragma region IMGUI
///information to display in the stats list


//This methods are used to display additonal information about
//a perticular critical point or atom
//number of displayVars is currently assumed to be 3
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

void criticalPointTypeInfo(string * displayVars, int criticalPointIndex) {
	displayVars[0] = "critical point Type";
	displayVars[1] = loadedCriticalPoints[criticalPointIndex].typeName;
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

int selectedCP = 0;
void drawSelectedCPStats() {
	if (loadedCPAmount == 0 || selectedCP < 0) {
		return;
	}
	if (selectedCP >= loadedCPAmount) {
		cout << "error in cp stat display: selected greater than number" << endl;
		return;
	}

	ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiSetCond_Appearing);
	ImGui::Begin("Selected Critical Point Information", false);
	const int numberOfColums = 3;

	ImGui::Columns(numberOfColums, "mycolumns");
	ImGui::Separator();
	ImGui::Text("Info Type"); ImGui::NextColumn();
	ImGui::Text("Value1"); ImGui::NextColumn();
	ImGui::Text("Value2"); ImGui::NextColumn();
	ImGui::Separator();

	string displayStats[numberOfColums];

	criticalPointTypeInfo(displayStats, selectedCP);
	displayCol(displayStats, numberOfColums);

	//CP stat 2
	//displayCol(displayStats, numberOfColums);

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

void drawToolBar(int screen_w, int screen_h,
                 bool * show_bonds, bool * show_cps, bool * show_atoms) {
	ImGui::SetNextWindowSize(ImVec2(50, screen_h),ImGuiSetCond_Once);
	ImGui::SetNextWindowPos(ImVec2(0, 0),ImGuiSetCond_Always);
  ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_AlwaysAutoResize;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoTitleBar;
	ImGui::Begin("ToolBar",false, flags);
  if (ImGui::Button("Load Molecule")){
      char const * lTheOpenFileName = tinyfd_openFileDialog(
    		"Select Molecule file",
    		"../../examples/data/benzene.wfx",
    		0,
        NULL,
    		NULL,
    		0);

      if (lTheOpenFileName == NULL) {
        return;
      }

      init_struct();
      call_structure(lTheOpenFileName, (int) strlen(lTheOpenFileName), 1);
      destructLoadedMolecule();
      destructCriticalPoints();
      loadAtoms();
      loadBonds();
  }
  if (ImGui::Button("Load Crystal")){
      char const * lTheOpenFileName = tinyfd_openFileDialog(
    		"Select Molecule file",
    		"../../examples/data/ammonia.big.vel.cube",
    		0,
    		NULL,
    		NULL,
    		0);

      if (lTheOpenFileName == NULL) {
        return;
      }

      init_struct();
      call_structure(lTheOpenFileName, (int) strlen(lTheOpenFileName), 0);
      destructLoadedMolecule();
      destructCriticalPoints();
      loadAtoms();
      loadBonds();
  }
  if (ImGui::Button("Generate Critical Points")){
    auto_cp();
  }
  if (ImGui::Button("Load Critical Points")) {
    destructCriticalPoints();
    loadCriticalPoints();
  }
  if (ImGui::Button("Clear")){
    destructLoadedMolecule();
    destructCriticalPoints();
  }
  ImGui::Checkbox("Bonds", show_bonds);
  ImGui::Checkbox("Crit Pts", show_cps);
  ImGui::Checkbox("Atoms", show_atoms);
 
  if (ImGui::Checkbox("Selc.find", &flashAtoms)) { //set flashing to defults
	  framesMax = 15; // ~0.5 seconds
	  framesLeft = 0;
	  otherAtomsVisable = true;
  }
	
  ImGui::End();
}


void drawTreeView(int screen_w, int screen_h) {
	ImGui::SetNextWindowSize(ImVec2(300, screen_h),ImGuiSetCond_Always);
	ImGui::SetNextWindowPos(ImVec2(screen_w-300, 0),ImGuiSetCond_Always);
  ImGuiWindowFlags flags = 0;
//    flags |= ImGuiWindowFlags_AlwaysAutoResize;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoMove;
	ImGui::Begin("Tree View",false, flags);

	int closeOthers = -1;
	for (size_t x = 0; x < loadedAtomsAmount; x++){
		if (ImGui::TreeNode(loadedAtoms[x].atomTreeName.c_str())) {
			if (loadedAtoms[x].selected == false) {
				loadedAtoms[x].selected = true;
				lookAtAtom(x);
				closeOthers = x;
			}

			//selection based on atoms bonds
			for (size_t i = 0; i < bondsAmount; i++) {
				if ( bonds[i].a1->atomTreePosition == loadedAtoms[x].atomTreePosition) {
					string bondName = "bondedTo" + charConverter(bonds[i].a2->atomTreePosition);
					if (ImGui::TreeNode(bondName.c_str())) {
						//select a2
						closeOthers = bonds[i].a2->atomTreePosition;
						cout << "going to atom" + charConverter(closeOthers) << endl;
						ImGui::TreePop();
					}
				} else if (bonds[i].a2->atomTreePosition == loadedAtoms[x].atomTreePosition) {
					string bondName = "bondedTo" + charConverter(bonds[i].a1->atomTreePosition);
					if (ImGui::TreeNode(bondName.c_str())) {
						//select a1
						closeOthers = bonds[i].a1->atomTreePosition;
						cout << "going to atom" + charConverter(closeOthers) << endl;
						ImGui::TreePop();
					}
				}
			}

			ImGui::TreePop();
		}else {
			loadedAtoms[x].selected = false;
		}
	}

	if (closeOthers != -1) {
		ImGui::GetStateStorage()->SetAllInt(0); // close all tabs
		ImGui::GetStateStorage()->SetInt(ImGui::GetID(loadedAtoms[closeOthers].atomTreeName.c_str()), 1);
		selectedAtom = closeOthers;
		closeOthers = -1;
	}


	ImGui::End();
	
	if (loadedCPAmount != 0) {
		ImGui::SetNextWindowSize(ImVec2(300, screen_h*.5), ImGuiSetCond_Appearing);
		ImGui::SetNextWindowPos(ImVec2(screen_w - 600, 0), ImGuiSetCond_Appearing);
		ImGuiWindowFlags flags = 0;
		// flags |= ImGuiWindowFlags_AlwaysAutoResize;
		// flags |= ImGuiWindowFlags_NoResize;
		ImGui::Begin("Cp type and ID", false, flags);

		closeOthers = -1;
		
		for (size_t i = 0; i < loadedCPAmount; i++) {
			if (ImGui::TreeNode((loadedCriticalPoints[i].typeName + ":" + charConverter(i)).c_str())) { //critical point tree node
				if (loadedCriticalPoints[i].selected == false) {
					loadedCriticalPoints[i].selected = true;
					lookAtCritPoint(i);
					closeOthers = i;
					selectedCP = i;
				} 
				ImGui::TreePop();
			} else {
				loadedCriticalPoints[i].selected = false;
			}
		}
		
		if (closeOthers != -1) { //only one cp tab should be open at a time
			ImGui::GetStateStorage()->SetAllInt(0); // close all tabs
			ImGui::GetStateStorage()->SetInt(ImGui::GetID((loadedCriticalPoints[closeOthers].typeName + ":" + charConverter(closeOthers)).c_str()), 1); // leave selected tab open
			closeOthers = -1;
		}

		ImGui::End();
	
		// start of cp info window
		drawSelectedCPStats();
	
	}
}



#pragma endregion


int main(int, char**)
{

    initialize();
    // char const * file = "/home/isaac/c2/critic2/examples/data/benzene.wfx";
    // init_struct();
    // call_structure(file, (int) strlen(file), 1);
    // loadAtoms();
    // loadBonds();
    // destructLoadedMolecule();
    // file = "/home/isaac/c2/critic2/examples/data/pyridine.wfx";
    // init_struct();
    // call_structure(file, (int) strlen(file), 1);
    // loadAtoms();
    // loadBonds();

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

    // Imgui static variables
    static bool show_bonds = true;
    static bool show_cps = false;
    static bool show_atoms = true;

    // input variables;
    // c means for current loop, l means last loop, p means last pressed
    static int cLMB;
    static int cRMB;
    static int lLMB;
    static int lRMB;
    static double cMPosX;
    static double cMPosY;
    static double lMPosX;
    static double lMPosY;
    static double pMPosX;
    static double pMPosY;
    static double scrollY;

    cam.Pos[0] = 0.f; cam.Pos[1] = 0.f; cam.Pos[2] = -10.f;
    cam.Target[0] = 0.f; cam.Target[1] = 0.f; cam.Target[2] = 1.f;
    cam.Up[0] = 0.f; cam.Up[1] = 1.f; cam.Up[2] = 0.f;

	time_t lastTime = time(0);
	time_t curTime = lastTime;
	double frameTime = 35.0;

    Vector3f curRotAxis = Vector3f(0, 0, 0);
    Vector3f lastRotAxis = Vector3f(0, 0, 0);
    Vector3f rotAxis = Vector3f(0, 0, 0);

    static float lastRotAng = 0;
    static float curRotAng = 0;
    static float rotAng = 0;

    static float diffX;
    static float diffY;

    Matrix4f lastRot;
    Matrix4f curRot;
    Matrix4f rot;
    lastRot.InitIdentity();
    curRot.InitIdentity();
    rot.InitIdentity();
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

        // Process input
        ImGuiIO& io = ImGui::GetIO();
        lLMB = cLMB;
        lRMB = cRMB;
        cLMB = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        cRMB = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        lMPosX = cMPosX;
        lMPosY = cMPosY;
        glfwGetCursorPos(window, &cMPosX, &cMPosY);

        float camPanFactor = 0.008f;
        float camZoomFactor = 1.f;
        float camRotateVectorFactor = 0.05f;
        float camRotateAngleFactor = 0.05f;
        if (!io.WantCaptureMouse) {
          if (cRMB == GLFW_PRESS){
            cam.Pos[0] -= camPanFactor * (cMPosX - lMPosX);
            cam.Pos[1] += camPanFactor * (cMPosY - lMPosY);
          }
          if (cLMB == GLFW_PRESS){
            if (lLMB != GLFW_PRESS){
              pMPosX = cMPosX;
              pMPosY = cMPosY;

              lastRot = rot;
            } else {

            diffX = (float)(cMPosX - pMPosX);
            diffY = (float)(cMPosY - pMPosY);

            curRotAxis = Vector3f(diffX, -diffY, 0);
            curRotAxis = curRotAxis.Cross(Vector3f(0, 0, 1));
            curRotAng = curRotAxis.Length() * camRotateAngleFactor;
            curRotAxis.Normalize();

            curRot.InitRotateAxisTransform(curRotAxis, curRotAng);
            rot = curRot * lastRot;
            }
          }
        }

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

        p.SetPersProjInfo(45, display_w, display_h, 1.f, 1000.f);
        p.SetOrthoProjInfo(-10.f, 10.f, -10.f, 10.f, -1000.f, 1000.f);
        p.SetPostRotationMatrix(rot);
        p.SetCamera(cam);

        glEnableVertexAttribArray(0);
        // molecule drawing
        if (show_atoms){
      		drawAllAtoms(&p, SphereVB, SphereIB);
        }
        if (show_bonds){
          drawAllBonds(&p, CylVB, CylIB);
        }
        if (show_cps){
          drawAllCPs(&p, SphereVB, SphereIB);
        }
		//color legend
		atomColorLegend();

        // imgui overlays
//    		printCamStats();
//        ShowAppMainMenuBar();
		    drawSelectedAtomStats();
		    drawTreeView(display_w, display_h);
        drawToolBar(display_w, display_h, &show_bonds, &show_cps, &show_atoms);

        glDisableVertexAttribArray(0);
        glUseProgram(lightshader);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        ImGui::Render();
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
      destructLoadedMolecule();
      loadAtoms();
      //loadBonds();
      //loadCriticalPoints();
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
      destructLoadedMolecule();
      loadAtoms();
      //loadBonds();
      //loadCriticalPoints();
    }
}
