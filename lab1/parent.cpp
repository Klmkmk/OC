#include <unistd.h>
#include <sys/wait.h>
#include <string>

// читает строку из fd по одному байту; false — если данные кончились
bool readLine(int fd, std::string &s) {
    s.clear();
    char c;
    while (read(fd, &c, 1) > 0) {
        if (c == '\n') return true;
        s += c;
    }
    return !s.empty();
}

void print(const std::string &s) {
    write(1, s.c_str(), s.size());
}

int main() {
    std::string fileName;
    if (!readLine(0, fileName)) return 1;   // первая строка — имя файла

    int pipe1[2], pipe2[2];                 // [0] — чтение, [1] — запись
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        print("pipe error\n");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        print("fork error\n");
        return 1;
    }

    if (pid == 0) {                         // дочерний процесс
        dup2(pipe1[0], 0);                  // stdin  <- pipe1
        dup2(pipe2[1], 1);                  // stdout -> pipe2
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        execl("./child", "child", fileName.c_str(), NULL);
        _exit(1);                           // сюда попадём только при ошибке exec
    }

    // родительский процесс
    close(pipe1[0]);
    close(pipe2[1]);

    std::string line;
    while (readLine(0, line)) {
        line += '\n';
        write(pipe1[1], line.c_str(), line.size());

        char status;
        if (read(pipe2[0], &status, 1) <= 0 || status == '0') {
            print("Division by zero\n");
            break;
        }
    }

    close(pipe1[1]);                        // ребёнок получит конец ввода
    close(pipe2[0]);
    waitpid(pid, NULL, 0);
    return 0;
}
