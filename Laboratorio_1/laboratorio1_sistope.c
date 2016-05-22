#include <stdio.h>
#include <string.h>  
#include <fcntl.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


struct parametros{
	int inicioFila; //inicio de fila i
	int distancia_enFilas; //Cantidad de filas que tiene asignadas @distancia d
	int * matriz; //matriz completa A
	int * fragmentoMatrizAsignado; //espacio de memoria a leer B
	int largoMatriz; //largo matriz ene
};

int max(int numero1, int numero2) {
   int resultado;
    if (numero1 > numero2)
      resultado = numero1;
   else
      resultado = numero2;
   return resultado; 
}
/**
Función calcular: Función que determina la expansión de pixeles para cada valor de posición del rango de filas asignados.
**/
void * calcular (void  * solucion){
	struct parametros *parametroSolucion;
	parametroSolucion = (struct parametros *)solucion;

	int * datosMatriz= (int *)parametroSolucion->matriz;
	int inicioFila = parametroSolucion->inicioFila;
	int distancia_enFilas = parametroSolucion->distancia_enFilas;
	int j=0, distancia_a_Recorrer = inicioFila + distancia_enFilas;
	int * datosSegmentoMatriz=(int*)parametroSolucion->fragmentoMatrizAsignado;
	int l = 0;
	int largo = parametroSolucion->largoMatriz;
	while (inicioFila < distancia_a_Recorrer){
		while(j < largo){
			if (inicioFila==0 || inicioFila ==largo-1 || j==0 || j==largo-1)
			{ 
				// Condición de borde
				datosSegmentoMatriz[l*largo+j] = datosMatriz[inicioFila*largo+j];
			}
			else 
			{
				// cuatro condiciones : 
				datosSegmentoMatriz[l*largo+j]= max(max(max(datosMatriz[(inicioFila-1)*largo+j],datosMatriz[(inicioFila+1)*largo+j]),max(datosMatriz[inicioFila*largo+j+1],datosMatriz[inicioFila*largo+j-1])),datosMatriz[inicioFila*largo+j]);
			}
			j=j+1;
			
		}
		j=0;
		inicioFila=inicioFila+1;
		l = l + 1;
	}
	printf("termine la hebra que termina en : %d\n", inicioFila);
	return NULL;
}
void imprimirMatriz(int * Matriz, int largo, int N){
	int i =0, j=0;
   		while(i<largo){
   			while (j < N){
   				printf (" %d",Matriz[i*N+j]);
   				j=j+1;
   			}
   			j=0;
   			printf("\n");
   			i=i+1;
   		}
}

int main(int argc, char **argv)
{
	char * nombreEntrada;
	char* nombreSalida;
	int N;
	int H;
	int imprime; //Sólo 1 o 0
	int opciones;

  	opterr = 0;
  	while ((opciones = getopt (argc, argv, "i:O:N:H:D:")) != -1)
	    switch (opciones)
	    {
	      case 'i':
	        nombreEntrada = optarg; 
	        break;
	      case 'O':
	        nombreSalida = optarg;
	        break;
	      case 'N':
	        N = atoi(optarg); //validar numeros y si hay errores exit
	        break;
	       case 'H':
	        H =  atoi(optarg);
	        break;
	      case 'D':
	        imprime =  atoi(optarg);
	     	if(imprime!=0 || imprime!=1)
	     		abort();
	        break;
	      case '?':
	       if (isprint (optopt))
	          fprintf (stderr, "Opción desconocida`-%c'.\n", optopt);
	        else
	          fprintf (stderr, "Optiones de caracter desconocida `\\x%x'.\n",optopt);
	        return 1;
	      default:
	        abort ();
	    }
      	//printf("i : %s, o : %s, n: %d, h: %d, imp: %d", nombreEntrada, nombreSalida, N, H, imprime);
		char* nombre = nombreEntrada;
		int leidos;
		int fichero = open(nombre,  O_RDONLY,0644);
		if (fichero ==-1){
			printf ("Archivo indicado no existe\n");
			return 0;
		}
		else
		{
			//printf("lo abrí\n");
			int *A = (int*) malloc(sizeof(int)*N*N);
			leidos = read(fichero, (void *) A, sizeof(int)*N*N);
			close(fichero);
			imprimirMatriz(A, N, N);
			int D = (int)N/H;
			int resto = N%H;
			printf ("D : %d, mod: %d\n", D, resto);

			struct parametros p[H];
			pthread_t Hebras[H];
			int i=0;
			int inicio = 0;
			while (i < H)
			{
				p[i].inicioFila = inicio;
				if(resto > 0)
				{
					p[i].distancia_enFilas = D+1;
					resto=resto-1;
				}
				else p[i].distancia_enFilas=D;
				p[i].matriz = A;
				p[i].largoMatriz = N;
				p[i].fragmentoMatrizAsignado = (int*)malloc(sizeof(int)*p[i].distancia_enFilas*N);
				pthread_create(& Hebras[i] , NULL , calcular , ( void *) &p[i]);
				inicio = inicio + p[i].distancia_enFilas;
				i=i+1;
			}
			i=0;
			while(i<H)
			{
				pthread_join ( Hebras[i] , NULL ) ;
				i=i+1;
			}
	  		i = 0;
	  		int fd2;
	  		fd2 = open(nombreSalida, O_WRONLY|O_CREAT|O_TRUNC, 0700);
			while(i<H)
			{
				write(fd2, (void*) p[i].fragmentoMatrizAsignado, 4*p[i].distancia_enFilas*N);
				i=i+1;
			}
			close(fd2);
			if (imprime==1)
			{
				i=0;
				while(i < H)
				{
					imprimirMatriz(p[i].fragmentoMatrizAsignado, p[i].distancia_enFilas, N);
					printf("---------\n");
					i=i+1;
				}
			}
			
		}
   		return 0;	
}
