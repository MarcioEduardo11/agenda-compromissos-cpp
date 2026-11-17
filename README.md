# Agenda de Compromissos em C++

Projeto Final de **Programação e Sistemas Computacionais II** — Centro Universitário Campo Real, Engenharia de Software, 2026/1.
Professor: Giovane Galvão · Squad: **Marcio Eduardo e Milena Mayara**

## Descrição do sistema

Agenda para organizar compromissos do dia a dia (provas, consultas, reuniões e eventos), feita inteiramente no **terminal**. Cada compromisso tem **nome, data, horário e local**. Dá para cadastrar, listar em ordem cronológica, buscar, editar e excluir, além de desfazer exclusões, manter uma fila de lembretes e ver um índice alfabético.

**Tema escolhido:** tema próprio — *Agenda de Compromissos*. Todas as estruturas são implementadas à mão (sem `std::vector`, `std::list`, `std::stack` etc.). Código em um único arquivo: `src/main.cpp`.

## Compilar e executar

    make
    ./agenda

Sem `make`: `g++ -std=c++17 -Wall -Wextra -pedantic -o agenda src/main.cpp`

Teste automático com detecção de vazamentos de memória: `make teste`

**Demonstração das buscas:** opção **10** (gera 3000 compromissos) e depois opção **9** com a data `26/12/2027`.

## Menu

| Opção | Função |
|---|---|
| 1 | Cadastrar compromisso |
| 2 | Listar ordenado por data e horário (Merge Sort) |
| 3 | Buscar por nome (busca sequencial) |
| 4 | Editar |
| 5 | Excluir (o excluído vai para a pilha) |
| 6 | Desfazer exclusão (pilha) |
| 7 | Lembretes: adicionar, mostrar próximo, listar (fila) |
| 8 | Índice alfabético e busca por nome exato (árvore binária de busca) |
| 9 | Comparar busca sequencial × binária por data |
| 10 | Gerar dados de teste |

## Diagrama das estruturas de dados

```
Agenda
 ├─ lista     : [C1]→[C2]→[C3]→ nullptr           lista encadeada (todos os compromissos)
 ├─ lixeira   : topo→[excl. 3]→[excl. 1]→ nullptr    pilha LIFO (desfazer exclusão)
 ├─ lembretes : inicio→[L1]→[L2]→[L3]← fim        fila FIFO (lembretes)
 └─ raiz      :        [Maria]                    árvore binária de busca (índice por nome)
                       /     \
                  [Ana]     [Zeca]

Vetor temporário Compromisso*[ ]  →  aponta para os nós da lista
   → ordenado por Merge Sort → usado na busca binária por data
```

| Estrutura | Onde é usada | Por quê |
|---|---|---|
| Lista encadeada | Todos os compromissos | Inserir e remover sem deslocar elementos; sem tamanho máximo |
| Pilha (LIFO) | Lixeira de excluídos | O último excluído é o primeiro a voltar |
| Fila (FIFO) | Lembretes | O primeiro marcado é o primeiro lembrado |
| Árvore binária de busca | Índice por nome | In-ordem dá ordem alfabética; busca O(log n) em média |
| Vetor de ponteiros (temporário) | Ordenação e busca binária | A busca binária precisa de índice; só ponteiros são trocados, sem copiar compromissos |

## Requisitos da avaliação

| Req. | Onde aparece (funções) |
|---|---|
| R1 Funções | Funções curtas (≤ 30 linhas) com comentário de cabeçalho. Por valor: `cadastrar(Agenda&, Compromisso c)`, `minuscula(string t)`; por referência: `Agenda&`, `lerData(Compromisso&)`; valor padrão: `lerTexto(rotulo, obrigatorio = true)` |
| R2 Ponteiros | `Compromisso**` (vetor de endereços), `long* comparacoes`, `char* fim` em `lerInteiro`, `int* restante` em `percorrerEmOrdem` |
| R3 Recursividade | `contarNos`, `inserirNaArvore`, `buscarNaArvore`, `percorrerEmOrdem`, `liberarArvore`, `mergeSort` |
| R4 Memória dinâmica | `new` em `inserirNoFim`, `empilhar`, `enfileirar`, `inserirNaArvore`; `new[]` em `mesclar` e `montarVetorOrdenado`; cada um com `delete`/`delete[]` |
| R5 Lista encadeada | `inserirNoFim`, `buscarPorId`, `removerPorId`, `liberarLista` |
| R6 Pilha | `empilhar`, `desempilhar` (menus 5 e 6) |
| R7 Fila | `enfileirar`, `desenfileirar` (menu 7) |
| R8 Árvore binária de busca | `inserirNaArvore`, `buscarNaArvore`, `percorrerEmOrdem` (menu 8) |
| R9 Ordenação | `mergeSort` + `mesclar`, complexidade comentada no código |
| R10 Busca | `buscaSequencial`, `buscaBinaria` e comparação por contagem (menu 9) |

## Complexidade

| Algoritmo | Melhor | Médio | Pior | Memória extra |
|---|---|---|---|---|
| Merge Sort (estável) | O(n log n) | O(n log n) | O(n log n) | O(n) |
| Busca sequencial | O(1) | O(n) | O(n) | O(1) |
| Busca binária (vetor ordenado) | O(1) | O(log n) | O(log n) | O(1) |
| Árvore binária de busca (sem balanceamento) | O(1) | O(log n) | O(n) | O(1) |

Medido com 3003 compromissos: Merge Sort ≈ 31 mil comparações (feito uma vez); busca sequencial 2968 comparações × busca binária 12.

## Gerenciamento de memória

- Todo nó é criado com `new` e liberado com `delete`; vetores temporários com `new[]`/`delete[]`; ponteiros liberados recebem `nullptr`.
- A árvore guarda **ponteiros** para os compromissos da lista, por isso é refeita antes de ser usada (evita ponteiro solto).
- `liberarAgenda` roda ao sair e libera lista, pilha, fila e árvore.
- Verificado com AddressSanitizer/UBSan (`make teste`): sem vazamentos.

## Divisão de responsabilidades

Em `src/main.cpp`, cada parte está em uma seção marcada por comentário.

| Integrante | Responsável por |
|---|---|
| **Marcio Eduardo** | Entrada de dados, **lista encadeada** e **fila** · R1, R4, R5, R7 · menus 1, 2, 4, 5 e 7 · relatório técnico |
| **Milena Mayara** | **Pilha**, **árvore binária**, **Merge Sort** e **buscas** · R3, R6, R8, R9, R10 · menus 3, 6, 8, 9 e 10 · roteiro de teste |

Cada integrante apresenta e explica o código que implementou.
