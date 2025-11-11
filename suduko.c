// sudoku.c
// Simple console Sudoku game in C
// Compile: gcc sudoku.c -o sudoku
// Run: ./sudoku

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>   // <-- ADD THIS LINE


#define N 9
#define EMPTY 0

// Prototypes
void print_board(int board[N][N], int fixed[N][N]);
int is_valid(int board[N][N], int row, int col, int num);
int solve_board(int board[N][N]);
int find_empty(int board[N][N], int *row, int *col);
void copy_board(int src[N][N], int dst[N][N]);
void generate_full_board(int board[N][N]);
int fill_board_backtrack(int board[N][N]);
int shuffle_arr(int *arr, int n);
void remove_cells(int board[N][N], int puzzle[N][N], int removed);
void prompt_instructions();
void clear_input_buffer();
void make_move(int puzzle[N][N], int fixed[N][N]);
void give_hint(int puzzle[N][N], int solution[N][N], int fixed[N][N]);
int board_complete(int puzzle[N][N]);
void show_menu();

static int global_rand_seeded = 0;
int randint(int a, int b) {
    if (!global_rand_seeded) {
        srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&randint);
        global_rand_seeded = 1;
    }
    return a + rand() % (b - a + 1);
}

void print_board(int board[N][N], int fixed[N][N]) {
    printf("\n    1 2 3   4 5 6   7 8 9\n");
    printf("  +-------+-------+-------+\n");
    for (int r = 0; r < N; ++r) {
        printf("%d | ", r+1);
        for (int c = 0; c < N; ++c) {
            if (board[r][c] == EMPTY)
                printf(". ");
            else {
                if (fixed[r][c])
                    printf("%d ", board[r][c]); // fixed (original) numbers
                else
                    printf("%d ", board[r][c]); // user-entered numbers
            }
            if ((c+1) % 3 == 0) printf("| ");
        }
        printf("\n");
        if ((r+1) % 3 == 0) printf("  +-------+-------+-------+\n");
    }
}

int is_valid(int board[N][N], int row, int col, int num) {
    // check row/col
    for (int i = 0; i < N; ++i) {
        if (board[row][i] == num) return 0;
        if (board[i][col] == num) return 0;
    }
    // check 3x3 box
    int startRow = row - row % 3, startCol = col - col % 3;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (board[startRow + r][startCol + c] == num)
                return 0;
    return 1;
}

int find_empty(int board[N][N], int *row, int *col) {
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            if (board[r][c] == EMPTY) {
                *row = r; *col = c;
                return 1;
            }
    return 0;
}

int solve_board(int board[N][N]) {
    int row, col;
    if (!find_empty(board, &row, &col)) return 1; // solved

    int nums[N];
    for (int i = 0; i < N; ++i) nums[i] = i+1;
    shuffle_arr(nums, N);
    for (int i = 0; i < N; ++i) {
        int num = nums[i];
        if (is_valid(board, row, col, num)) {
            board[row][col] = num;
            if (solve_board(board)) return 1;
            board[row][col] = EMPTY;
        }
    }
    return 0;
}

int shuffle_arr(int *arr, int n) {
    for (int i = n-1; i > 0; --i) {
        int j = randint(0, i);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
    return 1;
}

void copy_board(int src[N][N], int dst[N][N]) {
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            dst[r][c] = src[r][c];
}

int fill_board_backtrack(int board[N][N]) {
    // backtracking generator with randomized order
    int row, col;
    if (!find_empty(board, &row, &col)) return 1;
    int nums[N]; for (int i = 0; i < N; ++i) nums[i]=i+1;
    shuffle_arr(nums, N);
    for (int i = 0; i < N; ++i) {
        int n = nums[i];
        if (is_valid(board, row, col, n)) {
            board[row][col] = n;
            if (fill_board_backtrack(board)) return 1;
            board[row][col] = EMPTY;
        }
    }
    return 0;
}

void generate_full_board(int board[N][N]) {
    // start with empty board
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            board[r][c] = EMPTY;
    fill_board_backtrack(board);
}

void remove_cells(int board[N][N], int puzzle[N][N], int removed) {
    // Copy full board into puzzle
    copy_board(board, puzzle);

    // Create array of cell indices and shuffle
    int cells[N*N];
    for (int i = 0; i < N*N; ++i) cells[i] = i;
    shuffle_arr(cells, N*N);

    int removedCount = 0;
    for (int i = 0; i < N*N && removedCount < removed; ++i) {
        int idx = cells[i];
        int r = idx / N;
        int c = idx % N;
        int backup = puzzle[r][c];
        puzzle[r][c] = EMPTY;

        // To keep complexity reasonable, we won't do a full uniqueness check for each removal.
        // Instead we do a light check: attempt to solve; if solvable, accept removal.
        int tmp[N][N];
        copy_board(puzzle, tmp);
        if (solve_board(tmp)) {
            removedCount++;
        } else {
            // revert if unsolvable (rare)
            puzzle[r][c] = backup;
        }
    }
}

void prompt_instructions() {
    printf("\nCommands:\n");
    printf(" put r c v    -> place value v (1-9) at row r col c (e.g.: put 1 2 9)\n");
    printf(" hint         -> fills one empty cell correctly\n");
    printf(" check        -> checks if current board is valid so far\n");
    printf(" solve        -> reveals the full solution (ends current game)\n");
    printf(" restart      -> generate a new puzzle\n");
    printf(" quit         -> exit the game\n");
}

void clear_input_buffer() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

void make_move(int puzzle[N][N], int fixed[N][N]) {
    char cmd[32];
    if (scanf("%31s", cmd) != 1) { clear_input_buffer(); return; }

    if (strcmp(cmd, "put") == 0) {
        int r,c,v;
        if (scanf("%d %d %d", &r, &c, &v) != 3) {
            printf("Invalid input. Usage: put row col value\n");
            clear_input_buffer();
            return;
        }
        if (r<1||r>9||c<1||c>9||v<1||v>9) {
            printf("Values out of range. Rows/Cols/Values: 1..9\n");
            return;
        }
        r--; c--;
        if (fixed[r][c]) {
            printf("Cell %d,%d is fixed and cannot be changed.\n", r+1, c+1);
            return;
        }
        if (is_valid(puzzle, r, c, v)) {
            puzzle[r][c] = v;
            printf("Placed %d at %d,%d\n", v, r+1, c+1);
        } else {
            printf("Invalid move: %d conflicts with existing numbers.\n", v);
        }
    } else if (strcmp(cmd, "hint") == 0) {
        // handled by main loop via give_hint command
        // push a special marker into stdin? simpler: set a flag outside; instead, we'll call a global function from main.
        // Here we just put back an identifier into input stream: use an extern or simply print instruction.
    } else if (strcmp(cmd, "check") == 0) {
        int ok = 1;
        for (int r=0;r<N && ok;++r)
            for (int c=0;c<N && ok;++c)
                if (puzzle[r][c] != EMPTY) {
                    int val = puzzle[r][c];
                    puzzle[r][c] = EMPTY;
                    if (!is_valid(puzzle, r, c, val)) ok = 0;
                    puzzle[r][c] = val;
                }
        if (ok) printf("So far so good — no rule violations.\n");
        else printf("There is a conflict in the board.\n");
    } else if (strcmp(cmd, "solve") == 0) {
        // signal via a fake command back to main: we'll set puzzle[0][0] == -1 as sentinel (not ideal but simple)
        puzzle[0][0] = -1;
    } else if (strcmp(cmd, "restart") == 0) {
        puzzle[0][0] = -2; // sentinel restart
    } else if (strcmp(cmd, "quit") == 0) {
        puzzle[0][0] = -3; // sentinel quit
    } else {
        printf("Unknown command: %s\n", cmd);
        clear_input_buffer();
    }
}

void give_hint(int puzzle[N][N], int solution[N][N], int fixed[N][N]) {
    // find first empty which is not fixed and fill with solution
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (puzzle[r][c] == EMPTY) {
                if (!fixed[r][c]) {
                    puzzle[r][c] = solution[r][c];
                    printf("Hint: cell %d,%d set to %d\n", r+1, c+1, solution[r][c]);
                    return;
                }
            }
        }
    }
    printf("No empty cells to hint.\n");
}

int board_complete(int puzzle[N][N]) {
    for (int r = 0; r < N; ++r)
        for (int c = 0; c < N; ++c)
            if (puzzle[r][c] == EMPTY) return 0;
    return 1;
}

void show_menu() {
    printf("\n--- Sudoku ---\n");
    printf("Choose difficulty (1=Easy, 2=Medium, 3=Hard): ");
}

int main() {
    int full[N][N], puzzle[N][N], fixed[N][N], solution[N][N];
    char cmdline[64];

    printf("Welcome to Sudoku (console) — C version\n");
    while (1) {
        int diff = 2;
        show_menu();
        if (scanf("%d", &diff) != 1) diff = 2;
        if (diff < 1) diff = 1; if (diff > 3) diff = 3;
        int removed;
        if (diff == 1) removed = 36; // easy: remove ~36
        else if (diff == 2) removed = 46; // medium
        else removed = 56; // hard

        // generate
        generate_full_board(full);
        copy_board(full, solution);
        remove_cells(full, puzzle, removed);

        // mark fixed cells
        for (int r=0;r<N;++r)
            for (int c=0;c<N;++c)
                fixed[r][c] = (puzzle[r][c] != EMPTY);

        prompt_instructions();
        print_board(puzzle, fixed);

        clear_input_buffer();

        // main loop for this puzzle
        while (1) {
            printf("\nEnter command: ");
            // read a full line to parse. We will peek first token ourselves.
            if (!fgets(cmdline, sizeof(cmdline), stdin)) continue;
            // trim newline
            size_t L = strlen(cmdline);
            if (L && cmdline[L-1] == '\n') cmdline[L-1] = '\0';
            if (strlen(cmdline) == 0) continue;

            // parse first token
            char token[32];
            int consumed = 0;
            if (sscanf(cmdline, "%31s%n", token, &consumed) < 1) continue;

            if (strcmp(token, "put") == 0) {
                int r,c,v;
                if (sscanf(cmdline + consumed, "%d %d %d", &r, &c, &v) != 3) {
                    printf("Usage: put row col value   (e.g. put 1 2 9)\n");
                } else {
                    if (r<1||r>9||c<1||c>9||v<1||v>9) {
                        printf("Out of range. Use 1..9 for rows/cols/values.\n");
                    } else {
                        r--; c--;
                        if (fixed[r][c]) {
                            printf("Cell %d,%d is fixed and cannot be changed.\n", r+1, c+1);
                        } else {
                            if (is_valid(puzzle, r, c, v)) {
                                puzzle[r][c] = v;
                                printf("Placed %d at %d,%d\n", v, r+1, c+1);
                            } else {
                                printf("Invalid move: conflicts with existing numbers.\n");
                            }
                        }
                    }
                }
            } else if (strcmp(token, "hint") == 0) {
                give_hint(puzzle, solution, fixed);
            } else if (strcmp(token, "check") == 0) {
                int ok = 1;
                for (int r=0;r<N && ok;++r)
                    for (int c=0;c<N && ok;++c)
                        if (puzzle[r][c] != EMPTY) {
                            int val = puzzle[r][c];
                            puzzle[r][c] = EMPTY;
                            if (!is_valid(puzzle, r, c, val)) ok = 0;
                            puzzle[r][c] = val;
                        }
                if (ok) printf("So far so good — no rule violations.\n");
                else printf("There is a conflict in the board.\n");
            } else if (strcmp(token, "solve") == 0) {
                printf("Solution:\n");
                print_board(solution, fixed);
                break; // end puzzle
            } else if (strcmp(token, "restart") == 0) {
                printf("Restarting with a new puzzle...\n");
                break; // outer loop will restart
            } else if (strcmp(token, "quit") == 0) {
                printf("Goodbye!\n");
                return 0;
            } else if (strcmp(token, "help") == 0) {
                prompt_instructions();
            } else {
                printf("Unknown command. Type 'help' for instructions.\n");
            }

            print_board(puzzle, fixed);

            if (board_complete(puzzle)) {
                // verify correctness
                int temp[N][N];
                copy_board(puzzle, temp);
                if (solve_board(temp)) {
                    printf("\nCongratulations! You completed the puzzle!\n");
                    print_board(puzzle, fixed);
                } else {
                    printf("\nBoard is full but incorrect (conflict).\n");
                }
                break;
            }
        } // end puzzle loop

        // ask if play again
        printf("\nPlay again? (y/n): ");
        char resp[8];
        if (!fgets(resp, sizeof(resp), stdin)) break;
        if (resp[0] != 'y' && resp[0] != 'Y') {
            printf("Thanks for playing. Goodbye!\n");
            break;
        }
    } // outer loop

    return 0;
}