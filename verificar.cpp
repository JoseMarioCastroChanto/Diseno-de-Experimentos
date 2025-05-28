#include <fstream>
#include <iostream>
#include <map>
#include <string>

/*
g++ -std=c++17 -o verificar verificar.cpp
./verificar
*/

int main() {
    std::ifstream fin("resultados.csv");
    if (!fin.is_open()) {
        std::cerr << "No se pudo abrir resultados.csv\n";
        return 1;
    }

    std::string header;
    std::getline(fin, header);  // leer header

    std::map<std::string, int> counts;
    std::string line;
    while (std::getline(fin, line)) {
        // La clave será "Bloque|Algoritmo|Paralelizacion"
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        size_t pos3 = line.find(',', pos2 + 1);
        if (pos1 == std::string::npos || pos2 == std::string::npos || pos3 == std::string::npos) continue;

        std::string bloque = line.substr(0, pos1);
        std::string algoritmo = line.substr(pos1 + 1, pos2 - pos1 - 1);
        std::string paralelizacion = line.substr(pos2 + 1, pos3 - pos2 - 1);

        std::string key = bloque + "|" + algoritmo + "|" + paralelizacion;
        counts[key]++;
    }

    for (const auto &kv : counts) {
        std::cout << kv.first << " => " << kv.second << " observaciones\n";
    }

    return 0;
}
