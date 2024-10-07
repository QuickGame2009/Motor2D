#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#define WORLD_WIDTH 1050
#define WORLD_HEIGHT 1050

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

float MaxPlayerVelocity = 50.0f;
float MinPlayerVelocity = 5.0f;
float PlayerVelocity = 5.0f;

float scaleX, scaleY; // Escala para adaptar al tamaño de la ventana
bool ShowGameInfo = false;
bool F5Pressed = false;
bool ShiftEnable = false;

int quit = 0;

typedef struct {
    unsigned char r, g, b;
} Pixel;

Pixel* framebuffer;

struct Player {
    float x;
    float y;
};

struct Player player;
struct World {
    int r;
    int g;
    int b;
} World[WORLD_WIDTH][WORLD_HEIGHT];


HWND hwnd;
HDC hdc;
HDC hdcMem;
BITMAPINFO bmi;
HBITMAP hBitmap;
PAINTSTRUCT ps;



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Declaración de WindowProc
void resizeFramebuffer(int width, int height); // Declaración de resizeFramebuffer

void initWindow() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc; // Asignar la función de procedimiento de ventana
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = "MyWindowClass";
    RegisterClass(&wc);

    hwnd = CreateWindowEx(0, "MyWindowClass", "2D World Game", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, GetModuleHandle(NULL), NULL);

    hdc = GetDC(hwnd);
    hdcMem = CreateCompatibleDC(hdc);

    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;

    // Inicializar escala
    scaleX = 1.0f;
    scaleY = 1.0f;

    hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&framebuffer, NULL, 0);
    SelectObject(hdcMem, hBitmap);
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        resizeFramebuffer(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void resizeFramebuffer(int width, int height) {
    // Ajustar el tamaño de la ventana en incrementos de 20 en la coordenada x & y
    SCREEN_WIDTH  = (((width + 10) / 20) * 20)  + 20;
    SCREEN_HEIGHT = (((height + 10) / 20) * 20) + 40;

    // Liberar el bitmap antiguo
    if (hBitmap != NULL) {
        DeleteObject(hBitmap);
    }

    // Actualizar el tamaño del bitmap
    bmi.bmiHeader.biWidth = SCREEN_WIDTH;
    bmi.bmiHeader.biHeight = -SCREEN_HEIGHT;  // negative for top-down bitmap
    hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&framebuffer, NULL, 0);
    SelectObject(hdcMem, hBitmap);

    // Actualizar la escala
    scaleX = (float)WORLD_WIDTH / (float)SCREEN_WIDTH;
    scaleY = (float)WORLD_HEIGHT / (float)SCREEN_HEIGHT;

    // Mostrar el tamaño de la ventana en el título
    char title[50];
    sprintf(title, "2D World Game");
    SetWindowText(hwnd, title);
}


void destroyWindow() {
    ReleaseDC(hwnd, hdc);
    DeleteDC(hdcMem);
    DeleteObject(hBitmap);
    DestroyWindow(hwnd);
}


void drawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        int index = y * SCREEN_WIDTH + x;
        framebuffer[index].r = r;
        framebuffer[index].g = g;
        framebuffer[index].b = b;
    }
}


void clearScreen() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i].r = 0;
        framebuffer[i].g = 0;
        framebuffer[i].b = 0;
    }
}


void render() {
    StretchDIBits(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
        framebuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
}


void world(int x, int y, int r, int g, int b) {
    World[x][y].r = r;
    World[x][y].g = g;
    World[x][y].b = b;
}


void GameInfo() {
     
    if(ShowGameInfo == true) {
     
	char info[50];
    sprintf(info, "Player Coordinates: X=%.1f Y=%.1f", player.x, player.y);
    TextOut(hdcMem, 10, 0, info, strlen(info));
    sprintf(info, "Player Velocity: %.1f", PlayerVelocity);
    TextOut(hdcMem, 10, 20, info, strlen(info));
    sprintf(info, "FPS: Disabled");
    TextOut(hdcMem, 10, 40, info, strlen(info));
    sprintf(info, "Window Size: %dx%d", SCREEN_WIDTH, SCREEN_HEIGHT);
    TextOut(hdcMem, 10, 60, info, strlen(info)); // Ajustar la posición según sea necesario
    
    }
    
}

void DetectKeys() {
        
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000 && ShiftEnable) {
                if (PlayerVelocity < MaxPlayerVelocity) {
                    PlayerVelocity = PlayerVelocity + 1.0f;
                }
            } else {
        	    if (PlayerVelocity > MinPlayerVelocity) {
        		    PlayerVelocity = PlayerVelocity - PlayerVelocity / PlayerVelocity;
        	    }
            }
            
        
		if (GetAsyncKeyState(VK_F5) & 0x8000) {
            if (!F5Pressed) { // Si no estaba presionado antes
                 ShowGameInfo = !ShowGameInfo; // Cambia el estado de mostrar información
                 F5Pressed = true; // Marca que F5 fue presionado
            }
        } else {
            F5Pressed = false; // Resetea el estado cuando F5 no está presionado
		}
		
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            quit = 1;
            return;
        }
    	
    	//Movimiento del jugador con WASD y Flechas
    	ShiftEnable = false;
    	
        if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState('W') & 0x8000) {
            player.y -= PlayerVelocity;
            ShiftEnable = true;
        }    
        if (GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState('S') & 0x8000) {
            player.y += PlayerVelocity;
            ShiftEnable = true;
        }
        if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState('A') & 0x8000) {
            player.x -= PlayerVelocity;
            ShiftEnable = true;
        }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState('D') & 0x8000) {
            player.x += PlayerVelocity;
            ShiftEnable = true;
        }
        
}


void loadWorld() {
	
	for (int a = 0; a < WORLD_HEIGHT; a++) {
        for (int b = 0; b < WORLD_WIDTH; b++) {
            world(a,b, 0, 255, 0);
    	}
    }
    
    for (int y = 0; y < 255; y++) {
        for (int x = 0; x < 255; x++) {
            for (int z = 0; z < 255; z++) {
                world(y+255, x+255, x, y, z);
            }
        }
    }
    for (int y = 0; y < 255; y++) {
        for (int x = 0; x < 255; x++) {
            for (int z = 0; z < 255; z++) {
                world(y+255, x + 510, x, z, y);
            }
        }
    }
    for (int y = 0; y < 255; y++) {
        for (int x = 0; x < 255; x++) {
            for (int z = 0; z < 255; z++) {
                world(y + 510, x+255, y, x, z);
            }
        }
    }
    for (int y = 0; y < 255; y++) {
        for (int x = 0; x < 255; x++) {
            for (int z = 0; z < 255; z++) {
                world(y + 510, x + 510, y, z, x);
            }
        }
    }
    for (int y = 0; y < 255; y++) {
        for (int x = 0; x < 255; x++) {
            for (int z = 0; z < 255; z++) {
                world(y + 765, x+255, z, x, y);
            }
        }
    }
    for (int y = 0; y < 255; y++) {
        for (int x = 0; x < 255; x++) {
            for (int z = 0; z < 255; z++) {
                world(y + 765, x + 510, z, y, x);
            }
        }
    }

    world(40, 40, 0, 0, 255);    	//color (Azul)
    world(40, 45, 0, 255, 0);		//color (Verde)
    world(40, 50, 255, 0, 0);		//color (Rojo)
    world(45, 40, 255, 255, 0);		//color (Amarillo)
    world(45, 45, 255, 0, 255);     //color (Rosa)
    world(45, 50, 0, 255, 255);   	//color (Cian)
    world(50, 40, 255, 255, 255);	//color (Blanco)
    world(50, 50, 0, 0, 0); 		//color (Negro)
}

void drawWorld() {

    // Dibujar el mundo alrededor del jugador
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int worldX = player.x - (SCREEN_WIDTH / 2) + x;
            int worldY = player.y - (SCREEN_HEIGHT / 2) + y;

            if (worldX >= 0 && worldX < WORLD_WIDTH && worldY >= 0 && worldY < WORLD_HEIGHT) {
                drawPixel(x, y, World[worldX][worldY].r, World[worldX][worldY].g, World[worldX][worldY].b);
            } else {
                // Evitar rayas de colores diagonales asignando un color de fondo uniforme
                drawPixel(x, y, 255, 255, 255); // Fondo blanco
            }
        }
    }

    // Dibujar al jugador en el centro de la pantalla
    drawPixel(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, 0, 0); // Jugador (color negro)

}




int main() {
    initWindow();
    loadWorld();
    player.x = 50;
    player.y = 50;

    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WindowProc);

    MSG msg = {0};
    while (!quit) {
        clearScreen();
		DetectKeys();
        drawWorld();
        GameInfo();
        render();

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                quit = 1;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Simulación de la lógica del juego
        Sleep(16);
    }

    destroyWindow();
    return 0;
}

