#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <vector>
#include <algorithm>
#include <queue>


// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
    Action sigAccion;
    actual.fila        = sensores.posF;
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;
    actual.fila=sensores.posF;
    actual.columna=sensores.posC;
    actual.orientacion=sensores.sentido;
    destino.fila=sensores.destinoF;
    destino.columna=sensores.destinoC;

    if(sensores.nivel!=4){
        if(!hayplan){
            hayplan=pathFinding(sensores.nivel,actual,destino,plan);
        }


        if(hayplan && plan.size()>0){
            sigAccion=plan.front();
            plan.erase(plan.begin());
             switch(sigAccion){
				case actTURN_L:
					brujula=(brujula+1)%4;
				break;
				case actTURN_R:
					brujula=(brujula+3)%4;
				break;
				case actFORWARD:
					switch(brujula){
						case 0:	col--;
						break;
						case 1:	fil++;
						break;
						case 2:	col++;
						break;
						case 3:	fil--;
						break;
					}
				break;
			}
        }

    }

    //NIVEL 4 SIN INFORMACION
    else{
        pintarMapa(sensores,bikiniEncontrado,zapatillasEncontrado,stBikini,stZapatillas);
        for(int i=0;i<mapaResultado.size() && (!zapatillas || !bikini || !recargaEncontrado);i++){
            for(int j=0;j<mapaResultado.size();j++){
                esZapatillas(i,j,zapatillasEncontrado,stZapatillas);
                esBikini(i,j,bikiniEncontrado,stBikini);
                esRecarga(i,j,recargaEncontrado,stRecarga);
            }
        }

        if((bikiniEncontrado || buscandoBikini) && !bikini){
            if(!buscandoBikini || (plan.front()==actFORWARD && sensores.terreno[2] == 'P') ||
               (plan.front()==actFORWARD && sensores.terreno[2]=='M')){
                hayplan = pathFinding(sensores.nivel, actual, stBikini, plan);
            }
            buscandoBikini=true;
            if(mapaResultado[actual.fila][actual.columna]=='K'){
                bikini=true;
                buscandoBikini=false;
            }

        }
        else if((zapatillasEncontrado || buscandoZapatillas) && !zapatillas){
            if(!buscandoZapatillas || (plan.front()==actFORWARD && sensores.terreno[2]=='P') || (plan.front()==actFORWARD &&
                sensores.terreno[2]=='M')){
                hayplan = pathFinding(sensores.nivel, actual, stZapatillas, plan);
            }
            buscandoZapatillas=true;
            if(mapaResultado[actual.fila][actual.columna]=='D'){
                zapatillas=true;
                buscandoZapatillas=false;
            }

        }

        else if(((sensores.bateria<550 || (sensores.bateria<800 && !zapatillas)) || recargando) && recargaEncontrado && (tiempoRestante>250 || recargando)){
            if(((plan.empty() || sensores.bateria<300) && !recargando) || (recargando && ((sensores.terreno[2]=='A' && plan.front()==actFORWARD && !bikini) || (sensores.terreno[2]=='B' &&
                plan.front()==actFORWARD && !zapatillas) || (sensores.terreno[2]=='M' && plan.front()==actFORWARD) || (sensores.terreno[2]=='P' && plan.front()==actFORWARD)))){
                hayplan = pathFinding(sensores.nivel, actual, stRecarga, plan);
                for(int i=0;i<300;i++)
                    plan.push_back(actIDLE);
                recargando=true;
            }

            if(sensores.bateria>2500 || tiempoRestante<250 && sensores.bateria>1000 || tiempoRestante>250 && tiempoRestante<400 && sensores.bateria>1600){
                recargando=false;
                plan.clear();
            }
        }
        if((!hayplan || plan.empty() || (tiempoAnteriorPlan-tiempoRestante>=10 && descubierto) || (sensores.terreno[2]=='M' && plan.front()==actFORWARD) || (sensores.terreno[2]=='P' && plan.front()==actFORWARD)
            || (sensores.terreno[2]=='B' && plan.front()==actFORWARD && !zapatillas) || (sensores.terreno[2]=='A' && plan.front()==actFORWARD && !bikini))
             && !recargando && !buscandoBikini && !buscandoZapatillas) {
            hayplan= pathFinding(sensores.nivel,actual,destino,plan);
            tiempoAnteriorPlan=tiempoRestante;
            descubierto=false;
        }

        if(hayplan && plan.size()>0){
            sigAccion=plan.front();
            plan.erase(plan.begin());
            switch(sigAccion){
                case actTURN_L:
                    brujula=(brujula+1)%4;
                break;
                case actTURN_R:
                    brujula=(brujula+3)%4;
                break;
                case actFORWARD:
                    if(sensores.superficie[2]!='a'){
                        switch(brujula){
                            case 0:	col--;
                            break;
                            case 1:	fil++;
                            break;
                            case 2:	col++;
                            break;
                            case 3:	fil--;
                            break;
                        }
                    }
                    else
                        sigAccion=actIDLE;

                break;

                case actIDLE:
                    if(recargando && mapaResultado[actual.fila][actual.columna]!='X'){
                        hayplan = pathFinding(sensores.nivel, actual, stRecarga, plan);
                        for(int i=0;i<200;i++)
                            plan.push_back(actIDLE);
                    }

                break;

            }
        }

    }


    tiempoRestante--;
    return sigAccion;

}





// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen,destino,plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
                  return pathFinding_Coste(origen,destino,plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
                  return pathFinding_CosteNivel4(origen,destino,plan);
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M' )
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}

bool ComportamientoJugador::HayObstaculoDelanteCoste(estadoCoste &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}


struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};


struct nodo{
	estado st;
	list<Action> secuencia;

	bool operator==(nodo n2){
        if(st==n2.st && secuencia==n2.secuencia){
            return true;
        }
        else{
            return false;
        }
	}
};

struct nodoCoste{
    estadoCoste st;
	list<Action> secuencia;
};

struct ComparaEstadosCoste{
	bool operator()(const estadoCoste &a, const estadoCoste &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion) or
            (a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.gasto>n.gasto) or
            (a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.gasto==n.gasto and a.accesorios!=n.accesorios))
			return true;
		else
			return false;
	}
};

struct ComparaEstadosCoste4{
	bool operator()(const estadoCoste &a, const estadoCoste &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
	      return true;
		else
			return false;
	}
};

struct nodoDistancia{
    estado st;
    list<Action> secuencia;
    int distancia;

};


bool operator<(const nodoDistancia &n1,const nodoDistancia &n2){
    return n1.distancia>n2.distancia;
}

bool operator<(const nodoCoste &n1,const nodoCoste &n2){
    return n1.st.gasto > n2.st.gasto;
}


// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";

	//Si el destino es un obstaculo no hacemos el recorrido
	if(EsObstaculo(mapaResultado[destino.fila][destino.columna])){
		cout <<"El destino es un obstaculo no se puede realizar el recorrido"<<endl;
		return false;
	}
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  while (!pila.empty() and !esSolucion(current.st,destino)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo
		 hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";
	//plan.pop_back();
	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

/***************************************************************/
// Implementación de la búsqueda en anchura.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();

	//Si el destino es un obstaculo no hacemos el recorrido
	if(EsObstaculo(mapaResultado[destino.fila][destino.columna])){
		cout <<"El destino es un obstaculo no se puede realizar el recorrido"<<endl;
		return false;
	}

	//Lista de nodos ABIERTOS
	queue<nodo> cola;

	/*Como queremos recorrer el arbol por ANCHURA necesitamos recorrerlo por niveles
	para ello implementamos los nodos en un contenedor de tipo cola que es FIFO*/

	//Lista de nodos CERRADOS
	set<estado,ComparaEstados> explorados;

	//Nodo actual
  	nodo current;

	//El nodo actual se corresponde con el origen
  	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

	//Mientras que haya nodos y se no haya alcanzado el objetivo
	//seguimos con la busqueda
	while(!cola.empty() && !esSolucion(current.st,destino)){

		//Eliminamos el primer elemento de la lista de nodos
		cola.pop();

		//Insertamos en la lista de nodos creados, el nodo actual
		explorados.insert(current.st);

		//A continuacion calculamos los nodos hijos de nuestro nodo
		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (explorados.find(hijoTurnR.st) == explorados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}


		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (explorados.find(hijoTurnL.st) == explorados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generamos el nodo de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (explorados.find(hijoForward.st) == explorados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}


		// Tomo el siguiente valor de la cola

		if (!cola.empty()){
			//Nos devuelve el primer elemento de la cola
			current = cola.front();
		}



	}

	//Ya se ha terminado la busqueda del plan
	cout << "Terminada la busqueda\n";
	//plan.pop_back();

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;

}
/***************************************************************/
// Implementación de la búsqueda de coste uniforme.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
int ComportamientoJugador::costeEstado4(estadoCoste &st){
	unsigned char casilla=mapaResultado[st.fila][st.columna];
	int gasto;
	switch(casilla){
		case 'A': gasto= (bikini)? 10 : 100;	break;
		case 'B': gasto= (zapatillas)? 5 : 50;	break;
		case 'T': gasto=2;	break;
		case '?': gasto=2;  break;
		default : gasto=1; break;
	}

	return gasto;
}

int ComportamientoJugador::costeEstado(estadoCoste &st){
	unsigned char casilla=mapaResultado[st.fila][st.columna];
	int gasto;
	if(casilla=='K'){
        if(st.accesorios==0){
            st.accesorios=2;
        }
        else if(st.accesorios==1){
            st.accesorios=3;
        }

	}
    else if(casilla=='D'){
        if(st.accesorios==0){
            st.accesorios=1;
        }
        else if(st.accesorios==2){
            st.accesorios=3;
        }
    }
	switch(casilla){
		case 'A': gasto= (bikini || st.accesorios==2 || st.accesorios==3)? 10 : 100;	break;
		case 'B': gasto= (zapatillas || st.accesorios==1 || st.accesorios==3)? 5 : 50;	break;
		case 'T': gasto=2;	break;
		case '?': gasto= (bikini && zapatillas)? 10 : 20; break;
		default : gasto=1; break;
	}

	return gasto;
}

int ComportamientoJugador::costeEstadoAEstrella(estado &st){
	unsigned char casilla=mapaResultado[st.fila][st.columna];
	int gasto;

	switch(casilla){
		case 'A': gasto= (bikini)? 10 : 100;	break;
		case 'B': gasto= (zapatillas)? 5 : 50;	break;
		case 'T': gasto=2;	break;
		default : gasto=1; break;
	}
	//cout <<"GASTO en casilla "<<casilla <<" es "<<gasto<<endl;
	return gasto;
}




bool ComportamientoJugador::pathFinding_Coste(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();

	//Cuando encontremos la solucion pasa a ser false
	bool sigue=true;

	//Lista de nodos ABIERTOS
	priority_queue<nodoCoste> abiertos;

	//Lista de nodos CERRADOS
	set<estadoCoste,ComparaEstadosCoste> explorados;

	//El nodo actual se corresponde con el origen
	nodoCoste nodoMenorCoste;
  	nodoMenorCoste.st.fila = origen.fila;
  	nodoMenorCoste.secuencia.empty();
  	nodoMenorCoste.st.columna=origen.columna;
  	nodoMenorCoste.st.orientacion=origen.orientacion;
    nodoMenorCoste.st.accesorios=0;
	nodoMenorCoste.st.gasto=0;

	abiertos.push(nodoMenorCoste);


	while(!abiertos.empty() && sigue){
        abiertos.pop();
		//Insertamos en la lista de nodos creados, el nodo de menor coste
		explorados.insert(nodoMenorCoste.st);

		if(esSolucionCoste(nodoMenorCoste.st,destino)){
			sigue=false;
		}
		else{
			//Procedemos a expandir el nodo de menor coste
			// Generar descendiente de girar a la derecha
			nodoCoste hijoTurnR = nodoMenorCoste;
			hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
			hijoTurnR.st.gasto+=costeEstado(hijoTurnR.st);
			if (explorados.find(hijoTurnR.st) == explorados.end()){
				hijoTurnR.secuencia.push_back(actTURN_R);
				abiertos.push(hijoTurnR);
            }


			// Generar descendiente de girar a la izquierda
			nodoCoste hijoTurnL = nodoMenorCoste;
			hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
			hijoTurnL.st.gasto+=costeEstado(hijoTurnL.st);
			if (explorados.find(hijoTurnL.st) == explorados.end()){
				hijoTurnL.secuencia.push_back(actTURN_L);
				abiertos.push(hijoTurnL);
			}

			// Generamos el nodo de avanzar
			nodoCoste hijoForward = nodoMenorCoste;
			if (!HayObstaculoDelanteCoste(hijoForward.st)){
                hijoForward.st.gasto+=costeEstado(hijoForward.st);
				if (explorados.find(hijoForward.st) == explorados.end()){
					hijoForward.secuencia.push_back(actFORWARD);
					abiertos.push(hijoForward);
				}
			}
			if(!abiertos.empty()){
                nodoMenorCoste=abiertos.top();
                while(explorados.find(nodoMenorCoste.st)!=explorados.end()){
                    abiertos.pop();
                    nodoMenorCoste=abiertos.top();
                }
			}
		}
	}

	//Ya se ha terminado la busqueda del plan
	cout << "Terminada la busqueda\n";
	//plan.pop_back();

	if (esSolucionCoste(nodoMenorCoste.st,destino)){
		cout << "Cargando el plan\n";
		plan = nodoMenorCoste.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		cout << "Coste del plan: "<<nodoMenorCoste.st.gasto<<endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}
	return false;

}

bool ComportamientoJugador::pathFinding_CosteNivel4(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();

	//Cuando encontremos la solucion pasa a ser false
	bool sigue=true;

	//Lista de nodos ABIERTOS
	priority_queue<nodoCoste> abiertos;

	//Lista de nodos CERRADOS
	set<estadoCoste,ComparaEstadosCoste4> explorados;

	//El nodo actual se corresponde con el origen
	nodoCoste nodoMenorCoste;
  	nodoMenorCoste.st.fila = origen.fila;
  	nodoMenorCoste.secuencia.empty();
  	nodoMenorCoste.st.columna=origen.columna;
  	nodoMenorCoste.st.orientacion=origen.orientacion;
    nodoMenorCoste.st.accesorios=0;
	nodoMenorCoste.st.gasto=0;

	abiertos.push(nodoMenorCoste);


	while(!abiertos.empty() && sigue){
        abiertos.pop();
		//Insertamos en la lista de nodos creados, el nodo de menor coste
		explorados.insert(nodoMenorCoste.st);

		if(esSolucionCoste(nodoMenorCoste.st,destino)){
			sigue=false;
		}
		else{
			//Procedemos a expandir el nodo de menor coste
			// Generar descendiente de girar a la derecha
			nodoCoste hijoTurnR = nodoMenorCoste;
			hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
			hijoTurnR.st.gasto+=costeEstado4(hijoTurnR.st);
			if (explorados.find(hijoTurnR.st) == explorados.end()){
				hijoTurnR.secuencia.push_back(actTURN_R);
				abiertos.push(hijoTurnR);
            }


			// Generar descendiente de girar a la izquierda
			nodoCoste hijoTurnL = nodoMenorCoste;
			hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
			hijoTurnL.st.gasto+=costeEstado4(hijoTurnL.st);
			if (explorados.find(hijoTurnL.st) == explorados.end()){
				hijoTurnL.secuencia.push_back(actTURN_L);
				abiertos.push(hijoTurnL);
			}

			// Generamos el nodo de avanzar
			nodoCoste hijoForward = nodoMenorCoste;
			if (!HayObstaculoDelanteCoste(hijoForward.st)){
                hijoForward.st.gasto+=costeEstado4(hijoForward.st);
				if (explorados.find(hijoForward.st) == explorados.end()){
					hijoForward.secuencia.push_back(actFORWARD);
					abiertos.push(hijoForward);
				}
			}
			if(!abiertos.empty()){
                nodoMenorCoste=abiertos.top();
                while(explorados.find(nodoMenorCoste.st)!=explorados.end()){
                    abiertos.pop();
                    nodoMenorCoste=abiertos.top();
                }
			}
		}
	}

	//Ya se ha terminado la busqueda del plan
	cout << "Terminada la busqueda\n";
	//plan.pop_back();

	if (esSolucionCoste(nodoMenorCoste.st,destino)){
		cout << "Cargando el plan\n";
		plan = nodoMenorCoste.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}
	return false;

}

bool ComportamientoJugador::esSolucion(const estado &origen,const estado &destino){
	return origen.fila==destino.fila && origen.columna==destino.columna;
}

bool ComportamientoJugador::pathFinding_A_Estrella(const estado &origen, const estado &destino, list<Action> &plan){

    //Creamos una cola de proridad para ABIERTOS
    priority_queue<nodoDistancia> abiertos;

    //Creamos un set para CERRADOS
    set<estado,ComparaEstados> explorados;

    //Insertamos el origen a abiertos
    nodoDistancia actual;
    actual.st=origen;
    actual.distancia=distancia(origen,destino);
    abiertos.push(actual);

    bool sigue=true;

    while(abiertos.size()>0 && sigue){
        while(explorados.find(actual.st) != explorados.end()){
            abiertos.pop();
            actual = abiertos.top();
        }

        //Los aniadimos a cerrados
        explorados.insert(actual.st);

        //Pintamos el mapa con los sensores actuales
        //pintarMapa(sensores);


        //Si actual es destino
        if(esSolucion(actual.st,destino)){
            sigue=false;
			//cout <<"TENEMOS SOLUCION"<<endl;
        }

        else{
            // Generar descendiente de girar a la derecha
            nodoDistancia hijoTurnR = actual;
            hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
            if (explorados.find(hijoTurnR.st) == explorados.end()){
                hijoTurnR.secuencia.push_back(actTURN_R);
                hijoTurnR.distancia+=costeEstadoAEstrella(hijoTurnR.st)+distancia(hijoTurnR.st,destino);
                abiertos.push(hijoTurnR);
            }


            // Generar descendiente de girar a la izquierda
            nodoDistancia hijoTurnL = actual;
            hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
            if (explorados.find(hijoTurnL.st) == explorados.end()){
                hijoTurnL.secuencia.push_back(actTURN_L);
                hijoTurnL.distancia+=costeEstadoAEstrella(hijoTurnL.st)+distancia(hijoTurnL.st,destino);
                abiertos.push(hijoTurnL);
            }

            // Generamos el nodo de avanzar
            nodoDistancia hijoForward = actual;
            if (!HayObstaculoDelante(hijoForward.st)){
                if (explorados.find(hijoForward.st) == explorados.end()){
                    hijoForward.secuencia.push_back(actFORWARD);
                    hijoForward.distancia+=costeEstadoAEstrella(hijoForward.st)+distancia(hijoForward.st,destino);
                    abiertos.push(hijoForward);
                }
            }
        }

    }
    //Ya se ha terminado la busqueda del plan
	cout << "Terminada la busqueda\n";
	//plan.pop_back();

	if (!sigue){
		//cout << "Cargando el plan\n";
		plan = actual.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}
	return false;

}

//N-> ESTADO ACTUAL O->ESTADO OBJETIVO
int ComportamientoJugador::distancia(const estado &N,const estado &O){
    return sqrt((O.fila-N.fila)*(O.fila-N.fila)+(O.columna-N.columna)*(O.columna-N.columna));
}

bool ComportamientoJugador::casillaCorrecta(int i,int j){
    bool ok=true;
    if(i<0 || i>mapaResultado.size() || j<0 || j>mapaResultado.size()){
        ok=false;
    }
    return ok;
}

//BIKINI K
//ZAPATILLA D
void ComportamientoJugador::pintarMapa(Sensores sensores,bool &bikiniEncontrado,bool &zapatillasEncontrado,estado &bk,estado &zp){
    mapaResultado[sensores.posF][sensores.posC]=sensores.terreno[0];
    int contador=1;
    switch(actual.orientacion){
        case 0:
            for(int i=1;i<4;i++){
                for(int j=-1;j<2 && i==1;j++){
                    if(casillaCorrecta(sensores.posF-i,sensores.posC+j)){
                        if(mapaResultado[sensores.posF-i][sensores.posC+j]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF-i][sensores.posC+j]=sensores.terreno[contador];
                    }
                    contador++;
                }
                for(int j=-2;j<3 && i==2;j++){
                    if(casillaCorrecta(sensores.posF-i,sensores.posC+j)){
                        if(mapaResultado[sensores.posF-i][sensores.posC+j]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF-i][sensores.posC+j]=sensores.terreno[contador];
                    }
                    contador++;
                }
                for(int j=-3;j<4 && i==3;j++){
                    if(casillaCorrecta(sensores.posF-i,sensores.posC+j)){
                        if(mapaResultado[sensores.posF-i][sensores.posC+j]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF-i][sensores.posC+j]=sensores.terreno[contador];
                    }
                    contador++;
                }
            }

            break;
        case 1:
            for(int i=1;i<4;i++){
                for(int j=-1;j<2 && i==1;j++){
                    if(casillaCorrecta(sensores.posF+j,sensores.posC+i)){
                        if(mapaResultado[sensores.posF+j][sensores.posC+i]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+j][sensores.posC+i]=sensores.terreno[contador];;
                    }
                    contador++;
                }
                for(int j=-2;j<3 && i==2;j++){
                    if(casillaCorrecta(sensores.posF+j,sensores.posC+i)){
                         if(mapaResultado[sensores.posF+j][sensores.posC+i]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+j][sensores.posC+i]=sensores.terreno[contador];
                    }
                    contador++;
                }
                for(int j=-3;j<4 && i==3;j++){
                    if(casillaCorrecta(sensores.posF+j,sensores.posC+i)){
                        if(mapaResultado[sensores.posF+j][sensores.posC+i]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+j][sensores.posC+i]=sensores.terreno[contador];
                    }
                    contador++;
                }
            }

            break;
        case 2:
            for(int i=1;i<4;i++){
                for(int j=1;j>-2 && i==1;j--){
                    if(casillaCorrecta(sensores.posF+i,sensores.posC+j)){
                        if(mapaResultado[sensores.posF+i][sensores.posC+j]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+i][sensores.posC+j]=sensores.terreno[contador];
                    }
                    contador++;
                }
                for(int j=2;j>-3 && i==2;j--){
                    if(casillaCorrecta(sensores.posF+i,sensores.posC+j)){
                        if(mapaResultado[sensores.posF+i][sensores.posC+j]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+i][sensores.posC+j]=sensores.terreno[contador];
                    }
                    contador++;
                }
                for(int j=3;j>-4 && i==3;j--){
                    if(casillaCorrecta(sensores.posF+i,sensores.posC+j)){
                        if(mapaResultado[sensores.posF+i][sensores.posC+j]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+i][sensores.posC+j]=sensores.terreno[contador];
                    }
                    contador++;
                }
            }
            break;
        case 3:
            for(int i=1;i<4;i++){
                for(int j=1;j>-2 && i==1;j--){
                    if(casillaCorrecta(sensores.posF+j,sensores.posC-i)){
                        if(mapaResultado[sensores.posF+j][sensores.posC-i]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+j][sensores.posC+-i]=sensores.terreno[contador];
                    }
                    contador++;
                }
                for(int j=2;j>-3 && i==2;j--){
                    if(casillaCorrecta(sensores.posF+j,sensores.posC-i)){
                        if(mapaResultado[sensores.posF+j][sensores.posC-i]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+j][sensores.posC-i]=sensores.terreno[contador];
                    }
                    contador++;
                }
                for(int j=3;j>-4 && i==3;j--){
                    if(casillaCorrecta(sensores.posF+j,sensores.posC-i)){
                        if(mapaResultado[sensores.posF+j][sensores.posC-i]=='?'){
                            descubierto=true;
                        }
                        mapaResultado[sensores.posF+j][sensores.posC-i]=sensores.terreno[contador];
                    }
                    contador++;
                }
            }

            break;
        }
}



// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}

void ComportamientoJugador::esZapatillas(int fil,int col,bool &encontrado,estado &zp){
    if(mapaResultado[fil][col]=='D'){
        encontrado=true;
        zp.fila=fil;
        zp.columna=col;
    }
}

void ComportamientoJugador::esBikini(int fil,int col,bool &encontrado, estado &bk){
    if(mapaResultado[fil][col]=='K'){
        encontrado=true;
        bk.fila=fil;
        bk.columna=col;
    }
}

void ComportamientoJugador::esRecarga(int fil,int col,bool &encontrado, estado &bk){
    if(mapaResultado[fil][col]=='X'){
        encontrado=true;
        bk.fila=fil;
        bk.columna=col;
    }
}

 bool ComportamientoJugador::esSolucionCoste(const estadoCoste &st,const estado &destino){
    return (st.fila == destino.fila) && (st.columna == destino.columna);
 }

int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
