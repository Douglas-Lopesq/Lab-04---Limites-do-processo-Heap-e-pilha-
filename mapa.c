/*
 * mapa.c — onde cada variável mora, e o que o kernel diz sobre isso.
 *
 * Laboratório 2 da Aula 05 — Sistemas Operacionais · TADS/IFRN · 2026.1
 *
 * Compilar:  gcc -Wall -Wextra -O0 -o mapa mapa.c
 * Executar:  ./mapa &
 *
 * O programa imprime o endereço de uma variável de cada região do espaço de
 * endereçamento e depois FICA PARADO, para você inspecioná-lo de outro
 * terminal com /proc/<pid>/maps e /proc/<pid>/status.
 *
 * Compare os endereços impressos com as faixas do maps: cada variável tem que
 * cair dentro da região correspondente.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int global_zerada;                 /* ❶ .bss  — global sem valor inicial */
int global_inicializada = 2026;    /* ❷ .data — global com valor inicial */
const char *texto = "SO/IFRN";     /* o ponteiro é .data; a string é .rodata */

static void mostrar(const char *quem, const void *p, const char *regiao)
{
	printf("  %-26s %14p   %s\n", quem, p, regiao);
}

int main(void)
{
	int local = 1;                      /* ❸ pilha */
	char *bloco = malloc(4096);         /* ❹ heap  */

	if (!bloco) {
		perror("malloc");
		return 1;
	}
	memset(bloco, 0, 4096);

	printf("\nPID = %d\n\n", (int)getpid());
	printf("  %-26s %14s   %s\n", "o que é", "endereço", "região esperada");
	printf("  ---------------------------------------------------------------\n");
	mostrar("código (função main)",    (void *)(size_t)main,   "text  — só leitura + execução");
	mostrar("string literal",          (void *)texto,          "rodata — dentro da área de código");
	mostrar("global inicializada",     (void *)&global_inicializada, "data");
	mostrar("global zerada",           (void *)&global_zerada, "bss");
	mostrar("bloco de malloc(4096)",   (void *)bloco,          "[heap]");
	mostrar("variável local",          (void *)&local,         "[stack]");

	printf("\nAgora, de OUTRO terminal:\n");
	printf("  cat /proc/%d/maps\n", (int)getpid());
	printf("  grep -E 'VmSize|VmRSS|VmData|VmStk' /proc/%d/status\n\n", (int)getpid());
	printf("Confira: cada endereço acima cai dentro de uma faixa do maps.\n");
	printf("Encerro sozinho em 300 s — ou use  kill %d\n", (int)getpid());
	fflush(stdout);

	sleep(300);
	free(bloco);
	return 0;
}
