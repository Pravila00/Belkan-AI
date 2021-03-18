#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado {
  int fila;
  int columna;
  int orientacion;

  bool operator==(const estado &otro){
    if(fila==otro.fila && columna==otro.columna && orientacion==otro.orientacion){
        return true;
    }
    else{
        return false;
    }
  }
};

struct estadoCoste{
    int fila;
    int columna;
  int orientacion;
  int gasto;
  //0 nada | 1 zapatillas | 2 bikini | 3 todo
  int accesorios;

  bool operator==(const estado &otro){
    if(fila==otro.fila && columna==otro.columna && orientacion==otro.orientacion){
        return true;
    }
    else{
        return false;
    }
  }
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
      bikini=false;
        bikiniEncontrado=false;
        zapatillasEncontrado=false;
        recargaEncontrado=false;
      mapaDescubierto=false;
      recargando=false;
      zapatillas=false;
      buscandoBikini=false;
      buscandoZapatillas=false;
      tiempoRestante=3000;
      tiempoAnteriorPlan=3000;
      descubierto=false;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      bikini=false;
      zapatillas=false;

    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

    inline unsigned char getMapaResultado(int fil,int col){
        return mapaResultado[fil][col];
    }


  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado actual, destino, destinoCopia;
    list<Action> plan;
    bool hayplan;
    int tiempoRestante;
    bool recargando;
    bool mapaDescubierto;
    bool bikini,bikiniEncontrado;
    bool zapatillas,zapatillasEncontrado,recargaEncontrado;
    estado stBikini,stZapatillas,stRecarga,stRecargaMinima;
    bool buscandoBikini,buscandoZapatillas;
    int tiempoCalculoA,tiempoCalculoB;
    int tiempoAnteriorPlan;
    bool rutaCalculadaA,rutaCalculadaB;
    bool descubierto;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Coste(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_CosteNivel4(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_A_Estrella(const estado &origen, const estado &destino, list<Action> &plan);

    int costeEstado(estadoCoste &st);
    bool esSolucionCoste(const estadoCoste &st,const estado &destino);
    bool HayObstaculoDelanteCoste(estadoCoste &st);
    int distancia(const estado &actual,const estado &destino);
    bool casillaCorrecta(int i,int j);
    int costeEstado4(estadoCoste &st);

    bool esSolucion(const estado &origen,const estado &destino);
    void pintarMapa(Sensores sensores,bool &bikiniEncontrado,bool &zapatillasEncontrado,estado &bk,estado &zp);
    void esZapatillas(int fil,int col,bool &encontrado,estado &zp);
    void esBikini(int fil,int col,bool &encontrado, estado &bk);
    void esRecarga(int fil,int col,bool &encontrado, estado &bk);
    int costeEstadoAEstrella(estado &st);
    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);

};

#endif
