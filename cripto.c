/* Bruno Esteves 17/0100863 */

#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"

#define TRUE 1

#define NEG 5  //numero de negociadores

#define DOWN 0.95 //a cada venda, o valor do ativo cai em 5%
#define UP 1.05 //a cada compra, o valor do ativo sobe em 5%

//1 lock para cada ativo

pthread_mutex_t lock_BTC = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_ETH = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_AXS = PTHREAD_MUTEX_INITIALIZER;

//valor inicial dos ativos
double valor_BTC = 200000;
double valor_ETH = 16000;
double valor_AXS = 700;


//lock para a permissao de negociar
pthread_mutex_t lock_PERMISSAO = PTHREAD_MUTEX_INITIALIZER;


pthread_cond_t cond_negociadores = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_gerador = PTHREAD_COND_INITIALIZER;



//valor inicial das permissoes
int permissions = 0;

//assinatura das funcoes
void *negociate(void *arg);
void *gerador();

int main(){

    //threads serao negociadores de ativos, tanto para compra quanto para venda
	pthread_t negociators[NEG];

	int i;
    int *id;

    /* criando negociadores */
    for (i = 0; i < NEG ; i++) {

	    id = (int *) malloc(sizeof(int));
        *id = i;
		pthread_create(&negociators[i], NULL, negociate, (void *) (id));

	}

    /* criando gerador */
	gerador();

	return 0;
}

void *negociate(void *arg){

	int i = *((int *) arg);

	while(TRUE){              

        //valor aleatorio para qual dos 3 ativos vai ser negociado
        int moeda = rand()%3;
        //valor aleatorio para qual operacao sera feita, compra ou venda
        int operation = rand()%2;

        //para prosseguir na negociacao, um negociador precisa pegar uma permissao de negociacao
        //para checar se existe alguma permissao para ser utilizada, pega o lock_PERMISSAO
        pthread_mutex_lock(&lock_PERMISSAO);


            //se nao houverem permissoes, o gerador de permissoes sera sinalizado para gerar permissoes
            while(permissions == 0){
                pthread_cond_signal(&cond_gerador);
                pthread_cond_wait(&cond_negociadores, &lock_PERMISSAO);
            }

            //quando o gerador finalizar seu processo, 
            //o gerador sinaliza para os negociadores que as pemissoes estao disponiveis

            permissions--;
            

        pthread_mutex_unlock(&lock_PERMISSAO);
        

        //agora que os negociadores ja pegaram suas permissoes
        //cada negociador vai negociar sua moeda escolhida

        //cada moeda so pode ser negociada por um negociador de cada vez
        //mas ativos diferentes podem ser negociados ao mesmo tempo
        //por nao terem a mesma regiao critica

            //Legenda para moeda:
            //BTC = 0
            //ETH = 1
            //AXS = 2

            //Legenda para operation:
            //BUY = 0
            //SELL = 1

            if(moeda == 0){

                pthread_mutex_lock(&lock_BTC);

                    if(operation == 0){
                        valor_BTC *= UP;
                        printf("NEGOCIADOR %d: Comprando BTC, novo valor: %.2lf\n", i, valor_BTC); 
                    }
                    else{
                        valor_BTC *= DOWN; 
                        printf("NEGOCIADOR %d: Vendendo BTC, novo valor: %.2lf\n", i, valor_BTC); 
                    }

                pthread_mutex_unlock(&lock_BTC);

            }
            else if(moeda == 1){
                
                pthread_mutex_lock(&lock_ETH);

                    if(operation == 0){
                        valor_ETH *= UP; 
                        printf("NEGOCIADOR %d: Comprando ETH, novo valor: %.2lf\n", i, valor_ETH); 
                    }
                    else{
                        valor_ETH *= DOWN; 
                        printf("NEGOCIADOR %d: Vendendo ETH, novo valor: %.2lf\n", i, valor_ETH); 
                    }

                pthread_mutex_unlock(&lock_ETH);

            }
            else if(moeda == 2){
                
                pthread_mutex_lock(&lock_AXS);

                    if(operation == 0){
                        valor_AXS *= UP; 
                        printf("NEGOCIADOR %d: Comprando AXS, novo valor: %.2lf\n", i, valor_AXS); 
                    }
                    else{
                        valor_AXS *= DOWN; 
                        printf("NEGOCIADOR %d: Vendendo AXS, novo valor: %.2lf\n", i, valor_AXS); 
                    }

                pthread_mutex_unlock(&lock_AXS);
                
            }

            sleep(5);


        }

    pthread_exit(0);
}

void *gerador(){
 
  while(1){
     
    pthread_mutex_lock(&lock_PERMISSAO);

        while(permissions > 0){
            pthread_cond_wait(&cond_gerador, &lock_PERMISSAO);
        }
        
        printf("----------------------------\nGERADOR: Acordando!\n");
        
        int new_permissions = rand()%10;
        permissions += new_permissions;

        printf("GERADOR: Gerei %d permissões!\n----------------------------\n", new_permissions);
        
        sleep(5);

        if(permissions > 0) pthread_cond_broadcast(&cond_negociadores);

    pthread_mutex_unlock(&lock_PERMISSAO);

   }

}