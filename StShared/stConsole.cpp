/**
 * Copyright © 2009, 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StStrings/stConsole.h>

#if(defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <stdio.h>
#endif

int st::getch() {
#if(defined(_WIN32) || defined(__WIN32__))
    return _getch();
#else
    struct termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    struct termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

st::ostream& st::SYS_PAUSE_EMPTY(st::ostream& theOStream) {
    st::getch();
    return theOStream;
}

st::ostream& st::SYS_PAUSE(st::ostream& theOStream) {
    st::cout << stostream_text("Press any key to continue...");
    st::getch();
    st::cout << stostream_text('\n');
    return theOStream;
}

// console output text-color functions
st::ostream& st::COLOR_FOR_RED(st::ostream& theOStream) {
#if(defined(_WIN32) || defined(__WIN32__))
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
    theOStream << stostream_text("\e[31m");
#endif
    return theOStream;
}

st::ostream& st::COLOR_FOR_GREEN(st::ostream& theOStream) {
#if(defined(_WIN32) || defined(__WIN32__))
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
    theOStream << stostream_text("\e[32m");
#endif
    return theOStream;
}

st::ostream& st::COLOR_FOR_YELLOW_L(st::ostream& theOStream) {
#if(defined(_WIN32) || defined(__WIN32__))
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_RED);
#else
    // no intence attribute for ISO...
    theOStream << stostream_text("\e[33m");
#endif
    return theOStream;
}

st::ostream& st::COLOR_FOR_YELLOW(st::ostream& theOStream) {
#if(defined(_WIN32) || defined(__WIN32__))
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
    theOStream << stostream_text("\e[33m");
#endif
    return theOStream;
}

st::ostream& st::COLOR_FOR_BLUE(st::ostream& theOStream) {
#if(defined(_WIN32) || defined(__WIN32__))
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE);
#else
    theOStream << stostream_text("\e[34m");
#endif
    return theOStream;
}

st::ostream& st::COLOR_FOR_WHITE(st::ostream& theOStream) {
#if(defined(_WIN32) || defined(__WIN32__))
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    // no default value on Win
    SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    ///theOStream << stostream_text("\e[37m");
    theOStream << stostream_text("\e[0m"); // set defaults
#endif
    return theOStream;
}
