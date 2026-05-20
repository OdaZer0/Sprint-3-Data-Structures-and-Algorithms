

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define MAX_NOME        64
#define MAX_SINTOMA     128
#define MAX_MEDICO      64
#define HISTORICO_MAX   50   

typedef enum {
    PRIORIDADE_NORMAL    = 0,
    PRIORIDADE_URGENTE   = 1,
    PRIORIDADE_EMERGENCIA = 2
} Prioridade;

typedef struct {
    int        id;
    char       nome[MAX_NOME];
    int        idade;
    char       sintoma[MAX_SINTOMA];
    Prioridade prioridade;
    char       timestamp[32];
} Paciente;

typedef enum {
    ACAO_ENFILEIRAR,
    ACAO_CANCELAR,
    ACAO_REATRIBUIR
} TipoAcao;

typedef struct {
    TipoAcao tipo;
    Paciente paciente;
    char     descricao[256];
} Acao;


typedef struct NoFila {
    Paciente      dado;
    struct NoFila *proximo;
} NoFila;

typedef struct NoPilha {
    Paciente       dado;
    struct NoPilha *abaixo;
} NoPilha;

typedef struct NoAcao {
    Acao           dado;
    struct NoAcao  *abaixo;
} NoAcao;


typedef struct {
    NoFila *frente;
    NoFila *traseira;
    int     tamanho;
    char    nome[32];
} Fila;


typedef struct {
    NoPilha *topo;
    int      tamanho;
    int      capacidade;  
    char     nome[32];
} Pilha;


typedef struct {
    NoAcao *topo;
    int     tamanho;
} PilhaAcoes;


static int     proximo_id   = 1001;
static Fila    fila_normal;
static Fila    fila_urgente;
static Fila    fila_emergencia;
static Pilha   historico;
static PilhaAcoes pilha_acoes;


static void obter_timestamp(char *buf, size_t sz) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, sz, "%d/%m/%Y %H:%M:%S", tm_info);
}

static const char *prioridade_str(Prioridade p) {
    switch (p) {
        case PRIORIDADE_EMERGENCIA: return "EMERGENCIA";
        case PRIORIDADE_URGENTE:    return "URGENTE";
        default:                    return "NORMAL";
    }
}

static void linha(int n, char c) {
    for (int i = 0; i < n; i++) putchar(c);
    putchar('\n');
}

static void imprimir_paciente(const Paciente *p) {
    printf("  ID: %d | %s (%d anos)\n", p->id, p->nome, p->idade);
    printf("  Sintoma: %s\n", p->sintoma);
    printf("  Prioridade: %s | %s\n", prioridade_str(p->prioridade), p->timestamp);
}


void fila_init(Fila *f, const char *nome) {
    f->frente   = NULL;
    f->traseira = NULL;
    f->tamanho  = 0;
    strncpy(f->nome, nome, 31);
}

int fila_is_empty(const Fila *f) {
    return f->frente == NULL;
}

/* enqueue: insere no fim da fila — O(1) */
int fila_enqueue(Fila *f, Paciente p) {
    NoFila *novo = (NoFila *)malloc(sizeof(NoFila));
    if (!novo) { fprintf(stderr, "Erro: sem memoria.\n"); return -1; }
    novo->dado    = p;
    novo->proximo = NULL;
    if (fila_is_empty(f)) {
        f->frente   = novo;
        f->traseira = novo;
    } else {
        f->traseira->proximo = novo;
        f->traseira          = novo;
    }
    f->tamanho++;
    return 0;
}

int fila_dequeue(Fila *f, Paciente *out) {
    if (fila_is_empty(f)) return -1;
    NoFila *temp = f->frente;
    *out          = temp->dado;
    f->frente     = temp->proximo;
    if (f->frente == NULL) f->traseira = NULL;
    free(temp);
    f->tamanho--;
    return 0;
}

int fila_peek(const Fila *f, Paciente *out) {
    if (fila_is_empty(f)) return -1;
    *out = f->frente->dado;
    return 0;
}

void fila_imprimir(const Fila *f) {
    if (fila_is_empty(f)) {
        printf("  [Fila %s vazia]\n", f->nome);
        return;
    }
    printf("  Fila [%s] — %d paciente(s):\n", f->nome, f->tamanho);
    NoFila *cur = f->frente;
    int pos = 1;
    while (cur) {
        printf("  #%d ", pos++);
        imprimir_paciente(&cur->dado);
        cur = cur->proximo;
    }
}


void pilha_init(Pilha *p, const char *nome, int capacidade) {
    p->topo       = NULL;
    p->tamanho    = 0;
    p->capacidade = capacidade;
    strncpy(p->nome, nome, 31);
}

int pilha_is_empty(const Pilha *p) {
    return p->topo == NULL;
}

int pilha_push(Pilha *p, Paciente dado) {
    if (p->capacidade > 0 && p->tamanho >= p->capacidade) {
        NoPilha *cur = p->topo;
        if (cur->abaixo == NULL) {
            free(cur); p->topo = NULL; p->tamanho = 0;
        } else {
            while (cur->abaixo->abaixo != NULL) cur = cur->abaixo;
            free(cur->abaixo);
            cur->abaixo = NULL;
            p->tamanho--;
        }
    }
    NoPilha *novo = (NoPilha *)malloc(sizeof(NoPilha));
    if (!novo) { fprintf(stderr, "Erro: sem memoria.\n"); return -1; }
    novo->dado   = dado;
    novo->abaixo = p->topo;
    p->topo      = novo;
    p->tamanho++;
    return 0;
}

int pilha_pop(Pilha *p, Paciente *out) {
    if (pilha_is_empty(p)) return -1;
    NoPilha *temp = p->topo;
    *out           = temp->dado;
    p->topo        = temp->abaixo;
    free(temp);
    p->tamanho--;
    return 0;
}

int pilha_peek(const Pilha *p, Paciente *out) {
    if (pilha_is_empty(p)) return -1;
    *out = p->topo->dado;
    return 0;
}

void pilha_imprimir(const Pilha *p) {
    if (pilha_is_empty(p)) {
        printf("  [Pilha %s vazia]\n", p->nome);
        return;
    }
    printf("  Pilha [%s] — %d registro(s) (topo primeiro):\n", p->nome, p->tamanho);
    NoPilha *cur = p->topo;
    int pos = 1;
    while (cur) {
        printf("  #%d ", pos++);
        imprimir_paciente(&cur->dado);
        cur = cur->abaixo;
    }
}


void pilha_acoes_init(PilhaAcoes *pa) {
    pa->topo     = NULL;
    pa->tamanho  = 0;
}

void pilha_acoes_push(PilhaAcoes *pa, Acao a) {
    NoAcao *novo = (NoAcao *)malloc(sizeof(NoAcao));
    if (!novo) return;
    novo->dado   = a;
    novo->abaixo = pa->topo;
    pa->topo     = novo;
    pa->tamanho++;
}

int pilha_acoes_pop(PilhaAcoes *pa, Acao *out) {
    if (pa->topo == NULL) return -1;
    NoAcao *temp = pa->topo;
    *out          = temp->dado;
    pa->topo      = temp->abaixo;
    free(temp);
    pa->tamanho--;
    return 0;
}


static void registrar_acao(TipoAcao tipo, Paciente p, const char *desc) {
    Acao a;
    a.tipo    = tipo;
    a.paciente = p;
    strncpy(a.descricao, desc, 255);
    pilha_acoes_push(&pilha_acoes, a);
}

static Fila *fila_para(Prioridade pr) {
    switch (pr) {
        case PRIORIDADE_EMERGENCIA: return &fila_emergencia;
        case PRIORIDADE_URGENTE:    return &fila_urgente;
        default:                    return &fila_normal;
    }
}

void sistema_enfileirar(const char *nome, int idade,
                        const char *sintoma, Prioridade pr) {
    Paciente p;
    p.id         = proximo_id++;
    p.prioridade = pr;
    p.idade      = idade;
    strncpy(p.nome,    nome,    MAX_NOME    - 1);
    strncpy(p.sintoma, sintoma, MAX_SINTOMA - 1);
    obter_timestamp(p.timestamp, sizeof(p.timestamp));

    Fila *f = fila_para(pr);
    fila_enqueue(f, p);

    char desc[256];
    snprintf(desc, sizeof(desc), "Paciente %s (ID %d) enfileirado em [%s]",
             nome, p.id, f->nome);
    registrar_acao(ACAO_ENFILEIRAR, p, desc);

    printf("  ✔ Paciente enfileirado: %s | ID: %d | Fila: %s\n",
           nome, p.id, f->nome);
}

int sistema_atender(const char *medico) {
    Fila *filas[3] = { &fila_emergencia, &fila_urgente, &fila_normal };
    for (int i = 0; i < 3; i++) {
        if (!fila_is_empty(filas[i])) {
            Paciente p;
            fila_dequeue(filas[i], &p);
            pilha_push(&historico, p);

            printf("  ✔ Dr(a). %s atende: %s (ID %d) — Prioridade: %s\n",
                   medico, p.nome, p.id, prioridade_str(p.prioridade));
            return p.id;
        }
    }
    printf("  ℹ Nenhum paciente aguardando.\n");
    return -1;
}

int sistema_cancelar(Prioridade pr) {
    Fila *f = fila_para(pr);
    if (fila_is_empty(f)) {
        printf("  ℹ Fila [%s] está vazia.\n", f->nome);
        return -1;
    }
    Paciente p;
    fila_dequeue(f, &p);

    char desc[256];
    snprintf(desc, sizeof(desc),
             "Cancelamento de %s (ID %d) da fila [%s]",
             p.nome, p.id, f->nome);
    registrar_acao(ACAO_CANCELAR, p, desc);

    printf("  ✔ Consulta cancelada: %s (ID %d)\n", p.nome, p.id);
    return 0;
}

void sistema_desfazer(void) {
    Acao a;
    if (pilha_acoes_pop(&pilha_acoes, &a) < 0) {
        printf("  ℹ Nenhuma ação para desfazer.\n");
        return;
    }
    printf("  ↩ Desfazendo: %s\n", a.descricao);
    if (a.tipo == ACAO_CANCELAR) {
        Fila *f = fila_para(a.paciente.prioridade);
        NoFila *novo = (NoFila *)malloc(sizeof(NoFila));
        if (!novo) return;
        novo->dado    = a.paciente;
        novo->proximo = f->frente;
        f->frente     = novo;
        if (f->traseira == NULL) f->traseira = novo;
        f->tamanho++;
        printf("  ✔ Paciente %s reinserido na fila [%s]\n",
               a.paciente.nome, f->nome);
    } else {
        printf("  ✔ Ação revertida com sucesso.\n");
    }
}


void exibir_estado(void) {
    linha(60, '=');
    printf("  ESTADO ATUAL DO SISTEMA BLUA\n");
    linha(60, '-');
    fila_imprimir(&fila_emergencia);
    putchar('\n');
    fila_imprimir(&fila_urgente);
    putchar('\n');
    fila_imprimir(&fila_normal);
    putchar('\n');
    pilha_imprimir(&historico);
    linha(60, '=');
}


void cenario1(void) {
    linha(60, '=');
    printf("  CENARIO 1: Fluxo FIFO Basico de Teleconsultas\n");
    linha(60, '=');
    printf("\n  >> Chegada de 4 pacientes em ordem...\n\n");

    sistema_enfileirar("Ana Lima",      34, "Dor de cabeca persistente",  PRIORIDADE_NORMAL);
    sistema_enfileirar("Bruno Souza",   45, "Febre alta (39.5 graus)",    PRIORIDADE_NORMAL);
    sistema_enfileirar("Carla Mendes",  28, "Tosse seca ha 5 dias",       PRIORIDADE_NORMAL);
    sistema_enfileirar("Diego Alves",   52, "Pressao elevada",            PRIORIDADE_NORMAL);

    putchar('\n');
    fila_imprimir(&fila_normal);

    printf("\n  >> Medico Dr. Carlos chama pacientes sequencialmente...\n\n");
    sistema_atender("Carlos Pereira");
    sistema_atender("Carlos Pereira");
    sistema_atender("Carlos Pereira");

    printf("\n  >> Historico de consultas finalizadas (LIFO):\n");
    pilha_imprimir(&historico);
    putchar('\n');
}


void cenario2(void) {
    linha(60, '=');
    printf("  CENARIO 2: Fila Prioritaria — Emergencias ao Topo\n");
    linha(60, '=');
    printf("\n  >> Chegam pacientes com prioridades mistas...\n\n");

    sistema_enfileirar("Eduardo Neves",  61, "Consulta de rotina",         PRIORIDADE_NORMAL);
    sistema_enfileirar("Fernanda Costa", 38, "Dor no peito intensa",       PRIORIDADE_EMERGENCIA);
    sistema_enfileirar("Gustavo Rios",   29, "Alergia com urticaria",      PRIORIDADE_URGENTE);
    sistema_enfileirar("Helena Faria",   55, "Diabetes descompensada",     PRIORIDADE_URGENTE);
    sistema_enfileirar("Igor Barros",    44, "Check-up anual",             PRIORIDADE_NORMAL);

    printf("\n  >> Estado das filas antes do atendimento:\n");
    exibir_estado();

    printf("\n  >> Dra. Marcia chama os 5 pacientes (emergencia > urgente > normal)...\n\n");
    sistema_atender("Marcia Oliveira");
    sistema_atender("Marcia Oliveira");
    sistema_atender("Marcia Oliveira");
    sistema_atender("Marcia Oliveira");
    sistema_atender("Marcia Oliveira");
    putchar('\n');
}


void cenario3(void) {
    linha(60, '=');
    printf("  CENARIO 3: Cancelamento e Desfazer com Pilha de Acoes\n");
    linha(60, '=');
    printf("\n  >> Enfileirando pacientes...\n\n");

    sistema_enfileirar("Julia Tavares",  33, "Insonia cronica",           PRIORIDADE_NORMAL);
    sistema_enfileirar("Kaio Pereira",   47, "Dor nas costas",            PRIORIDADE_URGENTE);
    sistema_enfileirar("Larissa Pinto",  26, "Ansiedade generalizada",    PRIORIDADE_NORMAL);

    printf("\n  >> Estado antes do cancelamento:\n");
    fila_imprimir(&fila_urgente);
    fila_imprimir(&fila_normal);

    printf("\n  >> Recepcionista cancela proximo da fila URGENTE por engano...\n\n");
    sistema_cancelar(PRIORIDADE_URGENTE);

    printf("\n  >> Estado apos cancelamento:\n");
    fila_imprimir(&fila_urgente);

    printf("\n  >> Supervisor aciona DESFAZER...\n\n");
    sistema_desfazer();

    printf("\n  >> Estado apos desfazer (Kaio volta para a fila):\n");
    fila_imprimir(&fila_urgente);
    fila_imprimir(&fila_normal);

    printf("\n  >> Pilha de acoes registradas: %d acao(oes)\n",
           pilha_acoes.tamanho);
    putchar('\n');
}


static void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static int ler_int(const char *prompt, int min, int max) {
    int v;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &v) == 1 && v >= min && v <= max) {
            limpar_buffer();
            return v;
        }
        limpar_buffer();
        printf("  Valor invalido. Informe entre %d e %d.\n", min, max);
    }
}

static void ler_str(const char *prompt, char *buf, int sz) {
    printf("%s", prompt);
    fgets(buf, sz, stdin);
    buf[strcspn(buf, "\n")] = '\0';
}

void menu_enfileirar(void) {
    char nome[MAX_NOME], sintoma[MAX_SINTOMA];
    int  idade, pr;

    ler_str("  Nome do paciente: ", nome, MAX_NOME);
    idade = ler_int("  Idade: ", 0, 130);
    ler_str("  Sintoma principal: ", sintoma, MAX_SINTOMA);
    pr    = ler_int("  Prioridade (0=Normal, 1=Urgente, 2=Emergencia): ", 0, 2);

    sistema_enfileirar(nome, idade, sintoma, (Prioridade)pr);
}

void menu_atender(void) {
    char medico[MAX_MEDICO];
    ler_str("  Nome do medico: ", medico, MAX_MEDICO);
    sistema_atender(medico);
}

void menu_cancelar(void) {
    int pr = ler_int("  Cancelar da fila (0=Normal, 1=Urgente, 2=Emergencia): ", 0, 2);
    sistema_cancelar((Prioridade)pr);
}

void exibir_menu(void) {
    linha(60, '-');
    printf("  BLUA  Gerenciamento de Fila\n");
    linha(60, '-');
    printf("  [1] Enfileirar paciente\n");
    printf("  [2] Atender proximo paciente\n");
    printf("  [3] Cancelar atendimento\n");
    printf("  [4] Desfazer ultima acao\n");
    printf("  [5] Exibir estado completo\n");
    linha(60, '-');
    printf("  CENARIOS DE DEMONSTRACAO:\n");
    printf("  [6] Cenario 1 — Fluxo FIFO Basico\n");
    printf("  [7] Cenario 2 — Filas Prioritarias\n");
    printf("  [8] Cenario 3 — Cancelamento e Desfazer\n");
    linha(60, '-');
    printf("  [0] Sair\n");
    linha(60, '-');
}


void sistema_init(void) {
    fila_init(&fila_normal,     "NORMAL");
    fila_init(&fila_urgente,    "URGENTE");
    fila_init(&fila_emergencia, "EMERGENCIA");
    pilha_init(&historico, "HISTORICO", HISTORICO_MAX);
    pilha_acoes_init(&pilha_acoes);
}

int main(void) {
    sistema_init();

    printf("\n");
    linha(60, '*');
    printf("  BLUA  Plataforma de Telessaude Care Plus\n");
    printf("  Sistema de Gerenciamento de Fila de Atendimento\n");
    linha(60, '*');
    putchar('\n');

    int opcao;
    do {
        exibir_menu();
        opcao = ler_int("  Opcao: ", 0, 9);
        putchar('\n');
        switch (opcao) {
            case 1: menu_enfileirar(); break;
            case 2: menu_atender();    break;
            case 3: menu_cancelar();   break;
            case 4: sistema_desfazer(); break;
            case 5: exibir_estado();   break;
            case 6: cenario1();        break;
            case 7: cenario2();        break;
            case 8: cenario3();        break;
            case 0: printf("  Encerrando o sistema Blua\n\n"); break;
            default: printf(" pcao invalida\n"); break;
        }
        putchar('\n');
    } while (opcao != 0);

    return 0;
}
