#include <iostream>
#include "router.h"
#include "archivo.h"

using namespace std;

int main()
{   list <Router> topologia;

    while(1){
        int opcion;
        cout << "Escoja la opcion a realizar\n";
        cout << "1. Generar rutas aleatorias\n";
        cout << "2. Leer topologia desde un archivo\n";
        cout << "3. Salir del sistema\n";
        cout << "opcion: ";cin >> opcion;

        switch (opcion){
        case 1:
            generar_routers_Aleatorios(topologia);
            menus(topologia);
            break;
        case 2:
            bool existe_topo;
            existe_topo = leer_red_archivo(topologia);
            if(existe_topo) menus(topologia);
            break;
        case 3:
            return 0;
        default:
            cout << "Opcion invalida, por favor escoja alguna opcion del menu\n";
            break;
        }
    }
}
