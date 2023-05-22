#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LEITURA 0
#define ESCRITA 1

#define DEPOSITO 0
#define SAQUE    1


int decodificar(char *linha, int *operacao, int *id, int *valor) {
    sscanf(linha, "%d,%d,%d", &operacao, &id, &valor);

    if(*operacao != 0 || *operacao != 1 || *id < 0 || *valor < 0) {
        perror("Erro ao decodificar instrução");
        return -1;
    }

    return 0;
}

int main() {
  int TOTAL_TERMINAIS = 1;
  int atm_para_servidor[TOTAL_TERMINAIS][2];
  

  // TODO: Gerar dinamicamente filhos
  int atm_atual = 0;
  // ATM
  printf("ATM %d\n", atm_atual);
  char *arquivo_atm[100];
  int r = sprintf(arquivo_atm, "operacaoATM-%d.txt", atm_atual);
  printf("%s\n", arquivo_atm);
  printf("Lendo arquivo da ATM%d -> %s\n", atm_atual, arquivo_atm);
  char buffer[1024];

  close(atm_para_servidor[atm_atual][LEITURA]);
  FILE *arq = fopen(arquivo_atm, "r");
  
  if(arq == NULL){
      printf("erro na criacao do ponteiro para leitura do arquivo\n");
  }

  printf("Arquivo: %s\n", arquivo_atm);
  while(fgets(buffer, sizeof(buffer), arq)){
      printf("Lendo arquivo: %s\n", buffer);
      // Envia solicitação para o servidor
      int operacao = -1; int id = -1; int valor = -1;
      sscanf(buffer, "%d,%d,%d", &operacao, &id, &valor);

      if(operacao == -1 || id == -1 || valor == -1) {
          perror("Arquivo da ATM 1 esta mal formatado!");
          exit(-1);
      }

      sleep(1);
      printf("ATM-%d enviou solição ao servidor! -> %s\n", atm_atual, buffer);
      write(atm_para_servidor[atm_atual][ESCRITA], &buffer, sizeof(buffer));
  }

  fclose(arq);
  return 0;
}