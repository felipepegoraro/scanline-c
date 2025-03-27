#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define BTN_WIDTH 180
#define BTN_HEIGHT 50
#define BTN_GAP 10
#define BTN_RADIUS 0.3f
#define BTN_SEGMENTS 10

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define MAX_NUM_POINTS 256

#define LESS_THAN -1
#define GREATER_THAN 1
#define EQUAL 0

typedef struct {
    Rectangle bounds;
    Color color;
    const char *label;
} Button;

typedef struct {
    Vector2 position;
    Color color;
} Point;

Vector2 mousePos = {0, 0};
Point points[MAX_NUM_POINTS] = {};
int numPointsOnScreen = 0;

void m_DrawButton(Button button) {
    DrawRectangleRounded(button.bounds, BTN_RADIUS, BTN_SEGMENTS, button.color);
    DrawText(button.label, button.bounds.x + 20, button.bounds.y + 15, 20, WHITE);
}

void m_DrawPoints(void){
    for (int i = 0; i < numPointsOnScreen; i++) {
        const int size = (i==0 || i==numPointsOnScreen-1) ? 10 : 5;
        if (points[i].color.a > 0){
            DrawCircleV(points[i].position, size, points[i].color);
        }
    }
}

void m_DrawPointCount(Vector2 position) {
    char count[4]; 
    snprintf(count, sizeof(count), "%d", numPointsOnScreen);
    DrawText(count, position.x, position.y, 20, WHITE);
}

void m_DrawBoard(Rectangle *board){ DrawRectangleRec(*board, RAYWHITE); }

void m_ClearBoard(void){
    numPointsOnScreen = 0;
    for (int i = 0; i < MAX_NUM_POINTS; i++){
        points[i] = (Point){(Vector2){-10, -10}, RAYWHITE};
    }
}


/* 
    FUNÇAO PARA DESENHAR A LINHA ENTRE DOIS PONTOS NA TELA.

    EM VEZ DE USAR A FUNÇÃO PRONTA DRAWLINEV, CRIEI A FUNÇÃO 
    m_DrawLine POIS ESSA USA UMA FUNÇÃO MAIS BÁSICA.
    ESSA FUNÇÃO USA DRAW PIXEL QUE É SEMELHANTE AO WRITEPIXEL 
    DRAW PIXEL RECEBE OS PARAMETROS POSICAO X, Y E A COR DO PIXEL 
    A SER PINTADO.
*/
void m_DrawLine(Vector2 start, Vector2 end, Color color){
    int x1 = (int)start.x;
    int y1 = (int)start.y;
    int x2 = (int)end.x;
    int y2 = (int)end.y;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? GREATER_THAN : LESS_THAN;
    int sy = (y1 < y2) ? GREATER_THAN : LESS_THAN;
    int err = dx - dy;

    // WRITE PIXEL
    while (true) {
        DrawPixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) break;

        int e2 = err * 2;

        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void m_PaintPointOnBoard(void){
    if (numPointsOnScreen < MAX_NUM_POINTS){
        points[numPointsOnScreen++] = (Point){
            .position = mousePos,
            .color = (Color){
                GetRandomValue(50, 255),
                GetRandomValue(50, 255),
                GetRandomValue(50, 255), 255
            }
        };
    }
}

void m_ClickCallback(Vector2 mouse, Rectangle *rect, void (*callback)()){
    if (CheckCollisionPointRec(mouse, *rect)) callback();
}

void m_DrawEdges(void) {
    for (int i = 0; i < numPointsOnScreen - 1; i++) {
        int nextIndex = (i + 1) % numPointsOnScreen;
        m_DrawLine(points[i].position, points[nextIndex].position, DARKGRAY);
    }
}

int m_ComparePoints(const void *a, const void *b) {
    Point *pointA = (Point *)a;
    Point *pointB = (Point *)b;

    float cx = 0.0f, cy = 0.0f;
    for (int i = 0; i < numPointsOnScreen; i++) {
        cx += points[i].position.x;
        cy += points[i].position.y;
    }
    cx /= numPointsOnScreen;
    cy /= numPointsOnScreen;

    float angleA = atan2f(pointA->position.y - cy, pointA->position.x - cx);
    float angleB = atan2f(pointB->position.y - cy, pointB->position.x - cx);

    if (angleA < angleB) return LESS_THAN;
    if (angleA > angleB) return GREATER_THAN;

    return EQUAL;
}

int m_CompareIntegers(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void m_ScanlineFill(void) {
    qsort(points, numPointsOnScreen, sizeof(Point), m_ComparePoints);

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        int intersections[MAX_NUM_POINTS];
        int numIntersections = 0;

        for (int i = 0; i < numPointsOnScreen; i++) {
            int next = (i + 1) % numPointsOnScreen;

            Vector2 p1 = points[i].position;
            Vector2 p2 = points[next].position;

            if (((p1.y > y) != (p2.y > y)) && (y != (int)p1.y || y != (int)p2.y)) {
                float xIntersection = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                intersections[numIntersections++] = (int)xIntersection;
            }
        }

        qsort(intersections, numIntersections, sizeof(int), m_CompareIntegers);

        for (int i = 0; i < numIntersections; i += 2) {
            for (int x = intersections[i]; x < intersections[i + 1]; x++) {
                DrawPixel(x, y, DARKGRAY); 
            }
        }
    }
}

void m_PressShiftMousePos(){
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)){
        int mx = mousePos.x;
        int my = mousePos.y;
        int lastpx = points[numPointsOnScreen-1].position.x;
        int lastpy = points[numPointsOnScreen-1].position.y;

        int dx = abs(mx-lastpx);
        int dy = abs(my-lastpy);

        if (dx > dy) my = lastpy;
        else mx = lastpx;

        mousePos.x = mx;
        mousePos.y = my;
    }
}


void m_FinalizaEdges(void){
    if (numPointsOnScreen > 1)
        m_DrawLine(points[numPointsOnScreen-1].position, points[0].position, DARKGRAY);
    if (numPointsOnScreen > 2) m_ScanlineFill();
}


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Scanline Raylib");

    Button clearButton = (Button){
        .bounds = {BTN_GAP, BTN_GAP, BTN_WIDTH, BTN_HEIGHT}, 
        .color  = RED,
        .label  = "Clear"
    };

    Button fillButton = (Button){
        .bounds = { 
            clearButton.bounds.x + BTN_GAP + BTN_WIDTH,
            BTN_GAP,
            BTN_WIDTH,
            BTN_HEIGHT
        }, 
        .color = BLUE,
        .label = "Fill"
    };

    Rectangle board = (Rectangle){
        .x = BTN_GAP,
        .y = clearButton.bounds.y + BTN_HEIGHT + BTN_GAP,
        .width = GetScreenWidth() - 2 * BTN_GAP,
        .height = GetScreenHeight() - (clearButton.bounds.y + BTN_HEIGHT + 2 * BTN_GAP)
    };

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mousePos = GetMousePosition();
            m_PressShiftMousePos();
            m_ClickCallback(mousePos, &clearButton.bounds, m_ClearBoard);
            m_ClickCallback(mousePos, &board, m_PaintPointOnBoard);
        }

        BeginDrawing();
            ClearBackground(LIGHTGRAY);
        
            m_DrawPointCount((Vector2){GetScreenWidth() - 40, fillButton.bounds.y + (int)(BTN_HEIGHT / 2) - BTN_GAP});
            m_DrawBoard(&board);
            m_DrawButton(clearButton);
            m_DrawButton(fillButton);

            m_DrawPoints();

            m_ClickCallback(mousePos, &fillButton.bounds, m_FinalizaEdges);
            if (numPointsOnScreen > 1) m_DrawEdges();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

