#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define COR_VERDE "\x1B[32m"
#define COR_RESET "\x1B[0m"

#define LEITURA 0
#define ESCRITA 1

#define DEPOSITO 0
#define SAQUE    1

void print_contas(int *contas, int total_contas) {
    printf("-------------------\n");
    printf("Relatório de Contas\n");
    for(int i=0; i < total_contas; i++) {
        printf("Cliente[%d] -> %d\n", i, contas[i]);
    }
    printf("-------------------\n");
}


int main(){

    int TOTAL_TERMINAIS = 4;
    int atm_para_servidor[TOTAL_TERMINAIS][2];

    for(int atm_atual = 0; atm_atual < TOTAL_TERMINAIS; atm_atual++) {
        // Cria pipe para atm comunicar com o servidor
        if(pipe(atm_para_servidor[atm_atual]) == -1){
            perror("pipe cliente com erro\n");
        }

        pid_t atm_pid = fork();

        if(atm_pid < 0){
            perror("Erro ao iniciar ATM\n");
            exit(-1);
        }
        else if(atm_pid == 0){
            // ATM
            printf("ATM %d em execução\n", atm_atual);
            char *arquivo_atm[100];
            sprintf(arquivo_atm, "operacaoATM-%d.txt", atm_atual);
            char buffer[20];

            close(atm_para_servidor[atm_atual][LEITURA]);
            FILE *arq = fopen(arquivo_atm, "r");
            
            if(arq == NULL){
                printf("erro na criacao do ponteiro para leitura do arquivo\n");
            }

            while(fgets(buffer, sizeof(buffer), arq)){
                // Envia solicitação para o servidor
                int operacao = -1; int id = -1; int valor = -1;
                sscanf(buffer, "%d,%d,%d", &operacao, &id, &valor);

                if(operacao == -1 || id == -1 || valor == -1) {
                    perror("Arquivo da ATM esta mal formatado!");
                    exit(-1);
                }

                sleep(2);
                printf("%sATM-%d enviou solição ao servidor! -> %s%s\n", COR_VERDE, atm_atual, buffer, COR_RESET);
                write(atm_para_servidor[atm_atual][ESCRITA], &buffer, sizeof(buffer));
            }

            fclose(arq);
            close(atm_para_servidor[atm_atual][ESCRITA]);
            return 0;
        }
    }

    // SERVIDOR!
    int TOTAL_CONTAS = 6;
    int contas[] = {0, 0, 0, 0, 0, 0};

    // Escuta todos os clientes (atms)
    for(int atm = 0; atm < TOTAL_TERMINAIS; atm++) {
        close(atm_para_servidor[atm][ESCRITA]);
        char buffer[20];
        
        printf("Servidor atualmente esta ouvindo ATM-%d\n", atm);
        // Servidor lê todas as instruções da atm, até que o read retorne 0 (EOF)
        while(1) {
            ssize_t status = read(atm_para_servidor[atm][LEITURA], buffer, sizeof(buffer));
            if(status == 0) {
                printf("Servidor terminou de ler ATM-%d!\n", atm);
                printf("Fechando canal de leitura da ATM-%d\n", atm);
                close(atm_para_servidor[atm][LEITURA]);
                break;
            }else if(status == -1) {
                printf("Erro ao ler pipe!");
                break;
            }

            printf("Servidor, recebeu da ATM-%d a instrução: %s\n", atm, buffer);
        
            // Decodifica instrução
            int operacao = -1; int id = -1; int valor = -1;
            sscanf(buffer, "%d,%d,%d", &operacao, &id, &valor);

            if(operacao == -1 || id < 0 || id > TOTAL_CONTAS || valor < 0) {
                perror("Arquivo da ATM 1 esta mal formatado!");
                exit(-1);
            }

            printf("Operacao: %d, Conta: %d, Valor: %d\n", operacao, id, valor);
            switch (operacao)
            {
            case DEPOSITO:
                // printf("Servidor - realiza operação de deposito na conta %d\n", id);
                contas[id] += valor;
                break;
            case SAQUE:
                if(contas[id] - valor < 0) {
                    printf("Impossível sacar da conta %d -> Motivo: saldo negativo, ignorando solicitação\n", id);
                    break;
                }
                // printf("Servidor - realiza operação de saque na conta %d\n", id);
                contas[id] -= valor;
                break;
            default:
                perror("Operação indefinida para ATM!");
                exit(-1);
            }
        }
    }
    print_contas(contas, TOTAL_CONTAS);
    return 0;
}