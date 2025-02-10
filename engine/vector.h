#ifndef GAMEENGINE_VECTOR_H
#define GAMEENGINE_VECTOR_H

#include <cstdio>
#include <vector>
#include "../constants.h"

class Vector3 {
public:
    float x;
    float y;
    float z;
};

class Transform {
public:
    Vector3 pos = {0, 0, 0};
    Vector3 rot = {0, 0, 0};
    Vector3 scale = {1, 1, 1};
    float width = 1;
    float height = 1;

    void Translate(Vector3 vector){}
    void Rotate(float degrees){}
};

class Component {
public:
    virtual void Init() = 0;
    virtual void Update() = 0;
};

class GameObject {
public:
    Transform transform{};
    std::vector<Component*> components{};

    virtual void Update() {
        for (auto & component : components) {
            component->Update();
        }
    }
};

class Triangle : public GameObject {
public:
    Vector3 vertex1 = { 1.0f,  -1.0f, 0.0f};
    Vector3 vertex2 = {-1.0f, -1.0f, 0.0f};
    Vector3 vertex3 = {0, 1.0f, 0.0f};

    void Update() override {
        GameObject::Update();
        vertex1.x = transform.pos.x + transform.width/2;
        vertex1.y = transform.pos.y - transform.height/2;
        vertex2.x = transform.pos.x - transform.width/2;
        vertex2.y = transform.pos.y - transform.height/2;
        vertex3.x = transform.pos.x;
        vertex3.y = transform.pos.y + transform.height/2;
    }
};

enum FontChar {
//    Unknown = 0,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,
    Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
    Bang, QuestionMark
};
static const FontChar AllCharacters[] = { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
                                          a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,
                                          Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
                                          Bang, QuestionMark };
int FontCharToAtlasRow(FontChar c) {
    switch (c) {
//        case Unknown: return -1;
        case A: return 0;
        case B: return 1;
        case C: return 2;
        case D: return 3;
        case E: return 4;
        case F: return 5;
        case G: return 6;
        case H: return 7;
        case I: return 0;
        case J: return 1;
        case K: return 2;
        case L: return 3;
        case M: return 4;
        case N: return 5;
        case O: return 6;
        case P: return 7;
        case Q: return 0;
        case R: return 1;
        case S: return 2;
        case T: return 3;
        case U: return 4;
        case V: return 5;
        case W: return 6;
        case X: return 7;
        case Y: return 0;
        case Z: return 1;
        case a: return 2;
        case b: return 3;
        case ::c: return 4;
        case d: return 5;
        case e: return 6;
        case f: return 7;
        case g: return 0;
        case h: return 1;
        case i: return 2;
        case j: return 3;
        case k: return 4;
        case l: return 5;
        case m: return 6;
        case n: return 7;
        case o: return 0;
        case p: return 1;
        case q: return 2;
        case r: return 3;
        case s: return 4;
        case t: return 5;
        case u: return 6;
        case v: return 7;
        case w: return 0;
        case x: return 1;
        case y: return 2;
        case z: return 3;
        case Zero: return 4;
        case One: return 5;
        case Two: return 6;
        case Three: return 7;
        case Four: return 0;
        case Five: return 1;
        case Six: return 2;
        case Seven: return 3;
        case Eight: return 4;
        case Nine: return 5;
        case Bang: return 6;
        case QuestionMark: return 7;
    }
}
int CharToAtlasColumn(char c) {
    switch (c) {
        case 'A': return 0;
        case 'B': return 1;
        case 'C': return 2;
        case 'D': return 3;
        case 'E': return 4;
        case 'F': return 5;
        case 'G': return 6;
        case 'H': return 7;
        case 'I': return 0;
        case 'J': return 1;
        case 'K': return 2;
        case 'L': return 3;
        case 'M': return 4;
        case 'N': return 5;
        case 'O': return 6;
        case 'P': return 7;
        case 'Q': return 0;
        case 'R': return 1;
        case 'S': return 2;
        case 'T': return 3;
        case 'U': return 4;
        case 'V': return 5;
        case 'W': return 6;
        case 'X': return 7;
        case 'Y': return 0;
        case 'Z': return 1;
        case 'a': return 2;
        case 'b': return 3;
        case 'c': return 4;
        case 'd': return 5;
        case 'e': return 6;
        case 'f': return 7;
        case 'g': return 0;
        case 'h': return 1;
        case 'i': return 2;
        case 'j': return 3;
        case 'k': return 4;
        case 'l': return 5;
        case 'm': return 6;
        case 'n': return 7;
        case 'o': return 0;
        case 'p': return 1;
        case 'q': return 2;
        case 'r': return 3;
        case 's': return 4;
        case 't': return 5;
        case 'u': return 6;
        case 'v': return 7;
        case 'w': return 0;
        case 'x': return 1;
        case 'y': return 2;
        case 'z': return 3;
        case '0': return 4;
        case '1': return 5;
        case '2': return 6;
        case '3': return 7;
        case '4': return 0;
        case '5': return 1;
        case '6': return 2;
        case '7': return 3;
        case '8': return 4;
        case '9': return 5;
        case '!': return 6;
        case '?': return 7;
        default: return -1;
    }
}
int CharToAtlasRow(char c) {
    switch (c) {
        case 'A': return 0;
        case 'B': return 0;
        case 'C': return 0;
        case 'D': return 0;
        case 'E': return 0;
        case 'F': return 0;
        case 'G': return 0;
        case 'H': return 0;
        case 'I': return 1;
        case 'J': return 1;
        case 'K': return 1;
        case 'L': return 1;
        case 'M': return 1;
        case 'N': return 1;
        case 'O': return 1;
        case 'P': return 1;
        case 'Q': return 2;
        case 'R': return 2;
        case 'S': return 2;
        case 'T': return 2;
        case 'U': return 2;
        case 'V': return 2;
        case 'W': return 2;
        case 'X': return 2;
        case 'Y': return 3;
        case 'Z': return 3;
        case 'a': return 3;
        case 'b': return 3;
        case 'c': return 3;
        case 'd': return 3;
        case 'e': return 3;
        case 'f': return 3;
        case 'g': return 4;
        case 'h': return 4;
        case 'i': return 4;
        case 'j': return 4;
        case 'k': return 4;
        case 'l': return 4;
        case 'm': return 4;
        case 'n': return 4;
        case 'o': return 5;
        case 'p': return 5;
        case 'q': return 5;
        case 'r': return 5;
        case 's': return 5;
        case 't': return 5;
        case 'u': return 5;
        case 'v': return 5;
        case 'w': return 6;
        case 'x': return 6;
        case 'y': return 6;
        case 'z': return 6;
        case '0': return 6;
        case '1': return 6;
        case '2': return 6;
        case '3': return 6;
        case '4': return 7;
        case '5': return 7;
        case '6': return 7;
        case '7': return 7;
        case '8': return 7;
        case '9': return 7;
        case '!': return 7;
        case '?': return 7;
        default: return -1;
    }
}


class Sprite : public GameObject {
public:
    Vector3 vertex1 = {-0.5f,  0.5f, 0.0f};  // top left
    Vector3 vertex2 = {0.5f,  0.5f, 0.0f};  // top right
    Vector3 vertex3 = {0.5f, -0.5f, 0.0f};  // bottom right
    Vector3 vertex4 = {-0.5f, -0.5f, 0.0f};  // bottom left

    int atlasIndex = -1;
    int atlasRow = 0;
    int atlasColumn = 1;
    float atlasCellSize = 0.125f;

    void Update() override {
        GameObject::Update();
        vertex1.x = transform.pos.x - transform.width/2;
        vertex1.y = transform.pos.y + transform.height/2;
        vertex2.x = transform.pos.x + transform.width/2;
        vertex2.y = transform.pos.y + transform.height/2;
        vertex3.x = transform.pos.x + transform.width/2;
        vertex3.y = transform.pos.y - transform.height/2;
        vertex4.x = transform.pos.x - transform.width/2;
        vertex4.y = transform.pos.y - transform.height/2;

//        if (atlasIndex > -1 || (atlasRow > -1 and atlasColumn > -1)) {
//
//        }
    }
};

//class SpriteAtlas : public Sprite {
//public:
//    int index = 0;
//    float atlasCellWidth = 0.125f;
//    float atlasCellHeight = 0.125f;
//
//    void Update() override {
//        GameObject::Update();
//
//        vertex1.x =
//
//
//        vertex1.x = transform.pos.x - transform.width/2;
//        vertex1.y = transform.pos.y + transform.height/2;
//        vertex2.x = transform.pos.x + transform.width/2;
//        vertex2.y = transform.pos.y + transform.height/2;
//        vertex3.x = transform.pos.x + transform.width/2;
//        vertex3.y = transform.pos.y - transform.height/2;
//        vertex4.x = transform.pos.x - transform.width/2;
//        vertex4.y = transform.pos.y - transform.height/2;
//    }
//};


#endif //GAMEENGINE_VECTOR_H
