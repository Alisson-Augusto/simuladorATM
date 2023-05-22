#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LEITURA 0
#define ESCRITA 1

#define DEPOSITO 0
#define SAQUE    1

int main(){

    pid_t atm_pid, terminal2;
    int TOTAL_TERMINAIS = 1;
    int atm_para_servidor[TOTAL_TERMINAIS][2];
    

    // TODO: Gerar dinamicamente filhos
    int atm_atual = 0;
    if(pipe(atm_para_servidor[atm_atual]) == -1)
        perror("pipe cliente 1 com erro\n");
    
    atm_pid = fork();

    if(atm_pid < 0){
        perror("processo 1 com problemas\n");
    }
    else if(atm_pid == 0){
        // ATM
        printf("ATM %d\n", atm_atual);
        char *arquivo_atm[100];
        sprintf(arquivo_atm, "operacaoATM-%d.txt", atm_atual);
        char buffer[1024];

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
                perror("Arquivo da ATM 1 esta mal formatado!");
                exit(-1);
            }

            sleep(1);
            // printf("ATM-%d enviou solição ao servidor! -> %s\n", atm_atual, buffer);
            write(atm_para_servidor[atm_atual][ESCRITA], &buffer, sizeof(buffer));
        }

        fclose(arq);
        return 0;
    }
    else if(atm_pid > 0){
        // SERVIDOR!
        int TOTAL_CONTAS = 6;
        int contas[] = {0, 0, 0, 0, 0, 0};

        // Escuta todos os clientes (atms)
        for(int atm = 0; atm < TOTAL_TERMINAIS; atm++) {
            close(atm_para_servidor[atm][ESCRITA]);
            char *buffer[1024];
            
            // Servidor lê todas as instruções da atm, até que o read retorne 0 (EOF)
            while(read(atm_para_servidor[atm][LEITURA], &buffer, sizeof(buffer)) != 0) {
                printf("Servidor, recebeu instrução: %s\n", buffer);
            
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
                    printf("Servidor - realiza operação de deposito na conta %d\n", id);
                    contas[id] += valor;
                    break;
                case SAQUE:
                    if(contas[id] - valor < 0) {
                        printf("Impossível sacar da conta %d -> Motivo: saldo negativo, ignorando solicitação\n", id);
                        break;
                    }
                    printf("Servidor - realiza operação de saque na conta %d\n", id);
                    contas[id] -= valor;
                    break;
                default:
                    perror("Operação indefinida para ATM!");
                    exit(-1);
                }
            }

            close(atm_para_servidor[atm][LEITURA]);
        }


        return 0;
    }


    return 0;
}