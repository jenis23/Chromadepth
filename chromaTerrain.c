#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "matrix.h"
#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glut.h>
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define RADIANS_PER_PIXEL M_PI/(2*90.0)



/**
Authors: Jenis Modi
        Wayne Cochran

Desc: Generate a terrain from pgm file

**/



double centerX=0,centerY=0,centerZ=0;
double upX=0,upY=0,upZ=1;

GLfloat center;

double eyeX,eyeY,eyeZ;

int mouseX,mouseY;


double phi = M_PI/4.0,radius=3.0,theta=M_PI/4.0;

GLboolean changeFlag;

GLint ColorUniform;

GLfloat* verticesArray;
GLuint* indicesArray;
GLfloat* normalsArray;


unsigned long totalVertices;
unsigned long totalIndices;
unsigned long totalNormals;

GLfloat ModelView[4*4];
GLfloat Projection[4*4];
GLfloat Color[4];

GLfloat ambientLight[3] = {0.3, 0.3, 0.2};
GLfloat light0Color[3] = {1.0, 1.0, 1.0};
GLfloat light0Position[3] = {1.0, 1.0, 1.0};

GLfloat materialAmbient[3] = {0.2,0.2,0.2};
GLfloat materialDiffuse[3] = {1.0,1.0,1.0};
GLfloat materialSpecular[3] = {1.0,1.0,1.0};
GLfloat materialShininess = 20.0;


GLfloat nearPlane,farPlane;

GLint vertexPositionAttr;
GLint vertexNormalAttr;
GLint vertexTexCoordAttr;

GLint ModelViewProjectionUniform;
GLint ModelViewMatrixUniform;
GLint NormalMatrixUniform;

GLint ambientLightUniform;
GLint light0ColorUniform;
GLint light0PositionUniform;

GLint materialAmbientUniform;
GLint materialDiffuseUniform;
GLint materialSpecularUniform;
GLint materialShininessUniform;

GLint nearPlaneUniform;
GLint farPlaneUniform;


GLint texUnitUniform;

GLuint vertexShader;
GLuint fragmentShader;
GLuint program;

GLint texUnitUniform;

size_t height,width,maxVal;

int** gridData;

/**
* Method declaration
**/


void setCamera();
void computeMinimum();
/**
* Cross product of two vectors:
* |i  j  k |
  |x1 x2 x3 |  
  |y1 y2 y3 |

==> x [ 1 ] ⋅ y [ 2 ] − y [ 1 ] ⋅ x [ 2 ] x [ 2 ] ⋅ y [ 0 ] − y [ 2 ] ⋅ x [ 0 ] x [ 0 ] ⋅ y [ 1 ] − y [ 0 ] ⋅ x [ 1 ]


**/
  void crossProduct(GLfloat T[], GLfloat A[], GLfloat **n){
  //GLfloat n[3];
	(*n)= (GLfloat*) malloc(sizeof(GLfloat) * 3);
        (*n)[0] = T[1] * A[2] - T[2] * A[1];
        (*n)[1] = T[2] * A[0] - T[0] * A[2];
        (*n)[2] = T[0] * A[1] - T[1] * A[0];
  }

  void normalize(GLfloat **n){
        GLdouble modeN = sqrt((*n)[0]* (*n)[0] + (*n)[1] * (*n)[1] + (*n)[2] * (*n)[2]);
        (*n)[0] = (*n)[0]/modeN;
        (*n)[1] = (*n)[1]/modeN;
        (*n)[2] = (*n)[2]/modeN;
	
  }

  void sphericalToCartesian(double radius, double theta, double phi,
                          double *x_Cart, double *y_Cart, double *z_Cart) {
    *x_Cart = radius*cos(theta)*sin(phi);
    *y_Cart = radius*sin(theta)*sin(phi);
    *z_Cart = radius*cos(phi);
  }

  void checkOpenGLError(int line) {
    bool wasError = false;
    GLenum error = glGetError();
    while (error != GL_NO_ERROR) {
        printf("GL ERROR: at line %d: %s\n", line, gluErrorString(error));
        wasError = true;
        error = glGetError();
    }
    if (wasError) exit(-1);
  }

  void loadUniforms() {

    //
    // Load Matrices
    //

    GLfloat ModelViewProjection[4*4];
    GLfloat NormalMatrix[3*3];

    matrixMultiply(ModelViewProjection, Projection, ModelView);
    matrixNormal(ModelView, NormalMatrix);
    glUniformMatrix4fv(ModelViewProjectionUniform, 1, GL_FALSE,ModelViewProjection);

    glUniformMatrix4fv(ModelViewMatrixUniform, 1, GL_FALSE,ModelView);
    glUniformMatrix3fv(NormalMatrixUniform, 1, GL_FALSE, NormalMatrix);

    //
    // Load lights.
    //

    glUniform3fv(ambientLightUniform, 1, ambientLight);
    glUniform3fv(light0ColorUniform, 1, light0Color);
    glUniform3fv(light0PositionUniform, 1, light0Position);

    //
    // Load material properties.
    //

    glUniform3fv(materialAmbientUniform, 1, materialAmbient);
    glUniform3fv(materialDiffuseUniform, 1, materialDiffuse);

    glUniform3fv(materialSpecularUniform, 1, materialSpecular);
    glUniform1f(materialShininessUniform, materialShininess);

   // glUniform1f(nearPlaneUniform, nearPlane);
   // glUniform1f(farPlaneUniform, farPlane);

  }

  GLchar *getShaderSource(const char *fname) {

    FILE *f = fopen(fname, "r");
    if (f == NULL) {
        perror(fname); exit(-1);
    }
    fseek(f, 0L, SEEK_END);
    int len = ftell(f);
    rewind(f);
    GLchar *source = (GLchar *) malloc(len + 1);
    if (fread(source,1,len,f) != len) {
        if (ferror(f))
            perror(fname);
        else if (feof(f))
            fprintf(stderr, "Unexpected EOF when reading '%s'!\n", fname);
        else
           fprintf(stderr, "Unable to load '%s'!\n", fname);
        exit(-1);
    }

    source[len] = '\0';
    fclose(f);
    return source;
  }



//
// Install our shader programs and tell GL to use them.
// We also initialize the uniform variables.
//

  void installShaders(void) {

    //
    // (1) Create shader objects
    //

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //
    // (2) Load source code into shader objects.
    //
    const GLchar *vertexShaderSource = getShaderSource("vertex.vs");
    const GLchar *fragmentShaderSource = getShaderSource("fragment.fs");

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    //
    // (3) Compile shaders.
    //

    glCompileShader(vertexShader);
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[200];
        GLint charsWritten;
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), &charsWritten, infoLog);

        fprintf(stderr, "vertex shader info log:\n%s\n\n", infoLog);
    }

    checkOpenGLError(__LINE__);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[200];
        GLint charsWritten;
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), &charsWritten, infoLog);
        fprintf(stderr, "fragment shader info log:\n%s\n\n", infoLog);
    }

    checkOpenGLError(__LINE__);

    //
    // (4) Create program object and attach vertex and fragment shader.
    //

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    checkOpenGLError(__LINE__);

    //
    // (5) Link program.
    //

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[200];
        GLint charsWritten;
        glGetProgramInfoLog(program, sizeof(infoLog), &charsWritten, infoLog);
        fprintf(stderr, "program info log:\n%s\n\n", infoLog);
    }

    checkOpenGLError(__LINE__);
    //
    // (7) Get vertex attribute locations
    //

    vertexPositionAttr = glGetAttribLocation(program, "vertexPosition");
    vertexNormalAttr = glGetAttribLocation(program, "vertexNormal");
//    vertexTexCoordAttr = glGetAttribLocation(program, "vertexTexCoord");

    if (vertexPositionAttr == -1 || vertexNormalAttr == -1) {
        fprintf(stderr, "Error fetching vertex position or normal attribute !\n");
    }

    //
    // (8) Fetch handles for uniform variables in program.
    //

    ModelViewProjectionUniform = glGetUniformLocation(program, "ModelViewProjection");
    if (ModelViewProjectionUniform == -1) {
        fprintf(stderr, "Error fetching modelViewProjectionUniform		!\n");
        //    exit(-1);
    }

    ModelViewMatrixUniform = glGetUniformLocation(program, "ModelViewMatrix");
    if (ModelViewMatrixUniform == -1) {
        fprintf(stderr, "Error fetching modelViewMatrixUniform!\n");
        // exit(-1);
    }



    NormalMatrixUniform = glGetUniformLocation(program, "NormalMatrix");
    if (NormalMatrixUniform == -1) {
        fprintf(stderr, "Error fetching normalMatrixUniform!\n");
        // exit(-1);
    }
    ambientLightUniform = glGetUniformLocation(program, "ambientLight");
    if (ambientLightUniform == -1) {
        fprintf(stderr, "Error fetching ambientLightUniform!\n");
        exit(-1);
    }

    light0ColorUniform = glGetUniformLocation(program, "light0Color");
    if (light0ColorUniform == -1) {
        fprintf(stderr, "Error fetching light0ColorUniform!\n");
        // exit(-1);
    }

    light0PositionUniform = glGetUniformLocation(program, "light0Position");
    if (light0PositionUniform == -1) {
        fprintf(stderr, "Error fetching light0PositionUniform!\n");
        // exit(-1);
    }

    materialAmbientUniform = glGetUniformLocation(program, "materialAmbient");
    if (materialAmbientUniform == -1) {
        fprintf(stderr, "Error fetching materialAmbientUniform!\n");
        // exit(-1);
    }

    materialDiffuseUniform = glGetUniformLocation(program, "materialDiffuse");
    if (materialDiffuseUniform == -1) {

        fprintf(stderr, "Error fetching materialDiffuseUniform!\n");

        //exit(-1);

    }

    materialSpecularUniform = glGetUniformLocation(program, "materialSpecular");
    materialShininessUniform = glGetUniformLocation(program, "materialShininess");
    nearPlaneUniform = glGetUniformLocation(program, "nearPlane");
    farPlaneUniform = glGetUniformLocation(program, "farPlane");

    //
    // (9) Tell GL to use our program
    //

    glUseProgram(program);

  }

  double getNormalizedX(unsigned long x){
	return ((double)x/((width-1))*2.0 - 1.0);
  }

  double getNormalizedY(unsigned long y){
	return ((double)(height - 1 - y) /( (height - 1))* 2.0 - 1.0);
  }

  double getNormalizedZ(unsigned long x,unsigned long y){
	unsigned long yPrime = (height - 1) - y;
	return   (0.50 * gridData[yPrime][x]/ (double)maxVal);	
  }



/**
* Generate Normals for GL_TRIANGLES
**/ 

  void getNormals(GLfloat **hmX, GLfloat **hmY, GLfloat **hmZ, size_t row,size_t col,size_t height,size_t width, GLfloat **sum){

    (*sum)= (GLfloat*) malloc(sizeof(GLfloat) * 3);
    GLfloat *n;
    (*sum)[0]=0;
    (*sum)[1]=0;
    (*sum)[2]=0;
    GLfloat sumX, sumY, sumZ;
    sumX=0;
    sumY=0;
    sumZ=0;

    GLfloat v1[3], v2[3];

    GLfloat curX = hmX[row][col];
    GLfloat curY = hmY[row][col];
    GLfloat curZ = hmZ[row][col];

   if( row+1 < height && col+1 < width ){
	v1[0] =  hmX[row+0][col+1] - curX;
	v1[1] =  hmY[row+0][col+1] - curY;
	v1[2] =  hmZ[row+0][col+1] - curZ;

	v2[0] =  hmX[row+1][col+0] - curX;
	v2[1] =  hmY[row+1][col+0] - curY;
	v2[2] =  hmZ[row+1][col+0] - curZ;
	
	crossProduct(v1, v2, &n);
	normalize(&n);
	sumX += n[0];
	sumY += n[1];
	sumZ += n[2];

    }
    if( row+1 < height && col > 0 ){	
	v1[0] =  hmX[row+1][col+0] - curX;
	v1[1] =  hmY[row+1][col+0] - curY;
	v1[2] =  hmZ[row+1][col+0] - curZ;

	v2[0] =  hmX[row+1][col-1] - curX;
	v2[1] =  hmY[row+1][col-1] - curY;
	v2[2] =  hmZ[row+1][col-1] - curZ;
	
	crossProduct(v1, v2, &n);
	normalize(&n);
	sumX += n[0];
	sumY += n[1];
	sumZ += n[2];
	
    }

    if( row+1 < height && col > 0 ){
	
	v1[0] =  hmX[row+1][col-1] - curX;
	v1[1] =  hmY[row+1][col-1] - curY;
	v1[2] =  hmZ[row+1][col-1] - curZ;

	v2[0] =  hmX[row+0][col-1] - curX;
	v2[1] =  hmY[row+0][col-1] - curY;
	v2[2] =  hmZ[row+0][col-1] - curZ;
	
	crossProduct(v1, v2, &n);
	normalize(&n);
	sumX += n[0];
	sumY += n[1];
	sumZ += n[2];
    }
    if( row > 0 && col > 0 ){
	
	v1[0] =  hmX[row+0][col-1] - curX;
	v1[1] =  hmY[row+0][col-1] - curY;
	v1[2] =  hmZ[row+0][col-1] - curZ;

	v2[0] =  hmX[row-1][col+0] - curX;
	v2[1] =  hmY[row-1][col+0] - curY;
	v2[2] =  hmZ[row-1][col+0] - curZ;
	
	crossProduct(v1, v2, &n);
	normalize(&n);
	sumX += n[0];
	sumY += n[1];
	sumZ += n[2];
    }
    if( row > 0 && col+1 < width ){
	
	v1[0] =  hmX[row-1][col+0] - curX;
	v1[1] =  hmY[row-1][col+0] - curY;
	v1[2] =  hmZ[row-1][col+0] - curZ;

	v2[0] =  hmX[row-1][col+1] - curX;
	v2[1] =  hmY[row-1][col+1] - curY;
	v2[2] =  hmZ[row-1][col+1] - curZ;
	
	crossProduct(v1, v2, &n);
	normalize(&n);
	sumX += n[0];
	sumY += n[1];
	sumZ += n[2];
    }
    if( row > 0 && col+1 < width ){
	
	v1[0] =  hmX[row-1][col+1] - curX;
	v1[1] =  hmY[row-1][col+1] - curY;
	v1[2] =  hmZ[row-1][col+1] - curZ;

	v2[0] =  hmX[row+0][col+1] - curX;
	v2[1] =  hmY[row+0][col+1] - curY;
	v2[2] =  hmZ[row+0][col+1] - curZ;
	
	crossProduct(v1, v2, &n);
	normalize(&n);
	sumX += n[0];
	sumY += n[1];
	sumZ += n[2];
	}
	(*sum)[0]= sumX;
	(*sum)[1]= sumY;
	(*sum)[2]= sumZ;

	normalize(sum);
    	
	(*sum)[0]= -((*sum)[0]);
	(*sum)[1]= -((*sum)[1]);
	(*sum)[2]= -((*sum)[2]);
  }
 
  void loadPGMFile(){
    FILE *pgmFile;
    char pgm_name[10000];
    size_t x,y;
    
   unsigned int k=0;
    GLfloat **verticesArrayX, **verticesArrayY, **verticesArrayZ;

//   pgmFile = fopen("sthelens_after.pgm","r");
 pgmFile = fopen("dem/sthelens_before.pgm","r");
 //pgmFile = fopen("dem/mtbachelor.pgm","r");

    fgets(pgm_name,10000,pgmFile);

    fscanf(pgmFile,"%d", &width);
    fscanf(pgmFile,"%d", &height);
    fscanf(pgmFile,"%d", &maxVal);
	
    gridData = malloc(height * sizeof(int *));

    for( x = 0; x < height; x++){
       gridData[x] = malloc(width * sizeof(int));
         for(y = 0; y < width; y++){
            fscanf(pgmFile, "%d", &gridData[x][y]);
         }
    }

    verticesArray = malloc(width * height * 3 * sizeof(GLfloat));
    indicesArray = malloc(width * height * 6 * sizeof(GLuint));
    normalsArray = malloc(width * height * 3 * sizeof(GLfloat));

    verticesArrayX = malloc(height*sizeof(GLfloat*));
    verticesArrayY = malloc(height*sizeof(GLfloat*));
    verticesArrayZ = malloc(height*sizeof(GLfloat*));
   
    for(k=0;k<height;k++)
    {
	verticesArrayX[k] = malloc(width* sizeof(GLfloat));
	verticesArrayY[k] = malloc(width* sizeof(GLfloat));
	verticesArrayZ[k] = malloc(width* sizeof(GLfloat));
    }
   

    int indicesI=0;
    unsigned long i=0,j=0;
    unsigned long vertexIndex = 0;

    for(j=0;j<height;j++){
      for(i=0;i<width; i++){
	verticesArrayX[j][i] = getNormalizedX(i);
	verticesArrayY[j][i] = getNormalizedY(j);
	verticesArrayZ[j][i] = getNormalizedZ(i,j);
	verticesArray[vertexIndex++] = verticesArrayX[j][i];
	verticesArray[vertexIndex++] = verticesArrayY[j][i];
	verticesArray[vertexIndex++] = verticesArrayZ[j][i];

      } 
    } 

  indicesI=0;
  for( size_t row = 1; row < height; ++row ){
        for( size_t col = 1; col < width; ++col )
        {
            indicesArray[indicesI++] = ( (col-1) + (row-1) * width );
            indicesArray[indicesI++] = ( (col-0) + (row-1) * width );
            indicesArray[indicesI++] = ( (col-1) + (row-0) * width );
            indicesArray[indicesI++] = ( (col-1) + (row-0) * width );
            indicesArray[indicesI++] = ( (col-0) + (row-1) * width );
            indicesArray[indicesI++] = ( (col-0) + (row-0) * width );
        }
  }



  unsigned long normalIndex = 0;

  for(size_t row=0; row < height; ++row){
	for(size_t col =0; col < width; ++col){
	  GLfloat *normalResult;
	  getNormals(verticesArrayX,verticesArrayY, verticesArrayZ, row,col,height,width, &normalResult);
	  normalsArray[normalIndex++]= normalResult[0];
	  normalsArray[normalIndex++]= normalResult[1];
	  normalsArray[normalIndex++]= normalResult[2];
	}
  }


    totalVertices = width*height*3;
    totalIndices = indicesI;
    totalNormals = width*height*3;
  //  computeMinimum();
	
  }

  void computeMinimum(){
    unsigned long i,j;
    for(i=0;i < height; i++){
	for(j=0; j < width; j++){
	printf("%f",verticesArray[2]);
	printf("%f",radius-verticesArray[2]);	
	}
    }

  }

  void generateTerrain(){


    static GLuint indexBuffer;
    static GLuint vertexBuffer;
    static GLuint normalsBuffer;

    static GLboolean first = GL_TRUE;


    if(first){    

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, totalVertices * sizeof(GLfloat),
                     verticesArray, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,indexBuffer);
    glBufferData(GL_ARRAY_BUFFER, totalIndices * sizeof(GLuint),
                     indicesArray, GL_STATIC_DRAW);

    glGenBuffers(1, &normalsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,normalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, totalNormals * sizeof(GLfloat),
		    normalsArray, GL_STATIC_DRAW);

    first = GL_FALSE;
 
    } 
  
    loadUniforms();

    glEnableVertexAttribArray(vertexPositionAttr);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(vertexPositionAttr,3, GL_FLOAT,GL_FALSE, 0, (GLvoid*) 0);

    glEnableVertexAttribArray(vertexNormalAttr);
    glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
    glVertexAttribPointer(vertexNormalAttr, 3, GL_FLOAT,
                          GL_FALSE, 0, (GLvoid*) 0);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, totalIndices, GL_UNSIGNED_INT, (GLvoid*) 0);
    
  }


  void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
   // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    matrixPush(ModelView);
    generateTerrain();
    matrixPop(ModelView);
    glutSwapBuffers();
  }

  void keyboard(unsigned char key, int x, int y) {
  #define ESC 27
    if (key == ESC) exit(0);
    else if(key == 'z'){
	radius *= 1.05;
	setCamera();
	glutPostRedisplay();
	
    }else if(key == 'x'){
	radius *= 0.95;
	setCamera();
	glutPostRedisplay();

    }
	
  }

  void mouse(int button, int state, int x, int y) {
    mouseX = x;
    mouseY = y;
  }

#define RADIANS_PER_PIXEL M_PI/(2*90.0)
#define EPSILON 0.00001
  void mouseMotion(int x, int y) {
    float mouseBound = 0.5;
    int dx = x - mouseX;
    int dy = y - mouseY;
    theta -= dx * RADIANS_PER_PIXEL;
    phi -= dy * RADIANS_PER_PIXEL;
    if (phi < mouseBound)
        phi = mouseBound;
    else if (phi > M_PI/2 - mouseBound)
        phi = M_PI/2 - mouseBound;

    setCamera();
    mouseX = x;
    mouseY = y;
    glutPostRedisplay();

  }

  void idle(){
    GLfloat seconds = glutGet(GLUT_ELAPSED_TIME)/1000.0;
    GLfloat rotateSpeed = 360.0/40;
    center = rotateSpeed*seconds;
    glutPostRedisplay();
  }


  void initValues(){  
    loadPGMFile();
  }

  void setCamera() {
    sphericalToCartesian(radius, theta, phi,
                         &eyeX, &eyeY, &eyeZ);
    eyeX += centerX; eyeY += centerY; eyeZ += centerZ;

    matrixIdentity(ModelView);
    matrixLookat(ModelView, eyeX,    eyeY,    eyeZ,
                 centerX, centerY, centerZ,
                 upX,     upY,     upZ);
  }

  int main(int argc, char *argv[]) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800,800);
    glutInitWindowPosition(70,70);
    glutCreateWindow("ST HELENS TERRAIN");

    initValues();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutIdleFunc(idle);
  
    installShaders();
    setCamera();

//    nearPlane = radius - 1.3,farPlane = radius + 1.3;
    matrixIdentity(Projection);
    nearPlane = 1.3;
    farPlane = 4.0;
    matrixPerspective(Projection,
                      30, 1.0, (GLfloat) nearPlane, farPlane);


    glUniform1f(nearPlaneUniform, nearPlane);
    glUniform1f(farPlaneUniform, farPlane);


    glClearColor(0.1,0.1,0.1,0.0);

    glutMainLoop();

    //freeing the memory
    free(indicesArray);
    free(normalsArray);
    free(verticesArray);

    return 0;
  }
