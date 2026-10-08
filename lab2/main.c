#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int collectUni(const Puzzle* p, char* letters) {
    if (p == NULL || letters == NULL)
        return 0;

    int cnt = 0;

    for (int i = 0; i < p->termCount; i++) {
        const char* word = p->terms[i];

        for (int j = 0; word[j] != '\0'; j++) {
            char c = word[j];
            int was = 0;

            for (int k = 0; k < cnt; k++) {
                if (letters[k] == c) {
                    was = 1;
                    break;
                }
            }

            if (!was) {
                letters[cnt++] = c;
            }
        }
    }

    if (p->result != NULL) {
        const char* word = p->result;

        for (int j = 0; word[j] != '\0'; j++) {
            char c = word[j];
            int was = 0;

            for (int k = 0; k < cnt; k++)
            {
                if (letters[k] == c)
                {
                    was = 1;
                    break;
                }
            }

            if (!was)
            {
                letters[cnt++] = c;
            }
        }
    }

    return cnt;
}

int find_letter_index(char letter, const char letters[], int count) {
    for (int i = 0; i < count; i++) {
        if (letters[i] == letter)
            return i;
    }

    return -1;
}

long long wordToNumber(const char* word, const char letters[], 
    const int assignedDigits[], int letterCnt) {
    long long value = 0;

    for (int i = 0; word[i] != '\0'; i++) {
        int index = find_letter_index(word[i], letters, letterCnt);

        if (index == -1)
            return -1;

        if (assignedDigits[index] == -1)
            return -1;

        value = value * 10 + assignedDigits[index];
    }

    return value;
}

int has_zero_start(const Puzzle* p, const char letters[],
    const int assignedDigits[], int letterCnt) {
    for (int i = 0; i < p->termCount; i++) {
        int index = find_letter_index(p->terms[i][0], letters, letterCnt);

        if (assignedDigits[index] == 0)
            return 1;
    }

    int index = find_letter_index(p->result[0], letters, letterCnt);

    if (assignedDigits[index] == 0)
        return 1;

    return 0;
}

int checkSolution(const Puzzle* p, const char letters[], 
    const int assignedDigits[], int letterCnt) {
    if (has_zero_start(p, letters, assignedDigits, letterCnt))
        return 0;

    long long sum = 0;

    for (int i = 0; i < p->termCount; i++) 
        sum += wordToNumber(p->terms[i], letters, assignedDigits, letterCnt);

    long long result = wordToNumber(p->result, letters, assignedDigits, letterCnt);

    return sum == result;
}

int solve_recursive(const Puzzle* p, const char letters[], int letterCnt, int position, 
    int assignedDigits[], int digitUsed[]) {
    if (position == letterCnt)
        return checkSolution(p, letters, assignedDigits, letterCnt);

    for (int digit = 0; digit <= 9; digit++) {
        if (digitUsed[digit])
            continue;

        assignedDigits[position] = digit;
        digitUsed[digit] = 1;

        if (solve_recursive(p, letters, letterCnt, position + 1, assignedDigits, digitUsed))
            return 1;

        assignedDigits[position] = -1;
        digitUsed[digit] = 0;
    }

    return 0;
}

void printSolution(const Puzzle* p, const char letters[], const int assignedDigits[], 
    int letterCnt) {
    for (int i = 0; i < p->termCount; i++) {
        printf("%lld", wordToNumber(p->terms[i], letters, assignedDigits, letterCnt));

        if (i != p->termCount - 1)
            printf(" + ");
    }

    printf(" = ");

    printf("%lld\n", wordToNumber(p->result, letters, assignedDigits, letterCnt));
}

void solvePuzzle(const char* puzzle) {
    Puzzle* p = parsePuzzle(puzzle);

    if (p == NULL) {
        printf("Parse error\n");
        return;
    }

    char letters[26];
    int letterCnt = collectUni(p, letters);

    if (letterCnt > 10) {
        printf("Too many unique letters\n");
        freePuzzle(p);
        return;
    }

    int assignedDigits[26];
    int digitUsed[10];

    for (int i = 0; i < 26; i++)
        assignedDigits[i] = -1;

    for (int i = 0; i < 10; i++)
        digitUsed[i] = 0;

    int solved = solve_recursive(p, letters, letterCnt, 0, assignedDigits, digitUsed);

    if (solved) {
        printf("%s\n\n", puzzle);

        printSolution(p, letters, assignedDigits, letterCnt);

        printf("\n");
    }
    else
        printf("Internal error\n");

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

            readPuzzlesFromFile(filename);

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
