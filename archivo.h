#ifndef ARCHIVO_H
#define ARCHIVO_H

#include "router.h"
#include <list>
#include <string>

bool comprobarLectura(const std::string& nombredelarchivo);

bool leer_red_archivo(std::list<Router>& topologia);

bool guardar_red_archivo(const std::list<Router>& topologia, const std::string& nombre_archivo);

void menus(std::list<Router>& topologia);

#endif // ARCHIVO_H
