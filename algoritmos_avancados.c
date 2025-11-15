#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Limites gerais utilizados em todo o jogo
#define TAM_NOME 64
#define TAM_PISTA 128
#define TAM_HASH 11

// Estrutura que representa cada sala da mansao (nodo da arvore binaria)
typedef struct Sala {
    char nome[TAM_NOME];
    const char *pista;
    const char *suspeito;
    int pistaColetada;
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Nodo da arvore de busca contendo as pistas coletadas
typedef struct PistaNode {
    char texto[TAM_PISTA];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// Lista encadeada de pistas associadas a um suspeito
typedef struct Associacao {
    char pista[TAM_PISTA];
    struct Associacao *prox;
} Associacao;

// Nodo da tabela hash contendo os suspeitos
typedef struct SuspeitoNode {
    char nome[TAM_NOME];
    int citacoes;
    Associacao *pistas;
    struct SuspeitoNode *prox;
} SuspeitoNode;

typedef struct {
    SuspeitoNode *buckets[TAM_HASH];
} TabelaHash;

// ---------------- Funcoes de salas (arvore binaria) ----------------
static Sala *criarSala(const char *nome, const char *pista, const char *suspeito) {
    Sala *nova = malloc(sizeof(Sala));
    if (!nova) {
        fprintf(stderr, "Falha ao alocar sala\n");
        exit(EXIT_FAILURE);
    }
    strncpy(nova->nome, nome, TAM_NOME - 1);
    nova->nome[TAM_NOME - 1] = '\0';
    nova->pista = pista;
    nova->suspeito = suspeito;
    nova->pistaColetada = 0;
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

static void conectarSalas(Sala *origem, Sala *esquerda, Sala *direita) {
    if (!origem) {
        return;
    }
    origem->esquerda = esquerda;
    origem->direita = direita;
}

static void liberarSalas(Sala *sala) {
    if (!sala) {
        return;
    }
    liberarSalas(sala->esquerda);
    liberarSalas(sala->direita);
    free(sala);
}

// ---------------- Funcoes da arvore de pistas (BST) ----------------
static PistaNode *criarPistaNode(const char *texto) {
    PistaNode *novo = malloc(sizeof(PistaNode));
    if (!novo) {
        fprintf(stderr, "Falha ao alocar pista\n");
        exit(EXIT_FAILURE);
    }
    strncpy(novo->texto, texto, TAM_PISTA - 1);
    novo->texto[TAM_PISTA - 1] = '\0';
    novo->esquerda = NULL;
    novo->direita = NULL;
    return novo;
}

static void inserirPista(PistaNode **raiz, const char *texto) {
    if (!texto || !raiz) {
        return;
    }
    if (*raiz == NULL) {
        *raiz = criarPistaNode(texto);
        return;
    }
    int comparacao = strcmp(texto, (*raiz)->texto);
    if (comparacao < 0) {
        inserirPista(&((*raiz)->esquerda), texto);
    } else if (comparacao > 0) {
        inserirPista(&((*raiz)->direita), texto);
    }
}

static void listarPistas(const PistaNode *raiz) {
    if (!raiz) {
        return;
    }
    listarPistas(raiz->esquerda);
    printf("- %s\n", raiz->texto);
    listarPistas(raiz->direita);
}

static void liberarPistas(PistaNode *raiz) {
    if (!raiz) {
        return;
    }
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}

// ---------------- Funcoes da tabela hash de suspeitos ----------------
static void inicializarHash(TabelaHash *hash) {
    for (int i = 0; i < TAM_HASH; i++) {
        hash->buckets[i] = NULL;
    }
}

static unsigned int hashNome(const char *nome) {
    unsigned int soma = 0;
    for (const unsigned char *c = (const unsigned char *)nome; *c; c++) {
        soma += *c;
    }
    return soma % TAM_HASH;
}

static SuspeitoNode *criarSuspeito(const char *nome) {
    SuspeitoNode *novo = malloc(sizeof(SuspeitoNode));
    if (!novo) {
        fprintf(stderr, "Falha ao alocar suspeito\n");
        exit(EXIT_FAILURE);
    }
    strncpy(novo->nome, nome, TAM_NOME - 1);
    novo->nome[TAM_NOME - 1] = '\0';
    novo->citacoes = 0;
    novo->pistas = NULL;
    novo->prox = NULL;
    return novo;
}

static void adicionarPistaAoSuspeito(SuspeitoNode *suspeito, const char *pista) {
    if (!suspeito || !pista) {
        return;
    }
    Associacao *nova = malloc(sizeof(Associacao));
    if (!nova) {
        fprintf(stderr, "Falha ao alocar associacao\n");
        exit(EXIT_FAILURE);
    }
    strncpy(nova->pista, pista, TAM_PISTA - 1);
    nova->pista[TAM_PISTA - 1] = '\0';
    nova->prox = suspeito->pistas;
    suspeito->pistas = nova;
    suspeito->citacoes++;
}

static void inserirAssociacao(TabelaHash *hash, const char *suspeito, const char *pista) {
    if (!hash || !suspeito || !pista) {
        return;
    }
    unsigned int indice = hashNome(suspeito);
    SuspeitoNode *atual = hash->buckets[indice];
    while (atual) {
        if (strcmp(atual->nome, suspeito) == 0) {
            adicionarPistaAoSuspeito(atual, pista);
            return;
        }
        atual = atual->prox;
    }
    SuspeitoNode *novo = criarSuspeito(suspeito);
    adicionarPistaAoSuspeito(novo, pista);
    novo->prox = hash->buckets[indice];
    hash->buckets[indice] = novo;
}

static void listarAssociacoes(const TabelaHash *hash) {
    for (int i = 0; i < TAM_HASH; i++) {
        const SuspeitoNode *suspeito = hash->buckets[i];
        while (suspeito) {
            printf("%s (%d pista%s):\n", suspeito->nome, suspeito->citacoes, suspeito->citacoes > 1 ? "s" : "");
            const Associacao *assoc = suspeito->pistas;
            while (assoc) {
                printf("  - %s\n", assoc->pista);
                assoc = assoc->prox;
            }
            suspeito = suspeito->prox;
        }
    }
}

static SuspeitoNode *suspeitoMaisCitado(const TabelaHash *hash) {
    SuspeitoNode *mais = NULL;
    for (int i = 0; i < TAM_HASH; i++) {
        SuspeitoNode *suspeito = hash->buckets[i];
        while (suspeito) {
            if (!mais || suspeito->citacoes > mais->citacoes) {
                mais = suspeito;
            }
            suspeito = suspeito->prox;
        }
    }
    return mais;
}

static void liberarAssociacoes(Associacao *assoc) {
    while (assoc) {
        Associacao *prox = assoc->prox;
        free(assoc);
        assoc = prox;
    }
}

static void liberarHash(TabelaHash *hash) {
    for (int i = 0; i < TAM_HASH; i++) {
        SuspeitoNode *suspeito = hash->buckets[i];
        while (suspeito) {
            SuspeitoNode *prox = suspeito->prox;
            liberarAssociacoes(suspeito->pistas);
            free(suspeito);
            suspeito = prox;
        }
        hash->buckets[i] = NULL;
    }
}

// ---------------- Jogabilidade e fluxo principal ----------------
static void coletarPistaDaSala(Sala *sala, PistaNode **pistas, TabelaHash *hash) {
    if (!sala || sala->pistaColetada || !sala->pista) {
        return;
    }
    printf("Pista encontrada: %s\n", sala->pista);
    inserirPista(pistas, sala->pista);
    if (sala->suspeito) {
        inserirAssociacao(hash, sala->suspeito, sala->pista);
    }
    sala->pistaColetada = 1;
}

static void mostrarOpcoes(const Sala *sala) {
    if (sala->esquerda) {
        printf("e - Ir para %s\n", sala->esquerda->nome);
    } else {
        printf("e - Nenhuma sala a esquerda\n");
    }
    if (sala->direita) {
        printf("d - Ir para %s\n", sala->direita->nome);
    } else {
        printf("d - Nenhuma sala a direita\n");
    }
    printf("s - Encerrar exploracao\n");
}

static void explorarSalas(Sala *inicio, PistaNode **pistas, TabelaHash *hash) {
    if (!inicio) {
        return;
    }
    Sala *atual = inicio;
    char entrada[8];
    // Loop principal de navegacao nos corredores
    while (1) {
        printf("\n--- Voce esta em: %s ---\n", atual->nome);
        coletarPistaDaSala(atual, pistas, hash);
        mostrarOpcoes(atual);
        printf("Escolha: ");
        if (!fgets(entrada, sizeof(entrada), stdin)) {
            break;
        }
        char opcao = entrada[0];
        if (opcao == 's' || opcao == 'S') {
            printf("Exploracao encerrada.\n");
            break;
        }
        if ((opcao == 'e' || opcao == 'E')) {
            if (atual->esquerda) {
                atual = atual->esquerda;
            } else {
                printf("Nao existe sala a esquerda.\n");
            }
        } else if ((opcao == 'd' || opcao == 'D')) {
            if (atual->direita) {
                atual = atual->direita;
            } else {
                printf("Nao existe sala a direita.\n");
            }
        } else {
            printf("Opcao invalida.\n");
        }
    }
}

static Sala *montarMansao(void) {
    Sala *hall = criarSala("Hall de Entrada", "Pegadas de lama no tapete principal", "Jardineiro");
    Sala *biblioteca = criarSala("Biblioteca", "Livro raro fora de lugar", "Curador");
    Sala *observatorio = criarSala("Observatorio", "Telescopio apontando para a sacada errada", "Astronomo");
    Sala *arquivo = criarSala("Arquivo Antigo", "Mapa da mansao com anotacoes recentes", "Arquiteta");
    Sala *cozinha = criarSala("Cozinha", "Faca sumiu do suporte de chef", "Chef");
    Sala *despensa = criarSala("Despensa", "Luvas de couro escondidas atras de caixas", "Assistente");
    Sala *jardim = criarSala("Jardim de Inverno", "Ramos quebrados e perfume raro", "Socialite");

    conectarSalas(hall, biblioteca, cozinha);
    conectarSalas(biblioteca, arquivo, observatorio);
    conectarSalas(cozinha, despensa, jardim);

    return hall;
}

int main(void) {
    // Introducao do jogo e inicializacao das estruturas
    printf("Bem-vindo ao Detective Quest!\n");
    printf("Explore a mansao, colete pistas e associe-as aos suspeitos.\n");

    TabelaHash tabela;
    inicializarHash(&tabela);
    PistaNode *pistas = NULL;
    Sala *mansao = montarMansao();

    explorarSalas(mansao, &pistas, &tabela);

    printf("\n===== Pistas coletadas =====\n");
    if (pistas) {
        listarPistas(pistas);
    } else {
        printf("Nenhuma pista foi coletada.\n");
    }

    printf("\n===== Suspeitos e associacoes =====\n");
    listarAssociacoes(&tabela);

    SuspeitoNode *principal = suspeitoMaisCitado(&tabela);
    if (principal) {
        printf("\nSuspeito mais citado: %s (%d pista%s)\n",
               principal->nome,
               principal->citacoes,
               principal->citacoes > 1 ? "s" : "");
    } else {
        printf("\nNenhum suspeito foi associado ate o momento.\n");
    }

    liberarSalas(mansao);
    liberarPistas(pistas);
    liberarHash(&tabela);

    printf("\nObrigado por jogar Detective Quest!\n");
    return 0;
}
