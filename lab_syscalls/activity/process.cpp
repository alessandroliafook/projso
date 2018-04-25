#include "../xeu_utils/StreamParser.h"
#include "sys/types.h"
#include "unistd.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"

#include <iostream>
#include <vector>

using namespace xeu_utils;
using namespace std;

int process(const vector<Command>& commands) {

    int n = commands.size();
    int i, pp[n][2];

    if (n > 1) {
        for (i = 0; i < n - 1; i ++) {
            pipe(pp[i]);
#ifdef DEBUG_MODE
            cout << pp[i][0] << " " << pp[i][1] << endl;
#endif
        }
    }

    for (i = 0; i < n; i++) {

        if (fork() == 0) {

            if (n > 1) {
                
                if (i == 0) {
#ifdef DEBUG_MODE
                    cout << "primeiro "<< pp[i][1] << endl;
#endif

                    for (size_t j = 0; j < commands[i].io().size(); j++) {
                        IOFile io = commands[i].io()[j];
                        if (io.is_input()) {
                            int fdin = open(io.path().c_str(), O_RDONLY);
                            dup2(fdin, 0);
                            close(fdin);
                        }
                    }
                    close(pp[i][0]);
                    dup2(pp[i][1], 1);

                } else if (i < n - 1) {
#ifdef DEBUG_MODE
                    cout << "meio entrada: " << pp[i-1][0] << " saida: " << pp[i][1] << endl;
#endif

                    close(pp[i-1][1]);
                    dup2(pp[i-1][0], 0);

                    close(pp[i][0]);
                    dup2(pp[i][1], 1);

               } else {
#ifdef DEBUG_MODE
                    cout << "ultimo " << pp[i-1][0] << endl;
#endif

                    for (size_t j = 0; j < commands[i].io().size(); j++) {
                        IOFile io = commands[i].io()[j];
                        if (io.is_output()) {
                            int fdout = open(io.path().c_str(), O_WRONLY|O_CREAT, 0666);
                            dup2(fdout, 1);
                            close(fdout);
                        }
                    }
                    close(pp[i-1][1]);
                    dup2(pp[i-1][0], 0);
                }
            }

            for (size_t j = 0; j < commands[i].io().size(); j++) {
                IOFile io = commands[i].io()[j];
                if (io.is_input()) {
                    int fdin = open(io.path().c_str(), O_RDONLY);
                    dup2(fdin, 0);
                    close(fdin);
                }
            }
            for (size_t j = 0; j < commands[i].io().size(); j++) {
                IOFile io = commands[i].io()[j];
                if (io.is_output()) {
                    int fdout = open(io.path().c_str(), O_WRONLY|O_CREAT, 0666);
                    dup2(fdout, 1);
                    close(fdout);
                }
            }

            execvp(commands[i].filename(), commands[i].argv());

        } else {

            close(pp[i][1]);
            int statval = 0;
            pid_t wpid;

            while ((wpid = wait(&statval)) > 0);

        }
    }

    return 0;
}