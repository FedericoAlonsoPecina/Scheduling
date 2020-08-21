// Jackson_Quita_Kernel_14_CollaConsola01.cpp : Defines the entry point for the console application.
//

//Viene de Jackson_Quita_Kernel_14_Collapsed.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define CIEN_MILLONES 100000000
#define MAX_JOBS 4000
#define MAX_PROFUNDIDAD 2400
#define MAX_HASHING 2000003
#define C_COMPRESION 10000
#define COTA_RESPALDO 100000

struct struct_job {
	int release;
	int time_processing; //time processing
	int due_date; //Tiempo donde el trabajo debe estar listo
};

struct struct_solucion {
	int job;
	int time_ini;
	int time_fin;
};

struct struct_back_track {
	int nivel;
	int pos_job;
};

struct struct_job_modificado {
short int num;
unsigned short int *r; //Arreglo de fechas de Release
struct_job_modificado *next;
};

//struct_job_modificado L_kernel[MAX_PROFUNDIDAD];
struct_job_modificado L_job_modificado[MAX_HASHING];
time_t inicio_global,final_global;
struct_job job[MAX_JOBS],job_modificado[MAX_JOBS];
struct_solucion sol[MAX_JOBS];
int num_jobs,orden_job[MAX_JOBS],bandera[MAX_JOBS],tardanza_maxima=0,kernel[MAX_JOBS],mejor_tardanza;
int due_date_kernel[MAX_JOBS];
//Variables para el conteo y parte del tiempo
double suma_acu=0;
int total=1,max_nivel=1,nivel_optimo=-1;
long int num_sol=1,max_hashing_usado=0;
long int numero[160];
long long int cont_nivel[MAX_PROFUNDIDAD]={0};
struct_solucion respaldo_sol[MAX_PROFUNDIDAD+1][MAX_JOBS];
int r[MAX_JOBS];
//CUenta los emergings job por nivel
int cont_emerging_jobs[MAX_PROFUNDIDAD+1][MAX_PROFUNDIDAD+1];
int cont_kernels_diferentes[MAX_PROFUNDIDAD+1][MAX_PROFUNDIDAD+1];
int nivel_kernel[MAX_PROFUNDIDAD][MAX_PROFUNDIDAD],tot_nivel_kernel[MAX_PROFUNDIDAD];
int max_collapsed=0;
int max_kernels=0;
double seg_kernels[MAX_PROFUNDIDAD];
//int cont_max=0;
int max_time=3600;
int ban_time=1;

int mayor(int a[],unsigned short int b[],int n_a,int n_b)
{
int i,ban=0;
if(n_a>n_b)
	ban=1;
if(n_a<n_b)
	ban=-1;
for(i=0;i<n_a && !ban;++i)
	{
	if(a[i]>b[i])
		ban=1;
	if(a[i]<b[i])
		ban=-1;
	}
if(ban==1)
	return 1;
else
	return 0;
}

int igual(int a[],unsigned short int b[],int n_a,int n_b)
{
int ban_igual=1,i;
if(n_a!=n_b)
	ban_igual=0;
for(i=0;i<n_a && ban_igual;++i)
	if(a[i]!=b[i])
		ban_igual=0;
return ban_igual;
}

int inserta02(struct_job_modificado &L,int tot,int r[])
{
struct_job_modificado *p,*q;
int ban_igual=0,i;
p=&L;
q=L.next;
while(q!=NULL && mayor(r,q->r,tot,q->num))
	{
	p=p->next;
	q=q->next;
	}
if(q!=NULL)
	ban_igual=igual(r,q->r,tot,q->num);
if(q==NULL || !ban_igual)
	{
	q=new struct_job_modificado;
	q->num=tot;
	q->r=new unsigned short int[tot];
	for(i=0;i<tot;++i)
		q->r[i]=r[i];
	q->next=p->next;
	p->next=q;
	++L.num;
	/*++cont_max;
	if(cont_max%100000==0)
		printf("%d ",cont_max);*/
	return 1;
	}
else
	return 0;
}

void inserta_nivel_kernel(int nivel,int tot,int r[])
{
int i;
for(i=0;i<tot;++i)
	nivel_kernel[nivel][i]=r[i];
tot_nivel_kernel[nivel]=tot;
}


int lee(char archivo[])
{
FILE *in;
char entrada[80];
int i;
strcpy(entrada,archivo);
strcat(entrada,".in");
in=fopen(entrada,"rb+");
if(in!=NULL)
	{
	numero[0]=0;
	fscanf(in,"%d",&num_jobs);
	for(i=0;i<num_jobs;++i)
		{
		fscanf(in,"%d%d%d",&job[i].release,&job[i].time_processing,&job[i].due_date);
		job_modificado[i]=job[i];
		bandera[i]=0;
		orden_job[i]=i;
		/*L_job_modificado[i].num=0;
		L_job_modificado[i].next=NULL;
		L_job_modificado[i].r=NULL;*/
		}
	fclose(in);

	return 1;
	}
else
	{
	printf("NO se encontró el arhivo %s\n",archivo);
	return 0;
	}
}

int encuentra_candidato(int time)
{
int i,ban=0,j_best=-1,time_menor=-1;
//Busca el primer trabajo disponible
for(i=0;i<num_jobs && !ban;++i)
	if(!bandera[i])
		{
		if(time_menor==-1)
			time_menor=job[i].release;
		else
			if(job[i].release<time_menor)
				time_menor=job[i].release;
		if(job[i].release<=time)
			{
			ban=1;
			j_best=i;
			}
		}
//Si no encuentra trabajo liberado en el tiempo especificado, busca el que tenga el menor tiempo
if(!ban) 
	{
	for(i=0;i<num_jobs;++i)
		if(!bandera[i] && time_menor==job[i].release)
			{
			if(j_best==-1)
				j_best=i;
			else
				{
				if(job[i].due_date<job[j_best].due_date)
					j_best=i;
				else
					if(job[i].due_date==job[j_best].due_date)
						if(job[i].time_processing>job[j_best].time_processing)
							j_best=i;
				}
			}
	}
else//En este caso si encontro un trabajo que se libero antes del tiempo especificado, 
	for(;i<num_jobs;++i)  //escogera el que su fecha de termino (due date), sea mas cercano
		if(!bandera[i] && job[i].release<=time)
			{
			if(job[i].due_date<job[j_best].due_date)
					j_best=i;
				else
					if(job[i].due_date==job[j_best].due_date)
						if(job[i].time_processing>job[j_best].time_processing)
							j_best=i;
			}
return j_best;
}

int encuentra_candidato_modificado(int time)
{
int i,ban=0,j_best=-1,time_menor=-1;
//Busca el primer trabajo disponible
for(i=0;i<num_jobs && !ban;++i)
	if(!bandera[i])
		{
		if(time_menor==-1)
			time_menor=job_modificado[i].release;
		else
			if(job_modificado[i].release<time_menor)
				time_menor=job_modificado[i].release;
		if(job_modificado[i].release<=time)
			{
			ban=1;
			j_best=i;
			}
		}
//Si no encuentra trabajo liberado en el tiempo especificado, busca el que tenga el menor tiempo
if(!ban) 
	{
	for(i=0;i<num_jobs;++i)
		if(!bandera[i] && time_menor==job_modificado[i].release)
			{
			if(j_best==-1)
				j_best=i;
			else
				{
				if(job_modificado[i].due_date<job_modificado[j_best].due_date)
					j_best=i;
				else
					if(job_modificado[i].due_date==job_modificado[j_best].due_date)
						if(job_modificado[i].time_processing>job_modificado[j_best].time_processing)
							j_best=i;
				}
			}
	}
else//En este caso si encontro un trabajo que se libero antes del tiempo especificado, 
	for(;i<num_jobs;++i)  //escogera el que su fecha de termino (due date), sea mas cercano
		if(!bandera[i] && job_modificado[i].release<=time)
			{
			if(job_modificado[i].due_date<job_modificado[j_best].due_date)
					j_best=i;
				else
					if(job_modificado[i].due_date==job_modificado[j_best].due_date)
						if(job_modificado[i].time_processing>job_modificado[j_best].time_processing)
							j_best=i;
			}
return j_best;
}

void Jackson()
{
int i,j,r1,r2,j1,j2,aux,time=0,tard_1;
//Ordena los trabajos de menor a mayor, de acuerdo a su tiempo de liberacion
for(i=0;i<num_jobs;++i)
	for(j=i+1;j<num_jobs;++j)
		{
		j1=orden_job[i];
		j2=orden_job[j];
		r1=job[j1].release;
		r2=job[j2].release;
		if(r1>r2)
			{//swap
			aux=orden_job[i];
			orden_job[i]=orden_job[j];
			orden_job[j]=aux;
			}
		}
//Solucion Jackson
for(i=0;i<num_jobs;++i)
	{
	j1=encuentra_candidato(time);
	if(time<job[j1].release)
		time=job[j1].release;
	sol[i].job=j1;
	sol[i].time_ini=time;
	time+=job[j1].time_processing;
	sol[i].time_fin=time;
	if(sol[i].time_fin>job[j1].due_date)
		{
		tard_1=sol[i].time_fin-job[j1].due_date;
		if(tard_1>tardanza_maxima)
			tardanza_maxima=tard_1;
		}
	bandera[j1]=1;
	}
}

int Jackson_parcial_modificado(int pos,int t_actual)
{
int i,j1,time=0,tard_1,t_max;
if(pos)
	{
	j1=sol[pos-1].job;
	time=sol[pos-1].time_fin;
	}
t_max=t_actual;
//Solucion Jackson
for(i=pos;i<num_jobs;++i)
	{
	j1=encuentra_candidato_modificado(time);
	if(time<job_modificado[j1].release)
		time=job_modificado[j1].release;
	sol[i].job=j1;
	sol[i].time_ini=time;
	time+=job_modificado[j1].time_processing;
	sol[i].time_fin=time;
	//if(sol[i].time_fin>job_modificado[j1].due_date)
		{
		tard_1=sol[i].time_fin-job_modificado[j1].due_date;
		if(tard_1>t_max)
			t_max=tard_1;
		}
	bandera[j1]=1;
	}
return t_max;
}


int calcula_ceros(long int num)
{
long int limite=10000000;
int ceros=0,ban=0;
for(ceros=0;limite && !ban;++ceros)
	if(num>=limite)
		ban=1;
	else
		limite/=10;
return ceros-1;
}

void imprime(char archivo[])
{
FILE *out;
char salida[80],char_numero[30];
int i,j,j1,tard_1,num_ceros;
strcpy(salida,archivo);
strcat(salida,"_");
ltoa(num_sol,char_numero,10);
++num_sol;
strcat(salida,char_numero);
strcat(salida,".out");
out=fopen(salida,"wb+");
if(out!=NULL)
	{
	j1=sol[0].job;
	tardanza_maxima=sol[0].time_fin-job[j1].due_date;
	fprintf(out,"%d %d %d %d %d %d\r\n",sol[0].job,sol[0].time_ini,sol[0].time_fin,sol[0].time_fin-job[j1].due_date,job[j1].due_date,kernel[j1]);
	//fprintf(out,"%d %d %d %d\r\n",sol[0].job,sol[0].time_ini,sol[0].time_fin,tardanza_maxima);
	for(i=1;i<num_jobs;++i)
		{
		j1=sol[i].job;
		tard_1=sol[i].time_fin-job[j1].due_date;
		fprintf(out,"%d %d %d %d %d %d\r\n",sol[i].job,sol[i].time_ini,sol[i].time_fin,sol[i].time_fin-job[j1].due_date,job[j1].due_date,kernel[j1]);
		//fprintf(out,"%d %d %d %d\r\n",sol[i].job,sol[i].time_ini,sol[i].time_fin,tard_1);
		if(tardanza_maxima<tard_1)
			tardanza_maxima=tard_1;
		}
	fprintf(out,"Tardanza_maxima %d\r\n",tardanza_maxima);
	fprintf(out,"Numero de combinación \r\n");
	fprintf(out,"%ld",numero[total-1]);
	for(i=total-2;i>=0;--i)
		{
		num_ceros=calcula_ceros(numero[i]);
		for(j=0;j<num_ceros;++j)
			fprintf(out,"0");
		fprintf(out,"%ld",numero[i]);
		}
	fclose(out);
	}
}

void destruye_lista(struct_job_modificado &L)
{
struct_job_modificado *p;
p=L.next;
while(p!=NULL)
	{
	L.next=p->next;
	delete p;
	p=L.next;
	}
L.num=0;
}

//Esta funcion encuentra el kernel, detecta si la 
//Tambien guarda el kernel
int encuentra_kernel_job_modificado(int &r_k,int &pos,int &due_date,int tardanza_maxima,int &pos_job_overflow,int &inicio_kernel,int nivel)
{
int i,j,k1,ban,tard_1,j1,j2,j_ant,num_kernels=0,ban_optimo,primero,j_min,time_min,primero_r_k=1,pos_j_min=-1;
int pos_empieza_kernel=0,ban_calcula_inicio_kernel=0,r_tot;
r_k=-1;
num_kernels=0;
for(i=0;i<num_jobs;++i)
	kernel[i]=0;
primero=1;
for(i=0;i<num_jobs;++i)
	{
	j1=sol[i].job;
	tard_1=sol[i].time_fin-job_modificado[j1].due_date;
	if(tard_1==tardanza_maxima)
		{
		ban_calcula_inicio_kernel=0;
		if(primero)
			{
			pos_job_overflow=i;
			inicio_kernel=sol[i].time_ini;
			ban_calcula_inicio_kernel=1;
			primero=0;
			}
		j_ant=j1;
		++num_kernels;
		due_date_kernel[num_kernels]=job_modificado[j1].due_date;
		kernel[j1]=num_kernels;
		ban=1;
		for(j=i-1;j>=0 && ban;--j)
			{
			j2=sol[j].job;
			if(job_modificado[j2].due_date<=job_modificado[j1].due_date)
				{
				if(sol[j+1].time_ini==sol[j].time_fin)
					{
					kernel[j2]=num_kernels;
					if(ban_calcula_inicio_kernel)
						if(inicio_kernel>sol[j].time_ini)
							inicio_kernel=sol[j].time_ini;
					}
				else
					ban=0;
				}
			else
				ban=0;
			}
		}
	}
//Verifica si la solucion es optima o no
destruye_lista(L_job_modificado[nivel]);
ban_optimo=0;
for(k1=1;k1<=num_kernels && !ban_optimo;++k1)
	{
	primero=1;
	for(i=0;i<num_jobs;++i)
		{
		j1=sol[i].job;
		if(kernel[j1]==k1)
			if(primero)
				{
				j_min=j1;
				pos_j_min=i;
				pos_empieza_kernel=i;
				time_min=sol[i].time_ini;
				primero=0;
				}
			else
				{
				if(job_modificado[j1].release<job_modificado[j_min].release)
					{
					j_min=j1;
					pos_j_min=i;
					}
				}
		}
	if(primero)
		continue;
	r_tot=0;
	for(i=0;i<num_jobs;++i)
		{
		j1=sol[i].job;
		if(kernel[j1]==k1)
			{
			r[r_tot]=j1;
			++r_tot;
			}
		}
	inserta_nivel_kernel(nivel,r_tot,r);
	if(time_min==job_modificado[j_min].release)
		{
		ban_optimo=1;
		}
	else
		if(primero_r_k)
			{
			r_k=job_modificado[j_min].release;
			pos=pos_empieza_kernel;
			due_date=due_date_kernel[k1];
			primero_r_k=0;
			}	

	}
if(!num_kernels)
	ban_optimo=1;
return ban_optimo;
}

void suma_uno()
{
int i,acarreo=1;
long int num_act;
for(i=0;i<total && acarreo;++i)
	{
	num_act=numero[i]+acarreo;
	numero[i]=num_act%CIEN_MILLONES;
	acarreo=num_act/CIEN_MILLONES;
	}
if(acarreo)
	{
	++total;
	numero[i]=acarreo;
	}
}

long int busca_diferencias02(struct_job nuevo[],int &tot)
{
static int primero=1;
int i,cont=0;
long int hash_1;
long long int sum=0;
for(i=0;i<num_jobs;++i)
	if(nuevo[i].release!=job[i].release)
		{
		r[cont]=nuevo[i].release-job[i].release;
		r[cont]+=C_COMPRESION*i;
		sum+=r[cont]*job[i].release;
		++cont;
		}
tot=cont;
sum+=cont*10000;
hash_1=sum%MAX_HASHING;
if(hash_1<0)
	{
	if(primero)
		{
		printf("Algo hay muy mal con hash_1, Salio negativo");
		primero=0;
		}
	hash_1*=-1;
	}
return hash_1;
}

int encuentra_pos_min_cero(int nivel,int r[])
{
int pos_min=-1,i;
for(i=0;i<=nivel && pos_min==-1;++i)
	if(r[i]==0)
		pos_min=i;
return pos_min;
}

int encuentra_interseccion(int n1,unsigned short int r1[],int n2,unsigned short int r2[])
{
int i,j,ban=0;
for(i=0;i<n1 && !ban;++i)
	for(j=0;j<n2 && !ban;++j)
		if(r1[i]==r2[j])
			ban=1;
return ban;
}

int encuentra_interseccion(int niv_1,int niv_2)
{
int i,j,n1,n2,ban=0;
n1=tot_nivel_kernel[niv_1];
n2=tot_nivel_kernel[niv_2];
for(i=0;i<n1 && !ban;++i)
	for(j=0;j<n2 && !ban;++j)
		if(nivel_kernel[niv_1][i]==nivel_kernel[niv_2][j])
			ban=1;
return ban;
}

int cuenta_elementos_en_comun(int niv_1,int niv_2)
{
int i,j,n1,n2,cont=0;
n1=tot_nivel_kernel[niv_1];
n2=tot_nivel_kernel[niv_2];
for(i=0;i<n1;++i)
	for(j=0;j<n2;++j)
		if(nivel_kernel[niv_1][i]==nivel_kernel[niv_2][j])
			++cont;
return cont;
}

void imprime_collapsed(int nivel,char archivo[])
{
FILE *out;
static int num_corrida=1;
int i,j,n1;
char numero[20],salida[100];
strcpy(salida,archivo);
strcat(salida,"_colapsed_");
itoa(max_collapsed,numero,10);
strcat(salida,numero);
strcat(salida,".out");
out=fopen(salida,"wb+");
if(out!=NULL)
	{
	fprintf(out,"Nivel %d Max_Collapsed %d\r\n",nivel,max_collapsed);
	for(i=0;i<=nivel;++i)
		{
		n1=tot_nivel_kernel[i];
		for(j=0;j<n1;++j)
			fprintf(out,"%d ",nivel_kernel[i][j]);
		fprintf(out,"\r\n");
		if(i<nivel)
			{
			for(j=0;j<num_jobs;++j)
				fprintf(out,"(%d,%d)",respaldo_sol[i][j].job,respaldo_sol[i][j].time_ini);
			fprintf(out,"\r\n");
			}
		else
			{
			for(j=0;j<num_jobs;++j)
				fprintf(out,"(%d,%d)",sol[j].job,sol[j].time_ini);
			fprintf(out,"\r\n");
			}

		}
	fclose(out);
	}
else
	printf("No se pudo crear un archivo collapsed\n");
}

void imprime_num_kernels(int nivel,char archivo[])
{
FILE *out;
static int num_corrida=1;
int i,j,n1;
char numero[20],salida[100];
strcpy(salida,archivo);
strcat(salida,"_kernels");
//itoa(max_collapsed,numero,10);
//strcat(salida,numero);
strcat(salida,".out");
out=fopen(salida,"wb+");
if(out!=NULL)
	{
	fprintf(out,"MAX_KERNELS %d\r\n",max_kernels);
	for(i=0;i<max_kernels;++i)
		{
		fprintf(out,"%d %lg",i+1,seg_kernels[i]);
		fprintf(out,"\r\n");
		}
	fclose(out);
	}
else
	printf("No se pudo crear un archivo collapsed\n");
}


int encuentra_kernels_diferentes(int nivel,char archivo[])
{
double segundos;
int i,j,pos_min,cont=0,ban,collapsed_01=0,n1,n2;
for(i=0;i<=nivel;++i)
	r[i]=0;
pos_min=encuentra_pos_min_cero(nivel,r);
while(pos_min!=-1)
	{
	++cont;
	r[pos_min]=1;
	//p=L_kernel[pos_min].next;
	for(i=pos_min+1;i<=nivel;++i)
		if(!r[i])
			{
		//	q=L_kernel[i].next;
			ban=encuentra_interseccion(pos_min,i);
			if(ban)
				r[i]=1;
			}
	pos_min=encuentra_pos_min_cero(nivel,r);
	}
++cont_kernels_diferentes[nivel][cont];
if(cont>max_kernels)
	{
	max_kernels=cont;
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);
	seg_kernels[cont-1]=segundos;
	imprime_num_kernels(nivel,archivo);
	//printf("\El tiempo en segundos fue de: %10.1f\r\n",segundos+suma_acu);
	}

//Busca los kernels collapsed
/*for(i=0;i<=nivel;++i)
	{
	n1=tot_nivel_kernel[i];
	for(j=i+1;j<=nivel;++j)
		{
		n2=tot_nivel_kernel[j];
		cont=cuenta_elementos_en_comun(i,j);
		if(n1==n2+1 && cont==n2) //Si n1 es mayor que n2 en exactamente un elemento y hay n2 elementos en comun
			++collapsed_01;
		}
	}
if(collapsed_01>max_collapsed)
	{
	max_collapsed=collapsed_01;
	imprime_collapsed(nivel,archivo);
	}*/
return cont;
}

void imprime_respaldo(char archivo[])
{
FILE *out;
char salida[80];
double segundos;
int i,j,num_ceros;
strcpy(salida,archivo);
strcat(salida,"_recu.rec_1");
out=fopen(salida,"wb+");
if(out!=NULL)
	{
	final_global = time(NULL);
	segundos = difftime(final_global,inicio_global);
	fprintf(out,"Archivo %s \r\n",archivo);
	fprintf(out,"Total Combinaciones  \r\n");
	fprintf(out,"%ld",numero[total-1]);
	for(i=total-2;i>=0;--i)
		{
		num_ceros=calcula_ceros(numero[i]);
		for(j=0;j<num_ceros;++j)
			fprintf(out,"0");
		fprintf(out,"%ld",numero[i]);
		}
	fprintf(out,"\r\nEl tiempo en segundos fue de: %10.1f\r\n",segundos+suma_acu);
	fprintf(out,"Estadisticas de nivel:\r\n");
	for(i=0;i<=max_nivel;++i)
		fprintf(out,"Nivel %d Nodos %lld\r\n",i,cont_nivel[i]);
	fprintf(out,"El maximo nivel de hash_1 %ld\r\n",max_hashing_usado);
	fprintf(out,"El nivel donde encontro el optimo fue %d\r\n",nivel_optimo);
	fprintf(out,"El máximo nivel de profundidad fue %d\r\n",max_nivel);
	fprintf(out,"La mejor tardanza fue %d\r\n",mejor_tardanza);
	fprintf(out,"MATRIZ NIVEL-EMERGINGS JOBS\r\n");
	for(i=0;i<=max_nivel;++i)
		{
		for(j=0;j<=i;++j)
			fprintf(out,"%d ",cont_emerging_jobs[i][j]);
		fprintf(out,"\r\n");
		}
	fprintf(out,"MATRIZ KERNELS DIFERENTES\r\n");
	for(i=0;i<=max_nivel;++i)
		{
		for(j=0;j<=i+1;++j)
			fprintf(out,"%d ",cont_kernels_diferentes[i][j]);
		fprintf(out,"\r\n");
		}

	fclose(out);
	}
}

void busca(int pos,int nivel,char archivo[])
{
double segundos;
int cota=-1000,j1,tard_1,ban,r_k,pos02,due_date,respaldo,pos_job_overflow,mejora,ban_hueco=0,inicio_kernel,tot;
long int hash_1,i,j;
suma_uno();
++cont_nivel[nivel];
if(nivel>max_nivel)
	{
	max_nivel=nivel;
	//printf("Max nivel %d ",max_nivel);
	}
if(nivel>MAX_PROFUNDIDAD)
	return;
if(numero[0]%COTA_RESPALDO==0)
	imprime_respaldo(archivo);
//Aqui hace otro chequeo de mejora, sin embargo, ahorro muuuy poco en iteraciones y añade tiempo de procesamiento, no conviene utilizarlo
/*cota=job_modificado[0].release+job_modificado[0].time_processing-job_modificado[0].due_date;
for(i=1;i<num_jobs;++i)
	{
	tard_1=job_modificado[i].release+job_modificado[i].time_processing-job_modificado[i].due_date;
	if(tard_1>cota)
		cota=tard_1;
	}
if(cota>mejor_tardanza)
	return;*/
//Primero verifica que sea posible mejorar (o al menos no empeorar) la tardanza
for(i=0;i<num_jobs;++i)
	{
	bandera[i]=0;
	sol[i]=respaldo_sol[nivel-1][i];
	}
if(pos)
	{
	j1=sol[0].job;
	bandera[j1]=1;
	cota=sol[0].time_fin-job_modificado[j1].due_date;
//	cota=job_modificado[j1].release+job_modificado[j1].time_processing-job_modificado[j1].due_date;
	}
for(i=1;i<pos;++i)
	{
	j1=sol[i].job;
	bandera[j1]=1;
	tard_1=sol[i].time_fin-job_modificado[j1].due_date;
	//tard_1=job_modificado[j1].release+job_modificado[j1].time_processing-job_modificado[j1].due_date;
	if(tard_1>cota)
		cota=tard_1;
	}
//if(cota>mejor_tardanza)
	//return;  //Aqui termina la parte de la verificacion de la tardanza posible
tard_1=Jackson_parcial_modificado(pos,cota);
//if(nivel>50 || numero[0]%1000==0)
	//i=0;
if(tard_1<mejor_tardanza)
	{
	mejor_tardanza=tard_1;
	imprime(archivo);
	printf("La tardanza encontrada es %d\n",mejor_tardanza);
	nivel_optimo=nivel;
	}
ban=encuentra_kernel_job_modificado(r_k,pos02,due_date,tard_1,pos_job_overflow,inicio_kernel,nivel);
encuentra_kernels_diferentes(nivel,archivo);
//mejora=sol[pos_job_overflow].time_ini-r_k;
mejora=inicio_kernel-r_k;
if(tard_1-mejora>=mejor_tardanza) //No sera posible mejorar la solucion
	return;
if(!ban)
		{
		for(i=0;i<num_jobs;++i)
			respaldo_sol[nivel][i]=sol[i];
		//Trata de modificar, uno a uno los trabajos
		for(i=pos02-1;i>=0 && !ban_hueco && ban_time;--i)
			{
			final_global = time(NULL);
			segundos = difftime(final_global,inicio_global);
			if(segundos>max_time-1)
				ban_time=0;
			j1=respaldo_sol[nivel][i].job;
			if(respaldo_sol[nivel][i].time_fin<respaldo_sol[nivel][i+1].time_ini)
				ban_hueco=1;
			if(job_modificado[j1].due_date>due_date && !ban_hueco)
				{
				respaldo=job_modificado[j1].release;
				job_modificado[j1].release=r_k;
				hash_1=busca_diferencias02(job_modificado,tot);
				if(hash_1>=max_hashing_usado)
					{
					for(j=max_hashing_usado;j<=hash_1;++j)
						{
						L_job_modificado[j].next=NULL;
						L_job_modificado[j].num=0;
						L_job_modificado[j].r=NULL;
						}
					max_hashing_usado=hash_1+1;
					}
				ban=inserta02(L_job_modificado[hash_1],tot,r);
				if(ban)
					{
					++cont_emerging_jobs[nivel+1][tot];
					busca(i,nivel+1,archivo);
					}
				job_modificado[j1].release=respaldo;
				}
			//else
				//break;
			}
		}
}

void procesa(char archivo[])
{
double segundos;
int j1,r_k,ban,pos,respaldo,due_date,pos_job_overflow,ban_hueco=0,inicio_kernel,tot;
long int hash_1,i,j;
Jackson();
mejor_tardanza=tardanza_maxima;
suma_uno();
imprime(archivo);
printf("La tardanza encontrada es %d\n",mejor_tardanza);
for(i=0;i<MAX_PROFUNDIDAD;++i)
	{
//	L_kernel[i].num=0;
	//L_kernel[i].next=NULL;
	for(j=0;j<MAX_PROFUNDIDAD;++j)
		{
		cont_emerging_jobs[i][j]=0;
		cont_kernels_diferentes[i][j]=0;
		}
	}
ban=encuentra_kernel_job_modificado(r_k,pos,due_date,mejor_tardanza,pos_job_overflow,inicio_kernel,0);
encuentra_kernels_diferentes(0,archivo);
if(!ban)
	{
	cont_nivel[0]=1;
	//Respalda la solucion
	for(i=0;i<num_jobs;++i)
		respaldo_sol[0][i]=sol[i];
	//Trata de modificar, uno a uno los trabajos
	for(i=pos-1;i>=0 && !ban_hueco && ban_time;--i)
		{
		final_global = time(NULL);
		segundos = difftime(final_global,inicio_global);
		if(segundos>max_time-1)
			ban_time=0;
		j1=respaldo_sol[0][i].job;
		if(respaldo_sol[0][i].time_fin<respaldo_sol[0][i+1].time_ini)
			ban_hueco=1;
		if(job_modificado[j1].due_date>due_date && !ban_hueco)
			{
			respaldo=job_modificado[j1].release;
			job_modificado[j1].release=r_k;
			hash_1=busca_diferencias02(job_modificado,tot);
			if(hash_1>=max_hashing_usado)
				{
				for(j=max_hashing_usado;j<=hash_1;++j)
					{
					L_job_modificado[j].next=NULL;
					L_job_modificado[j].num=0;
					L_job_modificado[j].r=NULL;
					}
				max_hashing_usado=hash_1+1;
				}
			ban=inserta02(L_job_modificado[hash_1],tot,r);
			if(ban)
				{
				++cont_emerging_jobs[1][tot];
				busca(i,1,archivo);
				}
			job_modificado[j1].release=respaldo;
			}
		//else
			//break;
		}
	}
}

int main(int argc,char *argv[])
{
FILE *out;
char archivo[160]="V2.toy005.xml",salida[160]="V2.toy005_SOL.xml";  //fact//
double segundos;
int ban,i,j,ban_archivo=0,num_ceros;
inicio_global = time(NULL);  //Toma el tiempo al inicio
//Primero procesa los argumentos
if(argc==1 && ((*argv)[1])!='-')
	{
	printf("\n Wrong Syntax: ./executable -p instance_filename");
	exit(1);
	}
else
	if(argc>=2 && strcmp(argv[1], "-name")==0)
		{
		printf("\n ID_Team S7\n");
		exit(1);
		}
	else 
		for(i=1;i<argc;++i)
			{
			if(!_strcmpi(argv[i],"-p"))
				{
				strcpy(archivo,argv[i+1]);
				++i;
				ban_archivo=1;
				}
			else
				if(!_strcmpi(argv[i],"-t"))
						{
						max_time=atoi(argv[i+1]);
						++i;
						ban_time=1;
						}
			}
if(ban_archivo)	
	{
	inicio_global = time(NULL);  //Toma el tiempo al inicio
	ban=lee(archivo);
	if(ban)
		{
		procesa(archivo);
	//for(i=0;i<num_jobs;++i)
		//printf("%d ",sol[i].job);
		strcpy(salida,"EXA_JACK_");
		strcat(salida,archivo);
		strcat(salida,".out");
		final_global = time(NULL);
		segundos = difftime(final_global,inicio_global);
		out=fopen(salida,"wb+");
		fprintf(out,"Archivo %s \r\n",archivo);
		fprintf(out,"Total Combinaciones  \r\n");
		fprintf(out,"%ld",numero[total-1]);
		for(i=total-2;i>=0;--i)
			{
			num_ceros=calcula_ceros(numero[i]);
			for(j=0;j<num_ceros;++j)
				fprintf(out,"0");
			fprintf(out,"%ld",numero[i]);
			}
		fprintf(out,"\r\nEl tiempo en segundos fue de: %10.1f\r\n",segundos+suma_acu);
		fprintf(out,"Estadisticas de nivel:\r\n");
		for(i=0;i<=max_nivel;++i)
			fprintf(out,"Nivel %d Nodos %lld\r\n",i,cont_nivel[i]);
		fprintf(out,"El maximo nivel de hash_1 %ld\r\n",max_hashing_usado);
		fprintf(out,"El nivel donde encontro el optimo fue %d\r\n",nivel_optimo);
		fprintf(out,"El máximo nivel de profundidad fue %d\r\n",max_nivel);
		fprintf(out,"La mejor tardanza fue %d\r\n",mejor_tardanza);
		fprintf(out,"MATRIZ NIVEL-EMERGINGS JOBS\r\n");
		for(i=0;i<=max_nivel;++i)
			{
			for(j=0;j<=i;++j)
				fprintf(out,"%d ",cont_emerging_jobs[i][j]);
			fprintf(out,"\r\n");
			}
		fprintf(out,"MATRIZ KERNELS DIFERENTES\r\n");
		for(i=0;i<=max_nivel;++i)
			{
			for(j=0;j<=i+1;++j)
				fprintf(out,"%d ",cont_kernels_diferentes[i][j]);
			fprintf(out,"\r\n");
			}
		fclose(out);
		printf("\nLa tardanza maxima fue %d\n",mejor_tardanza);
		printf("El máximo nivel de profundidad fue %d\n",max_nivel);	
		}
	}
return 0;
}
