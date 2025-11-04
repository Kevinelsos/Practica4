#ifndef ROUTER_H
#define ROUTER_H

#include <iostream>
#include <vector>
#include <utility>
#include <climits>
#include <list>
#include <string>
#include <queue>

using namespace std;

class Router {
public:
    Router(char id);
    char idRouter;
    string camino;
    vector<pair<Router*, int>> vecinos; // vecinos y costo
    int distancia;
    bool visitado;
    void nuevoVecino(Router* vecino, int costo);
    void confDistancia(int distancia);
    void reinicio();

    bool operator==(const Router& otro) const {
        return idRouter == otro.idRouter;
    }
};

void dijkstra(Router* fuente, Router* destino, list<Router>& topologia);

bool verificarVecino(Router* a, const Router& b);

void verVecinos(const Router& a);

void generar_routers_Aleatorios(list<Router>& topologia);

bool verificarExistenciaRouter(list<Router>& topologia, char nombre, Router*& puntero);

void Buscando_camino(list<Router>& topologia);

void recibirConexiones(int& conexiones, int tamaño);

void crearRouters(list<Router>& topologia);

void eliminarRouter(list<Router>& topologia);

#endif // ROUTER_H



