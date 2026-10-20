#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct
{
    char** terms;
    int termCount;
    char* result;
} Puzzle;

void freePuzzle(Puzzle* p) {
    for (int i = 0; i < p->termCount; i++)
        free(p->terms[i]);

    free(p->terms);
    free(p->result);
    free(p);
}

char* readLine(FILE* stream) {
    size_t cap = 16;
    size_t len = 0;

    char* line = malloc(cap);

    if (line == NULL)
        return NULL;

    int c;
    while ((c = fgetc(stream)) != '\n' && c != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char* temp = realloc(line, cap);

            if (temp == NULL) {
                free(line);
                return NULL;
            }

            line = temp;
        }

		line[len++] = (char)c;
    }

    if (c == EOF && len == 0)
    {
        free(line);
        return NULL;
    }

    line[len] = '\0';
    return line;
}

Puzzle* parsePuzzle(const char* puzzle) {
    if (puzzle == NULL)
        return NULL;

    Puzzle* p = malloc(sizeof(Puzzle));

    if (p == NULL)
        return NULL;

    p->terms = NULL;
    p->termCount = 0;
    p->result = NULL;

	for (int i = 0; puzzle[i] != '\0';) {
        int cap = 16;
        int len = 0;
        char* word = malloc(cap);
        if (word == NULL)
            return NULL;
        while (puzzle[i] != '+' && puzzle[i] != '=' && puzzle[i] != '\0' && puzzle[i] != ' ')
        {
            int c = puzzle[i++];
            if (len + 1 >= cap) {
                cap *= 2;
                char* temp = realloc(word, cap);

                if (temp == NULL) {
                    free(word);
                    return NULL;
                }

                word = temp;
            }
            word[len++] = (char)c;
        }
        word[len] = '\0';

        while (puzzle[i] == ' ')
        {
            i++;
        }

        if (puzzle[i] == '+' || puzzle[i] == '=') {
            char** temp = realloc(p->terms, (p->termCount + 1) * sizeof(char*));
            if (temp == NULL)
            {
                free(word);
                return NULL;
            }

            p->terms = temp;
            p->terms[p->termCount] = word;
            p->termCount++;
            i++;
        }
        else if (puzzle[i] == '\0')
            p->result = word;
	}

    return p;
}

static int addLetter(char c, char* letters, int cnt) {
    for (int k = 0; k < cnt; k++)
        if (letters[k] == c)
            return cnt;

    letters[cnt] = c;
    return cnt + 1;
}

int collect_by_columns(const Puzzle* p, char* letters, int cols_need[]) {
    int resLen = (int)strlen(p->result);
    int cnt = 0;

    for (int col = 0; col < resLen; col++) {
        for (int i = 0; i < p->termCount; i++) {
            int len = (int)strlen(p->terms[i]);
            if (col < len)
                cnt = addLetter(p->terms[i][len - 1 - col], letters, cnt);
        }

        cnt = addLetter(p->result[resLen - 1 - col], letters, cnt);

        cols_need[col] = cnt;
    }

    return cnt;
}

long long wordToNumber(const char* word, const int letterToDigit[26]) {
    if (word == NULL)
        return -1;

    long long value = 0;

    for (int i = 0; word[i] != '\0'; i++) {
		int digit = letterToDigit[word[i] - 'A'];

		if (digit == -1)
			return -1;

        value = value * 10 + digit;
    }

    return value;
}

int checkSolution(const Puzzle* p, const int letterToDigit[26]) {
    long long sum = 0;

    for (int i = 0; i < p->termCount; i++) 
        sum += wordToNumber(p->terms[i], letterToDigit);

    long long result = wordToNumber(p->result, letterToDigit);

    return sum == result;
}

long long lastDigits(const char* word, const int letterToDigit[26], int cols) {
    int len = (int)strlen(word);
    int start = len > cols ? len - cols : 0;
    long long v = 0;

    for (int i = start; i < len; i++)
        v = v * 10 + letterToDigit[word[i] - 'A'];

    return v;
}

int columnsOk(const Puzzle* p, const int letterToDigit[26],
    const int colsNeed[], int resLen, int assigned) {
    int cols = 0;

    while (cols < resLen && colsNeed[cols] <= assigned)
        cols++;

    if (cols == 0 || colsNeed[cols - 1] != assigned)
        return 1;

    long long mod = 1;
    for (int i = 0; i < cols; i++)
        mod *= 10;

    long long sum = 0;
    for (int i = 0; i < p->termCount; i++)
        sum += lastDigits(p->terms[i], letterToDigit, cols);

    return sum % mod == lastDigits(p->result, letterToDigit, cols);
}

int solve_recursive(const Puzzle* p, const char letters[], int letterCnt, int position, 
    int letterToDigit[], int digitUsed[], const int isLeading[26], 
    const int colsNeed[], int resLen) {
    if (position == letterCnt)
        return checkSolution(p, letterToDigit);

    int index = letters[position] - 'A';

    int start = isLeading[index] ? 1 : 0;

    for (int digit = start; digit <= 9; digit++) {
        if (digitUsed[digit])
            continue;

        letterToDigit[index] = digit;
        digitUsed[digit] = 1;

        if (columnsOk(p, letterToDigit, colsNeed, resLen, position + 1) &&
            solve_recursive(p, letters, letterCnt, position + 1, letterToDigit, digitUsed,
                isLeading, colsNeed, resLen))
            return 1;

        letterToDigit[index] = -1;
        digitUsed[digit] = 0;
    }

    return 0;
}

void printSolution(const Puzzle* p, const int letterToDigit[26]) {
    for (int i = 0; i < p->termCount; i++) {
        printf("%lld", wordToNumber(p->terms[i], letterToDigit));

        if (i != p->termCount - 1)
            printf(" + ");
    }

    printf(" = ");

    printf("%lld\n", wordToNumber(p->result, letterToDigit));
}

void solvePuzzle(const char* puzzle) {
    Puzzle* p = parsePuzzle(puzzle);

    if (p == NULL) {
        printf("Parse error\n");
        return;
    }

    size_t resLen = strlen(p->result);

    for (int i = 0; i < p->termCount; i++) {
        if (strlen(p->terms[i]) > resLen) {
            printf("%s\n\nNo solution\n\n", puzzle);
            freePuzzle(p);
            return;
        }
    }

    char letters[26];
    int* colsNeed = malloc(resLen * sizeof(int));

    if (colsNeed == NULL) {
        printf("Memory allocation error\n");
        freePuzzle(p);
        return;
    }

    int letterCnt = collect_by_columns(p, letters, colsNeed);

    if (letterCnt > 10) {
        printf("Too many unique letters\n");
        freePuzzle(p);
        return;
    }

    int letterToDigit[26];
    int digitUsed[10] = { 0 };
    int isLeading[26] = { 0 };

    for (int i = 0; i < 26; i++)
        letterToDigit[i] = -1;

    for (int i = 0; i < p->termCount; i++)
        isLeading[p->terms[i][0] - 'A'] = 1;

    isLeading[p->result[0] - 'A'] = 1;

    int solved = solve_recursive(p, letters, letterCnt, 0, letterToDigit, digitUsed, isLeading, colsNeed, resLen);

    if (solved) {
        printf("%s\n\n", puzzle);

        printSolution(p, letterToDigit);

        printf("\n");
    }
    else
        printf("No solution\n");

    free(colsNeed);
    freePuzzle(p);
}


void readPuzzlesFromFile(const char* filename) {
	FILE* file = fopen(filename, "r");

	if (file == NULL) {
		printf("Could not open file %s\n", filename);
		return;
	}

	char* puz;
	while ((puz = readLine(file)) != NULL) {
		solvePuzzle(puz);
		free(puz);
	}

	fclose(file);
}

int main(void) {
    int mode;

    printf("1 - Read from file\n");
    printf("2 - Enter puzzle from keyboard\n");
    printf("Choice: ");

    if (scanf("%d", &mode) != 1) {
        printf("Invalid input\n");
        return 1;
    }

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);

    switch (mode) {
        case 1:
        {
            char* filename;

            printf("Enter filename: ");
            filename = readLine(stdin);

            if (filename == NULL) {
                printf("Memory allocation error\n");
                return 1;
            }

            clock_t start = clock();

            readPuzzlesFromFile(filename);

            clock_t end = clock();

            printf("Total processing time: %.6f seconds\n",
                (double)(end - start) / CLOCKS_PER_SEC);

            free(filename);
            break;
        }

        case 2:
        {
            char* puzzle;

            printf("Enter puzzle: ");
            puzzle = readLine(stdin);

            if (puzzle == NULL) {
                printf("Memory allocation error\n");
                return 1;
            }

            solvePuzzle(puzzle);

            free(puzzle);
            break;
        }

        default:
            printf("Invalid choice\n");
        }

    return 0;
}
