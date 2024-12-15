#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ClearScreen.h"

typedef struct _Point
{
    int x;
    int y;
} Point;

typedef enum _CellDisplay
{
    VISIBLE,
    FLAGGED,
    HIDDEN,
} CellDisplay;

typedef struct _Cell
{
    CellDisplay display;
    bool isBomb;
    char adjacentBombCount;
} Cell;

typedef struct _GameState
{
    int width;
    int height;
    int bombCount;
    Cell** board;
} GameState;

typedef struct _IndexBounds
{
    int xBounds[2];
    int yBounds[2];
} IndexBounds;

IndexBounds getIndexBounds(GameState* gameState, Point cellPos)
{
    int xStart = max(0, cellPos.x - 1);
    int xEnd = min(gameState->width - 1, cellPos.x + 1);
    int yStart = max(0, cellPos.y - 1);
    int yEnd = min(gameState->height - 1, cellPos.y + 1);
    IndexBounds indexBounds = { { xStart, xEnd }, { yStart, yEnd } };
    return indexBounds;
}

#define ITERATE_ADJACENT_DELEGATE(name) \
    void name(GameState* board, Cell* centerCell, Cell* adjacentCell, Point cellPos)

void iterateAdjacent(GameState* board, Point cellPos, ITERATE_ADJACENT_DELEGATE(*delegate))
{
    Cell* centerCell = board->board[cellPos.x] + cellPos.y;
    IndexBounds indexBounds = getIndexBounds(board, cellPos);
    int xStart = indexBounds.xBounds[0];
    int xEnd = indexBounds.xBounds[1];
    int yStart = indexBounds.yBounds[0];
    int yEnd = indexBounds.yBounds[1];
    for (int x = xStart; x <= xEnd; x++)
    {
        for (int y = yStart; y <= yEnd; y++)
        {
            Cell* adjacentCell = board->board[x] + y;
            Point cellPos = { x, y };
            (*delegate)(board, centerCell, adjacentCell, cellPos);
        }
    }
}

#define ITERATE_BOARD_DELEGATE(name) \
    void name(GameState* board, Cell* currCell, Point cellPos)

void iterateBoard(GameState* board, ITERATE_BOARD_DELEGATE(*delegate))
{
    for (int y = 0; y < board->height; y++)
    {
        for (int x = 0; x < board->width; x++)
        {
            Cell* currCell = board->board[x] + y;
            Point currPos = { x, y };
            (*delegate)(board, currCell, currPos);
        }
    }
}

ITERATE_ADJACENT_DELEGATE(setAdjacentBombCountDelegate)
{
    if (adjacentCell->isBomb && adjacentCell != centerCell)
    {
        centerCell->adjacentBombCount++;
    }
}

ITERATE_BOARD_DELEGATE(iterateBoardAdjacentBombCountDelegate)
{
    iterateAdjacent(board, cellPos, &setAdjacentBombCountDelegate);
}

void setBoardAdjacentBombCounts(GameState* board)
{
    iterateBoard(board, &iterateBoardAdjacentBombCountDelegate);
}


void placeBombs(GameState* board)
{
    for (int i = 0; i < board->bombCount; i++)
    {
        int xPos;
        int yPos;
        do
        {
            xPos = rand() % board->width;
            yPos = rand() % board->height;
        } while (board->board[xPos][yPos].isBomb == true);
        board->board[xPos][yPos].isBomb = true;
    }
}

void revealMultipleCells(GameState* gameState, Point cellPos)
{
    ITERATE_ADJACENT_DELEGATE(revealMultipleCellsDelegate);
    if (gameState->board[cellPos.x][cellPos.y].display == VISIBLE)
    {
        return;
    }
    iterateAdjacent(gameState, cellPos, &revealMultipleCellsDelegate);
}

ITERATE_ADJACENT_DELEGATE(revealMultipleCellsDelegate)
{
    if (adjacentCell->adjacentBombCount > 0)
    {
        adjacentCell->display = VISIBLE;
        return;
    }
    revealMultipleCells(board, cellPos);
}



ITERATE_BOARD_DELEGATE(displayDelegate) {
    switch (currCell->display)
    {
    case VISIBLE:
        if (currCell->adjacentBombCount == 0)
        {
            printf("%c", 'v');
        }
        else
        {
            printf("%i", currCell->adjacentBombCount);
        }
        break;
    case HIDDEN:
        printf("%c", 'h');
        break;
    case FLAGGED:
        printf("%c", 'f');
        break;
    }
    if (cellPos.x == board->width - 1)
    {
        printf("\n");
    }
}

void display(GameState* board)
{
    iterateBoard(board, &displayDelegate);
}

GameState newBoard(int width, int height, int bombCount)
{
    GameState board;
    board.width = width;
    board.height = height;
    board.bombCount = bombCount;
    board.board = malloc(sizeof(Cell*) * width);

    for (int x = 0; x < width; x++)
    {
        board.board[x] = calloc(height, sizeof(Cell));
        for (int y = 0; y < height; y++)
        {
            board.board[x][y].display = HIDDEN;
        }
    }
    placeBombs(&board);
    setBoardAdjacentBombCounts(&board);
    return board;
}

int main()
{
    GameState board = newBoard(25, 25, 10);
    Point p = { 5, 5 };
    revealMultipleCells(&board, p);
    display(&board);
}
