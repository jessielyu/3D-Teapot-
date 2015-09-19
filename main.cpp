//
//  main.cpp
//  TeapotContestMP3
//
//  Created by Jiexin Lyu  on 4/12/15.
//  Copyright (c) 2015 Jiexin Lyu . All rights reserved.
//

#include <iostream>
#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>
#include <vector>
#include "math.h"
#include "imageloader.h"

using namespace std;

/* struct the vertex struct and face struct */
typedef struct {
    float x;
    float y;
    float z;
} vertex_t;

typedef struct {
    float x;
    float y;
    float z;
} vector_t;

typedef struct {
    int first;
    int second;
    int third;
} face_t;


/* vectors to store vertices and faces */
vector<vertex_t>    vertices;
vector<face_t>      faces;
vector<vertex_t>    vertices_normal;
vector<vector_t>    faces_normal;

/* variables for the rotation */
float xRotated, yRotated, zRotated;

int _textureId[3]; //The id of the texture

int mode = 0;
int multi_texture = 0;


/* LoadObj()
 * To load obj file and store vertices and faces to vectors
 * calculate vertices and faces normal and store to vvectors
 * return true if success; false if fail
 */
void LoadObj(void) {
    FILE * fp;
    fp = fopen("teapot_0.obj", "r");
    if (fp == NULL) {
        cout<<"can't open the file"<<endl;
        fclose(fp);
        return;
    }
    
    while(1) {
        char line[128];
        int res = fscanf(fp, "%s", line);
        if (res == EOF)
            break;
        if (strcmp(line, "v") == 0) {
            vertex_t vertex;
            fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back(vertex);
        }
        else if (strcmp(line, "f") == 0) {
            face_t face;
            fscanf(fp, "%d %d %d\n", &face.first, &face.second, &face.third);
            faces.push_back(face);
        }
    }
    fclose(fp);
    
    /* calculate the face normal from face to face and store to face_normal vector*/
    double x,y,z,x1,y1,z1,x2,y2,z2;
    for (size_t i = 0; i < faces.size(); i++) {
        /* get vectors of each edge for normal vector calculation */
        x1 = vertices[faces[i].first].x - vertices[faces[i].second].x;
        x2 = vertices[faces[i].first].x - vertices[faces[i].third].x;
        
        y1 = vertices[faces[i].first].y - vertices[faces[i].second].y;
        y2 = vertices[faces[i].first].y - vertices[faces[i].third].y;
        
        z1 = vertices[faces[i].first].z - vertices[faces[i].second].z;
        z2 = vertices[faces[i].first].z - vertices[faces[i].third].z;
        
        /* formula for normal calculation */
        x = y1*z2-y2*z1;
        y = x2*z1-x1*z2;
        z = x1*y2-x2*y1;
        double norm = sqrtf(x*x + y*y + z*z);
        
        vector_t temp_vector;
        temp_vector.x = x/norm;
        temp_vector.y = y/norm;
        temp_vector.z = z/norm;
        
        faces_normal.push_back(temp_vector);
    }
    
    /* calculate the vertex normal */
    double count;
    for (size_t i = 1; i <= vertices.size(); i++) {
        count = x = y = z = 0;
        // Find the faces associated with the current vertex.
        for (size_t j = 0; j < faces.size(); j++) {
            if (faces[j].first == i || faces[j].second == i || faces[j].second == i) {
                x += faces_normal[j].x;
                y += faces_normal[j].y;
                z += faces_normal[j].z;
                count++;
            }
        }
        
        // Average and normalize to get the result.
        x = x/count;
        y = y/count;
        z = z/count;
        double norm = sqrtf(x*x + y*y + z*z);
        x = x/norm; y = y/norm; z = z/norm;
        
        vertex_t temp_vertex;
        temp_vertex.x = x;
        temp_vertex.y = y;
        temp_vertex.z = z;
        vertices_normal.push_back(temp_vertex);
    }
    
}


/*
 * loadTexture()
 * load texture for the teaport for the texture mapping
 */
int loadTexture(Image* image)
{
    GLuint textureId;
    glGenTextures(1, &textureId);               //Make room for our texture
    glBindTexture(GL_TEXTURE_2D, textureId);    //Tell OpenGL which texture to edit
    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 image->width, image->height,  //Width and height
                 0,                            //The border of the image
                 GL_RGB,                       //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE,             //GL_UNSIGNED_BYTE, because pixels are stored as unsigned numbers
                 image->pixels);               //The actual pixel data
    return textureId;                          //Returns the id of the texture
}


/* texture_init()
 * helper function to initialize textures
 */
void texture_init(void) {
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    
    if (multi_texture == 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureId[0]);
        glEnable(GL_TEXTURE_2D);
    }
    else if (multi_texture == 1) {
        glActiveTexture(GL_TEXTURE0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _textureId[0]);
        glBindTexture(GL_TEXTURE_2D, _textureId[1]);
        glEnable(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    
    /* initialize texture */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    return;
}


/* environment_init
 * helper function to initializa environment mapping
 */
void environment_init(void) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureId[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);    return;
}



/* image_init()
 * initialize _textureId
 */
void  image_init(void)
{
    /* load three images for texture and environment */
    Image* image0 = loadBMP("wood.bmp");
    _textureId[0] = loadTexture(image0);
    Image* image1 = loadBMP("rose.bmp");
    _textureId[1] = loadTexture(image1);
    Image* image2 = loadBMP("stpeters_probe.bmp");
    _textureId[2] = loadTexture(image2);
    delete image0;
    delete image1;
    delete image2;
}



/* initialization function
 * initializa the initial settings
 */
void init(void)
{
    /* initialize lighting and enable lighting */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    
    if (mode == 0)
        texture_init();
    else if (mode == 1)
        environment_init();
}


/* display()
 * display the objects
 */
void display(void)
{
    /* translation matrix */
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    
    /* set up the light */
    float ambientColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
    
    float lightColor0[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float lightPos0[] = {4.0f, 0.0f, 8.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    
    /* set up the rotation */
    glRotatef(xRotated, 1.0, 0.0, 0.0);
    glRotatef(yRotated, 0.0, 1.0, 0.0);
    glRotatef(zRotated, 0.0, 0.0, 1.0);
    
    /* draw the vertices */
    if (mode == 0) {
        cout<<multi_texture<<endl;
        if (multi_texture == 0) {
            for (int i = 0; i < faces.size(); i++) {
                face_t triangle = faces[i];
                vertex_t first_vertex = vertices[triangle.first-1];
                vertex_t second_vertex = vertices[triangle.second-1];
                vertex_t third_vertex = vertices[triangle.third-1];
                glColor3f(0.5, 0.5, 1.0);
                glBegin(GL_TRIANGLES);
                glNormal3f(first_vertex.x, first_vertex.y, first_vertex.z);
                glTexCoord2f(atan((first_vertex.z)/first_vertex.x), first_vertex.y);
                glVertex3f(first_vertex.x, first_vertex.y, first_vertex.z);
                glNormal3f(second_vertex.x, second_vertex.y, second_vertex.z);
                glTexCoord2f(atan((second_vertex.z)/second_vertex.x), second_vertex.y);
                glVertex3f(second_vertex.x, second_vertex.y, second_vertex.z);
                glNormal3f(third_vertex.x, third_vertex.y, third_vertex.z);
                glTexCoord2f(atan((third_vertex.z)/third_vertex.x), third_vertex.y);
                glVertex3f(third_vertex.x, third_vertex.y, third_vertex.z);
                glEnd();
            }
        }
        /* draw vertices when multiple textures */
        else if (multi_texture == 1) {
            for (int i = 0; i < faces.size(); i++) {
                face_t triangle = faces[i];
                vertex_t first_vertex = vertices[triangle.first-1];
                vertex_t second_vertex = vertices[triangle.second-1];
                vertex_t third_vertex = vertices[triangle.third-1];
                glColor3f(0.5, 0.5, 1.0);
                glBegin(GL_TRIANGLES);
                glNormal3f(first_vertex.x, first_vertex.y, first_vertex.z);
                glMultiTexCoord2f(GL_TEXTURE0, atan((first_vertex.z)/first_vertex.x), first_vertex.y);
                glMultiTexCoord2f(GL_TEXTURE1, atan((first_vertex.z)/first_vertex.x), first_vertex.y);
                glVertex3f(first_vertex.x, first_vertex.y, first_vertex.z);
                glNormal3f(second_vertex.x, second_vertex.y, second_vertex.z);
                glMultiTexCoord2f(GL_TEXTURE0, atan((second_vertex.z)/second_vertex.x), second_vertex.y);
                glMultiTexCoord2f(GL_TEXTURE1, atan((first_vertex.z)/first_vertex.x), first_vertex.y);
                glVertex3f(second_vertex.x, second_vertex.y, second_vertex.z);
                glNormal3f(third_vertex.x, third_vertex.y, third_vertex.z);
                glMultiTexCoord2f(GL_TEXTURE0, atan((third_vertex.z)/third_vertex.x), third_vertex.y);
                glMultiTexCoord2f(GL_TEXTURE1, atan((first_vertex.z)/first_vertex.x), first_vertex.y);
                glVertex3f(third_vertex.x, third_vertex.y, third_vertex.z);
                glEnd();
            }
        }
    }
    
    /* draw environment mapping */
    else if (mode == 1) {
        for (int i = 0; i < faces.size(); i++) {
            face_t triangle = faces[i];
            vertex_t first_vertex = vertices[triangle.first-1];
            vertex_t second_vertex = vertices[triangle.second-1];
            vertex_t third_vertex = vertices[triangle.third-1];
            glColor3f(0.5, 0.5, 1.0);
            glBegin(GL_TRIANGLES);
            glNormal3f(first_vertex.x, first_vertex.y, first_vertex.z);
            glTexCoord2f(atan((first_vertex.z)/first_vertex.x), first_vertex.y);
            glVertex3f(first_vertex.x, first_vertex.y, first_vertex.z);
            glNormal3f(second_vertex.x, second_vertex.y, second_vertex.z);
            glTexCoord2f(atan((second_vertex.z)/second_vertex.x), second_vertex.y);
            glVertex3f(second_vertex.x, second_vertex.y, second_vertex.z);
            glNormal3f(third_vertex.x, third_vertex.y, third_vertex.z);
            glTexCoord2f(atan((third_vertex.z)/third_vertex.x), third_vertex.y);
            glVertex3f(third_vertex.x, third_vertex.y, third_vertex.z);
            glEnd();
        }
    }
    
    glutSwapBuffers();
    glFlush ();
}


/* rotation()
 * for rotation animation
 */
void rotation(void)
{
    xRotated += 0.05;
    yRotated += 0.10;
    display();
}


/* reshape()
 * to reshape the windows
 */
void reshape(int x, int y)
{
    if (x == 0 || y == 0)   return;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(40.0, (double)x/(double)y, 0.5, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, x, y);
}


/* keyboard()
 * initialize keyboard for use
 */
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27) {
        // ESC hit, so quit
        printf("demonstration finished.\n");
        exit(0);
    }
    
    if (key == 'm') {
        //'m' is hit, switch to multitexture mapping
        printf("multitexture mapping.\n");
        multi_texture = multi_texture == 0 ? 1 : 0;
        init();
    }
    
    if (key == 'e') {
        //'s' is hit, switch mode from texture mapping to enviroment mapping;
        printf("enviroment mapping\n");
        mode = mode == 0 ? 1 : 0;
        init();
    }
}


/* main()
 * initialize all the helper functions
 */
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow (argv[0]);
    image_init();
    init ();
    LoadObj();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(rotation);
    glutMainLoop();
    return 0;
}