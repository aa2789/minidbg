#include <vector>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/personality.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "linenoise.h"

#include "debugger.hpp"
#include "registers.hpp"

using namespace minidbg;

std::vector<std::string> split(const std::string &s, char delimiter);
bool is_prefix(const std::string& s, const std::string& of);


void debugger::run() {
    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);

    char* line = nullptr;
    while((line = linenoise("minidbg> ")) != nullptr) {
        handle_command(line);
        linenoiseHistoryAdd(line);
        linenoiseFree(line);
    }
}

void debugger::handle_command(const std::string& line) {
    auto args = split(line,' ');
    auto command = args[0];

    if (is_prefix(command, "continue")) {
        /*
        the continue command is used in a debugger when you want to resume the execution
        of the traced or debugged process after it has been stopped due to a breakpoint, 
        signal, or other event, and you have completed any necessary inspection or modification 
        of the process's state.
        */
        continue_execution();
    }
    else {
        std::cerr << "Unknown command\n";
    }
}

void debugger::continue_execution() {
    ptrace(PTRACE_CONT, m_pid, nullptr, nullptr);

    int wait_status;
    auto options = 0;
    waitpid(m_pid, &wait_status, options);
}

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> out{};
    std::stringstream ss {s};
    std::string item;

    while (std::getline(ss,item,delimiter)) {
        out.push_back(item);
    }

    return out;
}

bool is_prefix(const std::string& s, const std::string& of) {
    if (s.size() > of.size()) return false;
    return std::equal(s.begin(), s.end(), of.begin());
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Program name not specified";
        return -1;
    }

    auto prog = argv[1];

    auto pid = fork();
    if (pid == 0) {
        //child
       // personality(ADDR_NO_RANDOMIZE);
        //execute_debugee(prog);
        /*
           ptrace() is a system call used for controlling and observing the execution of another process.
           Here, PTRACE_TRACEME is an option that tells the kernel to allow the parent process 
           (i.e., the debugger or tracer) to trace the current process. The next three arguments are not
           relevant for PTRACE_TRACEME and are thus set to 0 and nullptrs, respectively.
           
            execl() is a function used to replace the current process's image with a new program. It takes
            the path of the program (in this case, prog), followed by the list of arguments to pass to the
            program. The list of arguments should be terminated by a nullptr. Here, prog is both the path 
            of the program and the first argument, which typically represents the program's name when executed.
            Replacing the current process's image with a new program is a common technique in process management,
            particularly for creating new processes and executing different programs
        */

        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        execl(prog, prog, nullptr);
    }
    else if (pid >= 1)  {
        //parent
        std::cout << "Started debugging process " << pid << '\n';
        debugger dbg{prog, pid};
        dbg.run();
    }
}
