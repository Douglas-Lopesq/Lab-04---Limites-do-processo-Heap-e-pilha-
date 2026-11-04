/*
 * heap.c — crescendo o heap de 1 MiB por vez, até o limite.
 *
 * Laboratório 2 da Aula 05 — Sistemas Operacionais · TADS/IFRN · 2026.1
 *
 * Compilar:  gcc -Wall -Wextra -O0 -o heap heap.c
 * Executar:  ./heap                  (padrão: toca a memória, teto de 2048 MiB)
 *            ./heap 2048 nao         (NÃO toca: só reserva endereço virtual)
 *            ./heap 120 sim liberar  (toca e libera cada bloco em seguida)
 *
 * O segundo argumento é o que faz este programa valer a pena. Com "nao", o
 * programa só chama malloc() e nunca escreve no bloco: VmSize dispara e VmRSS
 * quase não sai do lugar, porque nenhuma página física chegou a ser entregue.
 * Com o padrão, cada bloco é escrito, e aí VmRSS sobe junto.
 *
 * SEGURANÇA: existe um teto (argumento 1, padrão 2048 MiB) e o programa
 * imprime uma linha por bloco. Rode sempre com limite de memória no container:
 *   docker run --memory=512m ...
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIB (1024UL * 1024UL)

/* Lê um campo em kB de /proc/self/status (VmSize, VmRSS, VmData…). */
static long vm(const char *chave)
{
	char linha[256];
	long v = -1;
	FILE *f = fopen("/proc/self/status", "r");

	if (!f)
		return -1;
	while (fgets(linha, sizeof(linha), f))
		if (strncmp(linha, chave, strlen(chave)) == 0) {
			sscanf(linha + strlen(chave), "%ld", &v);
			break;
		}
	fclose(f);
	return v;
}

int main(int argc, char *argv[])
{
	long teto = (argc > 1) ? atol(argv[1]) : 2048;          /* MiB */
	int tocar = !(argc > 2 && strcmp(argv[2], "nao") == 0);
	int liberar = (argc > 3 && strcmp(argv[3], "liberar") == 0);

	if (teto < 1)    teto = 1;
	if (teto > 8192) teto = 8192;      /* trava de segurança */

	printf("PID=%d  · teto=%ld MiB · %s a memória · %s blocos\n",
	       (int)getpid(), teto, tocar ? "TOCANDO" : "NÃO tocando",
	       liberar ? "LIBERANDO" : "mantendo");
	printf("Inicial: VmSize=%ld kB  VmRSS=%ld kB  VmData=%ld kB\n",
	       vm("VmSize:"), vm("VmRSS:"), vm("VmData:"));
	printf("Acompanhe de outro terminal:\n");
	printf("  watch -n 1 'grep -E \"VmSize|VmRSS|VmData\" /proc/%d/status'\n\n",
	       (int)getpid());
	printf("%6s  %12s  %12s  %12s\n", "MiB", "VmSize(kB)", "VmRSS(kB)", "VmData(kB)");
	fflush(stdout);

	for (long i = 1; i <= teto; i++) {
		char *bloco = malloc(MIB);      /* ❶ pede 1 MiB ao alocador */
		sleep(1);

		if (!bloco) {
			/* ❷ malloc devolveu NULL: o pedido foi RECUSADO. */
			printf("\n>>> malloc() FALHOU ao pedir o MiB nº %ld\n", i);
			printf(">>> o processo continua vivo e pôde tratar o erro.\n");
			fflush(stdout);
			return 1;
		}

		if (tocar)
			memset(bloco, 1, MIB);   /* ❸ escrever é o que exige RAM de verdade */
		if (liberar)
			free(bloco);             /* ❹ devolve cada bloco antes da próxima volta */

		if (i % 16 == 0 || i <= 4) {
			printf("%6ld  %12ld  %12ld  %12ld\n",
			       i, vm("VmSize:"), vm("VmRSS:"), vm("VmData:"));
			fflush(stdout);
		}
	}

	printf("\nCheguei ao teto de %ld MiB sem falhar.\n", teto);
	printf("Final:   VmSize=%ld kB  VmRSS=%ld kB  VmData=%ld kB\n",
	       vm("VmSize:"), vm("VmRSS:"), vm("VmData:"));
	printf("Se era para ver o limite, baixe a memória do container ou suba o teto.\n");
	return 0;
}
