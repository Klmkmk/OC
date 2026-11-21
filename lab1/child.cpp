#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>

bool readLine(int fd, std::string &s) {
    s.clear();
    char c;
    while (read(fd, &c, 1) > 0) {
        if (c == '\n') return true;
        s += c;
    }
    return !s.empty();
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    int file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) return 1;

    std::string line;
    while (readLine(0, line)) {             // stdin — это pipe1
        const char *p = line.c_str();
        char *end;

        float result = strtof(p, &end);     // первое число
        if (end == p) {                     // в строке нет чисел
            write(1, "1", 1);
            continue;
        }
        p = end;

        while (true) {
            float x = strtof(p, &end);      // следующее число
            if (end == p) break;            // числа закончились
            p = end;

            if (x == 0) {                   // деление на ноль
                write(1, "0", 1);           // сообщаем родителю через pipe2
                close(file);
                return 1;
            }
            result /= x;
        }

        std::string out = std::to_string(result) + "\n";
        write(file, out.c_str(), out.size());
        write(1, "1", 1);                   // всё хорошо
    }

    close(file);
    return 0;
}
