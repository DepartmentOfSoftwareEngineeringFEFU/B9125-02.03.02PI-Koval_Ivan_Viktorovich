#define NOMINMAX

#include "Physics.hpp"
#include <MMSystem.h>

#define PI 3.14159
#define M_PI 3.1415926535f
#define SQRT2 1.41421356237

typedef void (*BYTEBEAT)(int, int, PSHORT);

/*void playByteBeat(int hz, int sampleCount, BYTEBEAT beat) {
    HANDLE heap = GetProcessHeap();
    PSHORT samples = (PSHORT)malloc(sampleCount * 2);
    beat(hz, sampleCount, samples);
    WAVEFORMATEX wave = { WAVE_FORMAT_PCM, 2, hz, hz * 2, 2, 16, 0 };
    WAVEHDR hdr = { (LPSTR)samples, sampleCount * 2, 0, 0, 0, 0, 0, 0 };
    HWAVEOUT out;
    waveOutOpen(&out, WAVE_MAPPER, &wave, 0, 0, 0);
    waveOutPrepareHeader(out, &hdr, sizeof(hdr));
    waveOutWrite(out, &hdr, sizeof(hdr));
    Sleep(sampleCount * 1000 / hz);
}

void ByteBeat1(int hz, int sampleCount, PSHORT samples) {
    for (int t = 0; t < sampleCount; t++) {
        samples[t] = abs(9 * t ^ (t << ((t / 2000) % 8)) & t * t >> 8);
    }
}

void play1() {
    playByteBeat(8000, 20 * 8000, ByteBeat1);
}*/

typedef union _RGBQUAD {
    COLORREF rgb;
    struct {
        BYTE r;
        BYTE g;
        BYTE b;
        BYTE Reserved;
    };
} _RGBQUAD, * PRGBQUAD;

struct Vec3 {
    float x, y, z;
};

_RGBQUAD rainbow(float t) {
    float r = 0, g = 0, b = 0;
    if (t < 0.1667f) {
        r = 1;
        g = t / 0.1667f;
    }
    else if (t < 0.3333f) {
        r = 1 - (t - 0.1667f) / 0.1667f;
        g = 1;
    }
    else if (t < 0.5f) {
        g = 1;
        b = (t - 0.3333f) / 0.1667f;
    }
    else if (t < 0.6667f) {
        g = 1 - (t - 0.5f) / 0.1667f;
        b = 1;
    }
    else if (t < 0.8333f) {
        b = 1;
        r = (t - 0.6667f) / 0.1667f;
    }
    else {
        b = 1 - (t - 0.8333f) / 0.1667f;
        r = 1;
    }
    _RGBQUAD color;
    color.r = static_cast<BYTE>(r * 255);
    color.g = static_cast<BYTE>(g * 255);
    color.b = static_cast<BYTE>(b * 255);
    color.Reserved = 0;
    return color;
}

void define(int m, int k, std::vector<Vec3>& points, std::vector<_RGBQUAD>& colors) {
    points.clear();
    colors.clear();
    for (int i = 0; i <= m; i++) {
        float theta = i * M_PI / m;
        for (int j = 0; j < k; j++) {
            float phi = j * 2 * M_PI / k;
            Vec3 p;
            p.x = sin(theta) * cos(phi);
            p.y = sin(theta) * sin(phi);
            p.z = cos(theta);
            points.push_back(p);
            float t = (p.z + 1) / 2;
            colors.push_back(rainbow(t));
        }
    }
}

int Sphere()
{
    for (;;) {
        HDC hdcScreen = GetDC(0);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        INT w = GetSystemMetrics(SM_CXSCREEN);
        INT h = GetSystemMetrics(SM_CYSCREEN);

        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = h;

        PRGBQUAD rgbScreen;
        HBITMAP hbmTemp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&rgbScreen, NULL, NULL);
        SelectObject(hdcMem, hbmTemp);

        PRGBQUAD originalScreen;
        HBITMAP hbmOriginal = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&originalScreen, NULL, NULL);
        HDC hdcMemOriginal = CreateCompatibleDC(hdcScreen);
        SelectObject(hdcMemOriginal, hbmOriginal);

        int quality = 30;

        float radius = 1.0f;
        float spread = 200.0f;
        float velocity = 3.0f;

        Vec3 sphereVel = { velocity, velocity, 0.0f };
        Vec3 spherePos = { 0, 0, 0 };

        float angleY = 0.0f;
        float angleX = 0.0f;

        float angularSpeedX = 3.5f;
        float angularSpeedY = 7.0f;

        float tickSpeed = 0.05f;
        float pointRadius = 5.0f;

        std::vector<Vec3> unitSpherePoints;
        std::vector<_RGBQUAD> pointColors;
        define(quality, quality, unitSpherePoints, pointColors);

        unsigned long long frames = 0;

        angleY += angularSpeedY * tickSpeed;
        angleX += angularSpeedX * tickSpeed;
        spherePos.x += sphereVel.x * tickSpeed;
        spherePos.y += sphereVel.y * tickSpeed;
        spherePos.z += sphereVel.z * tickSpeed;

        float cx_pixel = spread * spherePos.x + w / 2.0f;
        float cy_pixel = h / 2.0f - spread * spherePos.y;
        float radius_pixel = spread * radius;

        if (cx_pixel - radius_pixel < 0) sphereVel.x = fabs(sphereVel.x);
        if (cx_pixel + radius_pixel > w) sphereVel.x = -fabs(sphereVel.x);
        if (cy_pixel - radius_pixel < 0) sphereVel.y = -fabs(sphereVel.y);
        if (cy_pixel + radius_pixel > h) sphereVel.y = fabs(sphereVel.y);

        BitBlt(hdcMemOriginal, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);

        BitBlt(hdcMem, 0, 0, w, h, hdcMemOriginal, 0, 0, SRCCOPY);

        for (size_t i = 0; i < unitSpherePoints.size(); i++) {
            Vec3 unitPoint = unitSpherePoints[i];

            float cosA = cos(angleY);
            float sinA = sin(angleY);
            float rotatedX = unitPoint.x * cosA + unitPoint.z * sinA;
            float rotatedZ = -unitPoint.x * sinA + unitPoint.z * cosA;
            float rotatedY = unitPoint.y;

            float cosAX = cos(angleX);
            float sinAX = sin(angleX);
            float finalX = rotatedX;
            float finalY = rotatedY * cosAX - rotatedZ * sinAX;
            float finalZ = rotatedY * sinAX + rotatedZ * cosAX;

            Vec3 p = { spherePos.x + radius * finalX, spherePos.y + radius * finalY, spherePos.z + radius * finalZ };

            float x_screen = spread * p.x;
            float y_screen = spread * p.y;
            int ix = static_cast<int>(x_screen + w / 2.0f);
            int iy = static_cast<int>(h / 2.0f - y_screen);

            int radius_int = static_cast<int>(pointRadius + 0.5f);
            for (int dy = -radius_int; dy <= radius_int; dy++) {
                for (int dx = -radius_int; dx <= radius_int; dx++) {
                    int px = ix + dx;
                    int py = iy + dy;
                    float dist = sqrt(dx * dx + dy * dy);
                    if (dist <= pointRadius && px >= 0 && px < w && py >= 0 && py < h) {
                        rgbScreen[py * w + px] = pointColors[i];
                    }
                }
            }
        }

        BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

        if (frames % 5 == 0) {
            Sleep(2);
            BitBlt(hdcScreen, 0, 0, w, h, hdcMemOriginal, 0, 0, SRCCOPY);
        }
        DeleteDC(hdcMem);
        DeleteDC(hdcMemOriginal);
        DeleteObject(hbmTemp);
        DeleteObject(hbmOriginal);
        ReleaseDC(NULL, hdcScreen);
    }

    
    return 0;
}

void Fractal1() {
    int angle = 5;
    int range = 500;
    float sine = sin(angle);
    float cosine = cos(angle);
    int threshold = 30;
    std::vector<int> randomInRange = std::vector<int>();
    for (;;) {
        // Get screen device context and dimensions
        HDC hdcScreen = GetDC(0);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);

        INT w = GetSystemMetrics(SM_CXSCREEN);
        INT h = GetSystemMetrics(SM_CYSCREEN);

        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = h;

        DWORD startTime = GetTickCount();

        const float Frequency = 5.0f;
        const float Amplitude = 0.05f;
        const float Trippy = 0.05f;
        const float Speed = 1.0f;

        float time = (GetTickCount() - startTime) / 1000.0f * Speed;
        int iterations = 0;

        PRGBQUAD rgbScreen;
        HBITMAP hbmTemp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&rgbScreen, NULL, NULL);
        SelectObject(hdcMem, hbmTemp);

        PRGBQUAD originalScreen;
        HBITMAP hbmOriginal = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&originalScreen, NULL, NULL);
        HDC hdcMemOriginal = CreateCompatibleDC(hdcScreen);
        SelectObject(hdcMemOriginal, hbmOriginal);
        BitBlt(hdcMemOriginal, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
        iterations++;
        for (INT i = 0; i < w * h; i++) {
            /*rgbScreen[i] = originalScreen[i];
            int randPos = (rand() % (range * 2)) * (rand() % (range * 2));
            int x1 = randPos % w;
            int y1 = randPos / w;
            int x2 = x1 - range / 2;
            int y2 = y1 - range / 2;
            x2 = x2 * sine - y2 * cosine;
            y2 = x2 * cosine + y2 * sine;
            int newPixelX = (x2 * (1 - x2 / range));
            int newPixelY = (y2 * (1 - y2 / range));
            int newPixel = newPixelX * newPixelY + (dw * dh / 2);
            rgbScreen[newPixel] = originalScreen[randPos];*/
            int zoom = 1;
            int x = i % w;
            int y = i / w;
            _RGBQUAD pixel = originalScreen[i];
            int xorValue = ((int)(((x + 1) & 2 * x) | ((x + 1) ^ y))) % 256;
            pixel.r = static_cast<BYTE>(pixel.r + xorValue*zoom) % 256;
            pixel.g = static_cast<BYTE>(pixel.g - xorValue*zoom) % 256;
            pixel.b = static_cast<BYTE>(pixel.b + xorValue*zoom) % 256;

            rgbScreen[i] = pixel;
        }

        /*INT x = i % w;
        INT y = i / w;

        float uv_x = static_cast<float>(x) / w;
        float uv_y = static_cast<float>(y) / h;

        float distortion_x = sin(uv_y * Frequency + time) * Amplitude;
        float distortion_y = sin(uv_x * Frequency + time) * Amplitude;

        distortion_x += sin((uv_x + uv_y) * Frequency + time) * Amplitude * 0.5f;
        distortion_y += sin((uv_x - uv_y) * Frequency + time) * Amplitude * 0.5f;

        distortion_x += sin(time) * Trippy;
        distortion_y += cos(time) * Trippy;

        float distortedUV_x = uv_x + distortion_x;
        float distortedUV_y = uv_y + distortion_y;

        float source_x = distortedUV_x * w;
        float source_y = distortedUV_y * h;

        int src_x = static_cast<int>(source_x + 0.5f) % w;
        int src_y = static_cast<int>(source_y + 0.5f) % h;
        if (src_x < 0) src_x += w;
        if (src_y < 0) src_y += h;*/
        /*int color = originalScreen[i].b | originalScreen[i].g << 8 | originalScreen[i].r << 16;
        rgbScreen[i].r = originalScreen[i].r | (byte)((color >> 16) * 1.5*sin(0.125*iterations * PI/180));
        rgbScreen[i].g = originalScreen[i].g | (byte)((color >> 8) * 1.5 * sin(0.125*iterations * PI / 180));
        rgbScreen[i].b = originalScreen[i].b | (byte)((color) * 1.5 * sin(0.125*iterations * PI / 180));*/
        BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
        BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY | CAPTUREBLT);
        DeleteDC(hdcMem);
        DeleteDC(hdcMemOriginal);
        DeleteObject(hbmTemp);
        DeleteObject(hbmOriginal);
    }
}

void Parabola() {
    int iterations = 0;
    for (;;) {
        // Get screen device context and dimensions
        HDC hdcScreen = GetDC(0);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);

        INT w = GetSystemMetrics(SM_CXSCREEN);
        INT h = GetSystemMetrics(SM_CYSCREEN);

        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = h;

        DWORD startTime = GetTickCount();

        PRGBQUAD rgbScreen;
        HBITMAP hbmTemp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&rgbScreen, NULL, NULL);
        SelectObject(hdcMem, hbmTemp);

        PRGBQUAD originalScreen;
        HBITMAP hbmOriginal = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void**)&originalScreen, NULL, NULL);
        HDC hdcMemOriginal = CreateCompatibleDC(hdcScreen);
        SelectObject(hdcMemOriginal, hbmOriginal);
        BitBlt(hdcMemOriginal, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);
        iterations++;
        for (INT i = 0; i < w * h; i++) {
            int x = i % w;
            int y = i / w;
            _RGBQUAD target = originalScreen[(int)((int)abs(x % (w / 8) - w / 4) * (int)abs(y % (h / 8) - h / 4))];
            if (iterations % 2 == 0) {
                rgbScreen[i].r = originalScreen[i].r ^ target.r/2;
                rgbScreen[i].g = originalScreen[i].g ^ target.g/2;
                rgbScreen[i].b = originalScreen[i].b ^ target.b/2;
            }
            if (iterations % 2 == 1) {
                rgbScreen[i].r = originalScreen[i].r | target.r/2;
                rgbScreen[i].g = originalScreen[i].g | target.g/2;
                rgbScreen[i].b = originalScreen[i].b | target.b/2;
            }
        }
        BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
        BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY | CAPTUREBLT);
        DeleteDC(hdcMem);
        DeleteDC(hdcMemOriginal);
        DeleteObject(hbmTemp);
        DeleteObject(hbmOriginal);
    }
}

void Fold() {
    while (true) {
        POINT fold1[] = { {100, 100}, {dw, 0}, {0, dh} };
        POINT fold2[] = { {0, 0}, {dw-100, 100}, {100, dh-100} };
        PlgBlt(desktop, fold1, desktop, 0, 0, dw, dh, 0, 0, 0);
        PlgBlt(desktop, fold2, desktop, 0, 0, dw, dh, 0, 0, 0);
        Sleep(20);
    }
}

void Unfold() {
    while (true) {  
        POINT leftTop = { 0, 0 };
        POINT halfTop = { dw / 2, 0 };
        Sleep(20);
    }
}

void Consume() {
    int q1 = dw / 2 * 0.8;
    int q2 = dh / 2 * 0.8;
    int q3 = -dw / 2 * 0.8;
    int q4 = -dh / 2 * 0.8;
    POINT one = { q3 * sin(PI / 45) - q2 + cos(PI / 45) + dw / 2, q3 * cos(PI / 45) + q2 * sin(PI / 45) + dh / 2};
    POINT two = { q1 * sin(PI / 45) - q2 + cos(PI / 45) + dw / 2, q1 * cos(PI / 45) + q2 * sin(PI / 45) + dh / 2 };
    POINT three = { q3 * sin(PI / 45) - q4 + cos(PI / 45) + dw / 2, q3 * cos(PI / 45) + q4 * sin(PI / 45) + dh / 2 };
    POINT pts[] = { one, two, three };
    while (true) {
        PlgBlt(desktop, pts, desktop, 0, 0, dw, dh, 0, 0, 0);
        Sleep(100);
    }
}

int main()
{
    desktop = GetDC(0);
    for (int i = 0; i < 10; i++) {
        buffers[i] = CreateCompatibleDC(desktop);
        HBITMAP deskBmp = CreateCompatibleBitmap(desktop, dw, dh);
        SelectObject(buffers[i], deskBmp);
        BitBlt(buffers[i], 0, 0, dw, dh, desktop, 0, 0, SRCCOPY | CAPTUREBLT);
    }
    //HANDLE fold = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Fold, 0, 0, NULL);
    for (int i = 0; i < 10; i++) {
        Point actualCenter = { dw / 2 + rand() % 64 - 32, dh / 2 + rand() % 64 - 2};
        POINT bufferCenter = { dw / 2, dh / 2 };
        Rect newRect = Rect(actualCenter, buffers[0], buffers[1], bufferCenter, 50, 0.0f, 10, 5);
        objedki.push_back(&newRect);
    }
    HANDLE objectUpdater = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateBouncingObjects, 0, 0, NULL);
    HANDLE shaderrr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Fractal1, 0, 0, NULL);
    //HANDLE parabola = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Parabola, 0, 0, NULL);
    HANDLE consume = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Fold, 0, 0, NULL);
    Sleep(10000);
    TerminateThread(objectUpdater, 0);
    TerminateThread(shaderrr, 0);
    TerminateThread(consume, 0);
    //TerminateThread(parabola, 0);
    InvalidateRect(0, 0, 0);
    Sleep(1000);
    //parabola = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Parabola, 0, 0, NULL);
    //shaderrr = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Fractal1, 0, 0, NULL);
    Sleep(10000);
    Sleep(-1);
}

