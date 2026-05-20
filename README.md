#  BLUA - Sistema de Gerenciamento de Fila de Atendimento

**Care Plus | Sprint: Estruturas de Dados Aplicadas à Saúde Digital**

Sistema de gerenciamento de teleconsultas da plataforma Blua, implementado em **C puro** com estruturas de dados construídas do zero (sem bibliotecas prontas de fila/pilha).

---

##  Estrutura do Projeto

```
blua_queue/
├── src/
│   └── blua.c          # Código-fonte completo (único arquivo executável)
├── docs/
│   └── relatorio.pdf   # Relatório técnico (até 3 páginas)
└── README.md
```

---

##  Estruturas de Dados Implementadas

| Estrutura | Tipo | Uso no Sistema |
|-----------|------|----------------|
| `Fila` (FIFO) | Lista encadeada simples | Teleconsultas normais, urgentes e emergências |
| `Pilha` (LIFO) | Lista encadeada simples | Histórico das últimas N consultas finalizadas |
| `PilhaAcoes` (LIFO) | Lista encadeada simples | Undo/redo de ações administrativas |

Todas as operações principais (**enqueue**, **dequeue**, **push**, **pop**, **peek**, **is_empty**) têm complexidade **O(1)** em tempo.

---

##  Como Compilar e Executar

### Pré-requisito
- GCC (qualquer versão recente)

### Compilar
```bash
gcc -o blua src/blua.c
```

### Executar
```bash
./blua
```

---

##  Menu do Sistema

Ao executar, o sistema apresenta um menu interativo:

```
------------------------------------------------------------
  BLUA  Gerenciamento de Fila de Atendimento
------------------------------------------------------------
  [1] Enfileirar paciente
  [2] Atender próximo paciente
  [3] Cancelar atendimento
  [4] Desfazer última ação
  [5] Exibir estado completo
------------------------------------------------------------
  CENÁRIOS DE DEMONSTRAÇÃO:
  [6] Cenário 1 — Fluxo FIFO Básico
  [7] Cenário 2 — Filas Prioritárias
  [8] Cenário 3 — Cancelamento e Desfazer
------------------------------------------------------------
  [0] Sair
------------------------------------------------------------
```

---

##  Cenários de Demonstração

### Cenário 1 — Fluxo FIFO Básico
- 4 pacientes chegam em ordem e são atendidos sequencialmente (FIFO)
- Demonstra `enqueue` e `dequeue` da fila normal
- Exibe o histórico de consultas na pilha (LIFO)

### Cenário 2 — Filas Prioritárias (Emergências ao Topo)
- Pacientes chegam com prioridades mistas: normal, urgente e emergência
- O sistema garante que **emergências são atendidas primeiro**, depois urgentes, depois normais
- Internamente usa 3 filas FIFO independentes; o despacho respeita a hierarquia

### Cenário 3 — Cancelamento e Desfazer (Pilha de Ações)
- Simula um cancelamento acidental por parte da recepcionista
- O supervisor aciona **desfazer (undo)** via pilha de ações
- O paciente é reinserido no início da fila correta

---

##  Complexidade das Operações

| Operação | Fila (FIFO) | Pilha (LIFO) |
|----------|:-----------:|:------------:|
| enqueue / push | O(1) | O(1) |
| dequeue / pop  | O(1) | O(1) |
| peek           | O(1) | O(1) |
| is_empty       | O(1) | O(1) |
| imprimir (N elementos) | O(N) | O(N) |

Uso de memória: **O(N)** — um nó alocado por elemento.

---

##  Segurança e Auditoria

- Toda ação administrativa (enfileirar, cancelar, reatribuir) é registrada na **Pilha de Ações**
- O histórico mantém as **últimas 50 consultas** finalizadas com timestamp
- Cancelamentos são reversíveis via undo em tempo O(1)

---

##  Autores

Ana Carolina - RM: 568401
Danilo Roberto - RM: 566966
Enzo Hitoshi - RM: 567313
Gabriel Drebtchinsky - RM: 566729
Henrique Fessel - RM: 567513
José Ribeiro - RM: 567692
