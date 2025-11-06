#include "router.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <sstream>

Router::Router(char id) : idRouter(id), camino(""), distancia(INT_MAX), visitado(false) {}

void Router::nuevoVecino(Router* vecino, int costo) {
    vecinos.emplace_back(vecino, costo);
}

void Router::confDistancia(int dist) {
    distancia = dist;
}

void Router::reinicio() {
    distancia = INT_MAX;
    visitado = false;
    camino = "";
}

void dijkstra(Router* fuente, Router* destino, std::list<Router>& topologia) {
    if (!fuente || !destino) {
        std::cout << "Fuente o destino invalido.\n";
        return;
    }

    // Reiniciar estado de la topologia
    for (auto& r : topologia) r.reinicio();

    fuente->confDistancia(0);
    fuente->camino = std::string(1, fuente->idRouter);

    using Node = std::pair<int, Router*>;
    auto cmp = [](const Node& a, const Node& b){ return a.first > b.first; }; // min-heap
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);
    pq.push({0, fuente});

    while (!pq.empty()) {
        Router* actual = pq.top().second;
        pq.pop();

        if (actual->visitado) continue;
        actual->visitado = true;

        for (auto& vec : actual->vecinos) {
            Router* sigRouter = vec.first;
            int costoBorde = vec.second;
            if (!sigRouter) continue;
            int nuevaDistancia = actual->distancia + costoBorde;
            if (nuevaDistancia < sigRouter->distancia) {
                sigRouter->confDistancia(nuevaDistancia);
                sigRouter->camino = actual->camino + " -> " + std::string(1, sigRouter->idRouter);
                pq.push({nuevaDistancia, sigRouter});
            }
        }
    }

    if (destino->distancia == INT_MAX) {
        std::cout << "No hay camino desde " << fuente->idRouter << " hacia " << destino->idRouter << std::endl;
    } else {
        std::cout << "Camino mas corto a " << destino->idRouter <<" desde "<<fuente->idRouter<< " es " << destino->camino << std::endl;
        std::cout << "El menor costo a " << destino->idRouter <<" desde "<<fuente->idRouter<< ": " << destino->distancia << std::endl;
    }
}

// Métodos utilitarios
bool verificarVecino(Router* a, const Router& b){
    for(const auto& iter : b.vecinos){
        if(iter.first == a){
            return true;
        }
    }
    return false;
}

void verVecinos(const Router& a){
    for(const auto& iter : a.vecinos){
        std::cout << iter.first->idRouter << " (cost: " << iter.second << ")\n";
    }
}

void generar_routers_Aleatorios(std::list<Router>& topologia){
    topologia.clear();

    std::random_device rd;
    std::mt19937 gen(rd());

    int routers = 0;
    std::cout << "Ingrese la cantidad de routers a generar: " << std::endl;
    std::cin >> routers;
    if(routers <= 0){
        std::cout << "Cantidad invalida.\n";
        return;
    }

    // Parámetros
    const int MIN_NEIGHBORS = 1;
    const int MAX_NEIGHBORS = 3;
    const int COST_MIN = 1;
    const int COST_MAX = 12;

    // Crear nodos A, B, C, ...
    char base = 'A';
    for(int i = 0; i < routers; ++i){
        topologia.emplace_back(static_cast<char>(base + i));
    }

    // Guardar punteros en un vector para acceso por índice (las direcciones en std::list son estables mientras no se reasigne la lista)
    std::vector<Router*> nodes;
    nodes.reserve(routers);
    for(auto& r : topologia) nodes.push_back(&r);

    // Distribuciones
    std::uniform_int_distribution<> costDist(COST_MIN, COST_MAX);
    std::uniform_int_distribution<> indexDist(0, std::max(0, routers - 1));
    std::uniform_int_distribution<> extraEdgeChance(0, 100);

    // --- 1) Garantizar conectividad: crear un spanning tree simple
    std::vector<int> indices(routers);
    for(int i = 0; i < routers; ++i) indices[i] = i;
    std::shuffle(indices.begin(), indices.end(), gen);

    for(int i = 0; i + 1 < (int)indices.size(); ++i){
        Router* a = nodes[indices[i]];
        Router* b = nodes[indices[i+1]];
        if(!verificarVecino(b, *a) && !verificarVecino(a, *b)){
            int cost = costDist(gen);
            a->nuevoVecino(b, cost);
            b->nuevoVecino(a, cost);
        }
    }

    // --- 2) Asegurar que todos tengan al menos MIN_NEIGHBORS
    if(routers > 1){
        auto neighborCount = [](Router* r)->int{ return static_cast<int>(r->vecinos.size()); };
        for(int i = 0; i < routers; ++i){
            Router* r = nodes[i];
            while(neighborCount(r) < MIN_NEIGHBORS){
                int target = indexDist(gen);
                if(target == i) continue;
                Router* t = nodes[target];
                if(neighborCount(t) >= MAX_NEIGHBORS) {
                    bool found = false;
                    for(int attempt = 0; attempt < routers; ++attempt){
                        int cand = indexDist(gen);
                        if(cand == i) continue;
                        Router* candptr = nodes[cand];
                        if(neighborCount(candptr) < MAX_NEIGHBORS && !verificarVecino(candptr, *r)){
                            int cost = costDist(gen);
                            r->nuevoVecino(candptr, cost);
                            candptr->nuevoVecino(r, cost);
                            found = true;
                            break;
                        }
                    }
                    if(!found) break;
                } else {
                    if(!verificarVecino(t, *r)){
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
    auto neighborCount = [](Router* r)->int{ return static_cast<int>(r->vecinos.size()); };
    const int MAX_ATTEMPTS = routers * 10;
    int attempts = 0;
    for(int i = 0; i < routers && attempts < MAX_ATTEMPTS; ++i){
        Router* a = nodes[i];
        if(neighborCount(a) >= MAX_NEIGHBORS) continue;
        int canAdd = MAX_NEIGHBORS - neighborCount(a);
        for(int tries = 0; tries < canAdd && attempts < MAX_ATTEMPTS; ++tries){
            ++attempts;
            int tIndex = indexDist(gen);
            if(tIndex == i) continue;
            Router* b = nodes[tIndex];
            if(neighborCount(b) >= MAX_NEIGHBORS) continue;
            if(verificarVecino(b, *a)) continue;
            if (extraEdgeChance(gen) < 60) {
                int cost = costDist(gen);
                a->nuevoVecino(b, cost);
                b->nuevoVecino(a, cost);
            }
        }
    }

    // Impresión final
    std::cout << "\nTopologia generada (cada linea: vecino (costo)):\n\n";
    for(const auto& rout : topologia){
        std::cout << rout.idRouter << " vecinos:\n";
        verVecinos(rout);
        std::cout<< std::endl;
    }
}

bool verificarExistenciaRouter(std::list<Router>& topologia, char nombre, Router*& puntero){
    for(auto& c_router: topologia){
        if(c_router.idRouter == nombre){
            puntero= &c_router;
            return true;
        }
    }
    return false;
}

void Buscando_camino(std::list<Router>& topologia){
    char id_router_fuente, id_router_destino;
    bool ban=true;
    Router* puntero_fuente = nullptr;
    Router* puntero_destino = nullptr;
    while(ban){
        std::cout<<"Ingrese la Id del router fuente: "; std::cin>>id_router_fuente;
        if(!verificarExistenciaRouter(topologia, id_router_fuente, puntero_fuente)){
            std::cout << "La id ingresada no existe\n";
            continue;
        }
        std::cout<<"Ingrese la Id del router destino: ";std::cin>>id_router_destino;
        if(!verificarExistenciaRouter(topologia, id_router_destino, puntero_destino)){
            std::cout << "La id ingresada no existe\n";
            continue;
        }
        dijkstra(puntero_fuente, puntero_destino, topologia);
        ban = false;
    }
}

void crearRouters(std::list<Router>& topologia){
    bool entrando = true;
    char nuevo;

    while(entrando){
        std::cout << "Ingrese el identificador del router (una letra del alfabeto) a agregar en la red: "; std::cin >> nuevo;
        bool existe = false;
        for(const auto& requisa : topologia){
            if(requisa.idRouter == nuevo){
                existe = true;
                break;
            }
        }
        if(existe){
            std::cout << "Ese router ya existe. Intente otro nombre." << std::endl;
            continue;
        }

        topologia.emplace_back(nuevo);
        Router* nuevoptr = &topologia.back();

        int conexiones = 0;
        int tamaño = static_cast<int>(topologia.size());
        recibirConexiones(conexiones, tamaño);

        for(int k = 0; k < conexiones; ){
            char vecinodelnuevo;
            std::cout <<"Nombre del vecino del router a agregar en la red: "; std::cin >> vecinodelnuevo;
            Router* requisaptr = nullptr;
            bool existe_vecino = verificarExistenciaRouter(topologia, vecinodelnuevo, requisaptr);
            if(!existe_vecino){
                std::cout << "No existe ese router. Intente de nuevo.\n";
                continue;
            }
            if(requisaptr->idRouter == nuevoptr->idRouter){
                std::cout << "No puedes conectar un router a si mismo.\n";
                continue;
            }
            if(verificarVecino(requisaptr, *nuevoptr)){
                std::cout<<"Este router ya es vecino del nuevo router"<<std::endl;
                continue;
            }

            int costodelnuevo = 0;
            std::cout <<"Ingrese el costo del nuevo router al vecino: "; std::cin >> costodelnuevo;
            // agregar aristas en ambas direcciones
            nuevoptr->nuevoVecino(requisaptr, costodelnuevo);
            requisaptr->nuevoVecino(nuevoptr, costodelnuevo);
            ++k;
        }

        entrando = false;
    }

    for(const auto& rout : topologia){
        verVecinos(rout);
        std::cout<< std::endl;
    }
}

void recibirConexiones(int& conexiones, int tamaño){
    while(true){
        std::cout << "Ingrese la cantidad de vecinos: "; std::cin >> conexiones; std::cout << std::endl;
        if(conexiones >= 0 && conexiones < tamaño){
            break;
        }else{
            std::cout << "Cantidad invalida (no hay suficientes routers o valor negativo). Trate de nuevo." << std::endl;
        }
    }
}

void eliminarRouter(std::list<Router>& topologia){
    char nuevo;
    Router *puntero = nullptr;
    while(true){
        std::cout << "Ingrese el identificador del router (una letra del alfabeto) a borrar en la red: "; std::cin >> nuevo;
        bool existe= verificarExistenciaRouter(topologia, nuevo, puntero);
        if(!existe){
            std::cout << "No existe ese router. Intente de nuevo.\n";
            continue;
        }

        for(auto& vec: topologia){
            vec.vecinos.erase(
                std::remove_if(vec.vecinos.begin(), vec.vecinos.end(),
                               [nuevo](const std::pair<Router*, int>& p){
                                   return p.first && p.first->idRouter == nuevo;
                               }),
                vec.vecinos.end()
                );
        }
        // eliminar router de la lista
        topologia.remove_if([nuevo](const Router& r){ return r.idRouter == nuevo; });
        break;
    }

    for(const auto& rout : topologia){
        verVecinos(rout);
        std::cout<< std::endl;
    }
}
