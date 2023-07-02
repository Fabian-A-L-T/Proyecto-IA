#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace chrono;

struct Contained
{
    const vector<int>& _sequence;
    Contained(const vector<int> &vec) : _sequence(vec) {}
    bool operator()(int i) const 
    { 
        return _sequence.end() != find(_sequence.begin(), _sequence.end(), i);
    }
};

/**********************************************
 * Funcion que calcula las variables que estan conectadas a xi.
 * ---------------------------
 * int i = variable xi a revisar
 * int n = tamano del problema
 * vector<int> xi_constraints = vector donde se guardaran las variables conectadas a xi
***********************************************/
void constraint_compute(int i, int n, vector<int> & xi_constraints){

    for(int index = 0; index < n; index++){
        if(index != i){
            xi_constraints.push_back(index);
        }
    }
    xi_constraints.push_back(-1); //Para saber si el conjunto esta vacio
}

/**********************************************
 * Funcion para saber si el valor asignado es consistente (no hay fallo)
 * ---------------------------
 * int i = variable xi a revisar
 * int n = tamano del problema
 * vector<int> xi = vector que contiene las instancias de las variables
***********************************************/
bool isConsistent(int i, int n, vector<int> xi){

    for(int index=0;index<n;index++){
        if(xi[index] == xi[i] && index != i){
            return false;
        }
    }
    return true;

}

/**********************************************
 * Funcion para asignar un valor a una variable
 * ---------------------------
 * int i = posicion de la variable a revisar
 * int n = tamano del problema
 * vector<int> xi = vector que contiene los valores instanciados de las variables
 * vector<int> xi_domain = vector que contiene el dominio de cada variable
***********************************************/
int assignValue(int i, int n, vector<int> xi, vector<int> & xi_domain){
    while(xi_domain.size()>0){
        xi[i] = xi_domain[0];
        xi_domain.erase(xi_domain.begin());
        if(isConsistent(i,n,xi)){
            return xi[i];
        }
    }
    return -1;
}

/**********************************************
 * Funcion donde se aplica el backjump de GBJ
 * ---------------------------
 * int i = posicion de la variable a revisar
 * int n = tamano del problema
 * vector<vector<int>> constraint_set = vector donde se guardaran las variables conectadas a xi
 * vector<vector<int>> xi_domain = vector que contiene el dominio de cada variable
 * vector<int> xi = vector que contiene los valores instanciados de las variables
***********************************************/
void GBJ_backjump(long & i, int n, vector<vector<int>> & constraint_set, vector<vector<int>> & xi_domain, vector<int> & xi){
    int i_prev = i;
    int recent_index = -1;
    int temp = 0;

    //se busca el indice de la variable mas recientemente instanciada en constraint_set
    for(int k=0;k<constraint_set[i_prev].size();k++){
        if(constraint_set[i_prev][k] > recent_index && constraint_set[i_prev][k] < i_prev){
            recent_index = constraint_set[i_prev][k];
            temp = k;
        }
    }
    i = recent_index; //Se hace el backjump a la variable mas recientemente instanciada segun el grafo de restricciones(constraint_set) (el elemento mas recientemente instanciado)

    //Se restaura el conjunto de variables desde el  del punto hacia donde se realiza el backjump.
    if(i != -1){
        for(int k=i; k<n; k++){
            xi[k] = -1;
        }
        //En esta seccion se actualiza la lista de constraint_set de la variable actual (despues del backjump) realizando la union de los conjuntos de
        //constraint_set de la variable anterior y la actual, excluyendo el constraint_set utilizado para hacer el backjump
        remove_copy_if(constraint_set[i_prev].begin(), constraint_set[i_prev].end(), back_inserter(constraint_set[i]), Contained(constraint_set[i]));
    }
}


/**********************************************
 * int n = tamaño del problema
 * vector<vector<int>> matrix1 = primera matriz(flujo) del problema
 * vector<vector<int>> matrix2 = segunda matriz(distancia) del problema
***********************************************/
int GBJ(int n, vector<vector<int>> matrix1, vector<vector<int>> matrix2, string filename){

    auto start = high_resolution_clock::now();

    long value=0, i=0, j=0, min_sum=INT32_MAX, sum=0;

    //Se genera el dominio de las variables
    auto domain = vector<int>();
    for(int index = 0; index < n;index++){
        domain.push_back(index);
    }

    auto xi = vector<int>(n, -1);                             //Vector donde se instanciaran las variables    
    auto best_xi = vector<int>(n, -1);                        //Vector donde se guardara la solucion mas optima encontrada                                    
    auto xi_domain = vector<vector<int>>(n, domain);          //Dominio de cada variable
    auto xi_domain_copy = xi_domain;                          //Copia del dominio
    auto constraint_set = vector<vector<int>>(n);             //Conjunto de las variables conectadas a xi por una restriccion      

    //En esta seccion se calculan las variables conectadas a xi por una restriccion
    for(int k=0;k<n;k++){
        constraint_compute(k,n,constraint_set[k]);
    }
    auto constraint_set_copy = constraint_set;                //Copia del conjunto de las variables conectadas a xi por una restriccion

    while(i>-1 && i<n){

        value = assignValue(i, n, xi, xi_domain_copy[i]);

        if(value == -1){
            GBJ_backjump(i,n,constraint_set_copy,xi_domain_copy,xi); //Si no quedan valores en el dominio, se hace backjump
            if(i == -1){
                break;
            }
        }
        else{

            //Se asigna el valor a la variable y se restaura la lista de ancestros de esa variable.
            xi[i] = value;
            
            if(i == n-1){
                sum = 0;
                for(int k = 0;k<n;k++){
                    for(int l = 0;l<n;l++){
                        sum += matrix1[xi[k]][xi[l]]*matrix2[k][l];
                    }
                }
                if(sum < min_sum){
                    min_sum = sum;
                    best_xi = xi;
                }
                GBJ_backjump(i,n,constraint_set_copy,xi_domain_copy,xi);
            }
            else{
                if(i != -1){
                    i++;
                }
                xi_domain_copy[i] = xi_domain[i];
                constraint_set_copy[i] = constraint_set[i];
            }
                
        }
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);

    string filename_out = filename.substr(0,filename.size()-4);
    ofstream fileout (filename_out + ".out");
    fileout << min_sum << " " << duration.count() << "s" << std::endl;
    fileout << n << std::endl;
    for(int index=0;index<n;index++){
        fileout << best_xi[index]+1 << " ";
    }
    fileout << std::endl;
    fileout.close();
    return min_sum;
}

int main(int argc, char* argv[]){

    //Vars
    string line;
    int n;// Tamano de la instancia

    //Argumentos
    if(argc == 1 || argc > 2){
        cout << "[Error] Se ha ingresado 0 o mas de 1 argumento\n   Se esperaba 'Nombre del archivo (instancia)'\n";
        return -1;
    }

    
    /**********************************************
     * Lectura y extraccion de datos del archivo
     **********************************************/
    fstream instance;
    instance.open(argv[1], ios::in);
    if(!instance){
        cout << "[Error] Archivo no encontrado\n";
        return -1;
    }
    instance >> n;

    auto m1 = vector<vector<int>>(n, vector<int>(n, 0));
    auto m2 = vector<vector<int>>(n, vector<int>(n, 0));

    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            instance >> m1[i][j];
        }
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            instance >> m2[i][j];
        }
    }
    instance.close();

    cout << "-Matrices extraidas\n";


    /**********************************************
    * Aplicacion de GBJ
    **********************************************/
    cout << "-Iniciando algoritmo\n";
    int result = GBJ(n, m1, m2, argv[1]);
    
    cout << "-Solucion encontrada: " << result << "\n";
    
    return 0;
}