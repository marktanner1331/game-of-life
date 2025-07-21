#include <windows.h>
#include "conway.h"

conway_GameOfLife *conway_initialize(unsigned char width, unsigned char height)
{
    size_t totalSize = sizeof(conway_GameOfLife) + 2 * width * height * sizeof(unsigned char);
    conway_GameOfLife *gameOfLife = (conway_GameOfLife *)malloc(totalSize);

    gameOfLife->width = width;
    gameOfLife->height = height;

    gameOfLife->currentState = (unsigned char *)(gameOfLife + 1);
    gameOfLife->previousState = gameOfLife->currentState + width * height;

    memset(gameOfLife->currentState, 0, 2 * width * height);

    return gameOfLife;
}

void conway_free(conway_GameOfLife *game)
{
    free(game);
}

void conway_tick(conway_GameOfLife *game)
{
    // Swap current and previous state buffers
    unsigned char *temp = game->currentState;
    game->currentState = game->previousState;
    game->previousState = temp;

// Helper macro to access cell state
#define CELL(x, y) game->previousState[(y) * game->width + (x)]
#define SET_CELL(x, y, val) game->currentState[(y) * game->width + (x)] = (val)

    for (unsigned char y = 0; y < game->height; y++)
    {
        for (unsigned char x = 0; x < game->width; x++)
        {
            int liveNeighbors = 0;

            // Check all 8 neighbors
            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dy == 0)
                        continue; // Skip self

                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < game->width && ny >= 0 && ny < game->height)
                    {
                        liveNeighbors += CELL(nx, ny);
                    }
                }
            }

            // Apply Game of Life rules
            if (CELL(x, y))
            { // Cell is alive
                SET_CELL(x, y, (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0);
            }
            else
            { // Cell is dead
                SET_CELL(x, y, (liveNeighbors == 3) ? 1 : 0);
            }
        }
    }

#undef CELL
#undef SET_CELL
}

void conway_render(conway_RenderContext *ctx, HDC hdc, conway_GameOfLife *game)
{
    if (!ctx, !game || !game->currentState)
        return;

    float cellWidth = (float)ctx->bufferWidth / game->width;
    float cellHeight = (float)ctx->bufferHeight / game->height;

    // Create brushes
    HBRUSH brushAlive = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH brushDead = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH brushBG = CreateSolidBrush(RGB(255, 255, 255)); // background

    // Clear buffer
    RECT fullRect = {0, 0, (LONG)ctx->bufferWidth, (LONG)ctx->bufferHeight};
    FillRect(ctx->backDC, &fullRect, brushBG);

    // Draw cells
    for (unsigned char y = 0; y < game->height; y++)
    {
        for (unsigned char x = 0; x < game->width; x++)
        {
            unsigned char alive = game->currentState[y * game->width + x];

            RECT cellRect;
            cellRect.left = (LONG)(x * cellWidth);
            cellRect.top = (LONG)(y * cellHeight);
            cellRect.right = (LONG)((x + 1) * cellWidth);
            cellRect.bottom = (LONG)((y + 1) * cellHeight);

            FillRect(ctx->backDC, &cellRect, alive ? brushAlive : brushDead);
        }
    }

    // Draw grid lines
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN oldPen = (HPEN)SelectObject(ctx->backDC, pen);

    for (unsigned char x = 1; x < game->width; x++)
    {
        int xpos = (int)(x * cellWidth);
        MoveToEx(ctx->backDC, xpos, 0, NULL);
        LineTo(ctx->backDC, xpos, ctx->bufferHeight);
    }

    for (unsigned char y = 1; y < game->height; y++)
    {
        int ypos = (int)(y * cellHeight);
        MoveToEx(ctx->backDC, 0, ypos, NULL);
        LineTo(ctx->backDC, ctx->bufferWidth, ypos);
    }

    // Blit buffer to screen
    BitBlt(hdc, 0, 0, ctx->bufferWidth, ctx->bufferHeight, ctx->backDC, 0, 0, SRCCOPY);

    // Clean up
    SelectObject(ctx->backDC, oldPen);
    DeleteObject(pen);

    DeleteObject(brushAlive);
    DeleteObject(brushDead);
    DeleteObject(brushBG);
}

/// @brief initializes a 'glider gun' into the currentState buffer
/// @return 0 on success
int conway_initGliderGun(conway_GameOfLife *game)
{
    if (game->width < 38 || game->height < 11)
        return 1;

    memset(game->currentState, 0, game->width * game->height);

    const char *gun[11] = {
        "........................X...........",
        "......................X.X...........",
        "............XX......XX............XX",
        "...........X...X....XX............XX",
        "XX........X.....X...XX..............",
        "XX........X...X.XX....X.X...........",
        "..........X.....X.......X...........",
        "...........X...X....................",
        "............XX......................",
        "....................................",
        "...................................."};

    int gunHeight = 11;
    int gunWidth = strlen(gun[0]);

    // Center the gun in the grid
    int offsetX = (game->width - gunWidth) / 2;
    int offsetY = (game->height - gunHeight) / 2;

    for (int y = 0; y < gunHeight; y++)
    {
        for (int x = 0; gun[y][x] != '\0'; x++)
        {
            if (gun[y][x] == 'X')
            {
                int gx = x + offsetX;
                int gy = y + offsetY;
                if (gx < game->width && gy < game->height)
                {
                    game->currentState[gy * game->width + gx] = 1;
                }
            }
        }
    }

    return 0;
}
