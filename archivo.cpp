#include "archivo.h"
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

bool comprobarLectura(const std::string &nombredelarchivo)
{
    std::ifstream archivo(nombredelarchivo);
    if (!archivo) {
        std::cerr << "\nNo se puede abrir el archivo. No se encuentra.\n" << std::endl;
        return false;
    } else {
        archivo.close();
        std::cout << "\nVerificacion exitosa\n" << std::endl;
        return true;
    }
}

bool leer_red_archivo(std::list<Router> &topologia)
{
    std::string nombre_archivo;
    std::string linea;
    std::cout << "Ingrese el nombre del archivo a leer (sin extensión): ";
    std::cin >> nombre_archivo;
    nombre_archivo += ".txt";
    if (!comprobarLectura(nombre_archivo))
        return false;

    std::ifstream archivo(nombre_archivo);
    if (!archivo.is_open()) {
        std::cerr << "Error abriendo el archivo despues de la verificacion.\n";
        return false;
    }

    topologia.clear();

    // Primera pasada: crear routers (primer caracter de cada linea que sea letra)
    archivo.clear();
    archivo.seekg(0);
    while (std::getline(archivo, linea)) {
        bool hasLetter = false;
        char id = '\0';
        for (char c : linea) {
            if (std::isalpha(static_cast<unsigned char>(c))) {
                id = c;
                hasLetter = true;
                break;
            }
        }
        if (!hasLetter)
            continue;
        topologia.emplace_back(id);
    }

    // Segunda pasada: parsear vecinos y costos
    archivo.clear();
    archivo.seekg(0);
    while (std::getline(archivo, linea)) {
        if (linea.empty())
            continue;

        // Encontrar primer caracter letra (id fuente)
        size_t pos = std::string::npos;
        for (size_t i = 0; i < linea.size(); ++i) {
            if (std::isalpha(static_cast<unsigned char>(linea[i]))) {
                pos = i;
                break;
            }
        }
        if (pos == std::string::npos)
            continue;
        char id_fuente = linea[pos];

        // Tomar el resto de la línea después de la primera letra
        std::string rest = linea.substr(pos + 1);

        // Parse robusto: buscar pares letra + número (número puede tener varios dígitos)
        size_t i = 0;
        while (i < rest.size()) {
            // buscar siguiente letra
            while (i < rest.size() && !std::isalpha(static_cast<unsigned char>(rest[i])))
                ++i;
            if (i >= rest.size())
                break;
            char id_vec = rest[i];
            ++i;
            // leer dígitos del costo
            std::string num;
            while (i < rest.size() && std::isdigit(static_cast<unsigned char>(rest[i]))) {
                num.push_back(rest[i]);
                ++i;
            }
            if (num.empty())
                continue;
            int cost = std::stoi(num);

            // Buscar punteros a routers en topologia
            Router *ptr_fuente = nullptr;
            Router *ptr_vec = nullptr;
            if (verificarExistenciaRouter(topologia, id_fuente, ptr_fuente)
                && verificarExistenciaRouter(topologia, id_vec, ptr_vec)) {
                ptr_fuente->nuevoVecino(ptr_vec, cost);
                // Si deseas red no dirigida, puedes también agregar la inversa:
                // ptr_vec->nuevoVecino(ptr_fuente, cost);
            }
        }
    }

    archivo.close();
    return true;
}

bool guardar_red_archivo(const std::list<Router> &topologia, const std::string &nombre_archivo)
{
    if (topologia.empty()) {
        std::cerr << "La topologia esta vacia. Nada que guardar.\n";
        return false;
    }

    std::ofstream archivo(nombre_archivo);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo para escribir: " << nombre_archivo << "\n";
        return false;
    }

    // Formato de salida: una linea por router: ID vecino1Costo vecino2Costo ...
    for (const auto &router : topologia) {
        std::ostringstream oss;
        oss << router.idRouter;
        for (const auto &par : router.vecinos) {
            if (par.first) {
                oss << ' ' << par.first->idRouter << par.second;
            }
        }
        archivo << oss.str() << '\n';
    }

    archivo.close();
    std::cout << "Topologia guardada en: " << nombre_archivo << std::endl;
    return true;
}

void menus(std::list<Router> &topologia)
{
    std::string respuesta_menu;
    bool menu = true;
    while (menu) {
        std::cout << "Menu" << std::endl;
        std::cout << "\t1. Agregar un router" << std::endl;
        std::cout << "\t2. Eliminar un router" << std::endl;
        std::cout << "\t3. Mostrar camino y costo mas eficiente" << std::endl;
        std::cout << "\t4. Guardar topologia en archivo" << std::endl;
        std::cout << "\t5. Salir del menu\n" << std::endl;
        std::cout << "Seleccione una opcion: ";
        std::cin >> respuesta_menu;
        if (respuesta_menu == "1") {
            crearRouters(topologia);
        } else if (respuesta_menu == "2") {
            eliminarRouter(topologia);
        } else if (respuesta_menu == "3") {
            Buscando_camino(topologia);
        } else if (respuesta_menu == "4") {
            std::string nombre;
            std::cout << "Ingrese el nombre del archivo donde guardar (sin extension): ";
            std::cin >> nombre;
            nombre += ".txt";
            guardar_red_archivo(topologia, nombre);
        } else if (respuesta_menu == "5") {
            menu = false;
        } else {
            std::cout << "Ingresaste una opcion invalida" << std::endl;
        }
    }
}
