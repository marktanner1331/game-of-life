#include <windows.h> // Required for Windows API functions
#include "conway.h"

// Global window handle (for potential future use, though not strictly needed for this simple example)
HWND g_hwnd = NULL;

conway_GameOfLife *game;

#define defaultWidth 400
#define defaultHeight 300

// Window Procedure: This function handles messages sent to the window.
// Every window created with RegisterClassEx needs a WindowProc.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static conway_RenderContext ctx = {0}; // Persistent context

    // Use a switch statement to handle different types of messages
    switch (uMsg)
    {
    case WM_CLOSE:
        // When the user clicks the close button (X), destroy the window.
        // This sends a WM_DESTROY message.
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        // When the window is destroyed, post a quit message to the message queue.
        // This causes GetMessage to return 0, ending the message loop.
        KillTimer(hwnd, 1);
        if (ctx.backBitmap)
        {
            SelectObject(ctx.backDC, ctx.oldBitmap);
            DeleteObject(ctx.backBitmap);
        }
        if (ctx.backDC)
            DeleteDC(ctx.backDC);

        PostQuitMessage(0);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);
        unsigned int windowWidth = rect.right - rect.left;
        unsigned int windowHeight = rect.bottom - rect.top;

        // Recreate back buffer if needed
        if (ctx.bufferWidth != windowWidth || ctx.bufferHeight != windowHeight)
        {
            if (ctx.backBitmap)
            {
                SelectObject(ctx.backDC, ctx.oldBitmap);
                DeleteObject(ctx.backBitmap);
            }
            if (!ctx.backDC)
                ctx.backDC = CreateCompatibleDC(hdc);
            
            ctx.backBitmap = CreateCompatibleBitmap(hdc, windowWidth, windowHeight);
            ctx.oldBitmap = (HBITMAP)SelectObject(ctx.backDC, ctx.backBitmap);
            ctx.bufferWidth = windowWidth;
            ctx.bufferHeight = windowHeight;
        }

        // Call renderer with the back buffer
        conway_render(&ctx, hdc, game);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_CREATE:
        game = conway_initialize(40, 30);
        conway_initGliderGun(game);

        SetTimer(hwnd, 1, 100, NULL); // 0.1-second interval
        break;
    case WM_TIMER:
        conway_tick(game);
        InvalidateRect(hwnd, NULL, TRUE); // trigger repaint
        break;
    case WM_ERASEBKGND:
        return 1; // Tell Windows we've handled it
    default:
        // For any messages we don't explicitly handle,
        // let the default window procedure handle them.
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0; // Message handled
}

// WinMain: The entry point for a Windows GUI application.
// It's similar to main() but specifically for Windows applications.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 1. Register the Window Class
    // This defines the characteristics of our window (e.g., its procedure, icon, cursor).
    WNDCLASSEX wc = {0}; // Initialize the structure to all zeros

    wc.cbSize = sizeof(WNDCLASSEX);                // Size of the structure
    wc.lpfnWndProc = WindowProc;                   // Pointer to our window procedure
    wc.hInstance = hInstance;                      // Handle to the application instance
    wc.lpszClassName = "MyWindowClass";            // Name of our window class (wide string)
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // Default arrow cursor
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default window background color

    // Register the window class. If it fails, return 0.
    if (!RegisterClassEx(&wc))
    {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // 2. Create the Window
    // This creates an actual window based on the registered class.
    g_hwnd = CreateWindowEx(
        0,                                // Extended window style (0 for default)
        "MyWindowClass",                  // Class name (must match the one registered above)
        "Simple Window",                  // Window title bar text
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Window style (standard overlapping window with title bar, minimize/maximize, close buttons, and initially visible)
        CW_USEDEFAULT, CW_USEDEFAULT,     // Initial X, Y position (let Windows decide)
        defaultWidth, defaultHeight,        // Width, Height of the window
        NULL,                             // Parent window handle (NULL for no parent)
        NULL,                             // Menu handle (NULL for no menu)
        hInstance,                        // Instance handle of the application
        NULL                              // Pointer to creation data (NULL for none)
    );

    // Check if window creation failed.
    if (g_hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // 3. Message Loop
    // This is the heart of a Windows GUI application. It retrieves and dispatches messages.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {                           // Loop until GetMessage returns 0 (WM_QUIT)
        TranslateMessage(&msg); // Translates virtual-key messages into character messages
        DispatchMessage(&msg);  // Dispatches the message to the window procedure
    }

    // When the message loop ends (WM_QUIT received), return the wParam of the WM_QUIT message.
    return (int)msg.wParam;
}
