#include "router.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <queue>        // std::priority_queue
#include <vector>       // std::vector
#include <functional>   // std::greater, std::less

using namespace std;


Router::Router(char id)
    : idRouter(id)
    , camino("")
    , distancia(INT_MAX)
    , visitado(false)
{}

void Router::nuevoVecino(Router *vecino, int costo)
{
    vecinos.emplace_back(vecino, costo);
}

void Router::confDistancia(int dist)
{
    distancia = dist;
}

void Router::reinicio()
{
    distancia = INT_MAX;
    visitado = false;
    camino = "";
}

void dijkstra(Router *fuente, Router *destino, list<Router> &topologia)
{
    if (!fuente || !destino) {
        cout << "Fuente o destino invalido.\n";
        return;
    }

    // Reiniciar estado de la topologia
    for (auto &r : topologia)
        r.reinicio();

    fuente->confDistancia(0);
    fuente->camino = string(1, fuente->idRouter);

    using Node = pair<int, Router *>;
    auto cmp = [](const Node &a, const Node &b) { return a.first > b.first; }; // min-heap
    priority_queue<Node, vector<Node>, decltype(cmp)> pq(cmp);
    pq.push({0, fuente});

    while (!pq.empty()) {
        Router *actual = pq.top().second;
        pq.pop();

        if (actual->visitado)
            continue;
        actual->visitado = true;

        for (auto &vec : actual->vecinos) {
            Router *sigRouter = vec.first;
            int costoBorde = vec.second;
            if (!sigRouter)
                continue;
            int nuevaDistancia = actual->distancia + costoBorde;
            if (nuevaDistancia < sigRouter->distancia) {
                sigRouter->confDistancia(nuevaDistancia);
                sigRouter->camino = actual->camino + " -> " + string(1, sigRouter->idRouter);
                pq.push({nuevaDistancia, sigRouter});
            }
        }
    }

    if (destino->distancia == INT_MAX) {
        cout << "No hay camino desde " << fuente->idRouter << " hacia " << destino->idRouter
                  << endl;
    } else {
        cout << "Camino mas corto a " << destino->idRouter << " desde " << fuente->idRouter
                  << " es " << destino->camino << endl;
        cout << "El menor costo a " << destino->idRouter << " desde " << fuente->idRouter
                  << ": " << destino->distancia << endl;
    }
}

// Métodos utilitarios
bool verificarVecino(Router *a, const Router &b)
{
    for (const auto &iter : b.vecinos) {
        if (iter.first == a) {
            return true;
        }
    }
    return false;
}

void verVecinos(const Router &a)
{
    for (const auto &iter : a.vecinos) {
        cout << iter.first->idRouter << " (cost: " << iter.second << ")\n";
    }
}

void generar_routers_Aleatorios(list<Router> &topologia)
{
    topologia.clear();

    random_device rd;
    mt19937 gen(rd());

    int routers = 0;
    cout << "Ingrese la cantidad de routers a generar: " << endl;
    cin >> routers;
    if (routers <= 0) {
        cout << "Cantidad invalida.\n";
        return;
    }

    // Parámetros
    const int MIN_NEIGHBORS = 1;
    const int MAX_NEIGHBORS = 3;
    const int COST_MIN = 1;
    const int COST_MAX = 12;

    // Crear nodos A, B, C, ...
    char base = 'A';
    for (int i = 0; i < routers; ++i) {
        topologia.emplace_back(static_cast<char>(base + i));
    }

    // Guardar punteros en un vector para acceso por índice (las direcciones en list son estables mientras no se reasigne la lista)
    vector<Router *> nodes;
    nodes.reserve(routers);
    for (auto &r : topologia)
        nodes.push_back(&r);

    // Distribuciones
    uniform_int_distribution<> costDist(COST_MIN, COST_MAX);
    uniform_int_distribution<> indexDist(0, max(0, routers - 1));
    uniform_int_distribution<> extraEdgeChance(0, 100);

    // --- 1) Garantizar conectividad: crear un spanning tree simple
   vector<int> indices(routers);
    for (int i = 0; i < routers; ++i)
        indices[i] = i;
    shuffle(indices.begin(), indices.end(), gen);

    for (int i = 0; i + 1 < (int) indices.size(); ++i) {
        Router *a = nodes[indices[i]];
        Router *b = nodes[indices[i + 1]];
        if (!verificarVecino(b, *a) && !verificarVecino(a, *b)) {
            int cost = costDist(gen);
            a->nuevoVecino(b, cost);
            b->nuevoVecino(a, cost);
        }
    }

    // --- 2) Asegurar que todos tengan al menos MIN_NEIGHBORS
    if (routers > 1) {
        auto neighborCount = [](Router *r) -> int { return static_cast<int>(r->vecinos.size()); };
        for (int i = 0; i < routers; ++i) {
            Router *r = nodes[i];
            while (neighborCount(r) < MIN_NEIGHBORS) {
                int target = indexDist(gen);
                if (target == i)
                    continue;
                Router *t = nodes[target];
                if (neighborCount(t) >= MAX_NEIGHBORS) {
                    bool found = false;
                    for (int attempt = 0; attempt < routers; ++attempt) {
                        int cand = indexDist(gen);
                        if (cand == i)
                            continue;
                        Router *candptr = nodes[cand];
                        if (neighborCount(candptr) < MAX_NEIGHBORS
                            && !verificarVecino(candptr, *r)) {
                            int cost = costDist(gen);
                            r->nuevoVecino(candptr, cost);
                            candptr->nuevoVecino(r, cost);
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                        break;
                } else {
                    if (!verificarVecino(t, *r)) {
                        int cost = costDist(gen);
                        r->nuevoVecino(t, cost);
                        t->nuevoVecino(r, cost);
                    } else {
                        continue;
                    }
                }
            }
        }
    }

    // --- 3) Añadir aristas aleatorias respetando MAX_NEIGHBORS
    auto neighborCount = [](Router *r) -> int { return static_cast<int>(r->vecinos.size()); };
    const int MAX_ATTEMPTS = routers * 10;
    int attempts = 0;
    for (int i = 0; i < routers && attempts < MAX_ATTEMPTS; ++i) {
        Router *a = nodes[i];
        if (neighborCount(a) >= MAX_NEIGHBORS)
            continue;
        int canAdd = MAX_NEIGHBORS - neighborCount(a);
        for (int tries = 0; tries < canAdd && attempts < MAX_ATTEMPTS; ++tries) {
            ++attempts;
            int tIndex = indexDist(gen);
            if (tIndex == i)
                continue;
            Router *b = nodes[tIndex];
            if (neighborCount(b) >= MAX_NEIGHBORS)
                continue;
            if (verificarVecino(b, *a))
                continue;
            if (extraEdgeChance(gen) < 60) {
                int cost = costDist(gen);
                a->nuevoVecino(b, cost);
                b->nuevoVecino(a, cost);
            }
        }
    }

    // Impresión final
    cout << "\nTopologia generada (cada linea: vecino (costo)):\n\n";
    for (const auto &rout : topologia) {
        cout << rout.idRouter << " vecinos:\n";
        verVecinos(rout);
        cout << endl;
    }
}

bool verificarExistenciaRouter(list<Router> &topologia, char nombre, Router *&puntero)
{
    for (auto &c_router : topologia) {
        if (c_router.idRouter == nombre) {
            puntero = &c_router;
            return true;
        }
    }
    return false;
}

void Buscando_camino(std::list<Router> &topologia)
{
    char id_router_fuente, id_router_destino;
    bool ban = true;
    Router *puntero_fuente = nullptr;
    Router *puntero_destino = nullptr;
    while (ban) {
        cout << "Ingrese la Id del router fuente: ";
        cin >> id_router_fuente;
        if (!verificarExistenciaRouter(topologia, id_router_fuente, puntero_fuente)) {
            cout << "La id ingresada no existe\n";
            continue;
        }
        cout << "Ingrese la Id del router destino: ";
        cin >> id_router_destino;
        if (!verificarExistenciaRouter(topologia, id_router_destino, puntero_destino)) {
            cout << "La id ingresada no existe\n";
            continue;
        }
        dijkstra(puntero_fuente, puntero_destino, topologia);
        ban = false;
    }
}

void crearRouters(list<Router> &topologia)
{
    bool entrando = true;
    char nuevo;

    while (entrando) {
        cout << "Ingrese el identificador del router (una letra del alfabeto) a agregar en la red: ";
        cin >> nuevo;
        bool existe = false;
        for (const auto &requisa : topologia) {
            if (requisa.idRouter == nuevo) {
                existe = true;
                break;
            }
        }
        if (existe) {
            cout << "Ese router ya existe. Intente otro nombre." << endl;
            continue;
        }

        topologia.emplace_back(nuevo);
        Router *nuevoptr = &topologia.back();

        int conexiones = 0;
        int tamaño = static_cast<int>(topologia.size());
        recibirConexiones(conexiones, tamaño);

        for (int k = 0; k < conexiones;) {
            char vecinodelnuevo;
            cout << "Nombre del vecino del router a agregar en la red: ";
            cin >> vecinodelnuevo;
            Router *requisaptr = nullptr;
            bool existe_vecino = verificarExistenciaRouter(topologia, vecinodelnuevo, requisaptr);
            if (!existe_vecino) {
                cout << "No existe ese router. Intente de nuevo.\n";
                continue;
            }
            if (requisaptr->idRouter == nuevoptr->idRouter) {
                cout << "No puedes conectar un router a si mismo.\n";
                continue;
            }
            if (verificarVecino(requisaptr, *nuevoptr)) {
                cout << "Este router ya es vecino del nuevo router" << endl;
                continue;
            }

            int costodelnuevo = 0;
            cout << "Ingrese el costo del nuevo router al vecino: ";
            cin >> costodelnuevo;
            // agregar aristas en ambas direcciones
            nuevoptr->nuevoVecino(requisaptr, costodelnuevo);
            requisaptr->nuevoVecino(nuevoptr, costodelnuevo);
            ++k;
        }

        entrando = false;
    }

    for (const auto &rout : topologia) {
        verVecinos(rout);
        cout << endl;
    }
}

void recibirConexiones(int &conexiones, int tamaño)
{
    while (true) {
        cout << "Ingrese la cantidad de vecinos: ";
        cin >> conexiones;
        cout << endl;
        if (conexiones >= 0 && conexiones < tamaño) {
            break;
        } else {
            std::cout << "Cantidad invalida (no hay suficientes routers o valor negativo). Trate "
                         "de nuevo."<< endl;
        }
    }
}

void eliminarRouter(list<Router> &topologia)
{
    char nuevo;
    Router *puntero = nullptr;
    while (true) {
        cout << "Ingrese el identificador del router (una letra del alfabeto) a borrar en la red: ";
        cin >> nuevo;
        bool existe = verificarExistenciaRouter(topologia, nuevo, puntero);
        if (!existe) {
            cout << "No existe ese router. Intente de nuevo.\n";
            continue;
        }

        for (auto &vec : topologia) {
            vec.vecinos.erase(std::remove_if(vec.vecinos.begin(),
                                             vec.vecinos.end(),
                                             [nuevo](const std::pair<Router *, int> &p) {
                                                 return p.first && p.first->idRouter == nuevo;
                                             }),
                              vec.vecinos.end());
        }
        // eliminar router de la lista
        topologia.remove_if([nuevo](const Router &r) { return r.idRouter == nuevo; });
        break;
    }

    for (const auto &rout : topologia) {
        verVecinos(rout);
        cout << endl;
    }
}
