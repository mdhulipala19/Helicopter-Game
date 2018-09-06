/* Mahesh Dhulipala HW4: Helicopter game
 You are trying to save little tigger from a big boss tigger. However, in order to recuse him, you need to collect at least 6 green trees. BE CAREFUL though! If you hit the red wooden trees, you will lose a life (but keep the current number of green trees you've collected). You crash if you hit the red trees 3 times. If you hit a red tree, you will restart. You can shoot at the red trees to get rid of them. Once you have 6 green trees, fly up to little tigger and pick him up by flying past him. Once you land on the ground with him, you have succeeded in your mission.
 
 Controls:
 Use i,j,k,l to turn and move forward/backward.
 Use w and s to fly up and down.
 Brake using b (for forward/backward motion) and q for up and down.
 Fire bullets using f.
 
 */
using namespace std;
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

#if defined(__APPLE__)
#include <GLUT/GLUT.h>
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <cstdio>
#include <cstdlib>

int score=0;
int lives=3;
int level=1;
bool saved=false;

void display(void);
void polygon(int a, int b, int c , int d);
void DrawCube();
void printtext(int x, int y, std::string String);

const unsigned int windowWidth = 512, windowHeight = 512;

int majorVersion = 3, minorVersion = 0;

bool keyboardState[256];

void getErrorInfo(unsigned int handle)
{
    int logLen;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);
    if (logLen > 0)
    {
        char * log = new char[logLen];
        int written;
        glGetShaderInfoLog(handle, logLen, &written, log);
        printf("Shader log:\n%s", log);
        delete log;
    }
}

void checkShader(unsigned int shader, char * message)
{
    int OK;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &OK);
    if (!OK)
    {
        printf("%s!\n", message);
        getErrorInfo(shader);
    }
}

void checkLinking(unsigned int program)
{
    int OK;
    glGetProgramiv(program, GL_LINK_STATUS, &OK);
    if (!OK)
    {
        printf("Failed to link shader program!\n");
        getErrorInfo(program);
    }
}

// row-major matrix 4x4
struct mat4
{
    float m[4][4];
public:
    mat4() {}
    mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33)
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }
    
    mat4 operator*(const mat4& right)
    {
        mat4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.m[i][j] = 0;
                for (int k = 0; k < 4; k++) result.m[i][j] += m[i][k] * right.m[k][j];
            }
        }
        return result;
    }
    operator float*() { return &m[0][0]; }
};


// 3D point in homogeneous coordinates
struct vec4
{
    float v[4];
    
    vec4(float x = 0, float y = 0, float z = 0, float w = 1)
    {
        v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    }
    
    vec4 operator*(const mat4& mat)
    {
        vec4 result;
        for (int j = 0; j < 4; j++)
        {
            result.v[j] = 0;
            for (int i = 0; i < 4; i++) result.v[j] += v[i] * mat.m[i][j];
        }
        return result;
    }
    
    vec4 operator+(const vec4& vec)
    {
        vec4 result(v[0] + vec.v[0], v[1] + vec.v[1], v[2] + vec.v[2], v[3] + vec.v[3]);
        return result;
    }
};

// 2D point in Cartesian coordinates
struct vec2
{
    float x, y;
    
    vec2(float x = 0.0, float y = 0.0) : x(x), y(y) {}
    
    vec2 operator+(const vec2& v)
    {
        return vec2(x + v.x, y + v.y);
    }
    
    vec2 operator*(float s)
    {
        return vec2(x * s, y * s);
    }
    
};

// 3D point in Cartesian coordinates
struct vec3
{
    float x, y, z;
    
    vec3(float x = 0.0, float y = 0.0, float z = 0.0) : x(x), y(y), z(z) {}
    
    static vec3 random() { return vec3(((float)rand() / RAND_MAX) * 2 - 1, ((float)rand() / RAND_MAX) * 2 - 1, ((float)rand() / RAND_MAX) * 2 - 1); }
    
    vec3 operator+(const vec3& v) { return vec3(x + v.x, y + v.y, z + v.z); }
    
    vec3 operator-(const vec3& v) { return vec3(x - v.x, y - v.y, z - v.z); }
    
    vec3 operator*(float s) { return vec3(x * s, y * s, z * s); }
    
    vec3 operator/(float s) { return vec3(x / s, y / s, z / s); }
    
    
    float length() { return sqrt(x * x + y * y + z * z); }
    
    vec3 normalize() { return *this / length(); }
    
    
    void print() { printf("%f \t %f \t %f \n", x, y, z); }
};

vec3 cross(const vec3& a, const vec3& b)
{
    return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}



class Geometry
{
protected:
    unsigned int vao;
    
public:
    Geometry()
    {
        glGenVertexArrays(1, &vao);
    }
    
    virtual void Draw() = 0;
};

class BackgroundGeometry:public Geometry
{
    unsigned int vbo[1];
public:
    BackgroundGeometry()
    {
        static float vertexPosCoords[] =     {
            -1,    -1,     0,
            1,     -1,     0,
            -1,      1,     0,
            1,      1,     0    };
        
        
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(3, &vbo[0]);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosCoords), vertexPosCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    }
    
    void Draw()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND); // necessary for transparent pixels
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertexPosCoords) * 3);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    }
};

class TexturedQuad : public Geometry
{
    
public:
    TexturedQuad()
    {
        unsigned int vbo[3];
        
        static float vertexPosCoords[] =
        {
            0,0,0,1,
            -1,0,1,0,
            1,0,1,0,
            1,0,-1,0,
            -1,0,-1,0,
            -1,0,1,0
            
            
        };
        
        
        float vertexTexCoords[8] =
        {
            0,1,
            1,1,
            1,0,
            0,0
            
        };
        
        float vertexNormalCoords[12] =
        {
            0,1,0,
            0,1,0,
            0,1,0,
            0,1,0
            
        };
        
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(3, &vbo[0]);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosCoords), vertexPosCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexCoords), vertexTexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexNormalCoords), vertexNormalCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        
        
        
    }
    
    void Draw()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND); // necessary for transparent pixels
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertexPosCoords) * 3);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
    }
    
};
class   PolygonalMesh : public Geometry
{
    struct  Face
    {
        int       positionIndices[4];
        int       normalIndices[4];
        int       texcoordIndices[4];
        bool      isQuad;
    };
    
    std::vector<std::string*> rows;
    std::vector<vec3*> positions;
    std::vector<std::vector<Face*>> submeshFaces;
    std::vector<vec3*> normals;
    std::vector<vec2*> texcoords;
    
    int nTriangles;
    
public:
    PolygonalMesh(const char *filename);
    ~PolygonalMesh();
    
    void Draw();
};



PolygonalMesh::PolygonalMesh(const char *filename)
{
    std::fstream file(filename);
    if(!file.is_open())
    {
        return;
    }
    
    char buffer[256];
    while(!file.eof())
    {
        file.getline(buffer,256);
        rows.push_back(new std::string(buffer));
    }
    
    submeshFaces.push_back(std::vector<Face*>());
    std::vector<Face*>* faces = &submeshFaces.at(submeshFaces.size()-1);
    
    for(int i = 0; i < rows.size(); i++)
    {
        if(rows[i]->empty() || (*rows[i])[0] == '#')
            continue;
        else if((*rows[i])[0] == 'v' && (*rows[i])[1] == ' ')
        {
            float tmpx,tmpy,tmpz;
            sscanf(rows[i]->c_str(), "v %f %f %f" ,&tmpx,&tmpy,&tmpz);
            positions.push_back(new vec3(tmpx,tmpy,tmpz));
        }
        else if((*rows[i])[0] == 'v' && (*rows[i])[1] == 'n')
        {
            float tmpx,tmpy,tmpz;
            sscanf(rows[i]->c_str(), "vn %f %f %f" ,&tmpx,&tmpy,&tmpz);
            normals.push_back(new vec3(tmpx,tmpy,tmpz));
        }
        else if((*rows[i])[0] == 'v' && (*rows[i])[1] == 't')
        {
            float tmpx,tmpy;
            sscanf(rows[i]->c_str(), "vt %f %f" ,&tmpx,&tmpy);
            texcoords.push_back(new vec2(tmpx,tmpy));
        }
        else if((*rows[i])[0] == 'f')
        {
            if(count(rows[i]->begin(),rows[i]->end(), ' ') == 3)
            {
                Face* f = new Face();
                f->isQuad = false;
                sscanf(rows[i]->c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                       &f->positionIndices[0], &f->texcoordIndices[0], &f->normalIndices[0],
                       &f->positionIndices[1], &f->texcoordIndices[1], &f->normalIndices[1],
                       &f->positionIndices[2], &f->texcoordIndices[2], &f->normalIndices[2]);
                faces->push_back(f);
            }
            else
            {
                Face* f = new Face();
                f->isQuad = true;
                sscanf(rows[i]->c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                       &f->positionIndices[0], &f->texcoordIndices[0], &f->normalIndices[0],
                       &f->positionIndices[1], &f->texcoordIndices[1], &f->normalIndices[1],
                       &f->positionIndices[2], &f->texcoordIndices[2], &f->normalIndices[2],
                       &f->positionIndices[3], &f->texcoordIndices[3], &f->normalIndices[3]);
                faces->push_back(f);
            }
        }
        else if((*rows[i])[0] == 'g')
        {
            if(faces->size() > 0)
            {
                submeshFaces.push_back(std::vector<Face*>());
                faces = &submeshFaces.at(submeshFaces.size()-1);
            }
        }
    }
    
    int numberOfTriangles = 0;
    for(int iSubmesh=0; iSubmesh<submeshFaces.size(); iSubmesh++)
    {
        std::vector<Face*>& faces = submeshFaces.at(iSubmesh);
        
        for(int i=0;i<faces.size();i++)
        {
            if(faces[i]->isQuad) numberOfTriangles += 2;
            else numberOfTriangles += 1;
        }
    }
    
    nTriangles = numberOfTriangles;
    
    float *vertexCoords = new float[numberOfTriangles * 9];
    float *vertexTexCoords = new float[numberOfTriangles * 6];
    float *vertexNormalCoords = new float[numberOfTriangles * 9];
    
    
    int triangleIndex = 0;
    for(int iSubmesh=0; iSubmesh<submeshFaces.size(); iSubmesh++)
    {
        std::vector<Face*>& faces = submeshFaces.at(iSubmesh);
        
        for(int i=0;i<faces.size();i++)
        {
            if(faces[i]->isQuad)
            {
                vertexTexCoords[triangleIndex * 6] =     texcoords[faces[i]->texcoordIndices[0]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 1] = 1-texcoords[faces[i]->texcoordIndices[0]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 2] = texcoords[faces[i]->texcoordIndices[1]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 3] = 1-texcoords[faces[i]->texcoordIndices[1]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 4] = texcoords[faces[i]->texcoordIndices[2]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 5] = 1-texcoords[faces[i]->texcoordIndices[2]-1]->y;
                
                
                vertexCoords[triangleIndex * 9] =     positions[faces[i]->positionIndices[0]-1]->x;
                vertexCoords[triangleIndex * 9 + 1] = positions[faces[i]->positionIndices[0]-1]->y;
                vertexCoords[triangleIndex * 9 + 2] = positions[faces[i]->positionIndices[0]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 3] = positions[faces[i]->positionIndices[1]-1]->x;
                vertexCoords[triangleIndex * 9 + 4] = positions[faces[i]->positionIndices[1]-1]->y;
                vertexCoords[triangleIndex * 9 + 5] = positions[faces[i]->positionIndices[1]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 6] = positions[faces[i]->positionIndices[2]-1]->x;
                vertexCoords[triangleIndex * 9 + 7] = positions[faces[i]->positionIndices[2]-1]->y;
                vertexCoords[triangleIndex * 9 + 8] = positions[faces[i]->positionIndices[2]-1]->z;
                
                
                vertexNormalCoords[triangleIndex * 9] =     normals[faces[i]->normalIndices[0]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 1] = normals[faces[i]->normalIndices[0]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 2] = normals[faces[i]->normalIndices[0]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 3] = normals[faces[i]->normalIndices[1]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 4] = normals[faces[i]->normalIndices[1]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 5] = normals[faces[i]->normalIndices[1]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 6] = normals[faces[i]->normalIndices[2]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 7] = normals[faces[i]->normalIndices[2]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 8] = normals[faces[i]->normalIndices[2]-1]->z;
                
                triangleIndex++;
                
                
                vertexTexCoords[triangleIndex * 6] =     texcoords[faces[i]->texcoordIndices[1]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 1] = 1-texcoords[faces[i]->texcoordIndices[1]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 2] = texcoords[faces[i]->texcoordIndices[2]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 3] = 1-texcoords[faces[i]->texcoordIndices[2]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 4] = texcoords[faces[i]->texcoordIndices[3]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 5] = 1-texcoords[faces[i]->texcoordIndices[3]-1]->y;
                
                
                vertexCoords[triangleIndex * 9] =     positions[faces[i]->positionIndices[1]-1]->x;
                vertexCoords[triangleIndex * 9 + 1] = positions[faces[i]->positionIndices[1]-1]->y;
                vertexCoords[triangleIndex * 9 + 2] = positions[faces[i]->positionIndices[1]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 3] = positions[faces[i]->positionIndices[2]-1]->x;
                vertexCoords[triangleIndex * 9 + 4] = positions[faces[i]->positionIndices[2]-1]->y;
                vertexCoords[triangleIndex * 9 + 5] = positions[faces[i]->positionIndices[2]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 6] = positions[faces[i]->positionIndices[3]-1]->x;
                vertexCoords[triangleIndex * 9 + 7] = positions[faces[i]->positionIndices[3]-1]->y;
                vertexCoords[triangleIndex * 9 + 8] = positions[faces[i]->positionIndices[3]-1]->z;
                
                
                vertexNormalCoords[triangleIndex * 9] =     normals[faces[i]->normalIndices[1]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 1] = normals[faces[i]->normalIndices[1]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 2] = normals[faces[i]->normalIndices[1]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 3] = normals[faces[i]->normalIndices[2]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 4] = normals[faces[i]->normalIndices[2]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 5] = normals[faces[i]->normalIndices[2]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 6] = normals[faces[i]->normalIndices[3]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 7] = normals[faces[i]->normalIndices[3]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 8] = normals[faces[i]->normalIndices[3]-1]->z;
                
                triangleIndex++;
            }
            else
            {
                vertexTexCoords[triangleIndex * 6] =     texcoords[faces[i]->texcoordIndices[0]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 1] = 1-texcoords[faces[i]->texcoordIndices[0]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 2] = texcoords[faces[i]->texcoordIndices[1]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 3] = 1-texcoords[faces[i]->texcoordIndices[1]-1]->y;
                
                vertexTexCoords[triangleIndex * 6 + 4] = texcoords[faces[i]->texcoordIndices[2]-1]->x;
                vertexTexCoords[triangleIndex * 6 + 5] = 1-texcoords[faces[i]->texcoordIndices[2]-1]->y;
                
                vertexCoords[triangleIndex * 9] =     positions[faces[i]->positionIndices[0]-1]->x;
                vertexCoords[triangleIndex * 9 + 1] = positions[faces[i]->positionIndices[0]-1]->y;
                vertexCoords[triangleIndex * 9 + 2] = positions[faces[i]->positionIndices[0]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 3] = positions[faces[i]->positionIndices[1]-1]->x;
                vertexCoords[triangleIndex * 9 + 4] = positions[faces[i]->positionIndices[1]-1]->y;
                vertexCoords[triangleIndex * 9 + 5] = positions[faces[i]->positionIndices[1]-1]->z;
                
                vertexCoords[triangleIndex * 9 + 6] = positions[faces[i]->positionIndices[2]-1]->x;
                vertexCoords[triangleIndex * 9 + 7] = positions[faces[i]->positionIndices[2]-1]->y;
                vertexCoords[triangleIndex * 9 + 8] = positions[faces[i]->positionIndices[2]-1]->z;
                
                
                vertexNormalCoords[triangleIndex * 9] =     normals[faces[i]->normalIndices[0]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 1] = normals[faces[i]->normalIndices[0]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 2] = normals[faces[i]->normalIndices[0]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 3] = normals[faces[i]->normalIndices[1]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 4] = normals[faces[i]->normalIndices[1]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 5] = normals[faces[i]->normalIndices[1]-1]->z;
                
                vertexNormalCoords[triangleIndex * 9 + 6] = normals[faces[i]->normalIndices[2]-1]->x;
                vertexNormalCoords[triangleIndex * 9 + 7] = normals[faces[i]->normalIndices[2]-1]->y;
                vertexNormalCoords[triangleIndex * 9 + 8] = normals[faces[i]->normalIndices[2]-1]->z;
                
                triangleIndex++;
            }
        }
    }
    
    glBindVertexArray(vao);
    
    unsigned int vbo[3];
    glGenBuffers(3, &vbo[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, nTriangles * 9 * sizeof(float), vertexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, nTriangles * 6 * sizeof(float), vertexTexCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, nTriangles * 9 * sizeof(float), vertexNormalCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    delete vertexCoords;
    delete vertexTexCoords;
    delete vertexNormalCoords;
}


void PolygonalMesh::Draw()
{
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, nTriangles * 3);
    glDisable(GL_DEPTH_TEST);
}


PolygonalMesh::~PolygonalMesh()
{
    for(unsigned int i = 0; i < rows.size(); i++) delete rows[i];
    for(unsigned int i = 0; i < positions.size(); i++) delete positions[i];
    for(unsigned int i = 0; i < submeshFaces.size(); i++)
        for(unsigned int j = 0; j < submeshFaces.at(i).size(); j++)
            delete submeshFaces.at(i).at(j);
    for(unsigned int i = 0; i < normals.size(); i++) delete normals[i];
    for(unsigned int i = 0; i < texcoords.size(); i++) delete texcoords[i];
}



class Shader
{
protected:
    unsigned int shaderProgram;
    
public:
    Shader()
    {
        shaderProgram = 0;
    }
    
    ~Shader()
    {
        if(shaderProgram) glDeleteProgram(shaderProgram);
    }
    
    void Run()
    {
        if(shaderProgram) glUseProgram(shaderProgram);
    }
    
    virtual void UploadM(mat4& M) { }
    virtual void UploadInvM(mat4& InVM) { }
    virtual void UploadVP(mat4& VP){}
    virtual void UploadMVP(mat4& MVP) { }
    virtual void UploadSamplerCubeID(){}
    virtual void UploadRayDirMatrix(mat4& rayDirMatrix) {}
    
    // virtual void UploadColor(vec4& color) { }
    
    
    virtual void UploadSamplerID() { }
    virtual void UploadMaterialAttributes(vec3& ka, vec3& kd, vec3& ks,float& shininess) {}
    virtual void UploadLightAttributes (vec3& La, vec3& Ld, vec4& worldLightPosition){}
    virtual void UploadEyePosition (vec3& wEye){}
};

class Light
{
    vec3 La;
    vec3 Le;
    vec4 worldLightPosition;
public:
    Light(vec4 WP=vec4(0,1,0,1),vec3 LA=vec3(1,1,1),vec3 LE=vec3(1,1,1))
    {
        La=LA;
        Le=LE;
        worldLightPosition=WP;
        
    }
    void UploadAttributes(Shader* shader)
    {
        
        shader->UploadLightAttributes(La,Le,worldLightPosition);
    }
    void SetPointLightSource(vec3& pos)
    {
        worldLightPosition.v[0]=pos.x;
        worldLightPosition.v[1]=pos.y;
        worldLightPosition.v[2]=pos.z;
        worldLightPosition.v[3]=1;
        
    }
    void SetDirectionalLightSource(vec3& dir)
    {
        worldLightPosition.v[0]=dir.x;
        worldLightPosition.v[1]=dir.y;
        worldLightPosition.v[2]=dir.z;
        worldLightPosition.v[3]=0;
    }
};
Light light(vec4(1,1,1,1));

//Pitch Black
class ShadowShader : public Shader
{
public:
    ShadowShader()
    {
        const char *vertexSource = " \n\
        #version 410 \n\
        precision highp float; \n\
        \n\
        in vec3 vertexPosition; \n\
        in vec2 vertexTexCoord; \n\
        in vec3 vertexNormal; \n\
        uniform mat4 M, VP; \n\
        uniform vec4 worldLightPosition; \n\
        \n\
        void main() { \n\
        vec4 p = vec4(vertexPosition, 1) * M; \n\
        vec3 s; \n\
        s.y = -0.999; \n\
        s.x = (p.x - worldLightPosition.x) / (p.y - worldLightPosition.y) * (s.y - worldLightPosition.y) + worldLightPosition.x; \n\
        s.z = (p.z - worldLightPosition.z) / (p.y - worldLightPosition.y) * (s.y - worldLightPosition.y) + worldLightPosition.z; \n\
        gl_Position = vec4(s, 1) * VP; \n\
        } \n\
        ";
        
        const char *fragmentSource = " \n\
        #version 410 \n\
        precision highp float; \n\
        \n\
        out vec4 fragmentColor; \n\
        \n\
        void main() { \n\
        fragmentColor = vec4(0.0, 0.1, 0.0, 1); \n\
        } \n\
        ";
        
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!vertexShader) { printf("Error in vertex shader creation\n"); exit(1); }
        
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkShader(vertexShader, "Vertex shader error");
        
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragmentShader) { printf("Error in fragment shader creation\n"); exit(1); }
        
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkShader(fragmentShader, "Fragment shader error");
        
        shaderProgram = glCreateProgram();
        if (!shaderProgram) { printf("Error in shader program creation\n"); exit(1); }
        
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        
        glBindAttribLocation(shaderProgram, 0, "vertexPosition");
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        glBindAttribLocation(shaderProgram, 2, "vertexNormal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
        
        glLinkProgram(shaderProgram);
        checkLinking(shaderProgram);
        
    }
    
    void UploadM(mat4& M)
    {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M cannot be set\n");
    }
    void UploadVP(mat4& VP)
    {
        int location = glGetUniformLocation(shaderProgram, "VP");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, VP);
        else printf("uniform VP cannot be set\n");
    }
    void UploadLightAttributes (vec3& La, vec3& Le,vec4& worldLightPosition)
    {
        
        int location = glGetUniformLocation(shaderProgram, "worldLightPosition");
        if (location >= 0) glUniform4fv(location, 1, &worldLightPosition.v[0]);
        else printf("uniform worldLightPosition cannot be set\n");
        
    }
    
    
    
};
class BackgroundShader :public Shader
{
public:
    BackgroundShader()
    {
        
        const char *vertexSource = " \n\
        #version 410 \n\
        precision highp float;\n\
        in vec4 vertexPosition;\n\
        uniform mat4 rayDirMatrix;\n\
        out vec4 worldPosition;\n\
        out vec3 rayDir;\n\
        \n\
        void main(){\n\
        worldPosition = vertexPosition;\n\
        rayDir = (vertexPosition * rayDirMatrix).xyz;\n\
        gl_Position = vertexPosition;\n\
        gl_Position.z = 0.999999;\n\
        }\n\
        ";
        
        const char *fragmentSource = " \n\
        #version 410 \n\
        precision highp float;\n\
        uniform samplerCube  environmentMap;\n\
        uniform mat4 rayDirMatrix;\n\
        in vec4 worldPosition;\n\
        in vec3 rayDir;\n\
        out vec4 fragmentColor;\n\
        \n\
        void main(){\n\
        vec3 texel = texture(environmentMap, rayDir).xyz;\n\
        fragmentColor = vec4(texel, 1);\n\
        }\n\
        ";
        
        
        
        
    }
    void UploadRayDirMatrix(mat4& rayDirMatrix)
    {
        int location = glGetUniformLocation(shaderProgram, "rayDirMatrix");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, rayDirMatrix);
        else printf("uniform rayDirMatrix cannot be set\n");
    }
    void UploadSamplerCubeID()
    {
        int samplerCube = 1;
        int location = glGetUniformLocation(shaderProgram, "environmentMap");
        glUniform1i(location, samplerCube);
        glActiveTexture(GL_TEXTURE0 + samplerCube);
    }
    
};

class MeshShader : public Shader
{
public:
    MeshShader()
    {
        const char *vertexSource = " \n\
        #version 410 \n\
        precision highp float; \n\
        in vec3 vertexPosition; \n\
        in vec2 vertexTexCoord; \n\
        in vec3 vertexNormal; \n\
        uniform mat4 M, InvM, MVP; \n\
        uniform vec3 worldEyePosition; \n\
        uniform vec4 worldLightPosition; \n\
        out vec2 texCoord; \n\
        out vec3 worldNormal; \n\
        out vec3 worldView; \n\
        out vec3 worldLight; \n\
        out vec3 R;\n\
        \n\
        void main() { \n\
        texCoord = vertexTexCoord; \n\
        vec4 worldPosition = vec4(vertexPosition, 1) * M; \n\
        worldLight  = worldLightPosition.xyz * worldPosition.w - worldPosition.xyz * worldLightPosition.w; \n\
        worldView = worldEyePosition - worldPosition.xyz; \n\
        worldNormal = (InvM * vec4(vertexNormal, 0.0)).xyz; \n\
        vec3 ViewDir=normalize(worldPosition.xyz-worldEyePosition);\n\
        R=reflect(ViewDir,worldNormal);\n\
        gl_Position = vec4(vertexPosition, 1) * MVP; \n\
        } \n\
        ";
        
        // R=ViewDir - 2.0 * dot(worldNormal, ViewDir) * worldNormal;\n\
        
        const char *fragmentSource = " \n\
        #version 410 \n\
        precision highp float; \n\
        uniform sampler2D samplerUnit; \n\
        uniform vec3 La, Le; \n\
        uniform vec3 ka, kd, ks; \n\
        uniform float shininess; \n\
        uniform samplerCube environmentMap;\n\
        in vec2 texCoord; \n\
        in vec3 worldNormal; \n\
        in vec3 worldView; \n\
        in vec3 worldLight; \n\
        in vec3 R;\n\
        out vec4 fragmentColor; \n\
        \n\
        void main() { \n\
        vec3 N = normalize(worldNormal); \n\
        vec3 V = normalize(worldView); \n\
        vec3 L = normalize(worldLight); \n\
        vec3 H = normalize(V + L); \n\
        vec3 texel = texture(samplerUnit, texCoord).xyz; \n\
        vec3 texel2=texture(environmentMap,R).xyz;\n\
        vec3 color = \n\
        La * ka +Le * kd * texel* max(0.0, dot(L, N))  + \n\
        Le * ks * pow(max(0.0, dot(H, N)), shininess); \n\
        fragmentColor = vec4(color*0.5+texel2*0.5, 1); \n\
        } \n\
        ";
        
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!vertexShader) { printf("Error in vertex shader creation\n"); exit(1); }
        
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkShader(vertexShader, "Vertex shader error");
        
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragmentShader) { printf("Error in fragment shader creation\n"); exit(1); }
        
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkShader(fragmentShader, "Fragment shader error");
        
        shaderProgram = glCreateProgram();
        if (!shaderProgram) { printf("Error in shader program creation\n"); exit(1); }
        
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        
        glBindAttribLocation(shaderProgram, 0, "vertexPosition");
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        glBindAttribLocation(shaderProgram, 2, "vertexNormal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
        
        glLinkProgram(shaderProgram);
        checkLinking(shaderProgram);
    }
    
    void UploadSamplerID()
    {
        int samplerUnit = 0;
        int location = glGetUniformLocation(shaderProgram, "samplerUnit");
        glUniform1i(location, samplerUnit);
        glActiveTexture(GL_TEXTURE0 + samplerUnit);
    }
    
    void UploadSamplerCubeID()
    {
        int samplerCube = 1;
        int location = glGetUniformLocation(shaderProgram, "environmentMap");
        glUniform1i(location, samplerCube);
        glActiveTexture(GL_TEXTURE0 + samplerCube);
    }
    
    void UploadM(mat4& M)
    {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M cannot be set\n");
    }
    void UploadInvM(mat4& InvM)
    {
        int location = glGetUniformLocation(shaderProgram, "InvM");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, InvM);
        else printf("uniform InvM cannot be set\n");
    }
    
    void UploadMVP(mat4& MVP)
    {
        int location = glGetUniformLocation(shaderProgram, "MVP");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVP);
        else printf("uniform MVP cannot be set\n");
    }
    
    void UploadMaterialAttributes (vec3& ka, vec3& kd, vec3& ks,float& shininess)
    {
        //std::cout<<"here1"<<std::endl;
        // printf("ka: %f,%f,%f\n",ka.x,ka.y,ka.z);
        int location = glGetUniformLocation(shaderProgram, "ka");
        if (location >= 0) glUniform3fv(location, 1,&ka.x);
        else printf("uniform ka cannot be set\n");
        
        // printf("kd: %f,%f,%f\n",kd.x,kd.y,kd.z);
        location = glGetUniformLocation(shaderProgram, "kd");
        if (location >= 0) glUniform3fv(location, 1,&kd.x);
        else printf("uniform kd cannot be set\n");
        
        // printf("ks: %f,%f,%f\n",ks.x,ks.y,ks.z);
        location = glGetUniformLocation(shaderProgram, "ks");
        if (location >= 0) glUniform3fv(location, 1,&ks.x);
        else printf("uniform ks cannot be set\n");
        
        //printf("shininess: %f\n",shininess);
        location = glGetUniformLocation(shaderProgram, "shininess");
        if (location >= 0) glUniform1f(location,shininess);
        else printf("uniform shininess cannot be set\n");
        
    }
    void UploadLightAttributes (vec3& La, vec3& Le,vec4& worldLightPosition)
    {
        // printf("La: %f\n",La.x);
        
        int location = glGetUniformLocation(shaderProgram, "La");
        if (location >= 0) glUniform3fv(location, 1,&La.x);
        else printf("uniform La cannot be set\n");
        
        //printf("Le: %f\n",Le.x);
        location = glGetUniformLocation(shaderProgram, "Le");
        if (location >= 0) glUniform3fv(location, 1,&Le.x);
        else printf("uniform Le cannot be set\n");
        
        // printf("%f,%f,%f\n: ",worldLightPosition.v[0],worldLightPosition.v[1],worldLightPosition.v[2]);
        
        //  printf("worldLightPosition: %f,%f,%f,%f\n",worldLightPosition.v[0],worldLightPosition.v[1],worldLightPosition.v[2],worldLightPosition.v[3]);
        location = glGetUniformLocation(shaderProgram, "worldLightPosition");
        if (location >= 0) glUniform4fv(location, 1, &worldLightPosition.v[0]);
        else printf("uniform worldLightPosition cannot be set\n");
        
    }
    void UploadEyePosition (vec3& wEye )
    {
        int location = glGetUniformLocation(shaderProgram, "worldEyePosition");
        if (location >= 0) glUniform3fv(location, 1,&wEye.x);
        else printf("uniform wEye cannot be set\n");
    }
};

class DeadSolidShader : public Shader
{
public:
    DeadSolidShader()
    {
        const char *vertexSource = " \n\
        #version 410 \n\
        precision highp float; \n\
        in vec3 vertexPosition; \n\
        in vec2 vertexTexCoord; \n\
        in vec3 vertexNormal; \n\
        uniform mat4 M, InvM, MVP; \n\
        uniform vec3 worldEyePosition; \n\
        uniform vec4 worldLightPosition; \n\
        out vec2 texCoord; \n\
        out vec3 worldNormal; \n\
        out vec3 worldView; \n\
        out vec3 worldLight; \n\
        out vec4 worldPosition; \n\
        out vec3 vertexPosition1; \n\
        \n\
        void main() { \n\
        texCoord = vertexTexCoord; \n\
        worldPosition = vec4(vertexPosition, 1) * M; \n\
        worldLight  = worldLightPosition.xyz * worldPosition.w - worldPosition.xyz * worldLightPosition.w; \n\
        worldView = worldEyePosition - worldPosition.xyz; \n\
        worldNormal = (InvM * vec4(vertexNormal, 0.0)).xyz; \n\
        gl_Position = vec4(vertexPosition, 1) * MVP; \n\
        vertexPosition1=vertexPosition; \n\
        } \n\
        ";
        
        
        const char *fragmentSource = " \n\
        #version 410 \n\
        precision highp float; \n\
        uniform sampler2D samplerUnit; \n\
        uniform vec3 La, Le; \n\
        uniform vec3 ka, kd, ks; \n\
        uniform float shininess; \n\
        in vec2 texCoord; \n\
        in vec3 worldNormal; \n\
        in vec3 worldView; \n\
        in vec3 worldLight; \n\
        in vec4 worldPosition;\n\
        in vec3 vertexPosition1;\n\
        out vec4 fragmentColor; \n\
        \n\
        void main() { \n\
        vec3 N = normalize(worldNormal); \n\
        vec3 V = normalize(worldView); \n\
        vec3 L = normalize(worldLight); \n\
        vec3 H = normalize(V + L); \n\
        float scale = 1;\n\
        float turbulence = 500;\n\
        float period = 8;\n\
        float sharpness = 10;\n\
        vec3 s = vec3(7502, 22777, 4767);\n\
        float w = 0.0;\n\
        for(int i=0; i<16; i++) {\n\
        w += sin(dot(s - vec3(32768, 32768, 32768),worldPosition.xyz * scale * 40.0) / 65536.0);\n\
        s = mod(s, 32768.0) * 2.0 + floor(s / 32768.0);\n\
        }\n\
        float value= w / 32.0 + 0.5;\n\
        w = worldPosition.x * period+pow(value, sharpness)*turbulence; \n\
        w -= int(w );  \n\
        vec3 lightWood=vec3(1, 0.3, 0) * w ;\n\
        vec3 darkWood=vec3(0.35, 0.1, 0.05) * (1-w);\n\
        vec3 result=lightWood+darkWood;\n\
        vec3 color = \n\
        La * ka +Le * kd * result* max(0.0, dot(L, N))  + \n\
        Le * ks * pow(max(0.0, dot(H, N)), shininess); \n\
        fragmentColor = vec4(color, 1); \n\
        } \n\
        ";
        //
        //        \n\
        //        float f(vec3 r){ \n\
        //            vec3 s = vec3(7502, 22777, 4767);\n\
        //            float w = 0.0;\n\
        //            for(int i=0; i<16; i++) {\n\
        //                w += sin(1);\n\
        //                w=sin( dot(s - vec3(32768, 32768, 32768),r * 40.0) / 65536.0);\n\
        //                s = mod(s, 32768.0) * 2.0 + floor(s / 32768.0);\n\
        //            }\n\
        //            return w / 32.0 + 0.5;\n\
        //        }\n\
        //        \n\
        //        vec3 getColor(vec3 position) {\n\
        //            float scale = 16;\n\
        //            float turbulence = 500;\n\
        //            float period = 8;\n\
        //            float sharpness = 10;\n\
        //            vec3 s = vec3(7502, 22777, 4767);\n\
        //            float w = position.x * period+pow(f(position * scale), sharpness)*turbulence; \n\
        //            w -= int(w + 10000.0);  \n\
        //            vec3 lightWood=vec3(1, 0.3, 0) * w ;\n\
        //            vec3 darkWood=vec3(0.35, 0.1, 0.05) * (1-w);\n\
        //            return lightWood+darkWood;\n\
        //        }\n\
        
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!vertexShader) { printf("Error in vertex shader creation\n"); exit(1); }
        
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkShader(vertexShader, "Vertex shader error");
        
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragmentShader) { printf("Error in fragment shader creation\n"); exit(1); }
        
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkShader(fragmentShader, "Fragment shader error");
        
        shaderProgram = glCreateProgram();
        if (!shaderProgram) { printf("Error in shader program creation\n"); exit(1); }
        
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        
        glBindAttribLocation(shaderProgram, 0, "vertexPosition");
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        glBindAttribLocation(shaderProgram, 2, "vertexNormal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
        
        glLinkProgram(shaderProgram);
        checkLinking(shaderProgram);
    }
    
    void UploadSamplerID()
    {
        int samplerUnit = 0;
        int location = glGetUniformLocation(shaderProgram, "samplerUnit");
        glUniform1i(location, samplerUnit);
        glActiveTexture(GL_TEXTURE0 + samplerUnit);
    }
    void UploadM(mat4& M)
    {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M cannot be set\n");
    }
    void UploadInvM(mat4& InvM)
    {
        int location = glGetUniformLocation(shaderProgram, "InvM");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, InvM);
        else printf("uniform InvM cannot be set\n");
    }
    
    void UploadMVP(mat4& MVP)
    {
        int location = glGetUniformLocation(shaderProgram, "MVP");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVP);
        else printf("uniform MVP cannot be set\n");
    }
    
    void UploadMaterialAttributes (vec3& ka, vec3& kd, vec3& ks,float& shininess)
    {
        //std::cout<<"here1"<<std::endl;
        // printf("ka: %f,%f,%f\n",ka.x,ka.y,ka.z);
        int location = glGetUniformLocation(shaderProgram, "ka");
        if (location >= 0) glUniform3fv(location, 1,&ka.x);
        else printf("uniform ka cannot be set\n");
        
        // printf("kd: %f,%f,%f\n",kd.x,kd.y,kd.z);
        location = glGetUniformLocation(shaderProgram, "kd");
        if (location >= 0) glUniform3fv(location, 1,&kd.x);
        else printf("uniform kd cannot be set\n");
        
        // printf("ks: %f,%f,%f\n",ks.x,ks.y,ks.z);
        location = glGetUniformLocation(shaderProgram, "ks");
        if (location >= 0) glUniform3fv(location, 1,&ks.x);
        else printf("uniform ks cannot be set\n");
        
        //printf("shininess: %f\n",shininess);
        location = glGetUniformLocation(shaderProgram, "shininess");
        if (location >= 0) glUniform1f(location,shininess);
        else printf("uniform shininess cannot be set\n");
        
    }
    void UploadLightAttributes (vec3& La, vec3& Le,vec4& worldLightPosition)
    {
        // printf("La: %f\n",La.x);
        
        int location = glGetUniformLocation(shaderProgram, "La");
        if (location >= 0) glUniform3fv(location, 1,&La.x);
        else printf("uniform La cannot be set\n");
        
        //printf("Le: %f\n",Le.x);
        location = glGetUniformLocation(shaderProgram, "Le");
        if (location >= 0) glUniform3fv(location, 1,&Le.x);
        else printf("uniform Le cannot be set\n");
        
        // printf("%f,%f,%f\n: ",worldLightPosition.v[0],worldLightPosition.v[1],worldLightPosition.v[2]);
        
        //  printf("worldLightPosition: %f,%f,%f,%f\n",worldLightPosition.v[0],worldLightPosition.v[1],worldLightPosition.v[2],worldLightPosition.v[3]);
        location = glGetUniformLocation(shaderProgram, "worldLightPosition");
        if (location >= 0) glUniform4fv(location, 1, &worldLightPosition.v[0]);
        else printf("uniform worldLightPosition cannot be set\n");
        
    }
    void UploadEyePosition (vec3& wEye )
    {
        int location = glGetUniformLocation(shaderProgram, "worldEyePosition");
        if (location >= 0) glUniform3fv(location, 1,&wEye.x);
        else printf("uniform wEye cannot be set\n");
    }
};


class InfiniteQuadShader  : public Shader
{
public:
    InfiniteQuadShader ()
    {
        const char *vertexSource = "\n\
        #version 410 \n\
        precision highp float; \n\
        \n\
        in vec4 vertexPosition; \n\
        in vec2 vertexTexCoord; \n\
        in vec3 vertexNormal; \n\
        uniform mat4 M, InvM, MVP; \n\
        \n\
        out vec2 texCoord; \n\
        out vec4 worldPosition; \n\
        out vec3 worldNormal; \n\
        \n\
        void main() { \n\
        texCoord = vertexTexCoord; \n\
        worldPosition = vertexPosition * M; \n\
        worldNormal = (InvM * vec4(vertexNormal, 0.0)).xyz; \n\
        gl_Position = vertexPosition * MVP; \n\
        } \n\
        ";
        
        
        const char *fragmentSource = "\n\
        #version 410 \n\
        precision highp float; \n\
        uniform sampler2D samplerUnit; \n\
        uniform vec3 La, Le; \n\
        uniform vec3 ka, kd, ks; \n\
        uniform float shininess; \n\
        uniform vec3 worldEyePosition; \n\
        uniform vec4 worldLightPosition; \n\
        in vec2 texCoord; \n\
        in vec4 worldPosition; \n\
        in vec3 worldNormal; \n\
        out vec4 fragmentColor; \n\
        void main() { \n\
        vec3 N = normalize(worldNormal); \n\
        vec3 V = normalize(worldEyePosition * worldPosition.w - worldPosition.xyz);\n\
        vec3 L = normalize(worldLightPosition.xyz * worldPosition.w - worldPosition.xyz * worldLightPosition.w);\n\
        vec3 H = normalize(V + L); \n\
        vec2 position = worldPosition.xz / worldPosition.w; \n\
        vec2 tex = position.xy - floor(position.xy); \n\
        vec3 texel = texture(samplerUnit, tex).xyz; \n\
        vec3 color = La * ka + Le * kd * texel* max(0.0, dot(L, N)) + Le * ks * pow(max(0.0, dot(H, N)), shininess); \n\
        fragmentColor = vec4(color, 1); \n\
        } \n\
        ";
        
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!vertexShader) { printf("Error in vertex shader creation\n"); exit(1); }
        
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkShader(vertexShader, "Vertex shader error");
        
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragmentShader) { printf("Error in fragment shader creation\n"); exit(1); }
        
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkShader(fragmentShader, "Fragment shader error");
        
        shaderProgram = glCreateProgram();
        if (!shaderProgram) { printf("Error in shader program creation\n"); exit(1); }
        
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        
        glBindAttribLocation(shaderProgram, 0, "vertexPosition");
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        glBindAttribLocation(shaderProgram, 2, "vertexNormal");
        
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor");
        
        glLinkProgram(shaderProgram);
        checkLinking(shaderProgram);
    }
    
    void UploadSamplerID()
    {
        int samplerUnit = 0;
        int location = glGetUniformLocation(shaderProgram, "samplerUnit");
        glUniform1i(location, samplerUnit);
        glActiveTexture(GL_TEXTURE0 + samplerUnit);
    }
    void UploadM(mat4& M)
    {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M cannot be set\n");
    }
    void UploadInvM(mat4& InvM)
    {
        int location = glGetUniformLocation(shaderProgram, "InvM");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, InvM);
        else printf("uniform InvM cannot be set\n");
    }
    
    void UploadMVP(mat4& MVP)
    {
        int location = glGetUniformLocation(shaderProgram, "MVP");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVP);
        else printf("uniform MVP cannot be set\n");
    }
    
    void UploadMaterialAttributes (vec3& ka, vec3& kd, vec3& ks,float& shininess)
    {
        //std::cout<<"here1"<<std::endl;
        // printf("ka: %f,%f,%f\n",ka.x,ka.y,ka.z);
        int location = glGetUniformLocation(shaderProgram, "ka");
        if (location >= 0) glUniform3fv(location, 1,&ka.x);
        else printf("uniform ka cannot be set\n");
        
        // printf("kd: %f,%f,%f\n",kd.x,kd.y,kd.z);
        location = glGetUniformLocation(shaderProgram, "kd");
        if (location >= 0) glUniform3fv(location, 1,&kd.x);
        else printf("uniform kd cannot be set\n");
        
        // printf("ks: %f,%f,%f\n",ks.x,ks.y,ks.z);
        location = glGetUniformLocation(shaderProgram, "ks");
        if (location >= 0) glUniform3fv(location, 1,&ks.x);
        else printf("uniform ks cannot be set\n");
        
        //printf("shininess: %f\n",shininess);
        location = glGetUniformLocation(shaderProgram, "shininess");
        if (location >= 0) glUniform1f(location,shininess);
        else printf("uniform shininess cannot be set\n");
        
    }
    void UploadLightAttributes (vec3& La, vec3& Le,vec4& worldLightPosition)
    {
        // printf("La: %f\n",La.x);
        
        int location = glGetUniformLocation(shaderProgram, "La");
        if (location >= 0) glUniform3fv(location, 1,&La.x);
        else printf("uniform La cannot be set\n");
        
        //printf("Le: %f\n",Le.x);
        location = glGetUniformLocation(shaderProgram, "Le");
        if (location >= 0) glUniform3fv(location, 1,&Le.x);
        else printf("uniform Le cannot be set\n");
        
        // printf("%f,%f,%f\n: ",worldLightPosition.v[0],worldLightPosition.v[1],worldLightPosition.v[2]);
        
        //  printf("worldLightPosition: %f,%f,%f,%f\n",worldLightPosition.v[0],worldLightPosition.v[1],worldLightPosition.v[2],worldLightPosition.v[3]);
        location = glGetUniformLocation(shaderProgram, "worldLightPosition");
        if (location >= 0) glUniform4fv(location, 1, &worldLightPosition.v[0]);
        else printf("uniform worldLightPosition cannot be set\n");
        
    }
    void UploadEyePosition (vec3& wEye )
    {
        int location = glGetUniformLocation(shaderProgram, "worldEyePosition");
        if (location >= 0) glUniform3fv(location, 1,&wEye.x);
        else printf("uniform wEye cannot be set\n");
    }
};



extern "C" unsigned char* stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);

class Texture
{
    unsigned int textureId;
    
public:
    Texture(const std::string& inputFileName)
    {
        unsigned char* data;
        int width; int height; int nComponents = 4;
        
        data = stbi_load(inputFileName.c_str(), &width, &height, &nComponents, 0);
        
        if(data == NULL)
        {
            return;
        }
        
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        if(nComponents == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        if(nComponents == 4) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        delete data;
    }
    
    void Bind()
    {
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
};


class TextureCube {
    unsigned int textureId;
public:
    TextureCube(
                const std::string& inputFileName0, const std::string& inputFileName1, const std::string& inputFileName2,
                const std::string& inputFileName3, const std::string& inputFileName4, const std::string& inputFileName5) {
        unsigned char* data[6]; int width[6]; int height[6]; int nComponents[6]; std::string filename[6];
        filename[0] = inputFileName0; filename[1] = inputFileName1; filename[2] = inputFileName2;
        filename[3] = inputFileName3; filename[4] = inputFileName4; filename[5] = inputFileName5;
        for(int i = 0; i < 6; i++) {
            data[i] = stbi_load(filename[i].c_str(), &width[i], &height[i], &nComponents[i], 0);
            if(data == NULL) return;
        }
        glGenTextures(1, &textureId); glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
        for(int i = 0; i < 6; i++) {
            if(nComponents[i] == 4) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width[i], height[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, data[i]);
            if(nComponents[i] == 3) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        for(int i = 0; i < 6; i++)  delete data[i];
    }
    
    void Bind() { glBindTexture(GL_TEXTURE_CUBE_MAP, textureId); }
};

class Material
{
    Shader* shader;
    Texture* texture;
    TextureCube *environmentMap;
    vec3 ka;
    vec3 kd;
    vec3 ks;
    float shininess;
    
    // vec4 color;
    
public:
    Material(Shader* s, Texture* t = 0,TextureCube *en=0,vec3 kA=vec3(0.1,0,0), vec3 kD=vec3(0,0,0), vec3 kS=vec3(0,0,0),float sh=0)
    {
        shader = s;
        texture = t;
        environmentMap=en;
        ka=kA;
        kd=kD;
        ks=kS;
        shininess=sh;
        
    }
    
    Shader* GetShader() { return shader; }
    
    void UploadAttributes()
    {
        if(texture)
        {
            //                                                                                                                                                                                                                                                                                                                                                  printf("%s","here");
            // std::cout<<"here2"<<std::endl;
            
            shader->UploadSamplerID();
            
            
            texture->Bind();
            shader->UploadSamplerCubeID();
            environmentMap->Bind();
        }
        shader->UploadMaterialAttributes(ka,kd,ks,shininess);
        //      else
        // shader->UploadColor(color);
    }
};

class Mesh
{
    Geometry* geometry;
    Material* material;
    
public:
    Mesh(Geometry* g, Material* m)
    {
        geometry = g;
        material = m;
    }
    
    Shader* GetShader() { return material->GetShader(); }
    
    void Draw()
    {
        material->UploadAttributes();
        geometry->Draw();
    }
};


vec3 avatarPosition;


class Camera {
public:
    vec3  wEye, wLookat, wVup;
    float fov, asp, fp, bp;
    vec3 wEyeOriginal;
    vec3 wLookatOriginal;
    bool heliCam=false;
    
public:
    Camera()
    {
        wEye = vec3(0.0, 0.0, 2.0);
        wLookat = vec3(0.0, 0.0, 0.0);
        wVup = vec3(0.0, 1.0, 0.0);
        fov = M_PI / 4.0; asp = 1.0; fp = 0.01; bp = 100.0;
        wEyeOriginal=wEye;
        wLookatOriginal=wLookat;
    }
    void setCamera(vec3 position, vec3 direction)
    {
        wEye=position;
        wLookat=position+direction;
    }
    void SetAspectRatio(float a) { asp = a; }
    
    mat4 GetViewMatrix()
    {
        vec3 w = (wEye - wLookat).normalize();
        vec3 u = cross(wVup, w).normalize();
        vec3 v = cross(w, u);
        
        return
        mat4(
             1.0f,    0.0f,    0.0f,    0.0f,
             0.0f,    1.0f,    0.0f,    0.0f,
             0.0f,    0.0f,    1.0f,    0.0f,
             -wEye.x, -wEye.y, -wEye.z, 1.0f ) *
        mat4(
             u.x,  v.x,  w.x,  0.0f,
             u.y,  v.y,  w.y,  0.0f,
             u.z,  v.z,  w.z,  0.0f,
             0.0f, 0.0f, 0.0f, 1.0f );
    }
    
    mat4 GetProjectionMatrix()
    {
        float sy = 1/tan(fov/2);
        return mat4(
                    sy/asp, 0.0f,  0.0f,               0.0f,
                    0.0f,   sy,    0.0f,               0.0f,
                    0.0f,   0.0f, -(fp+bp)/(bp - fp), -1.0f,
                    0.0f,   0.0f, -2*fp*bp/(bp - fp),  0.0f);
    }
    mat4 GetInverseViewMatrix() {
        vec3 w = (wEye - wLookat).normalize();
        vec3 u = cross(wVup, w).normalize();
        vec3 v = cross(w, u);
        return mat4(
                    u.x,  u.y,  u.z,  0.0f,
                    v.x,  v.y,  v.z,  0.0f,
                    w.x,  w.y,  w.z,  0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f );
    }
    mat4 GetInverseProjectionMatrix() {
        float sy = 1/tan(fov/2);
        return mat4(
                    asp/sy,        0.0f,        0.0f,        0.0f,
                    0.0f,        1.0/sy,        0.0f,        0.0f,
                    0.0f,        0.0f,        0.0f,        (fp - bp) / 2.0 / fp / bp,
                    0.0f,        0.0f,        -1.0f,        (fp + bp) / 2.0 / fp / bp);
    }
    
    
    void control()
    {
        
    }
    void UploadAttributes (Shader* shader)
    {
        shader->UploadEyePosition(wEye);
        mat4 rayDirMatrix =
        GetInverseProjectionMatrix() * GetInverseViewMatrix();
        
        shader->UploadRayDirMatrix (rayDirMatrix);
        
    }
    vec3 GetEyePosition()
    {
        return wEye;
    }
    void move(float dt,float t)
    {
        if(keyboardState['s'])
        {
            wEye=wEye+((wEye-wLookat).normalize()*dt);
            wLookat=wLookat+((wEye-wLookat).normalize()*dt);
        }
        if(keyboardState['w'])
        {
            wEye=wEye-((wEye-wLookat).normalize()*dt);
            wLookat=wLookat-((wEye-wLookat).normalize()*dt);
        }
        if(keyboardState['d'])
        {
            float angularVelocity=10;
            vec3 w=(wLookat-wEye).normalize();
            float l=w.length();
            vec3 v=wVup.normalize();
            vec3 u=cross(w,v);
            float alpha=(angularVelocity*dt)*M_PI /100.0;
            wLookat=wEye+(w*cos(alpha)+u*sin(alpha))*l;
        }
        if(keyboardState['a'])
        {
            //Put in separate GetAhead Method;
            //Control changes angular and regular velocity
            // Control also has return after changing velocity or angular velocity
            
            float angularVelocity=10;
            vec3 w=(wLookat-wEye).normalize();
            float l=w.length();
            vec3 v=wVup.normalize();
            vec3 u=cross(w,v);
            float alpha=(-(angularVelocity*dt)*M_PI /100.0);
            wLookat=wEye+(w*cos(alpha)+u*sin(alpha))*l;
        }
        //Keep Watching
        if(keyboardState['h'])
        {
            wEyeOriginal=wEye;//Helps with reseting camera when you use helicam and then tracking.
            heliCam=true;
        }
        if(keyboardState['u'])
        {
            wEyeOriginal=wEye;//Helps with reseting camera when you use helicam and then tracking.
            heliCam=false;
        }
        if(heliCam)
        {
            //            //Helicam
            wEye=avatarPosition+vec3(-0.5,2,-5);
            wLookat=avatarPosition;
            
            
        }
        
        
        
    }
    //Tracking
    void startTracking(float t)
    {
        //std::cout<<"Reached"<<std::endl;
        wEye=wEye+vec3(0.01*cos(t),0,0.01*sin(t));
        //wLookat=wLookat+10*cos(t);
    }
    
    void stopTracking(float t)
    {
        wEye=wEyeOriginal;
        //        wLookat=wLookatOriginal;
        
    }
    
    
    
    
};

Camera camera;


class Object
{
    
public:
    Shader* shader;
    Shader* shadowShader;
    Mesh *mesh;
    
    vec3 position;
    vec3 scaling;
    float orientation;
    
    
    vec3 velocity;
    vec3 acceleration;
    vec3 angularVelocity;
    vec3 angularAcceleration;
    
    vec3 pyr;
    bool alive=true;
    bool attached=false;
    bool dropped=false;
    
    
public:
    Object(Mesh *m, vec3 position = vec3(0.0, 0.0, 0.0), vec3 scaling = vec3(1.0, 1.0, 1.0), float orientation = 0.0) : position(position), scaling(scaling), orientation(orientation)
    {
        shader = m->GetShader();
        mesh = m;
    }
    
    vec3& GetPosition() { return position; }
    virtual void Interact(Object* object) { }
    enum OBJECT_TYPE { TIGGER, GREENTREE,REDTREE, GROUND,HELICOPTER,BULLET,PARENTTIGGER };
    virtual OBJECT_TYPE GetType() = 0;
    virtual void control(float dt) { }
    virtual void isAttached(){};
    virtual void drop(){};
    void Draw()
    {
        shader->Run();
        UploadAttributes();
        vec3 eye=camera.GetEyePosition();
        vec3 temp=vec3(camera.wLookat+vec3(0,3,0));
        light.SetPointLightSource(temp);// Spotlight
        
        if(keyboardState['p'])
        {
            light.SetPointLightSource(eye);
            // other=eye;
        }
        
        light.UploadAttributes(shader);
        camera.UploadAttributes(shader);
        mesh->Draw();
    }
    
    virtual void UploadAttributes(Shader* shadowShader=nullptr)
    {
        mat4 T = mat4(
                      1.0,            0.0,            0.0,            0.0,
                      0.0,            1.0,            0.0,            0.0,
                      0.0,            0.0,            1.0,            0.0,
                      position.x,        position.y,        position.z,        1.0);
        
        mat4 InvT = mat4(
                         1.0,            0.0,            0.0,            0.0,
                         0.0,            1.0,            0.0,            0.0,
                         0.0,            0.0,            1.0,            0.0,
                         -position.x,    -position.y,    -position.z,    1.0);
        
        mat4 S = mat4(
                      scaling.x,        0.0,            0.0,            0.0,
                      0.0,            scaling.y,        0.0,            0.0,
                      0.0,            0.0,            scaling.z,        0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvS = mat4(
                         1.0/scaling.x,    0.0,            0.0,            0.0,
                         0.0,            1.0/scaling.y,    0.0,            0.0,
                         0.0,            0.0,            1.0/scaling.z,    0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        float alpha = orientation / 180.0 * M_PI;
        
        mat4 R = mat4(
                      cos(alpha),        0.0,            sin(alpha),        0.0,
                      0.0,            1.0,            0.0,            0.0,
                      -sin(alpha),    0.0,            cos(alpha),        0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvR = mat4(
                         cos(alpha),        0.0,            -sin(alpha),    0.0,
                         0.0,            1.0,            0.0,            0.0,
                         sin(alpha),        0.0,            cos(alpha),        0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        mat4 M = S * R * T;
        mat4 InvM = InvT * InvR * InvS;
        mat4 VP =camera.GetViewMatrix() * camera.GetProjectionMatrix();
        mat4 MVP = M *camera.GetViewMatrix() * camera.GetProjectionMatrix();
        
        
        if(shadowShader)
        {
            shadowShader->UploadM(M);
            shadowShader->UploadVP(VP);
        }
        else
        {
            shader->UploadM(M);
            shader->UploadInvM(InvM);
            shader->UploadMVP(MVP);
            
        }
        
    }
    
    virtual void DrawShadow(Shader* shadowShader=nullptr)
    {
        shadowShader->Run();
        
        UploadAttributes(shadowShader);
        vec3 lightL=vec3(0.0, 100.0, 0.0);// Sunshine
        vec3 lightL2=vec3(10, 10, 0.0);//Sunset Boulevard
        light.SetDirectionalLightSource(lightL);
        if(keyboardState['n'])
        {
            light.SetDirectionalLightSource(lightL2);
        }
        
        light.UploadAttributes(shadowShader);
        
        camera.UploadAttributes(shadowShader);
        
        mesh->Draw();
    }
    virtual void move(float dt)
    {
        
    }
    
};

class GreenTree:public Object
{
public:
    using::Object::Object;
    
    OBJECT_TYPE GetType()
    {
        return GREENTREE;
    }
    void control(float dt)
    {
        
    }
    void move(float dt)
    {
        
    }
    
};
class RedTree:public Object
{
    
public:
    using::Object::Object;
    
    OBJECT_TYPE GetType()
    {
        return REDTREE;
    }
    void control(float dt)
    {
        
    }
    void move(float dt)
    {
        
    }
    void Interact(Object* secondObject)
    {
        OBJECT_TYPE id=secondObject->GetType();
        switch (id) {
            case Object::BULLET:
                if((position-secondObject->position).length()<1)
                {
                    scaling=vec3(0.00001,0.00001,0.00001);
                    alive=false;
                }
                
                break;
                
            default:
                break;
        }
    }
};
vec3 globalGetAhead;
float globalOrientation;


int sgn(float val) {
    return (float(0) < val) - (val < float(0));
};

class Avatar:public Object
{
public:
    
    //vec3  wEye, wLookat, wVup, forward;
    float accelerationConstant;
    bool crashed=false;
    
    
public:
    Avatar(Mesh *m, vec3 position = vec3(0.0, 0.0, 0.0), vec3 scaling = vec3(1.0, 1.0, 1.0), float orientation = 0.0):Object(m,position,scaling,orientation)
    {
        avatarPosition=position;
        velocity=vec3(0,0,0);
        acceleration=vec3(0,0,0);
        angularVelocity=vec3(0,0,0);
        angularAcceleration=vec3(0,0,0);
        accelerationConstant=0;
        
    }
    void Interact(Object* secondObject)
    {
        OBJECT_TYPE id=secondObject->GetType();
        switch (id) {
            case Object::REDTREE:
                if(abs(((position+getAhead())-secondObject->position).length())<1)
                {
                    //std::cout<<"Hit"<<std::endl;
                    if(secondObject->alive)
                    {
                        
                        lives--;
                         std::cout<<"Oh No! You crashed a tree, you have "<<lives<<" remaining!"<<std::endl;
                        if(lives==0)
                        {
                            crash();
                            alive=false;
                        }
                        else
                        {
                            position=vec3(-1, -0.99, -3);
                            velocity=vec3(0,0,0);
                            acceleration=vec3(0,0,0);
                        }
                       
                        
                    }
                    
                }
            case Object::GREENTREE:
                if(abs(((position+getAhead())-secondObject->position).length())<1)
                {
                    //std::cout<<"Hit"<<std::endl;
                    if(secondObject->alive)
                    {
                        secondObject->scaling=vec3(0.000001,0.000001,0.000001);
                        secondObject->alive=false;
                        score++;
                        std::cout<<"Nice! Your new score is: "<<score<<std::endl;
                    }
                    
                }
                
                break;
            case Object::TIGGER:
                if((abs(((position+getAhead())-secondObject->position).length())<1)&&!secondObject->dropped)
                {
                    //std::cout<<"Hit"<<std::endl;
                    if(secondObject->alive)
                    {
                        if(score>4)
                        {
                          secondObject->isAttached();
                        }
                        else
                        {
                            std::cout<<"Need more trees"<<std::endl;
                        }
                        
                    }
                    if(keyboardState['g'])
                    {
                        secondObject->drop();
                        
                    }
                    
                }
                
                break;
                
            default:
                break;
        }
    }
    void crash()
    {
        crashed=true;
    }
    virtual void UploadAttributes(Shader* shadowShader=nullptr)
    {
        mat4 T = mat4(
                      1.0,            0.0,            0.0,            0.0,
                      0.0,            1.0,            0.0,            0.0,
                      0.0,            0.0,            1.0,            0.0,
                      position.x,        position.y,        position.z,        1.0);
        
        mat4 InvT = mat4(
                         1.0,            0.0,            0.0,            0.0,
                         0.0,            1.0,            0.0,            0.0,
                         0.0,            0.0,            1.0,            0.0,
                         -position.x,    -position.y,    -position.z,    1.0);
        
        mat4 S = mat4(
                      scaling.x,        0.0,            0.0,            0.0,
                      0.0,            scaling.y,        0.0,            0.0,
                      0.0,            0.0,            scaling.z,        0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvS = mat4(
                         1.0/scaling.x,    0.0,            0.0,            0.0,
                         0.0,            1.0/scaling.y,    0.0,            0.0,
                         0.0,            0.0,            1.0/scaling.z,    0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        //  float alpha = orientation / 180.0 * M_PI;
        
        //        mat4 R = mat4(
        //                      cos(alpha),        0.0,            sin(alpha),        0.0,
        //                      0.0,            1.0,            0.0,            0.0,
        //                      -sin(alpha),    0.0,            cos(alpha),        0.0,
        //                      0.0,            0.0,            0.0,            1.0);
        //pyr (nose up and down, nose side to side, rolling)
        // pyr=vec3(0,2*M_PI,0)+(orientation/180*M_PI);
        
        //        pyr=vec3(0,0,0);
        mat4 Roll=mat4(
                       cos(pyr.z),-sin(pyr.z),0,0,
                       sin(pyr.z), cos(pyr.z),0,0,
                       0,0,1,0,
                       0,0,0,1);
        
        mat4 InvRoll=mat4(
                          cos(pyr.z),sin(pyr.z),0,0,
                          -sin(pyr.z), cos(pyr.z),0,0,
                          0,0,1,0,
                          0,0,0,1);
        
        mat4 Pitch=mat4(
                        1,0,0,0,
                        0, cos(pyr.x),-sin(pyr.x),0,
                        0,sin(pyr.x),cos(pyr.x),0,
                        0,0,0,1);
        
        mat4 InvPitch=mat4(
                           1,0,0,0,
                           0, cos(pyr.x),sin(pyr.x),0,
                           0,-sin(pyr.x),cos(pyr.x),0,
                           0,0,0,1);
        
        mat4 Yaw=mat4(
                      cos(pyr.y),0,sin(pyr.y),0,
                      0, 1,0,0,
                      -sin(pyr.y),0,cos(pyr.y),0,
                      0,0,0,1);
        
        mat4 InvYaw=mat4(
                         cos(pyr.y),0,-sin(pyr.y),0,
                         0, 1,0,0,
                         sin(pyr.y),0,cos(pyr.y),0,
                         0,0,0,1);
        
        mat4 R= Roll*Pitch*Yaw;
        
        
        //        mat4 InvR = mat4(
        //                         cos(alpha),        0.0,            -sin(alpha),    0.0,
        //                         0.0,            1.0,            0.0,            0.0,
        //                         sin(alpha),        0.0,            cos(alpha),        0.0,
        //                         0.0,            0.0,            0.0,            1.0);
        
        mat4 InvR=InvYaw*InvPitch*InvRoll;
        mat4 M = S * R * T;
        mat4 InvM = InvT * InvR * InvS;
        mat4 VP =camera.GetViewMatrix() * camera.GetProjectionMatrix();
        mat4 MVP = M *camera.GetViewMatrix() * camera.GetProjectionMatrix();
        
        
        if(shadowShader)
        {
            shadowShader->UploadM(M);
            shadowShader->UploadVP(VP);
        }
        else
        {
            shader->UploadM(M);
            shader->UploadInvM(InvM);
            shader->UploadMVP(MVP);
            
        }
        
    }
    vec3 getAhead()
    
    {
        globalGetAhead=vec3(cos(pyr.y+M_PI/2),0,sin(pyr.y+M_PI/2));
        return vec3(cos(pyr.y+M_PI/2),0,sin(pyr.y+M_PI/2));
    }
    void control(float dt)
    {
        if(keyboardState['i'])
        {
            // position=position-forward*dt;
            if(accelerationConstant<1)
            {
                accelerationConstant+=0.1*accelerationConstant+0.1;
                
            }
            
        }
        else if(keyboardState['k'])
        {
            //position=position+forward*dt;
            if(accelerationConstant>-1)
            {
                accelerationConstant-=0.1*accelerationConstant+0.1;
                
            }
            
        }
        //In the case you are not pushing any buttons, you should be slowing down
        if(keyboardState['b'])
        {
            accelerationConstant=0;
            velocity=velocity/1.05;
        }
        if(keyboardState['l'])
        {
            pyr.y+=0.5*dt;
            
            
        }
        if(keyboardState['j'])
        {
            pyr.y-=0.5*dt;
            
            
        }
        if (keyboardState['w']) {
            acceleration.y+=1*dt;
            
        }
        else if (keyboardState['s']) {
            acceleration.y-=2*dt;
            
        }
        else{
            acceleration.y=0;
        }
        if (keyboardState['q']) {
            accelerationConstant=0;
            velocity.y=velocity.y/1.05;
            
        }
        if (keyboardState['h']) {
            position=vec3(0,0,0);
            
        }
        if(saved&&position.y<0)
        {
            std::cout<<position.y<<std::endl;
            level++;
        }
    }
    void move(float dt)
    {
        // std::cout<<velocity.x<<std::endl;
        
        position=velocity*dt+position;
        if(velocity.length()<2)
        {
            velocity=acceleration*10*dt+velocity;
        }
        else
        {
            velocity=velocity/2;
        }
        
        //        acceleration.x = accelerationConstant*cos(pyr.y +M_PI/2)*10*dt;
        //        acceleration.z = accelerationConstant*sin(pyr.y+M_PI/2 )*10*dt;
        //
        acceleration=getAhead()*accelerationConstant;
        //  (getAhead().normalize()).print();
        
        //        camera.wEye=(position-getAhead()).normalize();
        //        camera.wLookat=position+vec3(0,1,0);
        
        if(!crashed)
        {
            camera.setCamera((position-getAhead()*5+vec3(0,1.5,0)), getAhead());
        }
        
        
        pyr.y=(angularVelocity.y*dt)+pyr.y;
        globalOrientation=pyr.y;
        avatarPosition=position;
        
        if(position.y>6)
        {
            velocity.y=0;
            acceleration.y=0;
            position.y=5.9999;
        }
        
        if(position.y<-1&&!crashed)
        {
            velocity.y=0;
            acceleration.y=0;
            position.y=-0.9999;
        }
        
        
        if(!crashed)
        {
            control(dt);
        }
        else
        {
            position=position+vec3(0,-1,0)*dt;
            velocity=vec3(0,0,0);
            acceleration=vec3(0,0,0);
            
        }
        
        
        
    }
    
    
    
    OBJECT_TYPE GetType()
    {
        return HELICOPTER;
    }
    
    
    
    
};
class Tigger:public Object
{

public:
    using::Object::Object;
    
    OBJECT_TYPE GetType()
    {
        return TIGGER;
    }
    void control(float dt)
    {
        
    }
    void move(float dt)
    {
        if(attached)
        {
            position=avatarPosition+vec3(-0.2,0,0);
            orientation=(-globalOrientation);
            saved=true;
        }
        if(dropped)
        {
            std::cout<<"This is continuously reached"<<std::endl;
            position=position+vec3(0,-1,0)*dt;
            alive=false;
        }
    }
    void isAttached()
    {
        attached=true;
    }
    void drop()
    {
        attached=false;
        dropped=true;
        
    }
    
};

class ParentTigger:public Object
{
    
public:
    using::Object::Object;
    
    OBJECT_TYPE GetType()
    {
        return PARENTTIGGER;
    }

    
};
class Bullet:public Object
{
public:
    vec3 direction;
public:
    Bullet(Mesh *m, vec3 position = vec3(0.0, 0.0, 0.0), vec3 scaling = vec3(1.0, 1.0, 1.0), float orientation = 100) :Object(m,position,scaling,orientation)
    {
        shader = m->GetShader();
        mesh = m;
        direction=globalGetAhead;
    }
    
    OBJECT_TYPE GetType()
    {
        return BULLET;
    }
    void control(float dt)
    {
        
    }
    void move(float dt)
    {
        position=position+direction*10*dt;
    }
};

class RotatingObject:public Object
{
    
public:
    using::Object::Object;
    void move(float dt)
    {
        pyr.y=pyr.y+10*dt;
        position=avatarPosition+vec3(0,0.75,0);
        if(position.y<-1)
        {
            alive=false;
        }
        control(dt);
    }
    OBJECT_TYPE GetType()
    {
        return HELICOPTER;
    }
    void control(float dt)
    {
        if(keyboardState['w'])
        {
            pyr.y+=1;
        }
        if(keyboardState['d'])
        {
            pyr.y-=1;
        }
        
        
    }
    
    virtual void UploadAttributes(Shader* shadowShader=nullptr)
    {
        mat4 T = mat4(
                      1.0,            0.0,            0.0,            0.0,
                      0.0,            1.0,            0.0,            0.0,
                      0.0,            0.0,            1.0,            0.0,
                      position.x,        position.y,        position.z,        1.0);
        
        mat4 InvT = mat4(
                         1.0,            0.0,            0.0,            0.0,
                         0.0,            1.0,            0.0,            0.0,
                         0.0,            0.0,            1.0,            0.0,
                         -position.x,    -position.y,    -position.z,    1.0);
        
        mat4 S = mat4(
                      scaling.x,        0.0,            0.0,            0.0,
                      0.0,            scaling.y,        0.0,            0.0,
                      0.0,            0.0,            scaling.z,        0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvS = mat4(
                         1.0/scaling.x,    0.0,            0.0,            0.0,
                         0.0,            1.0/scaling.y,    0.0,            0.0,
                         0.0,            0.0,            1.0/scaling.z,    0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        //  float alpha = orientation / 180.0 * M_PI;
        
        //        mat4 R = mat4(
        //                      cos(alpha),        0.0,            sin(alpha),        0.0,
        //                      0.0,            1.0,            0.0,            0.0,
        //                      -sin(alpha),    0.0,            cos(alpha),        0.0,
        //                      0.0,            0.0,            0.0,            1.0);
        //pyr (nose up and down, nose side to side, rolling)
        // pyr=vec3(0,2*M_PI,0)+(orientation/180*M_PI);
        
        //        pyr=vec3(0,0,0);
        mat4 Roll=mat4(
                       cos(pyr.z),-sin(pyr.z),0,0,
                       sin(pyr.z), cos(pyr.z),0,0,
                       0,0,1,0,
                       0,0,0,1);
        
        mat4 InvRoll=mat4(
                          cos(pyr.z),sin(pyr.z),0,0,
                          -sin(pyr.z), cos(pyr.z),0,0,
                          0,0,1,0,
                          0,0,0,1);
        
        mat4 Pitch=mat4(
                        1,0,0,0,
                        0, cos(pyr.x),-sin(pyr.x),0,
                        0,sin(pyr.x),cos(pyr.x),0,
                        0,0,0,1);
        
        mat4 InvPitch=mat4(
                           1,0,0,0,
                           0, cos(pyr.x),sin(pyr.x),0,
                           0,-sin(pyr.x),cos(pyr.x),0,
                           0,0,0,1);
        
        mat4 Yaw=mat4(
                      cos(pyr.y),0,sin(pyr.y),0,
                      0, 1,0,0,
                      -sin(pyr.y),0,cos(pyr.y),0,
                      0,0,0,1);
        
        mat4 InvYaw=mat4(
                         cos(pyr.y),0,-sin(pyr.y),0,
                         0, 1,0,0,
                         sin(pyr.y),0,cos(pyr.y),0,
                         0,0,0,1);
        
        mat4 R= Roll*Pitch*Yaw;
        
        
        //        mat4 InvR = mat4(
        //                         cos(alpha),        0.0,            -sin(alpha),    0.0,
        //                         0.0,            1.0,            0.0,            0.0,
        //                         sin(alpha),        0.0,            cos(alpha),        0.0,
        //                         0.0,            0.0,            0.0,            1.0);
        
        mat4 InvR=InvYaw*InvPitch*InvRoll;
        mat4 M = S * R * T;
        mat4 InvM = InvT * InvR * InvS;
        mat4 VP =camera.GetViewMatrix() * camera.GetProjectionMatrix();
        mat4 MVP = M *camera.GetViewMatrix() * camera.GetProjectionMatrix();
        
        
        if(shadowShader)
        {
            shadowShader->UploadM(M);
            shadowShader->UploadVP(VP);
        }
        else
        {
            shader->UploadM(M);
            shader->UploadInvM(InvM);
            shader->UploadMVP(MVP);
            
        }
        
    }
};


class RotatingObjectTail:public Object
{
public:
    using::Object::Object;
    void move(float dt)
    {
        orientation=orientation+50*dt;
    }
    void UploadAttributes(Shader* shadowShader=nullptr)
    {
        mat4 T = mat4(
                      1.0,            0.0,            0.0,            0.0,
                      0.0,            1.0,            0.0,            0.0,
                      0.0,            0.0,            1.0,            0.0,
                      position.x,        position.y,        position.z,        1.0);
        
        mat4 InvT = mat4(
                         1.0,            0.0,            0.0,            0.0,
                         0.0,            1.0,            0.0,            0.0,
                         0.0,            0.0,            1.0,            0.0,
                         -position.x,    -position.y,    -position.z,    1.0);
        
        mat4 S = mat4(
                      scaling.x,        0.0,            0.0,            0.0,
                      0.0,            scaling.y,        0.0,            0.0,
                      0.0,            0.0,            scaling.z,        0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvS = mat4(
                         1.0/scaling.x,    0.0,            0.0,            0.0,
                         0.0,            1.0/scaling.y,    0.0,            0.0,
                         0.0,            0.0,            1.0/scaling.z,    0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        float alpha = orientation / 180.0 * M_PI;
        
        mat4 R = mat4(
                      1,        0.0,            0,        0.0,
                      0.0,            cos(alpha),            -sin(alpha),            0.0,
                      0,    sin(alpha),            cos(alpha),        0.0,
                      0.0,            0.0,            0.0,            1.0);
        
        mat4 InvR = mat4(
                         1,        0.0,            0,    0.0,
                         0.0,            -cos(alpha),            sin(alpha),            0.0,
                         0,        -sin(alpha),            -cos(alpha),        0.0,
                         0.0,            0.0,            0.0,            1.0);
        
        mat4 M = S * R * T;
        mat4 InvM = InvT * InvR * InvS;
        mat4 VP =camera.GetViewMatrix() * camera.GetProjectionMatrix();
        mat4 MVP = M *camera.GetViewMatrix() * camera.GetProjectionMatrix();
        
        
        if(shadowShader)
        {
            shadowShader->UploadM(M);
            shadowShader->UploadVP(VP);
        }
        else
        {
            shader->UploadM(M);
            shader->UploadInvM(InvM);
            shader->UploadMVP(MVP);
        }
    }
    
};


class Ground: public Object
{
    
public:
    using::Object::Object;
    void DrawShadow(Shader* shader=nullptr)
    {
        
    }
    OBJECT_TYPE GetType()
    {
        return GROUND;
    }
    
    
};





class Scene
{
public:
    MeshShader *meshShader;
    InfiniteQuadShader  *groundShader;
    ShadowShader *shadowShader;
    DeadSolidShader *deadShader;
    BackgroundShader *backgroundShader;
    TextureCube *environmentMap;
    
    
    std::vector<Texture*> textures;
    std::vector<Material*> materials;
    std::vector<Geometry*> geometries;
    std::vector<Mesh*> meshes;
    std::vector<Object*> objects;
    
public:
    Scene()
    {
        meshShader = 0;
        groundShader=0;
        shadowShader=0;
        deadShader=0;
        backgroundShader=0;
    }
    
    void Initialize()
    {
        meshShader = new MeshShader();
        groundShader=new InfiniteQuadShader ();
        shadowShader=new ShadowShader();
        deadShader=new DeadSolidShader();
        backgroundShader=new BackgroundShader();
        environmentMap = new TextureCube("posx512.jpg", "negx512.jpg", "posy512.jpg", "negy512.jpg", "posz512.jpg", "negz512.jpg");
        
        //Tigger (Avatar) (Shining)
        textures.push_back(new Texture("tigger.png"));
        materials.push_back(new Material(meshShader, textures[0],environmentMap,vec3(0.1,0.1,0.1),vec3(0.6,0.6,0.6),vec3(0.3,0.3,0.3),50));
        geometries.push_back(new PolygonalMesh("tigger.obj"));
        meshes.push_back(new Mesh(geometries[0], materials[0]));
        
        //Tree with Wood Texture (Dead Solid Perfect)
        textures.push_back(new Texture("tree.png"));
        materials.push_back(new Material(deadShader, textures[1],environmentMap,vec3(0.1,0.1,0.1),vec3(0.9,0.9,0.9),vec3(0,0,0),0));
        geometries.push_back(new PolygonalMesh("tree.obj"));
        meshes.push_back(new Mesh(geometries[1], materials[1]));
        
        //Ground (Ground Zero)
        
        materials.push_back(new Material(groundShader, textures[1],environmentMap,vec3(0.1,0.1,0.1),vec3(0.9,0.9,0.9),vec3(0,0,0),50));
        geometries.push_back(new TexturedQuad());
        meshes.push_back(new Mesh(geometries[2], materials[2]));
        
        //Heli
        textures.push_back(new Texture("heliait.png"));
        materials.push_back(new Material(meshShader, textures[2],environmentMap,vec3(0.1,0.1,0.1),vec3(0.9,0.9,0.9),vec3(0,0,0),0));
        geometries.push_back(new PolygonalMesh("heli.obj"));
        meshes.push_back(new Mesh(geometries[3], materials[3]));
        
        //Main Rotor (The Matrix Revolutions)
        textures.push_back(new Texture("helidait.png"));
        materials.push_back(new Material(meshShader, textures[2],environmentMap,vec3(0.1,0.1,0.1),vec3(0.9,0.9,0.9),vec3(0,0,0),0));
        geometries.push_back(new PolygonalMesh("mainrotor.obj"));
        meshes.push_back(new Mesh(geometries[4], materials[4]));
        
        //Tail Rotor (The Matrix Revolutions)
        textures.push_back(new Texture("helidait.png"));
        materials.push_back(new Material(meshShader, textures[2],environmentMap,vec3(0.1,0.1,0.1),vec3(0.9,0.9,0.9),vec3(0,0,0),0));
        geometries.push_back(new PolygonalMesh("tailrotor.obj"));
        meshes.push_back(new Mesh(geometries[5], materials[5]));
        
        //Tree with Diffuse Shading (A Clockwork Orange)
        textures.push_back(new Texture("tree.png"));
        materials.push_back(new Material(meshShader, textures[1],environmentMap,vec3(0.1,0.1,0.1),vec3(0.9,0.9,0.9),vec3(0,0,0),0));
        geometries.push_back(new PolygonalMesh("tree.obj"));
        meshes.push_back(new Mesh(geometries[6], materials[6]));
        
        //Bullet
        textures.push_back(new Texture("steel.png"));
        materials.push_back(new Material(meshShader, textures[6],environmentMap,vec3(0.1,0.1,0.1),vec3(0.9,0.9,0.9),vec3(0,0,0),0));
        geometries.push_back(new PolygonalMesh("bullet.obj"));
        meshes.push_back(new Mesh(geometries[7], materials[7]));
        
        
        
        //EnvironmentMap
        
        materials.push_back(new Material(backgroundShader , textures[1],environmentMap,vec3(0.1,0.1,0.1),vec3(0.9,0.9,0.9),vec3(0,0,0),0));
        //textures.push_back(new Texture("tree.png"));
        geometries.push_back(new BackgroundGeometry());
        meshes.push_back(new Mesh(geometries[7], materials[7]));
        
        
        
        objects.push_back(new Avatar(meshes[3], vec3(-1, -0.99, -3), vec3(0.05, 0.05,0.05), 0));//Helicopter
        //  objects.push_back(new RedTree(meshes[1], vec3(0.5, -.5, 0.0), vec3(0.01, 0.01, 0.01), -60.0));//Wood Tree
        objects.push_back(new Ground(meshes[2], vec3(-0.3, -1.0, -2.0), vec3(1, 1,1), -60.0));//Ground
        //
        
        //
        objects.push_back(new RotatingObject(meshes[4], vec3(-2, 0.5, -3.), vec3(0.05, 0.05,0.05), -60.0));//Main Rotor
        
        //        objects.push_back(new RotatingObjectTail(meshes[5], vec3(-1.5+.035, 0, -4+3.6), vec3(0.1, 0.1,0.1), -60.0));//Tail Rotor
        ////
        //        objects.push_back(new GreenTree(meshes[6], vec3(1.5, -0.5, 0.0), vec3(0.01, 0.01, 0.01), -60.0));//Diffuse Tree
        
        objects.push_back(new Tigger(meshes[0], vec3(3.5, 4.5, -2.), vec3(0.05, 0.05, 0.05), 45));//Tigger
        objects.push_back(new ParentTigger(meshes[0], vec3(4.0, -1.0, 0.0), vec3(0.25, 0.25, 0.25), 45));//Tigger
        for(int i=0;i<10;i++)
        {
            if(i%2==0)
            {
                objects.push_back(new RedTree(meshes[1], vec3(0.5+i, -.5, 0.0), vec3(0.02, 0.02, 0.02), -60.0));//Wood Tree
            }
            else
            {
                objects.push_back(new GreenTree(meshes[6], vec3(1.5-i, -0.5, 0.0), vec3(0.02, 0.02, 0.02), -60.0));//Diffuse Tree
            }
            
            
        }
        for(int i=0;i<10;i++)
        {
            if(i%2==0)
            {
                objects.push_back(new RedTree(meshes[1], vec3(0.5, -.5, 0.0-i), vec3(0.02, 0.02, 0.02), -60.0));//Wood Tree
            }
            else
            {
                objects.push_back(new GreenTree(meshes[6], vec3(1.5, -0.5, 0.0+i), vec3(0.02, 0.02, 0.02), -60.0));//Diffuse Tree
            }
            
            
        }
        
        for(int i=0;i<10;i++)
        {
            if(i%2==0)
            {
                objects.push_back(new RedTree(meshes[1], vec3(0.5-i, 1.5, 0.0), vec3(0.02, 0.02, 0.02), -60.0));//Wood Tree
            }
            else
            {
                objects.push_back(new GreenTree(meshes[6], vec3(1.5+i, 1.5, 0.0), vec3(0.02, 0.02, 0.02), -60.0));//Diffuse Tree
            }
            
        }
        
        for(int i=0;i<10;i++)
        {

                objects.push_back(new RedTree(meshes[1], vec3(cos(i*M_PI), 2.5, sin(i*M_PI)), vec3(0.02, 0.02, 0.02), -60.0));//Wood Tree
 
        }
        if(level==2)
        {
            objects[0]->position=vec3(3.5, 4.5, -2.);
            objects[1]->position=vec3(4.0, -1.0, 0.0);
        }

        //        //EnvironmentMap
        //        objects.push_back(new Object(meshes[7], vec3(1.5, -0.5, 0.0), vec3(0.01, 0.01, 0.01), -60.0));
        
    }
    
    void Interact()
    {
        for(int i=0;i<objects.size();i++)
        {
            for(int j=i+1;j<objects.size();j++)
            {
                objects[i]->Interact(objects[j]);
                
            }
            
        }
    }
    void move(float dt,float t)
    {
        camera.move(dt,t);
        // std::cout<<"Reached"<<std::endl;
        if(keyboardState['t'])
        {
            camera.startTracking(t);
        }
        if(keyboardState['r'])
        {
            camera.stopTracking(t);
        }
        
        
        if(keyboardState['f'])
        {
            objects.push_back(new Bullet(meshes[7], objects[0]->position+vec3(0,0.5,0), vec3(0.0625, 0.0625, 0.0625)));//Bullet
        }
        for(int i=0;i<objects.size();i++) objects[i]->move(dt);
    }
    
    ~Scene()
    {
        for(int i = 0; i < textures.size(); i++) delete textures[i];
        for(int i = 0; i < materials.size(); i++) delete materials[i];
        for(int i = 0; i < geometries.size(); i++) delete geometries[i];
        for(int i = 0; i < meshes.size(); i++) delete meshes[i];
        for(int i = 0; i < objects.size(); i++) delete objects[i];
        
        if(meshShader) delete meshShader;
        
        if(environmentMap )delete environmentMap;
        
        
        //Should I delete bullets
        //        if(dynamic_cast<Bullet*>(objects[5]))
        //        {
        //           // delete objects[5];
        //        }
        
        
    }
    
    void Draw()
    {
        // light=new Light();
        for(int i = 0; i < objects.size(); i++)
        {
            objects[i]->Draw();
            if(objects[i]->alive)
            {
                objects[i]->DrawShadow(shadowShader);
            }
            
            
        }
    }
    
};

Scene scene;

void onInitialization()
{
    glViewport(0, 0, windowWidth, windowHeight);
    
    scene.Initialize();
}

void onExit()
{
    printf("exit");
}
void printtext(int x, int y, std::string String)
{
    //(x,y) is from the bottom left of the window
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    glRasterPos2i(x,y);
    for (int i=0; i<String.size(); i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, String[i]);
    }
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void onDisplay()
{
    
    glClearColor(0, 0, 0.5, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene.Draw();
    
    display();
    glutSwapBuffers();
    
}
void display(void)
{
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    glEnable(GL_DEPTH_TEST);
    
    char string[64];
    sprintf(string, "something");
    printtext(400,10,string);
    
    //glutSwapBuffers();
}
//void fakeDisplay()
//{
//    displayValues(10,10,"bob");
//    glutSwapBuffers();
//}

void onKeyboard(unsigned char key, int x, int y)
{
    keyboardState[key] = true;
}

void onKeyboardUp(unsigned char key, int x, int y)
{
    keyboardState[key] = false;
}

void onReshape(int winWidth, int winHeight)
{
    camera.SetAspectRatio((float)winWidth / winHeight);
    glViewport(0, 0, winWidth, winHeight);
}

void onIdle( ) {
    double t = glutGet(GLUT_ELAPSED_TIME) * 0.001;
    static double lastTime = 0.0;
    double dt = t - lastTime;
    lastTime = t;
    scene.move(dt,t);
    scene.Interact();
    
    glutPostRedisplay();
}


int main(int argc, char * argv[])
{
    glutInit(&argc, argv);
#if !defined(__APPLE__)
    glutInitContextVersion(majorVersion, minorVersion);
#endif
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(50, 50);
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//    glutInitWindowSize(windowWidth, windowHeight);
//    glutInitWindowPosition(0, 0);
//
//    glutCreateWindow("OpenGL Text Example");
    
#if defined(__APPLE__)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutCreateWindow("3D Mesh Rendering");

#if !defined(__APPLE__)
    glewExperimental = true;
    glewInit();
#endif
    printf("GL Vendor    : %s\n", glGetString(GL_VENDOR));
    printf("GL Renderer  : %s\n", glGetString(GL_RENDERER));
    printf("GL Version (string)  : %s\n", glGetString(GL_VERSION));
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    printf("GL Version (integer) : %d.%d\n", majorVersion, minorVersion);
    printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    onInitialization();
    
    glutDisplayFunc(onDisplay);
    glutIdleFunc(onIdle);
    glutKeyboardFunc(onKeyboard);
    glutKeyboardUpFunc(onKeyboardUp);
    glutReshapeFunc(onReshape);

    glutMainLoop();
    onExit();
    return 1;
}

