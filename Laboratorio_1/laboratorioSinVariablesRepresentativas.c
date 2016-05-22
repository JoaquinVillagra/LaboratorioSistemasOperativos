#include <stdio.h>
#include <string.h>  
#include <fcntl.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


struct parametros{
	int i;
	int d;
	int * A;
	int * B;
	int ene;
};

int max(int num1, int num2) {
   int result;
    if (num1 > num2)
      result = num1;
   else
      result = num2;
   return result; 
}
//void * calcular(void *inicial, void * expandido, int i, int d){
void * calcular (void  * solucion){
	printf ("Estoy en calcular");
	struct parametros *p;
	p = (struct parametros *)solucion;

	int * datosD= (int *)p->A;
	int i = p->i;
	int d = p->d;
	int j=0, k=i+d;
	int * datosC=(int*)p->B;
	int l =0;
	int N=p->ene;
	while (i<k){
		while(j<N){
			if (i==0 || i ==N-1 || j==0 || j==N-1){ // Condición de borde
			datosC[l*N+j]=datosD[i*N+j];}
			else {// cuatro condiciones : 
				datosC[l*N+j]= max(max(max(datosD[(i-1)*N+j],datosD[(i+1)*N+j]),max(datosD[i*N+j+1],datosD[i*N+j-1])),datosD[i*N+j]);
			}
			j=j+1;
			
		}
		j=0;
		i=i+1;
		l=l+1;
	}
	printf("termine la hebra que termina en : %d\n", i);
	return NULL;
}
void imprimirMatriz(int * Matriz, int largo, int N){
	int i =0,j=0;
   		while(i<largo){
   			while (j<N){
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
	int c;

  opterr = 0;
  while ((c = getopt (argc, argv, "i:O:N:H:D:")) != -1)
    switch (c)
      {
      case 'i':
        nombreEntrada = optarg;
        break;
      case 'O':
        nombreSalida= optarg;
        break;
      case 'N':
        N= atoi(optarg);
        break;
       case 'H':
        H=  atoi(optarg);
        break;
      case 'D':
        imprime=  atoi(optarg);
        break;
      case '?':
       if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }
      

      printf("i : %s, o : %s, n: %d, h: %d, imp: %d", nombreEntrada, nombreSalida, N, H, imprime);

	char* nombre = nombreEntrada;
	int leidos;
	int fichero =open ( nombre,  O_RDONLY,0644);
	if (fichero ==-1){
		printf ("Soy tonta\n");
	}else{
		printf("lo abrí\n");
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
		while (i<H){
			p[i].i = inicio;
			if(resto>0){
				p[i].d = D+1;
				resto=resto-1;
			}else p[i].d=D ;
			p[i].A = A;
			p[i].ene=N;
			p[i].B= (int*)malloc(sizeof(int)*p[i].d*N);
			pthread_create(& Hebras[i] , NULL , calcular , ( void *) &p[i]);
			
			inicio = inicio + p[i].d;
			i=i+1;
	}
		i=0;
		while(i<H){
			pthread_join ( Hebras[i] , NULL ) ;
			i=i+1;
		}
  		i=0;
  		int fd2;
  		fd2 = open(nombreSalida, O_WRONLY|O_CREAT|O_TRUNC, 0700);
		while(i<H){
			write(fd2, (void*) p[i].B, 4*p[i].d*N);
			i=i+1;
		}
		close(fd2);
		if (imprime==1){
			i=0;
		while(i<H){
			imprimirMatriz(p[i].B, p[i].d, N);
			printf("---------\n");
			i=i+1;
		}
	}
		
	}
   return 0;

	
}
