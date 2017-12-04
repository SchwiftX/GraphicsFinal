#include "glad.h"  //Include order can matter here
#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int screenWidth = 800;
int screenHeight = 600;
bool saveOutput = false;
float timePast = 0;

//SJG: Store the object coordinates
//You should have a representation for the state of each object
float objx=0, objy=0, objz=0;
float colR=1, colG=1, colB=1;


bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);

//srand(time(NULL));
float rand01(){
    return rand()/(float)RAND_MAX;
}

void drawGeometry(int shaderProgram, int SilouetteShader, int numVerts1, int numVerts2, glm::mat4 proj, glm::mat4 view);

int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
    
    //Ask SDL to get a recent version of OpenGL (3 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);//
    
    //Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    
    //Create a context to draw in
    SDL_GLContext context = SDL_GL_CreateContext(window);
    
    //Load OpenGL extentions with GLAD
    if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
        printf("\nOpenGL loaded\n");
        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n\n", glGetString(GL_VERSION));
    }
    else {
        printf("ERROR: Failed to initialize OpenGL context.\n");
        return -1;
    }
    
    
    //Load Model 1
    ifstream modelFile;
    modelFile.open("models/teapot.txt");
    int numLines = 0;
    modelFile >> numLines;
    float* model1 = new float[numLines];
    for (int i = 0; i < numLines; i++){
        modelFile >> model1[i];
    }
    printf("%d\n",numLines);
    int numVerts1 = numLines/8;
    modelFile.close();
    
    //Load Model 2
    modelFile.open("models/knot.txt");
    numLines = 0;
    modelFile >> numLines;
    float* model2 = new float[numLines];
    for (int i = 0; i < numLines; i++){
        modelFile >> model2[i];
    }
    printf("%d\n",numLines);
    int numVerts2 = numLines/8;
    modelFile.close();
    
    //SJG: I load each model in a different array, then concatenate everything in one big array
    //     There are better options, but this works.
    //Concatenate model arrays
    float* modelData = new float[(numVerts1+numVerts2)*8];
    copy(model1, model1+numVerts1*8, modelData);
    copy(model2, model2+numVerts2*8, modelData+numVerts1*8);
    int totalNumVerts = numVerts1+numVerts2;
    
    
    //// Allocate Texture 0 (Wood) ///////
    SDL_Surface* surface = SDL_LoadBMP("wood.bmp");
    if (surface==NULL){ //If it failed, print the error
        printf("Error: \"%s\"\n",SDL_GetError()); return 1;
    }
    GLuint tex0;
    glGenTextures(1, &tex0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w,surface->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    
    SDL_FreeSurface(surface);
    //// End Allocate Texture ///////
    
    
    //// Allocate Texture 1 (Brick) ///////
    SDL_Surface* surface1 = SDL_LoadBMP("brick.bmp");
    if (surface==NULL){ //If it failed, print the error
        printf("Error: \"%s\"\n",SDL_GetError()); return 1;
    }
    GLuint tex1;
    glGenTextures(1, &tex1);
    
    //Load the texture into memory
    glActiveTexture(GL_TEXTURE1);
    
    glBindTexture(GL_TEXTURE_2D, tex1);
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //How to filter
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface1->w,surface1->h, 0, GL_BGR,GL_UNSIGNED_BYTE,surface1->pixels);
    
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    SDL_FreeSurface(surface1);
    //// End Allocate Texture ///////
    
    //Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
    GLuint vao;
    glGenVertexArrays(1, &vao); //Create a VAO
    glBindVertexArray(vao); //Bind the above created VAO to the current context
    
    
    //Allocate memory on the graphics card to store geometry (vertex buffer object)
    GLuint vbo[1];
    glGenBuffers(1, vbo);  //Create 1 buffer called vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
    glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
    //GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
    //GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used
    
    int texturedShader = InitShader("vertexTex.glsl", "fragmentTex.glsl");
    int phongShader = InitShader("vertex.glsl", "fragment.glsl");
    
    
    
    //Tell OpenGL how to set fragment shader input
    GLint posAttrib = glGetAttribLocation(texturedShader, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
    //Attribute, vals/attrib., type, normalized?, stride, offset
    //Binds to VBO current GL_ARRAY_BUFFER
    glEnableVertexAttribArray(posAttrib);
    
    //GLint colAttrib = glGetAttribLocation(phongShader, "inColor");
    //glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    //glEnableVertexAttribArray(colAttrib);
    
    GLint normAttrib = glGetAttribLocation(texturedShader, "inNormal");
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(normAttrib);
    
    GLint texAttrib = glGetAttribLocation(texturedShader, "inTexcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    
    
    GLint sposAttrib = glGetAttribLocation(phongShader,"i_position");
    glVertexAttribPointer(sposAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
    glEnableVertexAttribArray(sposAttrib);
    
    GLint snormalAttrib = glGetAttribLocation(phongShader,"i_normal");
    glVertexAttribPointer(snormalAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
    glEnableVertexAttribArray(snormalAttrib);
    
    GLint uniView = glGetUniformLocation(texturedShader, "view");
    GLint uniProj = glGetUniformLocation(texturedShader, "proj");
    
    glBindVertexArray(0); //Unbind the VAO in case we want to create a new one
    
    
    glEnable(GL_DEPTH_TEST);
    
    //Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    bool quit = false;
    while (!quit){
        while (SDL_PollEvent(&windowEvent)){  //inspect all events in the queue
            if (windowEvent.type == SDL_QUIT) quit = true;
            //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            //Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
            quit = true; //Exit event loop
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f){ //If "f" is pressed
                fullscreen = !fullscreen;
                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen
            }
            
            //SJG: Use key input to change the state of the object
            //     We can use the ".mod" flag to see if modifiers such as shift are pressed
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_UP){ //If "up key" is pressed
                if (windowEvent.key.keysym.mod & KMOD_SHIFT) objx -= .1; //Is shift pressed?
                else objz += .1;
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_DOWN){ //If "down key" is pressed
                if (windowEvent.key.keysym.mod & KMOD_SHIFT) objx += .1; //Is shift pressed?
                else objz -= .1;
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LEFT){ //If "up key" is pressed
                objy -= .1;
            }
            if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_RIGHT){ //If "down key" is pressed
                objy += .1;
            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_c){ //If "c" is pressed
                colR = rand01();
                colG = rand01();
                colB = rand01();
            }
            
        }
        
        // Clear the screen to default color
        glClearColor(.2f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //glUseProgram(texturedShader);
        
        
        if (!saveOutput) timePast = SDL_GetTicks()/1000.f;
        if (saveOutput) timePast += .07; //Fix framerate at 14 FPS
        
        glm::mat4 view = glm::lookAt(
                                     glm::vec3(3.f, 0.f, 0.f),  //Cam Position
                                     glm::vec3(0.0f, 0.0f, 0.0f),  //Look at point
                                     glm::vec3(0.0f, 0.0f, 1.0f)); //Up
        glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
        
        glm::mat4 proj = glm::perspective(3.14f/4, screenWidth / (float) screenHeight, 1.0f, 10.0f); //FOV, aspect, near, far
        glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
        
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex0);
        glUniform1i(glGetUniformLocation(texturedShader, "tex0"), 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1);
        glUniform1i(glGetUniformLocation(texturedShader, "tex1"), 1);
        
        glBindVertexArray(vao);
        drawGeometry(texturedShader,phongShader,numVerts1,numVerts2,proj,view);
        
        
        if (saveOutput) Win2PPM(screenWidth,screenHeight);
        
        SDL_GL_SwapWindow(window); //Double buffering
    }
    
    //Clean Up
    glDeleteProgram(phongShader);
    glDeleteBuffers(1, vbo);
    glDeleteVertexArrays(1, &vao);
    
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}

void drawGeometry(int textureShader, int SilouetteShader, int numVerts1, int numVerts2, glm::mat4 proj, glm::mat4 view){
    

    
    glUseProgram(SilouetteShader);
    GLint uniMVP = glGetUniformLocation(SilouetteShader,"u_mvp_mat");
    glm::mat4 model1;
    model1 = glm::rotate(model1, 3.14f/2, glm::vec3(0.0f,0.0f,1.0f));
    glm::mat4 MVP = proj*view*model1;
    glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));

    GLint offset = glGetUniformLocation(SilouetteShader,"u_offset1");
    glUniform1f(offset,0.02f);
    GLint incolor1 = glGetUniformLocation(SilouetteShader,"u_color1");
    glm::vec3 back_color(0.0f,0.0f,0.0f);
    glUniform3fv(incolor1,1,glm::value_ptr(back_color));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDepthMask(GL_TRUE);
    glDrawArrays(GL_TRIANGLES, 0, numVerts1); //(Primitive Type, Start Vertex, End Vertex)
    
//        glm::vec3 back_color1(1.0f,1.0f,1.0f);
//        glUniform3fv(incolor1,1,glm::value_ptr(back_color1));
//        glUniform1f(offset,0.0f);
//        glCullFace(GL_BACK);
//        glDepthMask(GL_TRUE);
//        glDrawArrays(GL_TRIANGLES, 0, numVerts1); //(Primitive Type, Start Vertex, End Vertex)
    
    glUseProgram(textureShader);
    GLint uniColor = glGetUniformLocation(textureShader, "inColor");
    glm::vec3 colVec(colR,colG,colB);
    glUniform3fv(uniColor, 1, glm::value_ptr(colVec));
    GLint uniTexID = glGetUniformLocation(textureShader, "texID");
    glm::mat4 model;
    model = glm::rotate(model,3.14f/2,glm::vec3(0.0f, 0.0f, 1.0f));
    //model = glm::rotate(model,timePast * 3.14f/4,glm::vec3(1.0f, 0.0f, 0.0f));
    //model = glm::scale(model,glm::vec3(.2f,.2f,.2f)); //An example of scale
    GLint uniModel = glGetUniformLocation(textureShader, "model");
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(uniTexID, 0); //Set texture ID to use (-1 = no texture)
    //SJG: Here we draw only the first object (start at 0, draw numTris1 triangles)
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
    glDrawArrays(GL_TRIANGLES, 0, numVerts1); //(Primitive Type, Start Vertex, End Vertex)
    


    
    
//    //Instancing! Same model (e.g., same draw call), but different parameters
//    model = glm::mat4();
//    model = glm::translate(model,glm::vec3(-2,-1,-.4));
//    //model = glm::scale(model,2.f*glm::vec3(1.f,1.f,0.5f));
//    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
//    glUniform1i(uniTexID, 0); //Set texture ID to use
//    glDrawArrays(GL_TRIANGLES, 0, numVerts1); //(Primitive Type, Start Vertex, End Vertex)
//
//    //SJG: Let's change the model matrix before the next call to show how to change diffent
//    //     models independently
//    model = glm::mat4();
//    model = glm::scale(model,glm::vec3(.8f,.8f,.8f));
//    model = glm::translate(model,glm::vec3(objx,objy,objz));
//    glUniform1i(uniTexID, 1); //Set texture ID to use
//    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
//    //SJG: The 2nd model, starts at an offset of numTris1 and has numTris2 triangles
//    //      My approach here is not very scalable. You should do better
//    glDrawArrays(GL_TRIANGLES, numVerts1, numVerts2); //(Primitive Type, Start Vertex, End Vertex)
}

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile){
    FILE *fp;
    long length;
    char *buffer;
    
    // open the file containing the text of the shader code
    fp = fopen(shaderFile, "r");
    
    // check for errors in opening the file
    if (fp == NULL) {
        printf("can't open shader source file %s\n", shaderFile);
        return NULL;
    }
    
    // determine the file size
    fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
    length = ftell(fp);  // return the value of the current position
    
    // allocate a buffer with the indicated number of bytes, plus one
    buffer = new char[length + 1];
    
    // read the appropriate number of bytes from the file
    fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
    fread(buffer, 1, length, fp); // read all of the bytes
    
    // append a NULL character to indicate the end of the string
    buffer[length] = '\0';
    
    // close the file
    fclose(fp);
    
    // return the string
    return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName){
    GLuint vertex_shader, fragment_shader;
    GLchar *vs_text, *fs_text;
    GLuint program;
    
    // check GLSL version
    printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    // Create shader handlers
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read source code from shader files
    vs_text = readShaderSource(vShaderFileName);
    fs_text = readShaderSource(fShaderFileName);
    
    // error check
    if (vs_text == NULL) {
        printf("Failed to read from vertex shader file %s\n", vShaderFileName);
        exit(1);
    } else if (DEBUG_ON) {
        printf("Vertex Shader:\n=====================\n");
        printf("%s\n", vs_text);
        printf("=====================\n\n");
    }
    if (fs_text == NULL) {
        printf("Failed to read from fragent shader file %s\n", fShaderFileName);
        exit(1);
    } else if (DEBUG_ON) {
        printf("\nFragment Shader:\n=====================\n");
        printf("%s\n", fs_text);
        printf("=====================\n\n");
    }
    
    // Load Vertex Shader
    const char *vv = vs_text;
    glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
    glCompileShader(vertex_shader); // Compile shaders
    
    // Check for errors
    GLint  compiled;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printf("Vertex shader failed to compile:\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }
    
    // Load Fragment Shader
    const char *ff = fs_text;
    glShaderSource(fragment_shader, 1, &ff, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    
    //Check for Errors
    if (!compiled) {
        printf("Fragment shader failed to compile\n");
        if (DEBUG_ON) {
            GLint logMaxSize, logLength;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
            printf("printing error message of %d bytes\n", logMaxSize);
            char* logMsg = new char[logMaxSize];
            glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
            printf("%d bytes retrieved\n", logLength);
            printf("error message: %s\n", logMsg);
            delete[] logMsg;
        }
        exit(1);
    }
    
    // Create the program
    program = glCreateProgram();
    
    // Attach shaders to program
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    
    // Link and set program to use
    glLinkProgram(program);
    
    return program;
}

void Win2PPM(int width, int height){
    char outdir[10] = "out/"; //Must be exist!
    int i,j;
    FILE* fptr;
    static int counter = 0;
    char fname[32];
    unsigned char *image;
    
    /* Allocate our buffer for the image */
    image = (unsigned char *)malloc(3*width*height*sizeof(char));
    if (image == NULL) {
        fprintf(stderr,"ERROR: Failed to allocate memory for image\n");
    }
    
    /* Open the file */
    sprintf(fname,"%simage_%04d.ppm",outdir,counter);
    if ((fptr = fopen(fname,"w")) == NULL) {
        fprintf(stderr,"ERROR: Failed to open file for window capture\n");
    }
    
    /* Copy the image into our buffer */
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,image);
    
    /* Write the PPM file */
    fprintf(fptr,"P6\n%d %d\n255\n",width,height);
    for (j=height-1;j>=0;j--) {
        for (i=0;i<width;i++) {
            fputc(image[3*j*width+3*i+0],fptr);
            fputc(image[3*j*width+3*i+1],fptr);
            fputc(image[3*j*width+3*i+2],fptr);
        }
    }
    
    free(image);
    fclose(fptr);
    counter++;
}
