#include <pagmo/pagmo.hpp>
#include <pagmo/problems/ackley.hpp>
#include <pagmo/problems/griewank.hpp>
#include <pagmo/problems/lennard_jones.hpp>
#include <pagmo/problems/rastrigin.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/schwefel.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

/*
Instalar Pagmo
sudo apt-get install libboost-all-dev
git clone https://github.com/esa/pagmo2.git
cd pagmo2
git checkout 2.19.1
mkdir build && cd build
cmake ..
make
sudo make install

Compilar
g++ -std=c++17 -O2 -o experimento experimento.cpp -lpagmo -lboost_system -lboost_filesystem -pthread
./experimento
*/

using namespace pagmo;

// Función para mapear string a problema pagmo
problem get_problem_by_name(const std::string &name) {
    if (name == "ackley") return problem(ackley());
    else if (name == "griewank") return problem(griewank());
    else if (name == "lennard_jones") return problem(lennard_jones(5)); // 5 partículas por defecto
    else if (name == "rastrigin") return problem(rastrigin());
    else if (name == "rosenbrock") return problem(rosenbrock());
    else if (name == "schwefel") return problem(schwefel());
    else throw std::runtime_error("Problema desconocido: " + name);
}

// Mínimos teóricos conocidos para cada problema
double get_theoretical_minimum(const std::string &name) {
    if (name == "ackley") return 0.0;
    else if (name == "griewank") return 0.0;
    else if (name == "rastrigin") return 0.0;
    else if (name == "rosenbrock") return 0.0;
    else if (name == "schwefel") return -418.9829;
    else if (name == "lennard_jones") return -9.103852;
    else throw std::runtime_error("No se conoce el mínimo teórico del problema: " + name);
}

// Leer CSV simple con headers
struct ExperimentRow {
    std::string bloque;
    std::string algoritmo;
    std::string paralelizacion;
    int repeticion;
};

std::vector<ExperimentRow> load_experiment_csv(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("No se pudo abrir el CSV: " + filename);

    std::vector<ExperimentRow> data;
    std::string line;
    std::getline(file, line); // header

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string bloque, algoritmo, paralelizacion, repeticion_str;

        std::getline(ss, bloque, ',');
        std::getline(ss, algoritmo, ',');
        std::getline(ss, paralelizacion, ',');
        std::getline(ss, repeticion_str, ',');

        data.push_back({bloque, algoritmo, paralelizacion, std::stoi(repeticion_str)});
    }
    return data;
}

int main() {
    const std::string input_csv = "experimento_diseno.csv";
    const std::string output_csv = "resultados.csv";

    std::vector<ExperimentRow> experiment;
    try {
        experiment = load_experiment_csv(input_csv);
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    std::ofstream fout(output_csv);
    fout << "Bloque,Algoritmo,Paralelizacion,Repeticion,ErrorAbsoluto\n";

    for (const auto &row : experiment) {
        try {
            problem prob = get_problem_by_name(row.bloque);
            double theoretical_min = get_theoretical_minimum(row.bloque);

            algorithm algo = [&]() -> algorithm {
                if (row.algoritmo == "SGA") return algorithm(sga());
                else if (row.algoritmo == "PSO") return algorithm(pso());
                else throw std::runtime_error("Algoritmo desconocido: " + row.algoritmo);
            }();

            bool paralelo = (row.paralelizacion == "Paralela");

            population pop(prob, 30);

            if (paralelo) {
                archipelago archi;
                for (int i = 0; i < 4; ++i) {
                    archi.push_back(island{algo, prob, 30});
                }

                archi.evolve();
                archi.wait_check();

                double best_f = archi[0].get_population().champion_f()[0];
                for (int i = 1; i < 4; ++i) {
                    double current_f = archi[i].get_population().champion_f()[0];
                    if (current_f < best_f) best_f = current_f;
                }

                double diff = std::abs(best_f - theoretical_min);
                fout << row.bloque << "," << row.algoritmo << "," << row.paralelizacion << "," << row.repeticion << "," << diff << "\n";

            } else {
                algo.set_verbosity(0);
                algo.evolve(pop);

                double best_f = pop.champion_f()[0];
                double diff = std::abs(best_f - theoretical_min);
                fout << row.bloque << "," << row.algoritmo << "," << row.paralelizacion << "," << row.repeticion << "," << diff << "\n";
            }

        } catch (const std::exception &e) {
            std::cerr << "Error en fila: " << row.bloque << " " << row.algoritmo << " " << row.paralelizacion << " - " << e.what() << "\n";
        }
    }

    std::cout << "Simulaciones completadas. Resultados guardados en " << output_csv << "\n";
    return 0;
}
