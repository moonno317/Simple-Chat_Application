#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#define BUFFER_SIZE 1024

void wrapText(WINDOW* win, int starty, int startx, int width, const char* text) {
    int len = strlen(text);
    int currentRow = 0;
    const char* remainingText = text;

    while (len > 0) {
        int charsToPrint = (len > width) ? width : len;
        mvwprintw(win, starty + currentRow, startx, "%.*s", charsToPrint, remainingText);
        len -= charsToPrint;
        remainingText += charsToPrint;
        currentRow++; }}

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int terminalHeight, terminalWidth;
    getmaxyx(stdscr, terminalHeight, terminalWidth);

    int bigBoxHeight = terminalHeight - 5;
    int bigBoxWidth = terminalWidth - 4;
    int bigBoxY = 2;
    int bigBoxX = 2;

    int smallBoxHeight = 3;
    int smallBoxWidth = terminalWidth - 4;
    int smallBoxY = bigBoxY + bigBoxHeight - 0;
    int smallBoxX = bigBoxX + 0;

    WINDOW* bigBoxWin = newwin(bigBoxHeight, bigBoxWidth, bigBoxY, bigBoxX);
    WINDOW* smallBoxWin = newwin(smallBoxHeight, smallBoxWidth, smallBoxY, smallBoxX);

    box(bigBoxWin, 0, 0);
    box(smallBoxWin, 0, 0);

    refresh();
    wrefresh(bigBoxWin);
    wrefresh(smallBoxWin);

    wmove(smallBoxWin, 1, 1);
    wrefresh(smallBoxWin);

    char input[smallBoxWidth + 1];
    memset(input, 0, sizeof(input));
    int inputLength = 0;

    char** buffer = NULL;
    int bufferSize = 0;

    int ch;

    int clientSocket;
    struct sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Error creating client socket");
        exit(1); }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8888);

    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(1); }

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error connecting to the server");
        exit(1); }

    while ((ch = wgetch(smallBoxWin)) != KEY_F(1)) {
        if (ch == '\n' || ch == '\r') {
            if (inputLength > 0) {
                // Add the input to the buffer
                buffer = realloc(buffer, (bufferSize + 1) * sizeof(char*));
                buffer[bufferSize] = strdup(input);
                bufferSize++;

                memset(input, 0, sizeof(input));
                inputLength = 0;

                werase(smallBoxWin);
                box(smallBoxWin, 0, 0);
                wrefresh(smallBoxWin);

                werase(bigBoxWin);
                box(bigBoxWin, 0, 0);
                for (int i = 0; i < bufferSize; i++) {
                    wrapText(bigBoxWin, i + 1, 1, bigBoxWidth - 2, buffer[i]); }
                wrefresh(bigBoxWin);

                if (send(clientSocket, buffer[bufferSize - 1], strlen(buffer[bufferSize - 1]), 0) == -1) {
                    perror("Sending failed");
                    exit(1); }

                char message[BUFFER_SIZE];
                memset(message, 0, sizeof(message));
                if (recv(clientSocket, message, BUFFER_SIZE - 1, 0) == -1) {
                    perror("Receiving failed");
                    exit(1); }

                buffer = realloc(buffer, (bufferSize + 1) * sizeof(char*));
                buffer[bufferSize] = strdup(message);
                bufferSize++;

                werase(bigBoxWin);
                box(bigBoxWin, 0, 0);
                for (int i = 0; i < bufferSize; i++) {
                    wrapText(bigBoxWin,i + 1, 1, bigBoxWidth - 2, buffer[i]); }
                wrefresh(bigBoxWin); }} else if (ch == KEY_BACKSPACE || ch == 127) {
            if (inputLength > 0) {
                inputLength--;
                input[inputLength] = '\0';

                werase(smallBoxWin);
                box(smallBoxWin, 0, 0);

                wrapText(smallBoxWin, 1, 1, smallBoxWidth - 2, input);
                wrefresh(smallBoxWin); }} else {
            if (inputLength < smallBoxWidth) {
                input[inputLength] = ch;
                inputLength++;

                werase(smallBoxWin);
                box(smallBoxWin, 0, 0);

                wrapText(smallBoxWin, 1, 1, smallBoxWidth - 2, input);
                wrefresh(smallBoxWin); }}}

    for (int i = 0; i < bufferSize; i++) {
        free(buffer[i]); }
    free(buffer);

    close(clientSocket);

    endwin();

    return 0; }
