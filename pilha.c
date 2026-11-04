/*
 * pilha.c — recursão até a pilha acabar.
 *
 * Laboratório 2 da Aula 05 — Sistemas Operacionais · TADS/IFRN · 2026.1
 *
 * Compilar:  gcc -Wall -Wextra -O0 -o pilha pilha.c
 * Executar:  ./pilha            (imprime só de 500 em 500)
 *            ./pilha 1          (imprime todos os níveis — bem mais lento)
 *
 * Cada chamada de explode() cria um novo QUADRO na pilha (stack frame), com o
 * endereço de retorno, os parâmetros e as variáveis locais — aqui, um buffer
 * de 1 KiB, para o quadro ficar grande o bastante para a conta fechar rápido.
 *
 * O programa MORRE de propósito, com SIGSEGV. Não é bug: é o limite da pilha
 * sendo atingido. Compare a profundidade final com o valor de `ulimit -s`.
 *
 * O -O0 importa: com otimização o compilador pode transformar esta recursão em
 * laço (tail call) e o programa nunca estouraria.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static unsigned long intervalo = 500;
static char *base = NULL;      /* endereço aproximado do topo da pilha no início */

static void explode(unsigned long nivel)
{
	char buffer[1024];     /* ❶ 1 KiB de variável local: o grosso deste quadro */

	buffer[0] = (char)(nivel & 0xff);   /* toca o buffer para ele não ser removido */

	if (base == NULL)
		base = buffer;

	if (nivel % intervalo == 0) {
		/* ❷ a distância entre o quadro atual e o primeiro é, na prática,
		 *    quanta pilha já foi consumida. A pilha cresce para BAIXO,
		 *    então subtraímos na ordem que dá positivo. */
		long usados = (long)(base - buffer);
		printf("nível %8lu   pilha usada ≈ %7ld KiB   quadro em %p\n",
		       nivel, usados / 1024, (void *)buffer);
		fflush(stdout);
	}

	explode(nivel + 1);    /* ❸ nunca retorna: some um quadro a cada chamada */
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		unsigned long v = strtoul(argv[1], NULL, 10);
		if (v >= 1)
			intervalo = v;
	}

	printf("PID=%d\n", (int)getpid());
	printf("Limite da pilha agora:  ulimit -s  →  veja no shell (em KiB)\n");
	printf("Este programa vai morrer com SIGSEGV. É o objetivo.\n\n");
	fflush(stdout);

	explode(1);

	/* inalcançável */
	return 0;
}
