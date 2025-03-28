/*
                              Felipe Pegoraro - 837486 - 28/03/2025

Este código implementa uma aplicação gráfica simples utilizando raylib para desenhar e preencher
polígonos na tela. a lógica central se baseia na interação do usuário com a tela para adicionar
pontos, desenhar arestas entre os pontos e preencher o polígono formado usando a técnica de
preenchimento por linha de varredura (scanline).

Funções com prefixo `m` são funções próprias:
- `m_DrawButton`: desenha botão
- `m_DrawPoints`: desenha todos os pontos na tela
- `m_DrawPointCount`: exibe o número de pontos na tela
- `m_DrawBoard`: desenha a lousa onde os pontos podem ser adicionados
- `m_ClearBoard`: limpa todos os pontos e arestas

IMPORTANTE ===============================
- `m_DrawLine`: desenha uma linha entre dois pontos usando o algoritmo de Bresenham

- `m_PaintPointOnBoard`: adiciona um ponto na posição do mouse
- `m_ClickCallback`: click handler --> função callback associada
- `m_DrawEdges`: desenha as arestas do polígono conectando os pontos

IMPORTANTE ===============================
- `m_ScanlineFill`: preenche o polígono usando a técnica de scanline
   |    internamente scanline usa m_DrawLine.
   |    m_DrawLine NÃO É FUNÇÃO PRONTA.
   |    m_DrawLine usa DrawPixel da lib raylib.
   |    m_DrawLine usa algoritmo de Bresenham para pintar a linha com DrawPixel

- `m_PressShiftMousePos`: ajusta a posição do mouse quando a tecla shift é pressionada.
- `m_CompletePolygon`: fecha o polígono e preenche com a cor definida.
- `m_DrawColorPicker`: desenha um picker de cor
- `m_GetSelectedColor`: seleciona e aplica a cor do picker
*/


#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>

#define GAP 10

#define BTN_WIDTH 150
#define BTN_HEIGHT 50
#define BTN_RADIUS 0.3f
#define BTN_SEGMENTS 10

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define MAX_NUM_POINTS 256

#define LESS_THAN -1
#define GREATER_THAN 1
#define EQUAL 0

#define COLOR_PICKER_SIZE 100
#define COLOR_BOX_SIZE 25

#define FONT_SIZE 20

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

int numPointsOnScreen = 0;
Point points[MAX_NUM_POINTS] = {};

const Color COLORS[] = {
    RED, GREEN, BLUE, YELLOW, ORANGE, PURPLE,
    DARKGRAY, LIGHTGRAY, WHITE, BLACK
};
const unsigned colorsNum = sizeof(COLORS)/sizeof(Color);
Color selectedColor = COLORS[6];

bool shouldClose = false;

void m_DrawButton(Button button) {
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), button.bounds);

    Color buttonColor = isHovered ? ColorAlpha(button.color, 0.80f) : button.color;
    DrawRectangleRounded(button.bounds, BTN_RADIUS, BTN_SEGMENTS, buttonColor);

    DrawText(
        button.label,
        button.bounds.x + (int)(button.bounds.width/2) - (int)(MeasureText(button.label, FONT_SIZE)/2),
        button.bounds.y + (button.bounds.height / 2) - 10,
        FONT_SIZE, WHITE
    );
}


void m_DrawPoints(void){
    for (int i = 0; i < numPointsOnScreen; i++) {
        const int firstOrLast = (i==0 || i==numPointsOnScreen-1);

        if (points[i].color.a > 0){
            DrawCircleV(
                points[i].position,
                firstOrLast ? 7 : 5,
                firstOrLast ? selectedColor : points[i].color
            );
        }
    }
}


void m_DrawPointCount(Vector2 position) {
    char count[4]; 
    snprintf(count, sizeof(count), "%d", numPointsOnScreen);
    DrawText(count, position.x, position.y, FONT_SIZE, WHITE);
}


void m_DrawBoard(Rectangle *board){ DrawRectangleRec(*board, RAYWHITE); }


void m_ClearBoard(void){
    numPointsOnScreen = 0;
    for (int i = 0; i < MAX_NUM_POINTS; i++){
        points[i] = (Point){(Vector2){-10, -10}, RAYWHITE};
    }
}

void m_CloseWindow(void){ shouldClose = true; }

// FUNÇAO PARA DESENHAR A LINHA ENTRE DOIS PONTOS NA TELA.
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

    // VEJA QUE NÃO USA FUNÇÃO DE DRAWLINE PRONTA.
    // USA A FUNÇÃO MAIS PRIMITIVA => DrawPixel(x,y,color) <=
    // SEMELHANTE AO write_pixel(x,y,color);
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
        m_DrawLine(points[i].position, points[nextIndex].position, selectedColor);
    }
}


int m_CompareIntersection(const void *a, const void *b) {
    int x1 = *(int *)a;
    int x2 = *(int *)b;

    if (x1 < x2) return LESS_THAN;
    if (x1 > x2) return GREATER_THAN;
    return EQUAL;
}


// PREENCHE UM POLÍGONO UTILIZANDO A TÉCNICA DE SCANLINE
void m_ScanlineFill(void) {
    if (numPointsOnScreen < 3) return;

    // percorre y=0 --> y=ymax
    for (int y = 0; y < SCREEN_HEIGHT-(GAP*2-BTN_HEIGHT); y++) {
        int intersections[MAX_NUM_POINTS];
        int intersectionCount = 0;

        // para cada ponto...
        for (int i = 0; i < numPointsOnScreen; i++) {
            Vector2 p1 = points[i].position;
            Vector2 p2 = points[(i + 1) % numPointsOnScreen].position;

            // ...verifica se y intercepta ele
            if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
                float xIntersect = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                if (xIntersect >= 0 && xIntersect < SCREEN_WIDTH-GAP*2) {
                    intersections[intersectionCount++] = (int)xIntersect;
                }
            }
        }

        // ordena as interseções em ordem crescente de x
        qsort(intersections, intersectionCount, sizeof(int), m_CompareIntersection);

        // preenche os pixels entre os pares de interseções
        for (int i = 0; i < intersectionCount; i += 2) {
            int xStart = intersections[i];
            int xEnd = intersections[i + 1];

            m_DrawLine((Vector2){xStart, (float)y}, (Vector2){xEnd, (float)y}, selectedColor);
        }
    }
}


void m_PressShiftMousePos(){
    if (numPointsOnScreen == 0) return;

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


void m_CompletePolygon(void){
    if (numPointsOnScreen > 1) {
        m_DrawLine(points[numPointsOnScreen-1].position, points[0].position, selectedColor);
        m_ScanlineFill();
    }
}


void m_DrawColorPicker(Vector2 position) {
    DrawRectangle(position.x-1, position.y-1, COLOR_BOX_SIZE*5+2, COLOR_BOX_SIZE*2+2, WHITE);
    for (unsigned i = 0; i < colorsNum; i++) {
        int x = position.x + (i % 5) * COLOR_BOX_SIZE;
        int y = position.y + (int)(i / 5) * COLOR_BOX_SIZE;
        DrawRectangle(x, y, COLOR_BOX_SIZE, COLOR_BOX_SIZE, COLORS[i]);
    }
}


Color m_GetSelectedColor(Vector2 mousePos, Vector2 pickerPos) {
    int boxX = (mousePos.x - pickerPos.x) / COLOR_BOX_SIZE;
    int boxY = (mousePos.y - pickerPos.y) / COLOR_BOX_SIZE;

    if (boxX >= 0 && boxX < 5 && boxY >= 0 && boxY < (colorsNum / 5 + 1)) {
        int index = boxY * 5 + boxX;
        if (index < colorsNum) return COLORS[index];
    }

    return selectedColor;
}


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Scanline Raylib");

    Button clearButton = (Button){
        .bounds = {GAP, GAP, BTN_WIDTH, BTN_HEIGHT}, 
        .color  = RED,
        .label  = "Clear"
    };

    Button fillButton = (Button){
        .bounds = { 
            clearButton.bounds.x + GAP + BTN_WIDTH,
            GAP,
            BTN_WIDTH,
            BTN_HEIGHT
        }, 
        .color = BLUE,
        .label = "Fill"
    };
    
    Vector2 colorPickerPosition = { fillButton.bounds.x + GAP + BTN_WIDTH, GAP };

    Button closeButton = (Button){
        .bounds = { 
            SCREEN_WIDTH- GAP*5,
            GAP*2-5,
            (int)(BTN_WIDTH/4),
            (int)(BTN_HEIGHT/2)+GAP
        }, 
        .color = COLORS[6],
        .label = "x"
    };

    // tamanho fixo por enquanto
    // basta colocar dentro do loop e usar getScreen(Width|Height)()
    Rectangle board = (Rectangle){
        .x = GAP,
        .y = clearButton.bounds.y + BTN_HEIGHT + GAP,
        .width = SCREEN_WIDTH - 2 * GAP,
        .height = SCREEN_HEIGHT - (clearButton.bounds.y + BTN_HEIGHT + 2 * GAP)
    };

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            mousePos = GetMousePosition();
            selectedColor = m_GetSelectedColor(mousePos, colorPickerPosition);

            m_PressShiftMousePos();
            m_ClickCallback(mousePos, &clearButton.bounds, m_ClearBoard);
            m_ClickCallback(mousePos, &board, m_PaintPointOnBoard);
            m_ClickCallback(mousePos, &closeButton.bounds, m_CloseWindow);
        }

        if (shouldClose) break;

        BeginDrawing();
            ClearBackground(BLACK);
        
            m_DrawPointCount((Vector2){ SCREEN_WIDTH-80, (int)(BTN_HEIGHT / 2) });
            m_DrawBoard(&board);
            m_DrawButton(clearButton);
            m_DrawButton(fillButton);
            m_DrawButton(closeButton);
            m_DrawColorPicker(colorPickerPosition);
            m_DrawPoints();

            m_ClickCallback(mousePos, &fillButton.bounds, m_CompletePolygon);
            if (numPointsOnScreen > 1) m_DrawEdges();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

