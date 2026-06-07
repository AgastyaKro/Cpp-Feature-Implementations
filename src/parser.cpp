/*
Ex Input:
3
[2025-06-05 15:30:12] INFO: User logged in
[2025-06-05 15:31:05] ERROR: Disk not found
[2025-06-05 15:31:15] WARN: Low battery

Ex Output:
INFO User logged in
ERROR Disk not found
WARN Low battery
*/

#include <iostream>

int main(){
    int n;
    std::cin >> n;
    std::cin.ignore();

    for (int i = 0; i < n; i++){
        std::string line;
        std::getline(std::cin, line);

        size_t closeBracket = line.find(']');
        size_t colon = line.find(':', closeBracket);

        if (closeBracket == std::string::npos || colon == std::string::npos){
            std::cerr << "Malformed log line " << line << '\n';
            continue;
        }

        std::string logLevel = line.substr(closeBracket + 2, colon - closeBracket - 2);
        std::string message = line.substr(colon + 2);

        std::cout << logLevel << ' ' << message << '\n';

    }
    return 0;
}