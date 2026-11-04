/*
 * arquivo.c — ler um arquivo grande de duas maneiras, medindo a RAM usada.
 *
 * Laboratório 2 da Aula 05 — Sistemas Operacionais · TADS/IFRN · 2026.1
 *
 * Compilar:  gcc -Wall -Wextra -O0 -o arquivo arquivo.c
 * Executar:  ./arquivo incremental /tmp/grande.bin
 *            ./arquivo inteiro     /tmp/grande.bin
 *
 * Os dois modos processam O MESMO arquivo inteiro e chegam ao MESMO resultado
 * (a soma de todos os bytes). O que muda é quanta memória o processo precisa
 * ter ao mesmo tempo — e é isso que o VmRSS no fim vai mostrar.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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
	const char *modo, *caminho;
	int fd;
	struct stat st;
	unsigned long soma = 0;
	long pico;

	if (argc < 3) {
		fprintf(stderr, "uso: %s incremental|inteiro ARQUIVO\n", argv[0]);
		return 1;
	}
	modo = argv[1];
	caminho = argv[2];

	fd = open(caminho, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	if (fstat(fd, &st) < 0) {
		perror("fstat");
		return 1;
	}

	printf("arquivo: %s  (%.1f MiB)\n", caminho, st.st_size / 1048576.0);
	printf("VmRSS antes de ler: %ld kB\n", vm("VmRSS:"));
	fflush(stdout);

	if (strcmp(modo, "incremental") == 0) {
		/* ❶ um balde de 4 KiB, reutilizado do começo ao fim.
		 *    O arquivo pode ter gigabytes: o balde continua com 4 KiB. */
		char balde[4096];
		ssize_t n;

		while ((n = read(fd, balde, sizeof(balde))) > 0)
			for (ssize_t i = 0; i < n; i++)
				soma += (unsigned char)balde[i];

	} else if (strcmp(modo, "inteiro") == 0) {
		/* ❷ um bloco do TAMANHO DO ARQUIVO, tudo na memória de uma vez. */
		char *tudo = malloc((size_t)st.st_size);
		ssize_t lidos, total = 0;

		if (!tudo) {
			printf("\n>>> malloc() FALHOU ao pedir %.1f MiB de uma vez.\n",
			       st.st_size / 1048576.0);
			printf(">>> este é o ponto do experimento: a estratégia não cabe.\n");
			return 1;
		}
		while ((lidos = read(fd, tudo + total, (size_t)(st.st_size - total))) > 0)
			total += lidos;
		for (long i = 0; i < total; i++)
			soma += (unsigned char)tudo[i];
		/* não damos free() de propósito: queremos medir o pico ainda alocado */

	} else {
		fprintf(stderr, "modo desconhecido: %s\n", modo);
		return 1;
	}

	pico = vm("VmRSS:");
	close(fd);

	printf("soma de todos os bytes = %lu\n", soma);
	printf("VmRSS depois de ler:  %ld kB   <<< o número que interessa\n", pico);
	return 0;
}
